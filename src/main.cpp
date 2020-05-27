#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19", BLERead | BLENotify);

BLEService IMUService("1801");
BLEFloatCharacteristic axFloat("2A20", BLERead | BLENotify);
BLEFloatCharacteristic ayFloat("2A21", BLERead | BLENotify);
BLEFloatCharacteristic azFloat("2A22", BLERead | BLENotify);
BLEFloatCharacteristic gxFloat("2A23", BLERead | BLENotify);
BLEFloatCharacteristic gyFloat("2A24", BLERead | BLENotify);
BLEFloatCharacteristic gzFloat("2A25", BLERead | BLENotify);

const int numSamples = 119;

const int buttonPin = 12;     // the number of the pushbutton pin
const int ledPin =  11;      // the number of the LED pin

int buttonState = 0;    
int samplesRead = numSamples;

void setup()
{
      Serial.begin(9600);
      while (!Serial);

      // setup the built in led as an output
      pinMode(LED_BUILTIN, OUTPUT);
      // initialize the LED pin as an output:
      pinMode(ledPin, OUTPUT);
      // initialize the pushbutton pin as an input:
      pinMode(buttonPin, INPUT);

      if (!BLE.begin())
      {
            Serial.println("starting BLE failed!");
            while (1);
      }

      if (!IMU.begin())
      {
            Serial.println("Failed to initialize IMU!");
            while (1);
      }

      BLE.setLocalName("Gesture Monitor");
      BLE.setAdvertisedService(batteryService);
      batteryService.addCharacteristic(batteryLevelChar);
      BLE.addService(batteryService);

      BLE.setAdvertisedService(IMUService);
      IMUService.addCharacteristic(axFloat);
      IMUService.addCharacteristic(ayFloat);
      IMUService.addCharacteristic(azFloat);
      IMUService.addCharacteristic(gxFloat);
      IMUService.addCharacteristic(gyFloat);
      IMUService.addCharacteristic(gzFloat);
      BLE.addService(IMUService);

      BLE.advertise();
      Serial.println("Bluetooth device active, waiting for connections...");
}

void loop()
{
      float aX, aY, aZ, gX, gY, gZ;
      BLEDevice central = BLE.central();

      if (central)
      {
            Serial.print("Connected to central: ");
            Serial.println(central.address());
            digitalWrite(LED_BUILTIN, HIGH);
            while (central.connected())
            {
                  // wait for significant motion
                  while (samplesRead == numSamples) {
                        // read the state of the pushbutton value:
                        buttonState = digitalRead(buttonPin);
                        // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
                        if ((buttonState == HIGH) && (samplesRead == numSamples)) {
                              // turn LED on:
                              digitalWrite(ledPin, HIGH);
                              samplesRead = 0;
                        } else {
                              // turn LED off:
                              digitalWrite(ledPin, LOW);
                        }
                  }
                  while (samplesRead < numSamples) {
                        int battery = analogRead(A0);
                        int batteryLevel = map(battery, 0, 1023, 0, 100);
                        Serial.print("Battery Level % is now: ");
                        Serial.println(batteryLevel);
                        batteryLevelChar.writeValue(batteryLevel);
                        IMU.readAcceleration(aX, aY, aZ);
                        IMU.readGyroscope(gX, gY, gZ);
                        axFloat.writeValue(aX);
                        ayFloat.writeValue(aY);
                        azFloat.writeValue(aZ);
                        gxFloat.writeValue(gX);
                        gyFloat.writeValue(gY);
                        gzFloat.writeValue(gZ);               
                        samplesRead++;
                        Serial.print("Samples Read = "); 
                        Serial.println(samplesRead);
                        
                        if (samplesRead == numSamples) {
                               // add an empty line if it's the last sample
                               Serial.println();
                        }                       
                  }
            }
            delay(200);
      }
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print("Disconnected from central: ");
      Serial.println(central.address());
}