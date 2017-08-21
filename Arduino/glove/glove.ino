#include <CurieIMU.h>
#include <MadgwickAHRS.h>
#include <CurieBLE.h>
#include <SPI.h>

// Values used for timing of the BLE notification step.
unsigned long notificationInterval, lastNotification;
// Values used for timing of the sensor update step.
unsigned long sensorInterval, lastSensor;

// BLE - Bluetooth Low Energy
// The BLE interface allows wireless data communication.
BLEPeripheral blePeripheral;
BLEService stretchSenseService("00601001-7374-7265-7563-6873656e7365"); //give your custom service an ID
BLECharacteristic stretchChar("00601002-7374-7265-7563-6873656e7365", BLERead | BLENotify, 21); //give your custom characteristic an ID, properties, and length
BLECharacteristic imuChar("00601003-7374-7265-7563-6873656e7365", BLERead | BLENotify, 13); //give your custom characteristic an ID, properties, and length
BLECharacteristic idChar("00601004-7374-7265-7563-6873656e7365", BLERead | BLEWrite, 4);

// IMU - Inertial Motion Unit
// The IMU tracks orientation using the Madgwick algorithm.
// Scaling parameters to define the resolution for the accelerometer and gyroscope.
float accelScale, gyroScale;
// Array used to store data from the IMU for BLE communication.
// Values are stored as two-byte unsigned shorts.
int RawDataIMU[12];
// The filtering algorithm, developed by Sebastian Madgwick and ported to Arduino by X-IO.
Madgwick filter;

// StretchSense - stretch sensors
// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):
const int InterruptPin   =    6;
// this pin is 7 on the old prototype and 10 on the sardines prototype
const int chipSelectPin  =    10;

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
#define RATE_2kHZ            0x09

// INT - Interrupt Mode
#define INTERRUPT_DISABLED    0x00
#define INTERRUPT_ENABLED     0x01

// TRG - Trigger Mode
#define TRIGGER_DISABLED     0x00
#define TRIGGER_ENABLED      0x01

// FILTER - Filter Mode
// This is the amount of points in the filter used for the SPI circuit.
// More points means a smoother, but slower and less accurate signal.
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
#define RESOLUTION_1pF       0x00 // 0 - 65535pf
#define RESOLUTION_100fF     0x01 // 0 - 6553.5pf
#define RESOLUTION_10fF      0x02 // 0 - 655.35pf
#define RESOLUTION_1fF       0x03 // 0 - 65.535pf

// Config Transfer
#define PADDING              0x00

// Configuration Setup
// MODIFY THESE PARAMETERS TO CHANGE CIRCUIT FUNCTION
int   ODR_MODE        =      RATE_500HZ;
int   INTERRUPT_MODE   =     INTERRUPT_DISABLED;
int   TRIGGER_MODE    =      TRIGGER_DISABLED;
int   FILTER_MODE     =      FILTER_32PT;
int   RESOLUTION_MODE =      RESOLUTION_100fF;

#define NOTIFICATION_FREQUENCY 12
#define SENSOR_FREQUENCY 80

//SPI Configuration
SPISettings SPI_settings(2000000, MSBFIRST, SPI_MODE1);

//Default scaling factor
int   CapacitanceScalingFactor = 100; //Default value
int   RawDataCapacitance[22];

float timestamp = 0;
float counter = 0;
float previous_value_counter = 0;

