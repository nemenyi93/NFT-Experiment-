/*
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 52
 ** CS - pin 53 (for MKRZero SD: SDCARD_SS_PIN)

 */
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "sensor.h"

const int chipSelect = 53;
const String fileName = "datalog.txt";
const String delimeter = ",";
Sensor* es2Sensor = new Es2Sensor();

void setupSDCard() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");
}

void writeHeaderToSD() {
  String header = "Format: EC, VMC, Temp";
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.println(header);
  }
  dataFile.close();
}

void writeDataToSD(char* data) {
  File dataFile = SD.open(fileName, FILE_WRITE);

  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
    Serial.println(data);  // print to serial port too (for testing)
  } else {
    Serial.println("Error opening " + fileName);
  }
  delay(3000);
}

void setup() {
  es2Sensor->setup();
  setupSDCard();
  writeHeaderToSD();
}

void loop() {
  String formattedData = "";
  char* ES2Data;

  ES2Data = es2Sensor->collectData();

  //TODO: Format data and verify that it writes to SD Card
  writeDataToSD(ES2Data);
  delay(500);
}
