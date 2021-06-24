/*
   RadioLib SX127x Receive Example

   This example listens for LoRa transmissions using SX127x Lora modules.
   To successfully receive data, the following settings have to be the same
   on both transmitter and receiver:
    - carrier frequency
    - bandwidth
    - spreading factor
    - coding rate
    - sync word
    - preamble length

   Other modules from SX127x/RFM9x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx127xrfm9x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define DIO0 36
#define DIO1 27

#define LORA_RST 33
#define LORA_CS 32

#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14


#define MP_ESP32_SSD1306_I2C_ADDR 0x3C
#define MP_ESP32_SSD1306_WIDTH 128 // OLED display width, in pixels
#define MP_ESP32_SSD1306_HEIGHT 64 // OLED display height, in pixels
#define MP_ESP32_SSD1306_RST -1

#define MP_ESP32_I2C_SDA 4
#define MP_ESP32_I2C_SCL 5

Adafruit_SSD1306 display(MP_ESP32_SSD1306_WIDTH, MP_ESP32_SSD1306_HEIGHT, &Wire, MP_ESP32_SSD1306_RST);

/*
Begin method:
Carrier frequency: 434.0 MHz (for SX1276/77/78/79 and RFM96/98) or 915.0 MHz (for SX1272/73 and RFM95/97)
Bandwidth: 125.0 kHz (dual-sideband)
Spreading factor: 9
Coding rate: 4/7
Sync word: SX127X_SYNC_WORD (0x12)
Output power: 10 dBm
Preamble length: 8 symbols
Gain: 0 (automatic gain control enabled)
Other:
Over-current protection: 60 mA
Inaccessible:
LoRa header mode: explicit
Frequency hopping: disabled

*/

#define FREQUENCY 434.0
#define BANDWIDTH 125.0
#define SPREADING_FACTOR 9
#define CODING_RATE 7
#define OUTPUT_POWER 10
#define PREAMBLE_LEN 8
#define GAIN 0

// SX1278 has the following connections:
// NSS pin:   10
// DIO0 pin:  2
// RESET pin: 9
// DIO1 pin:  3
//SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1);
SX1278 radio = new Module(LORA_CS, DIO0, LORA_RST, DIO1, SPI, SPISettings());

// or using RadioShield
// https://github.com/jgromes/RadioShield
//SX1278 radio = RadioShield.ModuleA;

int last_time;

void setup()
{
    Serial.begin(115200);

    // initialize SX1278 with default settings
    Serial.print(F("[SX1278] Initializing ... "));
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    int state = radio.begin(FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SX127X_SYNC_WORD, OUTPUT_POWER, PREAMBLE_LEN, GAIN);
    //int state = radio.begin();
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

    Wire.begin(MP_ESP32_I2C_SDA, MP_ESP32_I2C_SCL);
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
   
    display.clearDisplay();
    display.setTextSize(2);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println(F("Makerfabs"));
    display.display();
    delay(1000);
    last_time = millis();
}

String ADCvalue[2];



void loop()
{
    //Serial.print(F("[SX1278] Waiting for incoming transmission ... "));

    // you can receive data as an Arduino String
    // NOTE: receive() is a blocking method!
    //       See example ReceiveInterrupt for details
    //       on non-blocking reception method.
    String str;
    String sensorADC;
    int state = radio.receive(str);

    // you can also receive data as byte array
    /*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */


    if (state == ERR_NONE)
    {        
        Serial.println(str);

        if(str.indexOf("SOIL1")>-1)
        {   sensorADC = str.substring(str.indexOf("ADC:")+4,str.indexOf("ADC:")+7);
            
            ADCvalue[0] =  sensorADC;
            Serial.println("1: "+ADCvalue[0]);         
            }
            
        if(str.indexOf("SOIL2")>-1)
        {   sensorADC = str.substring(str.indexOf("ADC:")+4,str.indexOf("ADC:")+7);

            ADCvalue[1] =  sensorADC;
            Serial.println("2: "+ADCvalue[1]);
            
            }
    }
    if((millis()-last_time)>1000)
    {
      last_time = millis();
      display.clearDisplay();
      display.setTextSize(2);              // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE); // Draw white text
      display.setCursor(0, 0);             // Start at top-left corner
      display.print(F("SOIL1: "));
      display.println(ADCvalue[0]);
      display.print(F("SOIL2: "));
      display.println(ADCvalue[1]);
      display.display();
      delay(1000);
      }
    delay(100);
}
