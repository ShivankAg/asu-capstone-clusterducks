# MQSensorsTemplate.ino Instructions
> This is a guide on how to use the [MQSensorsTemplate.ino](https://github.com/ShivankAg/asu-capstone-clusterducks/blob/9e0fd9f0b65a797f935d75cd878a990ec39564f2/MQSensorsTemplate.ino) for MQ sensors integration with the [ClusterDuck-Protocol](https://github.com/Call-for-Code/ClusterDuck-Protocol)

## ClusterDuck-Protocol Installation
- For an in-depth guide on how to install the ClusterDuck-Protocol refer to the [CDP-Wiki](https://github.com/Call-for-Code/ClusterDuck-Protocol/wiki)

## Libraries
- The main library referenced in the template file is the [MQSensorsLib](https://github.com/miguel5612/MQSensorsLib)

## How to use [MQSensorsTemplate.ino](https://github.com/ShivankAg/asu-capstone-clusterducks/blob/9e0fd9f0b65a797f935d75cd878a990ec39564f2/MQSensorsTemplate.ino)
- Thoroughly read through the comments of the file especially block comments like these 
```
/*
Refer to https://github.com/miguel5612/MQSensorsLib/tree/master/examples to grab a specified Ratio number
For example, for MQ4: https://github.com/miguel5612/MQSensorsLib/blob/master/examples/MQ-4/MQ-4.ino
#define         RatioMQ4CleanAir 4.4 //RS / R0 = 60 ppm
Uncomment the line below and replace with the appropriate value
*/
#define RatioMQ4CleanAir 4.4 //RS / R0 = 60 ppm 
```
- For the various calculation variables and formulas refer to this full list of example ino files from the [MQSensorsLib](https://github.com/miguel5612/MQSensorsLib) library:
    - [MQ-131](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-131/MQ-131.ino)
    - [MQ-135](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-135/MQ-135.ino)
    - [MQ-136](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-136/MQ-136.ino)
    - [MQ-2](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-2/MQ-2.ino)
    - [MQ-3](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-3/MQ-3.ino)
    - [MQ-4](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-4/MQ-4.ino)
    - [MQ-5](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-5/MQ-5.ino)
    - [MQ-6](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-6/MQ-6.ino)
    - [MQ-7](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-7/MQ-7.ino)
    - [MQ-8](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-8/MQ-8.ino)
    - [MQ-9](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-9/MQ-9.ino)
- Please read thoroughly through the setup() method of the MQ sensor you are trying to implement and follow the comments in [MQSensorsTemplate.ino](https://github.com/ShivankAg/asu-capstone-clusterducks/blob/9e0fd9f0b65a797f935d75cd878a990ec39564f2/MQSensorsTemplate.ino)
- An example integration ```setup()``` method for the [MQ-4](https://github.com/miguel5612/MQSensorsLib/blob/c927504bf5ce53d82e5b0c98db5c8155b3573e9a/examples/MQ-4/MQ-4.ino) sensor with the ClusterDuck-Protocol would look like this:
```
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
```
- As you can see some parts of code are directly copied from the example file of the [MQSensorsLib](https://github.com/miguel5612/MQSensorsLib) library
- These 2 example files for MQ-4 and MQ-7 sensors are tested with integrations with CDP
  - [MQ-4](https://github.com/ShivankAg/asu-capstone-clusterducks/blob/9e0fd9f0b65a797f935d75cd878a990ec39564f2/MQ4Example/MQ4Example.ino)
  - [MQ-7](https://github.com/ShivankAg/asu-capstone-clusterducks/blob/9e0fd9f0b65a797f935d75cd878a990ec39564f2/MQ7Example/MQ7Example.ino)
  - > Note that some variables will still need to be changed in order for it to work with your Duck setup  

## Disclaimer
- For further readings on the MQ sensors please refer to the [MQSensorsLib](https://github.com/miguel5612/MQSensorsLib) library.
- For questions regarding the ClusterDuck-Protocol please refer to their [GitHub page](https://github.com/Call-for-Code/ClusterDuck-Protocol).
- Some of the MQ sensors were not tested and are not guaranteed to work with CDP.