float roll, pitch, heading;

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// setup() /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);

  // Initialise the BLE ////////////////////////////////////////
  blePeripheral.setLocalName("StretchSense"); //broadcast device name
  blePeripheral.setDeviceName("StretchSense");

  blePeripheral.setAdvertisedServiceUuid(stretchSenseService.uuid());  // add the service UUID
  blePeripheral.addAttribute(stretchSenseService);   // add your custom service

  blePeripheral.addAttribute(imuChar); // add your custom characteristic
  blePeripheral.addAttribute(stretchChar); // add your custom characteristic
  blePeripheral.addAttribute(idChar);

  //set connection/disconnection events
  blePeripheral.setEventHandler(BLEConnected, BLECentralConnect);
  blePeripheral.setEventHandler(BLEDisconnected, BLECentralDisconnect);

  blePeripheral.begin();

  //Serial.println("Bluetooth device active, waiting for connections...");

  //Initialise the IMU ////////////////////////////////////////
  // start the IMU and filter
  CurieIMU.begin();
  CurieIMU.setGyroRate(25);
  CurieIMU.setAccelerometerRate(25);
  filter.begin(25);

  // Set the accelerometer range to 2G
  CurieIMU.setAccelerometerRange(2);
  // Set the gyroscope range to 250 degrees/second
  CurieIMU.setGyroRange(250);

  // initialize variables to pace updates to correct rate
  notificationInterval = 1000 / NOTIFICATION_FREQUENCY;//update BLE characteristics at 10Hz
  sensorInterval = 1000 / SENSOR_FREQUENCY;//update madgwick etc at 25Hz
  
  lastNotification = lastSensor = millis();

  // Initialise the SPI //////////////////////////////////////

  //Initialise SPI port
  SPI.begin();
  SPI.beginTransaction(SPI_settings);

  // Initalize the  data ready and chip select pins:
  pinMode(InterruptPin, INPUT);
  pinMode(chipSelectPin, OUTPUT);

  //Configure 16FGV1.0:
  writeConfiguration();
  //Get capacitance scaling factor
  CapacitanceScalingFactor = getCapacitanceScalingFactor(RESOLUTION_MODE);

  // give the circuit time to set up:
  delay(0.1);
}

