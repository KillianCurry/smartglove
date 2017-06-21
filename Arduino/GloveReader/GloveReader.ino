/*
 This project was developed and tested with the Arduino/Genuino 101
 
 Circuit:
 16FGV1.0 sensor attached to pins 6, 7, 10 - 13:
 INTERUPT : pin 6
 NSS      : pin 7
 MOSI     : pin 11
 MISO     : pin 12
 SCK      : pin 13

 About:
 This is a demostration for how to stream data from a StretchSense 16FGV1.0 circuit in continuous mode.
 The SPI data from the StretchSense board is read in through the Arduino/Genuino 101 circuit and then relayed
 back to a PC via the USB/Serial port.

 Note:
 Due to the relatively slow data rates available over the USB/serial comms it will mean that the highest data rates will be limited.  
 
 created 06 July 2016
 by Alan Deacon
 */

// the StretchSense circuit [16FGV1.0] communicates using SPI
#include <SPI.h>
#include <CurieIMU.h>
#include <MadgwickAHRS.h>

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
const int InterruptPin   =    6;
const int chipSelectPin  =    7;

// ---- DEFINITIONS ----//

// Data package options
#define DATA    0x00  // 16FGV1.0 data packet
#define CONFIG  0x01  // 16FGV1.0 configuration command

// ODR - Output Data Rate
#define RATE_OFF             0x00
#define RATE_25HZ            0x01
#define RATE_50HZ            0x02
#define RATE_100HZ           0x03
#define RATE_166HZ           0x04
#define RATE_200HZ           0x05
#define RATE_250HZ           0x06
#define RATE_500HZ           0x07
#define RATE_1kHZ            0x08

// INT - Interrupt Mode
#define INTERRUPT_DISABLED    0x00
#define INTERRUPT_ENABLED     0x01

// TRG - Trigger Mode
#define TRIGGER_DISABLED     0x00
#define TRIGGER_ENABLED      0x01

// FILTER - Filter Mode
#define FILTER_1PT           0x01
#define FILTER_2PT           0x02
#define FILTER_4PT           0x04
#define FILTER_8PT           0x08
#define FILTER_16PT          0x10
#define FILTER_32PT          0x20
#define FILTER_64PT          0x40
#define FILTER_128PT         0x80
#define FILTER_255PT         0xFF

// RES - Resolution Mode
#define RESOLUTION_1pF       0x00
#define RESOLUTION_100fF     0x01
#define RESOLUTION_10fF      0x02
#define RESOLUTION_1fF       0x03

// Config Transfer
#define PADDING              0x00

// Configuration Setup
// MODIFY THESE PARAMETERS TO CHANGE CIRCUIT FUNCTION
int   ODR_MODE        =      RATE_500HZ;//process rate for processing capacitances
int   INTERRUPT_MODE   =     INTERRUPT_DISABLED;
int   TRIGGER_MODE    =      TRIGGER_DISABLED;
int   FILTER_MODE     =      FILTER_16PT;
int   RESOLUTION_MODE =      RESOLUTION_100fF;

//SPI Configuration (2000000 = transfer through com port)
SPISettings SPI_settings(2000000, MSBFIRST, SPI_MODE1); 

//Default scaling factor
int   CapacitanceScalingFactor = 100; //Default value
int   RawData[20];

//madgwick filter
Madgwick filter;
unsigned long microsPerReading, microsPrevious;

///////////////////////////////////////////////////////////////
//  void setup()
//
//  @breif: 
//  @params: 
///////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);

  //Initialise SPI port
  SPI.begin();
  SPI.beginTransaction(SPI_settings);

  //Initialise IMU and Madgwick filter
  CurieIMU.begin();
  CurieIMU.setGyroRate(25);
  CurieIMU.setAccelerometerRate(25);
  filter.begin(25);

  //Set accelerometer range to 2G
  CurieIMU.setAccelerometerRange(2);
  //Set gyroscope range to 250 degrees/second
  CurieIMU.setGyroRange(250);

  //Pace updates
  microsPerReading = 1000000 / 25;
  microsPrevious = micros();

  // Initalize the  data ready and chip select pins:
  pinMode(InterruptPin, INPUT);
  pinMode(chipSelectPin, OUTPUT);

  //configure 16FGV1.0:
  writeConfiguration();
  //get capacitance scaling factor
  CapacitanceScalingFactor = getCapacitanceScalingFactor(RESOLUTION_MODE);

  //give the SPI time to set up
  delay(0.1);
}

