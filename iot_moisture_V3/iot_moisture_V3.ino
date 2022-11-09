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

#define FREQUENCY 434.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 9
#define CODING_RATE 7
#define OUTPUT_POWER 10 //Sets transmission output power. Allowed values range from 2 to 17 dBm.
#define PREAMBLE_LEN 8
#define GAIN 0 //1 to 6 where 1 is the highest gain. Set to 0 to enable automatic gain control (recommended).

//pin set
#define VOLTAGE_PIN A3
#define PWM_OUT_PIN 9
#define SENSOR_POWER_PIN 5
#define ADC_PIN A2
#define SX1278_SYNC_WORD 0x12
#define VERBOSE true

#define HUMIDITY_HIGH 860
#define HUMIDITY_LOW 508
#define MILLI_VOLT_HIGH 2700
#define MILLI_VOLT_LOW 1900

SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1);

AHT10 humiditySensor;
int sensorPin = A2; // select the input pin for the potentiometer
int sensorPowerCtrlPin = 5;
int ADC_O_1; // ADC Output First 8 bits
int ADC_O_2; // ADC Output Next 2 bits

void setup()
{

    Serial.begin(115200);

    // initialize SX1278 with default settings
#if VERBOSE
    Serial.print(F("Initializing ... "));
#endif
    //int state = radio.begin();
    int state = radio.begin(FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SX1278_SYNC_WORD, OUTPUT_POWER, PREAMBLE_LEN, GAIN);
    if (state == RADIOLIB_ERR_NONE)
    {
#if VERBOSE      
        Serial.println(F("success!"));
#endif
    }
    else
    {
#if VERBOSE
        Serial.print(F("failed, code "));
        Serial.println(state);
#endif
        while (true)
            ;
    }
#if VERBOSE
    Serial.print(F("Waiting for incoming transmission ... "));
#endif
    pinMode(sensorPowerCtrlPin, OUTPUT);
    digitalWrite(sensorPowerCtrlPin, HIGH); //Sensor power on
    // set up Timer 1
    pinMode(PWM_OUT_PIN, OUTPUT);

    Wire.begin();
    if (humiditySensor.begin() == false)
    {
#if VERBOSE
        Serial.println("AHT10 not detected. Please check wiring. Freezing.");
#endif
    }
    else {

#if VERBOSE
        Serial.println("AHT10 acknowledged.");
#endif
    }
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
      
        //Serial.println(  " V:" + (String)readVcc() );
        String str;
        int HumidityPercent = 0;
        String HP;
        int VoltsgePercent = 0;
        String BP;
        int state = radio.receive(str);
        //Serial.println(state);
        if (state == RADIOLIB_ERR_NONE)
        {
#if VERBOSE
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
#endif
            if (str.startsWith("PREPARE"))
            {
                int i = 0;
                for (i = 0; i < 5; i++)
                {
                    if (humiditySensor.available() == true)
                    {
                        temperature = humiditySensor.getTemperature();
                        humidity = humiditySensor.getHumidity();
                    }
                    if (isnan(humidity) || isnan(temperature))
                    {
#if VERBOSE
                        Serial.println(F("Failed to read from AHT sensor!"));
#endif
                    }
                }

                getADC();
#if VERBOSE
                Serial.print("Temperature: ");
                Serial.print(temperature, 2);
                Serial.print(" C\t");
                Serial.print("Humidity: ");
                Serial.print(humidity, 2);
                Serial.println("% RH");
                Serial.print(F("Moisture ADC : "));
                Serial.println(sensorValue);
#endif
            }
            //Serial.println(NODENAME);
            if (str.startsWith(NODENAME))
            {

                HumidityPercent = 100 - GetSensorPercentage(HUMIDITY_LOW, HUMIDITY_HIGH, sensorValue); //reverse %
                HP = FormatPercentage(HumidityPercent);
                
                VoltsgePercent = GetSensorPercentage(MILLI_VOLT_LOW, MILLI_VOLT_HIGH, readVcc());
                BP = FormatPercentage(VoltsgePercent);

//                String message = "#" + (String)packetnum + " Humidity:" + (String)humidity + "% Temperature:" + (String)temperature + "C" + " ADC:" + (String)sensorValue + " V:" + (String)readVcc() ;
                String message = "#" + (String)packetnum + " Humidity:" + (String)humidity + "% Temperature:" + (String)temperature + "C" + " HS:" + (String)HumidityPercent + "% B:" + (String)VoltsgePercent + "%";
#if VERBOSE
                Serial.println(message);
#endif


                String lora_msg = "HA:" + (String)humidity + "% T:" + (String)temperature + "C" + " H:" + HP + "% B:" + BP + "%";
                packetnum++;
                radio.transmit(lora_msg);
                Serial.println(lora_msg);
            }
        }
    }
}

void getADC()
{
    //Instead 555
    TCCR1A = bit(COM1A0);            // toggle OC1A on Compare Match
    TCCR1B = bit(WGM12) | bit(CS10); // CTC, scale to clock
    OCR1A = 1;

    ADMUX = _BV(REFS0) | _BV(MUX1);
    ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0);
    delay(50);
    for (int i = 0; i < 3; i++)
    {
        //start ADC conversion
        ADCSRA |= (1 << ADSC);

        delay(10);

        if ((ADCSRA & 0x40) == 0)
        {
            ADC_O_1 = ADCL;
            ADC_O_2 = ADCH;
            sensorValue = (ADC_O_2 << 8) + ADC_O_1;
            ADCSRA |= 0x40;

//            Serial.print("ADC:");
//            Serial.println(sensorValue);
        }
        ADCSRA |= (1 << ADIF); //reset as required
        delay(50);
    }

    return;
}

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result ; // Back-calculate AVcc in mV
  return result; 
}

#define HUMIDITY_HIGH 860
#define HUMIDITY_LOW 650
#define MILLI_VOLT_HIGH 2700
#define MILLI_VOLT_LOW 1900


int GetSensorPercentage(int LowSensorValue, int HighSensorValue, int ActualSensorValue) {

float SensorRange = HighSensorValue - LowSensorValue;
float Scaler = 100 / SensorRange;
float SensorPercentage = Scaler * (ActualSensorValue - LowSensorValue);
return int constrain(SensorPercentage, 0, 100) ;
}

String FormatPercentage( int percentage){
String str = String(percentage);   
String space = "----";
str.trim();
int slen = str.length();
return space.substring(1, 4-slen) + str;  

}
