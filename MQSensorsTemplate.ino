/**
 * @file MQSensorsTemplate.ino
 * @brief Template file for sensor integration with CDP
 * 
 * @date 2024-01-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <MamaDuck.h>
#include <arduino-timer.h>
#include <string>   

#ifdef SERIAL_PORT_USBVIRTUAL
    #define Serial SERIAL_PORT_USBVIRTUAL
#endif

// Include the MQ library
#include <MQUnifiedsensor.h>

//Definitions
#define placa "T-Beam" 
#define Voltage_Resolution 3.3 //ADC pins on ESP32 boards can only read from 0 - 3.3V otherwise use 5V
/*
Note on pin choice: On ESP32 Boards with Wifi function (T-Beams and Heltec Wifi Lora 32),
while using the Wifi function, some ADC pins are unavailable
For T-Beams, tested using pin 35.
If error "Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply" is shown
it is probably because of this problem.
*/
#define pin 35 //Pin connected to A0 pin of the MQ sensor
#define type "" //Insert the name of your MQ sensor here (e.g. MQ4)
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO: 10, for ESP32 devices (T-Beam, Heltec Wifi LoRa): 12

/*
Refer to https://github.com/miguel5612/MQSensorsLib/tree/master/examples to grab a specified Ratio number
For example, for MQ4: https://github.com/miguel5612/MQSensorsLib/blob/master/examples/MQ-4/MQ-4.ino
#define         RatioMQ4CleanAir 4.4 //RS / R0 = 60 ppm
Uncomment the line below and replace with the appropriate value
*/
// #define RatioMQ4CleanAir 4.4 //RS / R0 = 60 ppm 

//Declare Sensor, replace MQName with your appropriate sensor name (e.g. MQ4)
MQUnifiedsensor MQName(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);


// create a built-in mama duck
MamaDuck duck;

auto timer = timer_create_default();
const int INTERVAL_MS = 60000;
char message[32]; 
int counter = 1;

void setup() {
  // We are using a hardcoded device id here, but it should be retrieved or
  // given during the device provisioning then converted to a byte vector to
  // setup the duck NOTE: The Device ID must be exactly 8 bytes otherwise it
  // will get rejected
  std::string deviceId("MAMA0001");
  std::vector<byte> devId;
  devId.insert(devId.end(), deviceId.begin(), deviceId.end());

  // Use the default setup provided by the SDK
  duck.setupWithDefaults(devId);
  Serial.println("MAMA-DUCK...READY!");

  /*
  Copy the setup method from the example ino file from MQSensorLib: https://github.com/miguel5612/MQSensorsLib/tree/master/examples
  For example, for the MQ4 sensor: https://github.com/miguel5612/MQSensorsLib/blob/master/examples/MQ-4/MQ-4.ino
  The setup() method is defined as follows:
  MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ4.setA(1012.7); MQ4.setB(-2.786); // Configurate the equation values to get CH4 concentration
  MQ4.init();
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i<=10; i ++) {
    MQ4.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.print(".");
  }
  MQ4.setR0(calcR0/10);
  Serial.println("  done!.");

  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  MQ4.serialDebug(true);
  */

  // Back to the CDP code base
  // initialize the timer. The timer thread runs separately from the main loop
  // and will trigger sending a counter message.
  timer.every(INTERVAL_MS, runSensor);
}

void loop() {
  timer.tick();
  // Use the default run(). The Mama duck is designed to also forward data it receives
  // from other ducks, across the network. It has a basic routing mechanism built-in
  // to prevent messages from hoping endlessly.
  duck.run();
}

bool runSensor(void *) {
    // Replace MQName with the appropriate name for the MQ sensor as defined above
    MQName.update(); // Update data, the arduino will be read the voltage on the analog pin
    MQName.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
    MQName.serialDebug(); // Will print the table on the serial port

    float sensorRead = MQName.readSensor(); 
    String sensorVal = String(sensorRead);
    // Serial.println(sensorVal);

  duck.sendData(topics::sensor, sensorVal);
  return true;
}

