#include "lib/SDISerial.h"
#include "Sensor.h"

//in order to recieve data you must choose a pin that supports interupts
#define DATALINE_PIN 2
#define INVERTED 1

class Es2Sensor : public Sensor {

private:
   SDISerial sdi_serial_connection(DATALINE_PIN, INVERTED);

   char* get_measurement() {
     char* service_request = sdi_serial_connection.sdi_query("?M!",1000);
     //you can use the time returned above to wait for the service_request_complete
     char* service_request_complete = sdi_serial_connection.wait_for_response(1000);
     //dont worry about waiting too long it will return once it gets a response
     return sdi_serial_connection.sdi_query("?D0!",1000);
   }

public:
  void setup() {
    sdi_serial_connection.begin(); // start our SDI connection
    Serial.begin(9600); // start our uart
    Serial.println("OK INITIALIZED"); // startup string echo'd to our uart
    delay(3000); // startup delay to allow sensor to powerup and output its DDI serial string
  }

  char* collectData() {
    uint8_t wait_for_response_ms = 1000;
    char* response = get_measurement();
    return response;
  }
};
