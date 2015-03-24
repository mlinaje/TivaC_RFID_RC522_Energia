/*
 * ----------------------------------------------------------------------------
 * Bsed on the example and library of Miguel Balboa
 * availablee at https://github.com/miguelbalboa/rfid
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * Example sketch/program working on Intel Galileo Gen 2 (it should work at least
 * on Arduino UNO without changes).
 * It reads Tags or Cards using a MFRC522 based RFID Reader on the Arduino SPI interface.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs
 * presented (that is: multiple tag reading). So if you stack two or more
 * PICCs on top of each other and present them to the reader, it will first
 * output all details of the first and then the next PICC. Note that this
 * may take some time as all data blocks are dumped, so keep the PICCs at
 * reading distance until complete.
 * 
 * Typical pin layout used:
 * ------------------------------------------------------------
 *             MFRC522      Arduino         Arduino   Arduino
 *             Reader/PCD   Uno or Galileo  Mega      Nano v3
 * Signal      Pin          Pin             Pin       Pin
 * ------------------------------------------------------------
 * RST/Reset   RST          9               5         D9
 * SPI SS      SDA(SS)      10              53        D10
 * SPI MOSI    MOSI         11 / ICSP-4     51        D11
 * SPI MISO    MISO         12 / ICSP-1     50        D12
 * SPI SCK     SCK          13 / ICSP-3     52        D13
 */

// Core library for code-sense
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(ENERGIA) // LaunchPad, FraunchPad and StellarPad specific
#include "Energia.h"
#elif defined(CORE_TEENSY) // Teensy specific
#include "WProgram.h"
#elif defined(ARDUINO) && (ARDUINO >= 100) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#elif defined(ARDUINO) && (ARDUINO < 100) // Arduino 23 specific
#include "WProgram.h"
#else // error
#error Platform not defined
#endif


#include <SPI.h>
#include <MFRC522.h>

MFRC522 mfrc522(PA_3, PB_2);	// Create MFRC522 instance(SS_PIN, RST_PIN)

#define NUM_KNOWN_KEYS   8
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NUM_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
  {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
  {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
  {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
  {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
  {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
  {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};

#define NUM_KNOWN_CARDS  2
byte goodCards[NUM_KNOWN_CARDS][8] = {  {0xEB, 0x08, 0xDC, 0x5E},
  {0xEB, 0x08, 0xDC, 0x5F}  //EB 08 DC 5F = 235822095
};


/*********************************************************************************************/

/*
 * Try using the PICC (the tag/card) with the given key to access block 0.
 * @return true when the given key worked, false otherwise.
 */
boolean try_key(MFRC522::MIFARE_Key *key)
{
    byte buffer[18];
    byte block = 0;
    
    // Serial.println("Authenticating using key A...");
    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid)) == MFRC522::STATUS_OK) {
      byte byteCount = sizeof(buffer);

      // Read block
      if (mfrc522.MIFARE_Read(block, buffer, &byteCount) == MFRC522::STATUS_OK) {
        // Successful read
        mfrc522.PICC_HaltA();       // Halt PICC
        mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
        return true;
      }
    }
    return false;
}

/*
 * Search if the uid (of the tag/card) is in the "list" (array) of validCards
 * @return true when the given key worked, false otherwise.
*/
boolean is_user_auth(){
  for (byte k = 0; k < NUM_KNOWN_CARDS; k++)
    if (memcmp(&goodCards[k][0], &mfrc522.uid.uidByte, mfrc522.uid.size) == 0)
      return true;
  return false;
}
  
/*********************************************************************************************/

void setup() {
  Serial.begin(9600);		// Initialize serial communications with the PC
  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
}

void loop() {
  // Look for new cards
  if ( mfrc522.PICC_IsNewCardPresent())
    if ( mfrc522.PICC_ReadCardSerial()){
      // Try the known default keys
      MFRC522::MIFARE_Key key;

      for (byte k = 0; k < NUM_KNOWN_KEYS; k++) {
        // Copy the known key into the MIFARE_Key structure
        memcpy(&key.keyByte, &knownKeys[k][0], MFRC522::MF_KEY_SIZE);
        // Try the key
        if ( try_key(&key) )
          // Found and reported on the key and block, no need to try other keys for this PICC
          if ( is_user_auth() )
            //The user is authenticated
            Serial.println("You shall pass Gandalf!");
          else
            Serial.println("You should not pass Dani ;-) !");
        else
          break;
      }
    }
}


