/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */
#include <WiFi.h>
#include <WiFiUdp.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_L3GD20_U.h>
//The udp library class
WiFiUDP udp;
/* Assign a unique ID to the sensors */
Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

// WiFi network name and password:
const char * networkName = "Linksys02108";
const char * networkPswd = "ssfjswsbmi";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "192.168.1.115";
const int udpPort = 80;

//Are we currently connected?
boolean connected = false;


/**************************************************************************/
/*!
    @brief  Initialises all the sensors used by this example
*/
/**************************************************************************/

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

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

void setup(){
  // Initilize hardware serial:
    Serial.begin(115200);
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    } 
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

//    listDir(SD, "/", 0);
//    createDir(SD, "/mydir");
//    listDir(SD, "/", 0);
//    removeDir(SD, "/mydir");
//    listDir(SD, "/", 2);
    writeFile(SD, "/accel_log.txt", "x,y,x\n");

//    readFile(SD, "/hello.txt");
//    deleteFile(SD, "/foo.txt");
//    renameFile(SD, "/hello.txt", "/foo.txt");
//    readFile(SD, "/foo.txt");
//    testFileIO(SD, "/test.txt");
//    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
//    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

    
  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
      /* Initialise the sensors */
  initSensors();
  Serial.println(F("Adafruit 9 DOF Pitch/Roll/Heading Example")); Serial.println("");
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
    }
}

void loop(){
  //only send data when connected
  if(connected){
    //Send a packet
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
                udp.beginPacket(udpAddress,udpPort);
                String oroll = String(orientation.roll);
                udp.print(oroll);
                udp.printf(",");
                String opitch = String(orientation.pitch);
                udp.print(opitch);
                udp.printf(",");
                String oheading = String(orientation.heading);
                udp.print(oheading);
                udp.printf(",");
                String accelx = String(accel_event.acceleration.x);
                udp.print(accelx);
                udp.printf(",");
                String accely = String(accel_event.acceleration.y);
                udp.print(accely);
                udp.printf(",");
                String accelz = String(accel_event.acceleration.z);
                udp.print(accelz);
//                char buf;
                String bingo_ling = oroll+","+opitch+","+oheading+","+accelx+","+accely+","+accelz+"\n";
//                const char bing_cc = bingo_ling.toCharArray(buf, bingo_ling.length());
                appendFile(SD, "/accel_log.txt", bingo_ling.c_str());
    
    udp.printf(" ESP32 uptime: %u", millis(
      )/1000);
    udp.endPacket();
//    Serial.println(String(orientation.roll));
  }
  //Wait for 1 second
  delay(10);

}
}
