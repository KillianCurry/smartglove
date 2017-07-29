#include <CurieIMU.h>
#include <MadgwickAHRS.h>
#include <CurieBLE.h>
#include <SPI.h>


// Declaration BLE
BLEPeripheral blePeripheral;
BLEService stretchSenseService("00601001-7374-7265-7563-6873656e7365"); //give your custom service an ID
BLECharacteristic stretchSenseChar("00601002-7374-7265-7563-6873656e7365", BLERead | BLENotify, 20); //give your custom characteristic an ID, properties, and length
BLECharacteristic imuChar("00601050-7374-7265-7563-6873656e7365", BLERead | BLENotify, 20); //give your custom characteristic an ID, properties, and length
int counter_LED = 0;

// Declaration IMU
Madgwick filter;
unsigned long notificationInterval, lastNotification, updateInterval, lastUpdate;
float accelScale, gyroScale;
int RawDataIMU[22];



// Declaration StretchSense

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
int   ODR_MODE        =      RATE_50HZ;
int   INTERRUPT_MODE   =     INTERRUPT_DISABLED;
int   TRIGGER_MODE    =      TRIGGER_DISABLED;
int   FILTER_MODE     =      FILTER_1PT;
int   RESOLUTION_MODE =      RESOLUTION_100fF;

//SPI Configuration
SPISettings SPI_settings(2000000, MSBFIRST, SPI_MODE1);

//Default scaling factor
int   CapacitanceScalingFactor = 100; //Default value
int   RawDataCapacitance[22];

float timestamp = 0;
float counter = 0;
float previous_value_counter = 0;


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
  blePeripheral.addAttribute(stretchSenseChar); // add your custom characteristic

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
  notificationInterval = 1000 / 10;//update BLE characteristics at 10Hz
  updateInterval = 1000 / 25;//update madgwick etc at 25Hz
  
  lastNotification = lastUpdate = millis();

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
    float roll, pitch, heading;
    int roll_int, pitch_int, heading_int;
    float accel_x, accel_y, accel_z;
    int accel_x_int, accel_y_int, accel_z_int;
    String str = "";
    String str2 = "";

    // check if it's time to read data and update the filter
    if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
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
    // shift the value to be positive
    pitch = pitch + 90;
    roll = roll + 180;
    accel_x = ax + 2;
    accel_y = ay + 2;
    accel_z = az + 2;
    // shift the decimal place and cast into integer
    heading_int = heading * 100;
    pitch_int = pitch * 100;
    roll_int = roll * 100;
    accel_x_int = accel_x * 100;
    accel_y_int = accel_y * 100;
    accel_z_int = accel_z * 100;
    //Serial.println("converting IMU values");
    // separate each integer IMU into 2 headecimal values
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
    RawDataIMU[12] = 0;
    RawDataIMU[13] = 0;
    RawDataIMU[14] = 0;
    RawDataIMU[15] = 0;
    RawDataIMU[16] = 0;
    RawDataIMU[17] = 0;
    RawDataIMU[18] = 0;
    RawDataIMU[19] = 0;



    /////////////////////////////////////////////////////////////
    // BLE Mode //////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////

    //Serial.println("BLE communication");
    BLECentral central = blePeripheral.central();
    //Serial.println("central found");
    if (central) { // if a central is connected to peripheral
      const unsigned char imuCharArray[20] = {
        RawDataIMU[0], RawDataIMU[1], RawDataIMU[2], RawDataIMU[3], RawDataIMU[4],
        RawDataIMU[5], RawDataIMU[6], RawDataIMU[7], RawDataIMU[8], RawDataIMU[9],
        RawDataIMU[10], RawDataIMU[11], RawDataIMU[12], RawDataIMU[13], RawDataIMU[14],
        RawDataIMU[15], RawDataIMU[16], RawDataIMU[17], RawDataIMU[18], RawDataIMU[19]
      };
      //Serial.println("IMU converted");

      imuChar.setValue(imuCharArray, 20); //notify central with new data
    }
    //Serial.println("imu updated");
    if (central) { // if a central is connected to peripheral
      const unsigned char capaCharArray[20] = {
        RawDataCapacitance[0], RawDataCapacitance[1], RawDataCapacitance[2], RawDataCapacitance[3], RawDataCapacitance[4],
        RawDataCapacitance[5], RawDataCapacitance[6], RawDataCapacitance[7], RawDataCapacitance[8], RawDataCapacitance[9],
        RawDataCapacitance[10], RawDataCapacitance[11], RawDataCapacitance[12], RawDataCapacitance[13], RawDataCapacitance[14],
        RawDataCapacitance[15], RawDataCapacitance[16], RawDataCapacitance[17], RawDataCapacitance[18], RawDataCapacitance[19]
      };
      //Serial.println("capa converted");

      stretchSenseChar.setValue(capaCharArray, 20); //notify central with new data
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
    // don't do anything until the interupt pin goes low:
    while (digitalRead(InterruptPin) == HIGH);
  }

  // Set the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);

  SPI.transfer(DATA);                   //  Select Data Package
  SPI.transfer(PADDING);                //  Get Sequence Number
  //Read capacitance
  Serial.print("RAW: ");
  for (int a = 0; a < 10; a++) {
    int num = 0;
    for (int i = 0; i < 2; i++) {
      raw[a*2+i] =  SPI.transfer(PADDING);    //  Pad out the remaining configuration package
      if (i == 0) num += raw[a*2+i]*256;
      else num += raw[a*2+i];
    }
    if (a > 0) 
    {
      Serial.print(num);
    }
  }

  Serial.println();

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


