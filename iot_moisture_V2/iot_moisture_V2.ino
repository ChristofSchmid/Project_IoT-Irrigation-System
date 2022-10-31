#include <RadioLib.h>
#include "I2C_AHT10.h"
#include <Wire.h>

#define NODENAME "Soil2"

#define DIO0 2
#define DIO1 6
#define DIO2 7
#define DIO5 8

#define LORA_RST 4
#define LORA_CS 10

#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK 13

#define FREQUENCY 433.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 9
#define CODING_RATE 7
#define OUTPUT_POWER 10
#define PREAMBLE_LEN 8
#define GAIN 0
#define SX1278_SYNC_WORD 0x12


SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1);

AHT10 humiditySensor;
int sensorPin = A2; // select the input pin for the potentiometer
int sensorPowerCtrlPin = 5;

void setup()
{
    Serial.begin(115200);

    // initialize SX1278 with default settings
    Serial.print(F("Initializing ... "));

    //int state = radio.begin();
    int state = radio.begin(FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SX1278_SYNC_WORD, OUTPUT_POWER, PREAMBLE_LEN, GAIN);
    if (state == RADIOLIB_ERR_NONE)
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
    Serial.print(F("Waiting for incoming transmission ... "));

    pinMode(sensorPowerCtrlPin, OUTPUT);
    digitalWrite(sensorPowerCtrlPin, HIGH); //Sensor power on

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
              Serial.println(state);
        if (state == RADIOLIB_ERR_NONE)
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
            if (str.startsWith("PREPARE"))
            {
                int i = 0;
                for (i = 0; i < 5; i++)
                {
                    sensorValue = analogRead(sensorPin);
                    delay(200);

                    if (humiditySensor.available() == true)
                    {
                        temperature = humiditySensor.getTemperature();
                        humidity = humiditySensor.getHumidity();
                    }
                    if (isnan(humidity) || isnan(temperature))
                    {
                        Serial.println(F("Failed to read from AHT sensor!"));
                    }
                }
                Serial.print("Temperature: ");
                Serial.print(temperature, 2);
                Serial.print(" C\t");
                Serial.print("Humidity: ");
                Serial.print(humidity, 2);
                Serial.println("% RH");
                Serial.print(F("Moisture ADC : "));
                Serial.println(sensorValue);
            }
            if (str.startsWith(NODENAME))
            {
                String message = "#" + (String)packetnum + " Humidity:" + (String)humidity + "% Temperature:" + (String)temperature + "C" + " ADC:" + (String)sensorValue;
                Serial.println(message);
                String lora_msg = "H:" + (String)humidity + "% T:" + (String)temperature + "C" + " ADC:" + (String)sensorValue;
                packetnum++;
                radio.transmit(lora_msg);
            }
        }
    }
}
