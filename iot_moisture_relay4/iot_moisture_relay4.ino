#include <RadioLib.h>
#include "I2C_AHT10.h"
#include <Wire.h>

#define NODENAME "RELAY"

#define LED 5

#define DIO0 2
#define DIO1 6
#define DIO2 7
#define DIO5 8

#define LORA_RST 9
#define LORA_CS 10

#define SPI_MOSI 11
#define SPI_MISO 12
#define SPI_SCK 13

#define FREQUENCY 433.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 9
#define CODING_RATE 7
#define OUTPUT_POWER 10 //Sets transmission output power. Allowed values range from 2 to 17 dBm.
#define PREAMBLE_LEN 8
#define GAIN 0 //1 to 6 where 1 is the highest gain. Set to 0 to enable automatic gain control (recommended).

//pin set

#define SX1278_SYNC_WORD 0x12
#define VERBOSE true


SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1);
int relay[] = {4, 3, A3, A2};

void setup()
{
 
  pinMode(5, OUTPUT);
  //digitalWrite(5, LOW);
  
   for (int i = 0; i < 4; i++) { 
     pinMode(relay[i], OUTPUT);
     delay(5);
     digitalWrite(relay[i], LOW);  
   }  
 
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

    
}


int16_t packetnum = 0;   // packet counter, we increment per xmission
String errcode = "";
String str = "";

void loop()
{
    while (1)
    {
        //Listen
        //digitalWrite(5, LOW); //LED off
        int state = radio.receive(str);
        //Serial.println(state);
        if (state == RADIOLIB_ERR_NONE)
        {
          if (str == ""){
          //digitalWrite(5, HIGH); //LED on
          }
          
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
            { }

            }
            //Serial.println(NODENAME);
            if (str.startsWith(NODENAME))
            
            {

TurnOnRelay();
  delay(2000);
TurnOffRelay();

                String message = "#" + (String)packetnum + " Reply here.......";
#if VERBOSE
                Serial.println(message);
#endif


                String lora_msg = "Relay 1 ON";
                packetnum++;
                radio.transmit(lora_msg);
                Serial.println(lora_msg);
            }
        }
    }




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




int count=0;

void TurnOnRelay(void)
{
  for (int i = 0; i < 4; i++) { 
     digitalWrite(relay[i], HIGH);
     delay(1000) ;
  }
  //digitalWrite(4, HIGH);    //RELAY 1
  //digitalWrite(3, HIGH);    //RELAY 2
  //digitalWrite(A3, HIGH);   //RELAY 3
  //digitalWrite(A2, HIGH);   //RELAY 4

  

  //delay(100);
}

void TurnOffRelay(void)
{
  for (int i = 0; i < 4; i++) { 
     digitalWrite(relay[i], LOW);
     delay(1000) ;
  }
  
  //digitalWrite(4, LOW);
  //digitalWrite(3, LOW);
  //digitalWrite(A3, LOW);
  //digitalWrite(A2, LOW);

  
  //delay(100);
}
