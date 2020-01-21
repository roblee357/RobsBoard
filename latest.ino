/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>

/* Assign a unique ID to the sensors */
Adafruit_9DOF                dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

/* Update this with the correct SLP for accurate altitude measurements */
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
int previous_file_No;
String f_name;

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

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
      previous_file_No = -1;
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            String fname = file.name();
            if(fname.indexOf("accel_log") > 0){
              Serial.print("  Log file: ");
              int xval = fname.indexOf( '_',8);
              int yval = fname.indexOf( '.',8);
              int fileNo = fname.substring(xval+1,yval).toInt();
              if (fileNo > previous_file_No) previous_file_No = fileNo;
              Serial.print("  xval " + String(xval)+"  yval " + String(yval) + "  " + fname.substring(xval+1,yval));
            }
            
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.print (file.size());
            Serial.print("  File: ");
            Serial.println(String(previous_file_No));
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

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
        Serial.println("Failed to open file for appending f_name:" + f_name);
        listDir(SD, "/", 0);
        f_name = "/accel_log_"+ String(previous_file_No+1) +".csv";
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void setup(){
    pinMode(2, OUTPUT);
    Serial.begin(115200);
    Serial.println(F("Adafruit 9 DOF Pitch/Roll/Heading Example")); Serial.println("");
  
  /* Initialise the sensors */
    initSensors();
    while(!SD.begin()){
        Serial.println("Card Mount Failed");
//            SD.begin();
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
    listDir(SD, "/", 0);
    f_name = "/accel_log_"+ String(previous_file_No+1) +".csv";
    writeFile(SD, f_name.c_str(), "roll,pitch,accel.x,accel.y,accel.z,heading/n");
    Serial.println( f_name + " " + String(previous_file_No % 2));
    if ( (previous_file_No % 2) == 0){
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
                 // wait for a second
}
    else {
      digitalWrite(2, LOW);
        
    //    const char ccf_name = f_name.c_str();
        
    //    appendFile(SD, "/hello.txt", "World!\n");
    //    readFile(SD, "/hello.txt");
    //    deleteFile(SD, "/foo.txt");
    //    renameFile(SD, "/hello.txt", "/foo.txt");
    //    readFile(SD, "/foo.txt");
    //    testFileIO(SD, "/test.txt");
    //    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    //    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    }
}

void loop(){
  Serial.println( String(previous_file_No % 2));
    if ( (previous_file_No % 2) == 0){
        sensors_event_t accel_event;
        sensors_event_t mag_event;
        sensors_vec_t   orientation;
        Serial.println( "writing file" + String(previous_file_No % 2));
              /* Calculate pitch and roll from the raw accelerometer data */
        accel.getEvent(&accel_event);
        if (dof.accelGetOrientation(&accel_event, &orientation))
        {
          /* 'orientation' should have valid .roll and .pitch fields */
//          Serial.print(F("Roll: "));
          Serial.print(orientation.roll);
          Serial.print(F(","));
//          Serial.print(F("Pitch: "));
          Serial.print(orientation.pitch);
          Serial.print(F(","));
//          Serial.print(F("Accel_X: "));
          Serial.print(accel_event.acceleration.x);
          Serial.print(F(","));
//          Serial.print(F("Accel_Y: "));
          Serial.print(accel_event.acceleration.y);
          Serial.print(F(","));
//          Serial.print(F("Accel_Z: "));
          Serial.print(accel_event.acceleration.z);
          Serial.print(F(","));
        }
          /* Calculate the heading using the magnetometer */
        mag.getEvent(&mag_event);
        if (dof.magGetOrientation(SENSOR_AXIS_Z, &mag_event, &orientation))
        {
          /* 'orientation' should have valid .heading data now */
//          Serial.print(F("Heading: "));
          Serial.print(orientation.heading);
          Serial.print(F("\n"));
          String writeS = String(orientation.roll) + "," + String(orientation.pitch) + "," + String(accel_event.acceleration.x) + "," + String(accel_event.acceleration.x) + "," + String(accel_event.acceleration.z) + "," + String(orientation.heading) + "\n";
          appendFile(SD, f_name.c_str(), writeS.c_str());
        }
//      appendFile(SD, f_name.c_str(), "Yo,Yo,Yo\n");
      delay(10);                       // wait for a second
//  Serial.println( String(previous_file_No % 2));
//  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);                       // wait for a second
//  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);                       // wait for a second
}
}
