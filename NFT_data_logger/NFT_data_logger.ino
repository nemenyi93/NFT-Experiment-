/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 52
 ** CS - pin 53 (for MKRZero SD: SDCARD_SS_PIN)

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>
#include <SDISerial.h>

//in order to recieve data you must choose a pin that supports interupts
#define DATALINE_PIN 2
#define INVERTED 1

SDISerial sdi_serial_connection(DATALINE_PIN, INVERTED);
const int chipSelect = 53;
const String fileName = "datalog.txt";
const String delimeter = ",";

void setupES2Sensor() {
  sdi_serial_connection.begin(); // start our SDI connection 
  Serial.begin(9600); // start our uart
  Serial.println("OK INITIALIZED"); // startup string echo'd to our uart
  delay(3000); // startup delay to allow sensor to powerup and output its DDI serial string
}

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
    // don't do anything more:
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

char* get_measurement(){
  char* service_request = sdi_serial_connection.sdi_query("?M!",1000);
  //you can use the time returned above to wait for the service_request_complete
  char* service_request_complete = sdi_serial_connection.wait_for_response(1000);
  //dont worry about waiting too long it will return once it gets a response
  return sdi_serial_connection.sdi_query("?D0!",1000);
}

char* collectES2Data() {
  uint8_t wait_for_response_ms = 1000;
  char* response = get_measurement(); // get measurement data
  //if you you didnt need a response you could simply do
  //         sdi_serial_connection.sdi_cmd("0A1") 
  // Serial.println(response!=NULL&&response[0] != '\0'?response:"No Response!"); //just a debug print statement to the serial port
  return response;
}

void writeDataToSD(char* data) {
// open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(fileName, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
    // print to the serial port too:
    Serial.println(data);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("Error opening " + fileName);
  }
  delay(3000);
}

void setup() { 
  setupES2Sensor();
  setupSDCard();
  writeHeaderToSD();
}

void loop() {
  String formattedData = "";
  char* ES2Data;
  
  ES2Data = collectES2Data();

  //TODO: Format data and verify that it writes to SD Card
  writeDataToSD(ES2Data);
  delay(500);
}