/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Loop() /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void loop() {
  int aix, aiy, aiz;
    int gix, giy, giz;
    float ax, ay, az;
    float gx, gy, gz;
    int roll_int, pitch_int, heading_int;
    float accel_x, accel_y, accel_z;
    int accel_x_int, accel_y_int, accel_z_int;

    Serial.print("keep alive\n");

    // check if it's time to read data and update the filter
    if ((millis() - lastSensor) >= sensorInterval) {
    lastSensor = millis();
    /////////////////////////////////////////////////////////////
    // IMU Mode /////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    //Serial.println("reading IMU");
    // read raw data from CurieIMU
    CurieIMU.readMotionSensor(aix, aiy, aiz, gix, giy, giz);

    // convert from raw data to gravity and degrees/second units
    ax = convertRawAcceleration(aix);
    ay = convertRawAcceleration(aiy);
    az = convertRawAcceleration(aiz);
    gx = convertRawGyro(gix);
    gy = convertRawGyro(giy);
    gz = convertRawGyro(giz);

    // update the filter, which computes orientation
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    // get the heading, pitch and roll
    heading = filter.getYaw();
    pitch = filter.getPitch();
    roll = filter.getRoll();

    
    /////////////////////////////////////////////////////////////
    // StretchSense Mode ////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    //Serial.println("reading SPI");
    //Read the sensor Data
    readCapacitance(RawDataCapacitance);
    }

    // check if it's time to read data and update the BLE
    if (millis() - lastNotification >= notificationInterval) {
    lastNotification = millis();
    blePeripheral.poll();
    // get positive orientation values
    accel_x = ax + 2;
    accel_y = ay + 2;
    accel_z = az + 2;
    // shift the decimal place and cast into integer
    heading_int = heading;
    pitch_int = (pitch+90);
    roll_int = (roll+180);
    Serial.print(heading_int);
    Serial.print(" ");
    Serial.print(pitch_int);
    Serial.print(" ");
    Serial.print(roll_int);
    Serial.println();
    accel_x_int = accel_x;
    accel_y_int = accel_y;
    accel_z_int = accel_z;
    // encode IMU integers into an array of 2-byte shorts
    RawDataIMU[0] = heading_int / 256;
    RawDataIMU[1] = heading_int - ((heading_int / 256) << 8);
    RawDataIMU[2] = pitch_int / 256;
    RawDataIMU[3] = pitch_int - ((pitch_int / 256) << 8);
    RawDataIMU[4] = roll_int / 256;
    RawDataIMU[5] = roll_int - ((roll_int / 256) << 8);
    RawDataIMU[6] = accel_x_int / 256;
    RawDataIMU[7] = accel_x_int - ((accel_x_int / 256) << 8);
    RawDataIMU[8] = accel_y_int / 256;
    RawDataIMU[9] = accel_y_int - ((accel_y_int / 256) << 8);
    RawDataIMU[10] = accel_z_int / 256;
    RawDataIMU[11] = accel_z_int - ((accel_z_int / 256) << 8);

    /////////////////////////////////////////////////////////////
    // BLE Mode //////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////

    //Serial.println("BLE communication");
    BLECentral central = blePeripheral.central();

    if (central) { // if a central is connected to peripheral
      central.poll();
      //update the IMU characteristic
      const unsigned char imuCharArray[13] = {
        idChar.value()[0], RawDataIMU[0], RawDataIMU[1], RawDataIMU[2], RawDataIMU[3], RawDataIMU[4],
        RawDataIMU[5], RawDataIMU[6], RawDataIMU[7], RawDataIMU[8], RawDataIMU[9],
        RawDataIMU[10], RawDataIMU[11]
      };
      //imuChar.setValue(imuCharArray, 13); //notify central with new data
      
      //update the stretch sensor characteristic
      const unsigned char capaCharArray[21] = {
        idChar.value()[0], RawDataCapacitance[0], RawDataCapacitance[1], RawDataCapacitance[2], RawDataCapacitance[3], RawDataCapacitance[4],
        RawDataCapacitance[5], RawDataCapacitance[6], RawDataCapacitance[7], RawDataCapacitance[8], RawDataCapacitance[9],
        RawDataCapacitance[10], RawDataCapacitance[11], RawDataCapacitance[12], RawDataCapacitance[13], RawDataCapacitance[14],
        RawDataCapacitance[15], RawDataCapacitance[16], RawDataCapacitance[17], RawDataCapacitance[18], RawDataCapacitance[19]
      };
      stretchChar.setValue(capaCharArray, 21); //notify central with new data
    }
    //Serial.println("updating time");
    // increment previous time, so we keep proper pace

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
  for (int i = 0; i < 16; i++) {
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
  if (INTERRUPT_MODE == INTERRUPT_ENABLED) {
    // don't do anything until the interrupt pin goes low:
    while (digitalRead(InterruptPin) == HIGH);
  }

  // Set the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(DATA);                   //  Select Data Package
  SPI.transfer(PADDING);                //  Get Sequence Number
  //Read capacitance
  //Serial.print("RAW: ");
  for (int a = 0; a < 10; a++) {
    int num = 0;
    for (int i = 0; i < 2; i++) {
      raw[a*2+i] =  SPI.transfer(PADDING);    //  Pad out the remaining configuration package
      if (i == 0) num += raw[a*2+i]*256;
      else num += raw[a*2+i];
    }
    if (false) 
    {
      Serial.print(num);
      Serial.print(" ");
    }
  }

  //Serial.println();

  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);

  //Wait for next data packet to start sampling
  if (INTERRUPT_MODE == INTERRUPT_ENABLED) {
    while (digitalRead(InterruptPin) == LOW);
  }
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

  switch (Resolution_Config) {
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
float extractCapacitance(int raw[], int channel) {

  float Capacitance = 0;

  Capacitance = (raw[2 * channel]) * 256 + raw[2 * channel + 1];
  Capacitance /= CapacitanceScalingFactor;

  return Capacitance;

}

void BLECentralConnect(BLECentral& central)
{
  
}

void BLECentralDisconnect(BLECentral& central)
{
  
}

