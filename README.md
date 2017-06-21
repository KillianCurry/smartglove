# smartglove
An open-source project to process data from the StretchSense glove.

Includes:
* Arduino code for an Arduino 101 with attached 10-port StretchSense SPI Adaptor
* C++ DLL code for a portable plugin that processes data from the Arduino
* C# Unity code that demonstrates proper use of the DLL with tech demos

Basic Setup:
* Attach the StretchSense ten-channel sensing circuit to the TinyTile via header pins and StretchSense shield.
* Then, attach the stretch sensors to the sensing circuit. If using a five-sensor configuration, attach a sensor to every odd channel.
* Download GloveReader.ino and write this program to the TinyTile.
* Use the Arduino IDE's Serial Monitor to ensure the correct output is being sent by the TinyTile. You should see a list of 13 numbers, separated by commas. The first three numbers are the roll, pitch, and yaw of the gyroscope; the last ten are the ten channels of the sensing circuit.
* Download the Library and compile it using Visual Studio (using x64, Release mode).
* Download the Unity project and open in Unity.
* Place the generated DLL from the x64/Release folder in the Assets/Plugins folder of the Unity project.
* Enter Play mode. A hand should be generated. Enter the serial port number the TinyTile is connected to, then click the Connect Serial button in the inspector. The other buttons in the inspector should become active.
* To calibrate the glove, first stretch out your fingers and point the glove toward the screen, palm down, then click Calibrate Minimum.
* Then, squeeze your fingers inward and click Calibrate Maximum.
* Finally, squeeze your thumb inward and click the '1' below Calibrate Maximum to calibrate the thumb individually.
