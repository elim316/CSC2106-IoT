#include <RH_RF95.h>
#include <SPI.h>
#include <Wire.h>
#include "time.h"

// Change to 434.0 or other frequency, must match RX's freq!
// The greater NodeID is the parent
#define RF95_FREQ 915.0
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// LoRa radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define displayUpdateDelay 3000
#define parentBroadcastDelay 3000
#define parentTimeoutDelay 15000

#define MAX_CHILD_NODES 10
#define CHILD_NODE_TIMEOUT 10000 // In milliseconds (10 seconds)
#define buildingName "BUILDING-B-NodeID5"

// ELECTION initiates the process when a node detects a failure in the parent node.
// ELECTION_CHALLENGE is used by nodes to participate in the election by challenging the initiator.
// ELECTION_ACCEPTED confirms the election of a new parent node, allowing the network to continue operating with the new parent.
// Wait i'm stupid regardless whether it's broadcast, it still sends to all people
enum MessageType
{
    BROADCAST,
    ELECTION,
    BROADCAST_RESPONSE,
    TOGGLELIGHTS,
    LIGHT_MODE,
    DARK_MODE,
};

struct Message
{
    MessageType type;
    char name[16];
    uint8_t senderID;
    uint8_t receiverID; // If it's 0 means sent to all user
    char payload[32];   // Adjust size as needed
};

struct ChildNodeInfo
{
    uint8_t id;
    char name[16];
    unsigned long lastResponseTime; // Timestamp of the last response
};


// Function prototype
void setupLoRa();
void broadcastMessage(const char *command);
void sendMessage(Message msg);
void handleReceivedMessage();
void checkParentHealth();
void addOrUpdateChildNode(ChildNodeInfo childInfo);
void removeChildNode(uint8_t childID);
void setup();
void loop();

// Node ID and parent ID
uint8_t nodeID = 9999;   // My node ID
uint8_t parentID = 0; // Initial parent ID, 0 means no parent
int childNodeCount = 0;
unsigned long lastCheckTime = 0;
ChildNodeInfo childNodes[MAX_CHILD_NODES];

// Timestamp for last parent broadcast
unsigned long lastParentBroadcast = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long electionInitiatedTime = 0;

// Function to initialize the LoRa radio
void setupLoRa()
{
    // Manually reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!rf95.init())
    {
        Serial.println("LoRa radio init failed");
        while (1)
            ;
    }
    // Set frequency
    if (!rf95.setFrequency(RF95_FREQ))
    {
        Serial.println("Set frequency failed");
        while (1)
            ;
    }
    // Set Tx power
    // Transmit Power Level (23 dBm): This is the power level at which the RFM95/RFM96 module will transmit data.
    // The value is specified in dBm (decibels relative to 1 milliwatt). A higher dBm value generally means a higher transmit power, which can increase the range of the transmission but may also increase the power consumption and potentially the risk of interference with other devices.
    // rf95.setTxPower(23, false);
    Serial.println("LoRa setup has been successful");
}

// Function to broadcast a message
// This is usually used by the "MAIN" node to broadcast that it is still alive.
void broadcastMessage(const char *command)
{
    Message msg;
    msg.type = BROADCAST;
    msg.senderID = nodeID; // Assuming nodeID is the ID of the main node
    msg.receiverID = 0;    // Broadcast to all nodes
    strncpy(msg.name, buildingName, sizeof(msg.name));
    strncpy(msg.payload, command, sizeof(msg.payload));
    sendMessage(msg);
}

void sendMessage(Message msg)
{
    // Convert the struct to a byte array for transmission
    uint8_t buf[sizeof(Message)];
    memcpy(buf, &msg, sizeof(Message));
    rf95.send(buf, sizeof(buf));
    // rf95.waitPacketSent();
}

