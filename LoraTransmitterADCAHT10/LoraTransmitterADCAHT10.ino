#include <RadioLib.h>
#include "I2C_AHT10.h"
#include <Wire.h>

#define NODENAME "soil1"

AHT10 humiditySensor;

const int DIO0 = 2;
const int DIO1 = 6;
const int DIO2 = 7;
const int DIO5 = 8;

const int LORA_RST = 4;
const int LORA_CS = 10;

const int SPI_MOSI = 11;
const int SPI_MISO = 12;
const int SPI_SCK = 13;

int sensorPin = A2; // select the input pin for the potentiometer
int sensorPowerCtrlPin = 5;

//SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST, SPI, SPISettings());
SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1, SPI, SPISettings());
//SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST,DIO1);

// or using RadioShield
// https://github.com/jgromes/RadioShield
//SX1278 radio = RadioShield.ModuleA;


void setup()
{
    Serial.begin(115200);
    SPI.begin();

    // initialize SX1278 with default settings
    Serial.print(F("[SX1278] Initializing ... "));
    int state = radio.begin();
    if (state == ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }

    pinMode(sensorPowerCtrlPin, OUTPUT);
    sensorPowerOn();

    Wire.begin();
    if (humiditySensor.begin() == false)
    {
        Serial.println("AHT10 not detected. Please check wiring. Freezing.");
    }
    else
        Serial.println("AHT10 acknowledged.");
}

int sensorValue = 0;     // variable to store the value coming from the sensor
int16_t packetnum = 0;   // packet counter, we increment per xmission
float temperature = 0.0; //
float humidity = 0.0;
String errcode = "";

void loop()
{
    while (1)
    {
        String str;
        int state = radio.receive(str);
        if (state == ERR_NONE)
        {
            Serial.println(F("success!"));
            Serial.print(F("[SX1278] Data:\t\t\t"));
            Serial.println(str);
            Serial.print(F("[SX1278] RSSI:\t\t\t"));
            Serial.print(radio.getRSSI());
            Serial.println(F(" dBm"));
            Serial.print(F("[SX1278] SNR:\t\t\t"));
            Serial.print(radio.getSNR());
            Serial.println(F(" dB"));
            Serial.print(F("[SX1278] Frequency error:\t"));
            Serial.print(radio.getFrequencyError());
            Serial.println(F(" Hz"));
            if (str.startsWith(NODENAME))
            {
                delay(100);
                sensorPowerOn();
                delay(100);
                sensorValue = analogRead(sensorPin);
                delay(200);

                if (humiditySensor.available() == true)
                {
                    temperature = humiditySensor.getTemperature();
                    humidity = humiditySensor.getHumidity();

                    Serial.print("Temperature: ");
                    Serial.print(temperature, 2);
                    Serial.print(" C\t");
                    Serial.print("Humidity: ");
                    Serial.print(humidity, 2);
                    Serial.println("% RH");
                }
                else
                {
                    radio.transmit("AHT10ERR ADC:" + (String)sensorValue);
                    break;
                }
                if (isnan(humidity) || isnan(temperature))
                {
                    Serial.println(F("Failed to read from AHT sensor!"));
                }

                delay(100);

                Serial.print(F("Moisture ADC : "));
                Serial.println(sensorValue);

                String message = "#" + (String)packetnum + " Humidity:" + (String)humidity + "% Temperature:" + (String)temperature + "C" + " ADC:" + (String)sensorValue;
                Serial.println(message);
                String lora_msg = "H:" + (String)humidity + "% T:" + (String)temperature + "C" + " ADC:" + (String)sensorValue;
                packetnum++;
                radio.transmit(lora_msg);
            }
        }
    }
}

void sensorPowerOn(void)
{
    digitalWrite(sensorPowerCtrlPin, HIGH); //Sensor power on
}
