/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_L3GD20_U.h>

/* Assign a unique ID to the sensors */
Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

/**************************************************************************/
/*!
    @brief  Initialises all the sensors used by this example
*/
/**************************************************************************/
void initSensors()
{
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
    while(1);
  }
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
}


// Set these to your desired credentials.
const char* ssid = "RobsBoard";
const char* password = "SooBoard";

WiFiServer server(80);


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
    /* Initialise the sensors */
  initSensors();
  Serial.println(F("Adafruit 9 DOF Pitch/Roll/Heading Example")); Serial.println("");
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
//            client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
//            client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");
//            client.print("<br>");
            
              sensors_event_t accel_event;
              sensors_event_t mag_event;
              sensors_vec_t   orientation;
            
              /* Read the accelerometer and magnetometer */
              accel.getEvent(&accel_event);
              mag.getEvent(&mag_event);
            
              /* Use the new fusionGetOrientation function to merge accel/mag data */  
              if (dof.fusionGetOrientation(&accel_event, &mag_event, &orientation))
              {
                /* 'orientation' should have valid .roll and .pitch fields */
                // Roll, Pitch, Heading, AccX, AccY, AccZ 
                client.print(orientation.roll);
                client.print(",");
                client.print(orientation.pitch);
                client.print(",");
                client.print(orientation.heading);
                client.print(",");
                client.print(accel_event.acceleration.x);
                client.print(",");
                client.print(accel_event.acceleration.y);
                client.print(",");
                client.print(accel_event.acceleration.z);
//                client.println("<br>");

//                /* 'orientation' should have valid .roll and .pitch fields */
//                Serial.print(F("Orientation: "));
//                Serial.print(orientation.roll);
//                Serial.print(F(" "));
//                Serial.print(orientation.pitch);
//                Serial.print(F(" "));
//                Serial.print(orientation.heading);
//                Serial.println(F(""));
//                    /* 'orientation' should have valid .roll and .pitch fields */
//                Serial.print(F("accel_event: "));
//                Serial.print(accel_event.acceleration.x);
//                Serial.print(F(" "));
//                Serial.print(accel_event.acceleration.y);
//                Serial.print(F(" "));
//                Serial.print(accel_event.acceleration.z);
//                Serial.println(F(""));
              }
              
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