///////////////////////////////////////////////////////////////
//  void loop()
//
//  @breif: 
//  @params: 
///////////////////////////////////////////////////////////////
void loop(){
  int aix, aiy, aiz;
  int gix, giy, giz;
  float ax, ay, az;
  float gx, gy, gz;
  float roll, pitch, heading;
  unsigned long microsNow;

  //check whether it's time to update
  microsNow = micros();
  if (microsNow - microsPrevious >= microsPerReading)
  {
    //read raw data from IMU
    CurieIMU.readMotionSensor(aix, aiy, aiz, gix, giy, giz);

    //convert data using Madgwick algorithms
    ax = convertRawAcceleration(aix);
    ay = convertRawAcceleration(aiy);
    az = convertRawAcceleration(aiz);
    gx = convertRawGyro(gix);
    gy = convertRawGyro(giy);
    gz = convertRawGyro(giz);

    //update Madgwick filter
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    //print heading, pitch, and roll
    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();
    Serial.print(heading);
    Serial.print(",");
    Serial.print(pitch);
    Serial.print(",");
    Serial.print(roll);
    Serial.print(",");
    loop_in_float_mode();
    //delay(100);
    
    // increment previous time, so we keep proper pace
    microsPrevious = microsPrevious + microsPerReading;
    //loop_in_rmse_mode();
  }
  
}

float convertRawAcceleration(int aRaw) {
  // since we are using 2G range
  // -2g maps to a raw value of -32768
  // +2g maps to a raw value of 32767
 
  float a = (aRaw * 2.0) / 32768.0;
  return a;
}

float convertRawGyro(int gRaw) {
  // since we are using 250 degrees/seconds range
  // -250 maps to a raw value of -32768
  // +250 maps to a raw value of 32767
 
  float g = (gRaw * 250.0) / 32768.0;
  return g;
}


///////////////////////////////////////////////////////////////
//  void loop_in_float_mode()
//
//  @breif: 
//  @params: 
///////////////////////////////////////////////////////////////
void loop_in_float_mode() {

  float capacitance = 0;

  //Check if interrupt mode is enabled (in configuration)
  if(INTERRUPT_MODE == INTERRUPT_ENABLED){
    // don't do anything until the interupt pin goes low:
    while (digitalRead(InterruptPin) == HIGH);
  }
  
  //Read the sensor Data
  readCapacitance(RawData);

  // convert the raw data to capacitance:
  for (int i=0; i<10; i++){
    
    capacitance = extractCapacitance(RawData,i);
    Serial.print(capacitance);  //Output capacitance values
    Serial.print(',');          //Output data as comma seperated values
    
  }
 Serial.print("\n");

  //Wait for next data packet to start sampling
   if(INTERRUPT_MODE == INTERRUPT_ENABLED){
      while (digitalRead(InterruptPin) == LOW);  
   }
}

*/
///////////////////////////////////////////////////////////////
//  void writeConfiguration()
//
//  @breif: 
//  @params: 
///////////////////////////////////////////////////////////////
void writeConfiguration() {

  // 16FGV1.0 requires a configuration package to start streaming data

  // Set the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(CONFIG);                 //  Select Config Package
  SPI.transfer(ODR_MODE);               //  Set output data rate
  SPI.transfer(INTERRUPT_MODE);         //  Set interrupt mode
  SPI.transfer(TRIGGER_MODE);           //  Set trigger mode
  SPI.transfer(FILTER_MODE);            //  Set filter
  SPI.transfer(RESOLUTION_MODE);        //  Set Resolution
  for (int i=0;i<16;i++){
    SPI.transfer(PADDING);              //  Pad out the remaining configuration package
  }

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
}

///////////////////////////////////////////////////////////////
//  void readCapacitance(int raw[])
//
//  @breif: 
//  @params: int raw[] - Raw sensing data from 16FGV1.0
///////////////////////////////////////////////////////////////
void readCapacitance(int raw[]) {

  // 16FGV1.0 transmits data in the form of 10, 16bit capacitance values

  // Set the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(DATA);                   //  Select Data Package
  SPI.transfer(PADDING);                //  Get Sequence Number
  for (int i=0;i<20;i++){
    raw[i] =  SPI.transfer(PADDING);    //  Pad out the remaining configuration package
  }

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);

}


///////////////////////////////////////////////////////////////
//  void setCapacitanceScalingFactor(int Resolution)
//
//  @breif: The 16FGV1.0 has an adjustable LSB resolution this function scales raw data to capacitance based on 
//  configuration settings.
//
//  @params: int Resolution_Config
///////////////////////////////////////////////////////////////
int getCapacitanceScalingFactor (int Resolution_Config)
{
  //TODO change to just do basic math on the resolution variable rather than a case statement?
  switch(Resolution_Config){
    case (RESOLUTION_1pF):
      return 1;  
    break;

    case (RESOLUTION_100fF):
      return 10;  
    break;

    case (RESOLUTION_10fF):
      return 100;  
    break;

    case (RESOLUTION_1fF):
      return 1000;  
    break;

  }
  return 1;

}

///////////////////////////////////////////////////////////////
//  float extractCapacitance(int[] raw, int channel)
//
//  @breif: 
//  @params: 
///////////////////////////////////////////////////////////////
float extractCapacitance(int raw[], int channel){

  float Capacitance =0;

  Capacitance = (raw[2*channel])*256+raw[2*channel+1];
  Capacitance /= CapacitanceScalingFactor;

  return Capacitance;

}


