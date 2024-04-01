import { Component, OnInit } from '@angular/core';
import { ArduinoService } from '../services/arduino.service';


@Component({
  selector: 'app-tab1',
  templateUrl: 'tab1.page.html',
  styleUrls: ['tab1.page.scss']
})
export class Tab1Page implements OnInit {
  selectedBuildings: string = 'all'; // Default selected building
  buildingData: any[] = []; // Array to store building data retrieved from server
  displayedBuildingData: any[] = []; // Data to display, after filter

  constructor(private arduinoService: ArduinoService) { }
  ngOnInit() {
    this.fetchDataFromArduino();
  }

  fetchDataFromArduino() {
    this.arduinoService.getData().subscribe(data => {
      // Update buildingData array with the received data
      this.buildingData = data.buildings;
      this.displayedBuildingData = [...this.buildingData]; // Initially display all
    });
  }

  filterBuildings() {
    if (this.selectedBuildings.length) {
      this.displayedBuildingData = this.buildingData.filter(building => 
        this.selectedBuildings.includes(building.name)
      );
    } else {
      this.displayedBuildingData = [...this.buildingData]; // No selection, show all
    }
  }

  toggleLight(building: any) {
    // Send update to server or perform any other action based on toggle change
    console.log("Toggling light for building:", building.name);
    console.log("New light value:", building.sensor.light);
  }
}
