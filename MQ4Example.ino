/**
 * @file MQ4Example.ino
 * @brief Uses the built in Mama Duck with a MQ4 sensor attached
 * 
 * @date 2023-11-06
 * 
 * @copyright Copyright (c) 2023
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
#define pin 35 //Pin connected to A0 of MQ4
#define type "MQ-4" //MQ4
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO: 10, for ESP32 devices (T-Beam, Heltec Wifi LoRa): 12
#define RatioMQ4CleanAir 4.4 //RS / R0 = 60 ppm

//Declare Sensor
MQUnifiedsensor MQ4(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);


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

  //Set math model to calculate the PPM concentration and the value of constants
  MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ4.setA(1012.7); MQ4.setB(-2.786); // Configurate the equation values to get CH4 concentration

  // init the sensor
  /*****************************  MQ Init *********************************
  Input:  pin, type 
  Output:  
  Remarks: This function create the sensor object.
  *****************************************************************************/
  MQ4.init();
  /*****************************  MQ CAlibration ******************************/
  // Explanation: 
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
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
  /*****************************  MQ CAlibration ******************************/
  MQ4.serialDebug(true);

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
    MQ4.update(); // Update data, the arduino will be read the voltage on the analog pin
    MQ4.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup
    MQ4.serialDebug(); // Will print the table on the serial port

    float sensorRead = MQ4.readSensor(); 
    String sensorVal = String(sensorRead);
    // Serial.println(sensorVal);

  duck.sendData(topics::sensor, sensorVal);
  return true;
}