void handleReceivedMessage()
{
    uint8_t buf[sizeof(Message)];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
        // Converts received buffer back into a struct
        Message receivedMsg;
        memcpy(&receivedMsg, buf, sizeof(Message));

        // Ensure that we don't receive our own message
        if (receivedMsg.senderID != nodeID)
        {
            switch (receivedMsg.type)
            {
                case BROADCAST:
                    if (nodeID == parentID) {
                        // I am the parent
                        if (receivedMsg.senderID < nodeID) {
                            Message responseMessage;
                            responseMessage.senderID = nodeID;
                            responseMessage.receiverID = receivedMsg.senderID;
                            strncpy(responseMessage.name, buildingName, sizeof(responseMessage.name));
                            responseMessage.type = ELECTION;
                            sendMessage(responseMessage);
                        }
                    } else {
                        if (receivedMsg.senderID > nodeID) {
                            parentID = receivedMsg.senderID;
                            lastParentBroadcast = millis();

                            Message responseMessage;
                            responseMessage.senderID = nodeID;
                            responseMessage.receiverID = receivedMsg.senderID;
                            strncpy(responseMessage.name, buildingName, sizeof(responseMessage.name));
                            responseMessage.type = BROADCAST_RESPONSE;
                            sendMessage(responseMessage);
                        }
                    }
                    // No matter if i become the parent or they are still the parent, we are still broadcasting
                    lastParentBroadcast = millis();
                    break;
                case ELECTION:
                    // Someone has called for an election and i am the parent
                    if (parentID == nodeID) {
                        if (receivedMsg.senderID > nodeID) {
                            parentID = nodeID;
                            lastParentBroadcast = millis();
                        }
                    }
                    break;
                case BROADCAST_RESPONSE:
                    // Upon getting the broadcast response if we are the parentID
                    // If we get a broadcast response we are definitely the child
                    if (parentID == nodeID) {
                        ChildNodeInfo childInfo;
                        childInfo.lastResponseTime = millis();
                        childInfo.id = receivedMsg.senderID;
                        strncpy(childInfo.name, buildingName, sizeof(childInfo.name));
                        addOrUpdateChildNode(childInfo);
                    }
                    break;
                case TOGGLELIGHTS:
                    // Run through UART to toggle the lights
                    String data = "TOGGLELIGHTS\n";
                    Serial.write(data);
                    break;
                default:
                    break;
            }
        }
    }
}

// Function to check if the parent is dead and initiate an election if necessary
void checkParentHealth()
{
    unsigned long currentMillis = millis();
    if (((currentMillis - lastParentBroadcast) > parentTimeoutDelay) && (electionInitiatedTime == 0))
    {
        // Timeout, indicate that parent is dead.
        // Call for election now.
        Message msg;
        msg.type = ELECTION;
        msg.senderID = nodeID;
        msg.receiverID = 0;
        strncpy(msg.name, buildingName, sizeof(msg.name));
        strncpy(msg.payload, "ELECTION_TIME", sizeof(msg.payload)); // This doesn't matter here.
        sendMessage(msg);
        Serial.println("Either parent has died or there's no one left in the call.");
        electionInitiatedTime = currentMillis;
        parentID = 0;
    }

    // Check if the election has been initiated and if enough time has passed
    if ((electionInitiatedTime > 0) && (currentMillis - electionInitiatedTime >= 1000))
    {
        Serial.println("AM I HERE");
        if (parentID == 0)
        { // Check if no other node has challenged the election
            parentID = nodeID;
            Serial.println("Node has become its own parent.");
            // Reset the electionInitiatedTime to prevent this block from executing again
            electionInitiatedTime = 0;
        }
    }
}

// Either add the child to the array or update the
void addOrUpdateChildNode(ChildNodeInfo childInfo)
{
    // Check if the child node already exists in the array
    bool exists = false;
    for (int i = 0; i < childNodeCount; i++)
    {
        if (childNodes[i].id == childInfo.id)
        {
            childNodes[i].lastResponseTime = millis();
            char output[50]; 
            sprintf(output, "Updated ChildID %u lastResponseTime", childInfo.id);
            Serial.println(output);
            exists = true;
            break;
        }
    }

    // If the child node does not exist, add it to the array
    if (!exists)
    {
        if (childNodeCount < MAX_CHILD_NODES)
        {
            childInfo.lastResponseTime = millis(); // Set the last response time to now
            childNodes[childNodeCount++] = childInfo;
            Serial.print("Child node ");
            Serial.print(childInfo.name);
            Serial.println(" added.");
        }
        else
        {
            Serial.println("Maximum number of child nodes reached.");
        }
    }
    else
    {
        Serial.print("Child node ");
        Serial.print(childInfo.name);
        Serial.println(" already exists.");
    }
}

