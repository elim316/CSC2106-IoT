import { Component, OnInit } from '@angular/core';
import { ArduinoService } from '../services/arduino.service';


@Component({
  selector: 'app-tab1',
  templateUrl: 'tab1.page.html',
  styleUrls: ['tab1.page.scss']
})
export class Tab1Page implements OnInit {
  selectedBuilding: string = 'all'; // Default selected building
  buildingData: any[] = []; // Array to store building data retrieved from server

  constructor(private arduinoService: ArduinoService) { }
  ngOnInit() {
    this.fetchDataFromArduino();
  }

  fetchDataFromArduino() {
    this.arduinoService.getData().subscribe(data => {
      // Update buildingData array with the received data
      this.buildingData = data.buildings;
    });
  }

  toggleLight(building: any) {
    // Send update to server or perform any other action based on toggle change
    console.log("Toggling light for building:", building.name);
    console.log("New light value:", building.sensor.light);
  }
}
