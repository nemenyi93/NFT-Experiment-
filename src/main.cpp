/*
circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MISO - pin 50
 ** MOSI - pin 51
 ** CLK  - pin 52
 ** CS   - pin 53 (for MKRZero SD: SDCARD_SS_PIN)
 */

#include <stdint.h>
#include <SPI.h>
#include <SD.h>
#include <SDISerial.h>
#include <SparkFunBME280.h>
#include <Wire.h>

//in order to recieve data you must choose a pin that supports interupts
#define DATALINE_PIN 2
#define INVERTED 1

/*===========================================================*/
/*======================= DEFINITIONS =======================*/
/*===========================================================*/
SDISerial sdi_serial_connection(DATALINE_PIN, INVERTED);
const int chipSelect = 53;
const String fileName = "datalog.txt";
const String delimeter = ",";
const String es2Header = "ec,Wtemp";
const String phHeader = "voltage,pH";
const String BME280Header = "Atemp,rH";

// pH
#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate
#define samplingInterval 500
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0;

// BME280
BME280 mySensor;

/*===========================================================*/
/*=========================== I/O ===========================*/
/*===========================================================*/
void setupSDCard() {
  // Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void writeHeaderToSD() {
  String header = es2Header + delimeter + phHeader + BME280Header;
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.println(header);
  }
  dataFile.close();
}

void writeDataToSD(char* data) {
// open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open(fileName, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("Error opening " + fileName);
  }
  delay(3000);
}

/*===========================================================*/
/*=========================== ES2 ===========================*/
/*===========================================================*/
void setupES2Sensor() {
  sdi_serial_connection.begin(); // start our SDI connection
  delay(3000); // startup delay to allow sensor to powerup and output its DDI serial string
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

char* formatES2Data(char* rawData) {
    // depth, ec, temp
    char* formattedData = rawData + 2;
    String formattedStr = String(formattedData);
    formattedStr.replace("+", ",");
    formattedStr.toCharArray(formattedData, formattedStr.length());
    return formattedData;
}

/*===========================================================*/
/*=========================== pH ============================*/
/*===========================================================*/
double avergeArray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}

String collectPhData() {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;

  char* response;

  if (millis() - samplingTime > samplingInterval) {
      pHArray[pHArrayIndex++]=analogRead(SensorPin);
      if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
      voltage = avergeArray(pHArray, ArrayLenth)*5.0/1024;
      pHValue = 3.5*voltage+Offset;
      samplingTime=millis();
  }

  String formattedStr =  String(voltage) + delimeter + String(pHValue);
  //Serial.println(formattedStr);
  //formattedStr.toCharArray(response, formattedStr.length());

  return formattedStr; //response;
}

/*===========================================================*/
/*========================= BME280 ==========================*/
/*===========================================================*/
void setupBME280Sensor() {
  //***Driver settings********************************//
  mySensor.settings.commInterface = I2C_MODE;
	mySensor.settings.I2CAddress = 0x77;

  //***Operation settings*****************************//
  mySensor.settings.runMode = 3; //  3, Normal mode
  mySensor.settings.tStandby = 0; //  0, 0.5ms
  mySensor.settings.filter = 0; //  0, filter off
  //tempOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.tempOverSample = 1;
  //pressOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.pressOverSample = 1;
  //humidOverSample can be:
  //  0, skipped
  //  1 through 5, oversampling *1, *2, *4, *8, *16 respectively
  mySensor.settings.humidOverSample = 1;

  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

  mySensor.begin(); //Calling .begin() causes the settings to be loaded
}

String collectBME280Data() {
  return mySensor.readTempC() + delimeter +
         mySensor.readFloatAltitudeMeters();
}



/*===========================================================*/
/*=========================== MAIN ==========================*/
/*===========================================================*/
void setup() {
  Serial.begin(9600); // start our uart
  setupES2Sensor();
  setupBME280Sensor();
  setupSDCard();
  writeHeaderToSD();
}

void loop() {
  char* ES2Data;
  //char* phData;
  String phData;
  String dataStr;
  String BME280Data;
  char formattedData[50];

  ES2Data = formatES2Data(collectES2Data());
  phData = collectPhData();
  BME280Data = collectBME280Data();

  dataStr = ES2Data + delimeter +
            phData  + delimeter +
            BME280Data;

  dataStr.toCharArray(formattedData, 50);
  Serial.println(formattedData);

  writeDataToSD(formattedData);
  delay(samplingInterval);
}