void removeChildNode(uint8_t childID)
{
    int indexToRemove = -1;
    for (int i = 0; i < childNodeCount; i++)
    {
        if (childNodes[i].id == childID)
        {
            indexToRemove = i;
            break;
        }
    }

    if (indexToRemove != -1)
    {
        // Shift the remaining elements to fill the gap
        for (int i = indexToRemove; i < childNodeCount - 1; i++)
        {
            childNodes[i] = childNodes[i + 1];
        }
        childNodeCount--; // Decrement the count of child nodes
    }
}

void setup()
{
    Serial.begin(115200);
    setupLoRa();
    Serial.println("Node initialized");
}

// The LoRaServer will not be in charge of challenging any of the nodes nor will it be a server.
void loop()
{
    // handleReceivedMessage();

    // unsigned long currentMillis = millis();
    // // If this node is the parent, it sends a broadcast with message HELLO to all the nodes indicating that it's alive
    // if (parentID == nodeID)
    // {
    //     // Broadcast a "HELLO" message to all node every 5 seconds indicate that it is still available
    //     if ((currentMillis - lastParentBroadcast) > parentBroadcastDelay)
    //     {
    //         broadcastMessage("PARENT_ALIVE");
    //         lastParentBroadcast = currentMillis;
    //         // Await for callback and all responses will then be saved into the surviving nodes list.
    //         // (This will ensure that the main node is able to send commands to individual nodes correctly)
    //         Serial.println("Broadcasting to children that i'm alive");
    //     }

    //     // Check if there's no updates from child nodes (Check every 1 second second)
    //     if ((currentMillis - lastCheckTime >= 1000))
    //     {
    //         lastCheckTime = currentMillis;
    //         for (int i = 0; i < childNodeCount; i++)
    //         {
    //             if (currentMillis - childNodes[i].lastResponseTime > CHILD_NODE_TIMEOUT)
    //             { // Nodes will expire in 1000 seconds
    //                 Serial.print("Child node ");
    //                 Serial.print(childNodes[i].name);
    //                 Serial.println(" has timed out.");
    //                 removeChildNode(childNodes[i].id);
    //                 // Adjust the loop index since the array has shifted
    //                 i--;
    //             }
    //         }
    //     }
    // }
    // else
    // {
    //     // Only the child should keep track of the parent health
    //     checkParentHealth();
    // }

    // We got an information from UART 
    // This is a single point of failure so we do not need to handle for parent call because we are just send communicate one way through
    if (Serial.available() > 0) {
        Serial.println("Received message from M5Stick-Server");
        String incomingString = Serial.readStringUntil('\n');
        Serial.println(incomingString);
        
        if (incomingString == "TOGGLIGHTS") {
            // Send it to the corresponding node (We are just hard targetting it to Node 3)
            Message responseMsg;
            responseMsg.senderID = nodeID;
            responseMsg.receiverID = 3;
            responseMsg.type = TOGGLELIGHTS;
            strncpy(responseMsg.name, buildingName, sizeof(responseMsg.name));
            sendMessage(responseMsg);
        }
    }

    // Update the display every 3 seconds
    // if (currentMillis - lastDisplayUpdate >= displayUpdateDelay)
    // {
    //     // Display node information on the OLED
    //     char output[50]; 
    //     sprintf(output, "Current ParentID is: %u", parentID);
    //     Serial.println(output);
    //     // Reset the last display update time
    //     lastDisplayUpdate = currentMillis;
    // }
}
