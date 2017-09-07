# StretchSense Sensing Glove
An open-source project to process data from the StretchSense sensing glove, for use by hobbyists, software developers, and makers. Currently supported on Windows.

Includes:
* Arduino code for an Arduino 101 with attached StretchSense SPI circuit.
* C++ DLL code that processes data from the Arduino
* C# Unity code that demonstrates proper use of the DLL

Basic Setup:
* Attach the Intel TinyTile to the StretchSense SPI circuit on the StretchSense sensing glove.
* Download the [Arduino program](Arduino), set desired hardware parameters at the top of the program, and upload this program to the TinyTile. You can use the serial monitor or Bluetooth's BLE nRF toolbox application to ensure the glove is operating.
* Download [library](Library) and compile it using Visual Studio (using Release mode).
* Download the [Unity project](Unity).
* Place the generated .dll file from the library's build folder in the Assets/Plugins folder of the Unity project.
* Open the project, and navigate to MainInterface's Start() function, where UUIDs are generated. Enter the appropriate UUIDs corresponding to your gloves using GenerateGlove. Make sure the UUIDs are wrapped in curly braces.
* Enter Play mode. Click 'Add' on the panel labelled with your desired UUID, and new glove geometry should be generated.
* Next, click 'Connect'. If a BLE connection is established, the button will change to 'Disconnect', and a blue circle indicating connection will appear. If the glove stops sending BLE notifications, this circle will fade.
* With a few clenches of your fingers and thumb, the glove will be automatically calibrated to your hand. If this calibration seems accurate, simply click the 'Reset' button and try again. 'Reset' also resets the IMU's heading, so make sure the glove is pointed at your screen when this happens.