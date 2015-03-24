/*
	Example file for communicating with the NFRC522. The program prints the card data.
	Created by Eelco Rouw - Originally adapted by Grant Gibson. 
        Modified by phaseform for use with the Stellaris
*/


// pin mapping for SPI(2) on the Stellaris (also tiva??) using Rei Vilo's pinmap
// SDA  -  CS(2) -> PB_5 = pin 2 as an integer
// SCK  -  SCK(2) -> PB_4
// Mosi -  MOSI(2) -> PB_7
// Miso -  MISO(2) -> PB_6
// IRQ  - Not connected
// GND  - GND 
// RST  - reset -> PF_0
// VCC  - +3.3V = pin 1

#include <Mfrc522.h>
#include <SPI.h>

int chipSelectPin = 2;  //SDA on reader - connected to pin2 aka CS(2) 
int NRSTPD = 17; //aka PF_0
Mfrc522 Mfrc522(chipSelectPin, NRSTPD); //changed NRSTDP to NRSTPD
unsigned char serNum[5];
unsigned char sectorKey[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
unsigned char readData[16];

void setup() 
{  
	Serial.begin(9600);            
        SPI.setModule(2); // using SPI module 2...
        pinMode(chipSelectPin, OUTPUT); //moved this here from Mfrc522.cpp
	digitalWrite(chipSelectPin, LOW);
        pinMode(NRSTPD, OUTPUT); //moved this here from Mfrc522.cpp
        digitalWrite(NRSTPD, HIGH); //moved this here from Mfrc522.cpp
	pinMode(BLUE_LED, OUTPUT);
        pinMode(RED_LED, OUTPUT);
        pinMode(GREEN_LED, OUTPUT);
	Mfrc522.Init(); 
}

void loop()
{
	unsigned char i,tmp;
	unsigned char status;
	unsigned char str[MAX_LEN];
	unsigned char RC_size;
	unsigned char blockAddr;
	String mynum = "";
  	
	status = Mfrc522.Request(PICC_REQIDL, str);	
	if (status == MI_OK)
	{
		Serial.println("Card detected");
		Serial.print(str[0], BIN);
		Serial.print(" , ");
		Serial.print(str[1], BIN);
		Serial.println(" ");
	}

	status = Mfrc522.Anticoll(str);
	memcpy(serNum, str, 5);
	if (status == MI_OK)
	{
		Serial.println("The card's number is  : ");
		Serial.print(serNum[0], HEX);
		Serial.print(" , ");
		Serial.print(serNum[1], HEX);
		Serial.print(" , ");
		Serial.print(serNum[2], HEX);
		Serial.print(" , ");
		Serial.print(serNum[3], HEX);
		Serial.print(" , ");
		Serial.print(serNum[4], HEX);
		Serial.println(" ");
                Mfrc522.SelectTag(serNum);
                status = Mfrc522.Auth(PICC_AUTHENT1A,1,sectorKey,serNum);
                if (status == MI_OK)
                {
                  Serial.println("Authenticated...\r\n");
                } else
                {
                  Serial.println("Error authenticating...\r\n");
                }
                
                status = Mfrc522.ReadBlock(1, readData);
                if (status == MI_OK)
                {
                  for(i=0; i<16; i++)
                  {      
                             
                    Serial.write(readData[i]);
                    
                    delay(10);
                  }                  
                } else {
                  Serial.println("Error reading.");
                }
		// Should really check all pairs, but for now we'll just use the first

		/*if(serNum[0] == 29) 
		{
			Serial.println("Hello Eelco");
			digitalWrite(RED_LED, HIGH);
			delay(1000);
			digitalWrite(RED_LED, LOW);
		}*/     
		delay(1000);
                Mfrc522.Init();	
	}

	Mfrc522.Halt();	   
}
