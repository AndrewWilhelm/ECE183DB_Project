
/*
  Getting Started example sketch for nRF24L01+ radios
  This is a very basic example of how to send data from one node to another
  Updated: Dec 2014 by TMRh20
*/

#include <Arduino.h>
#include <SPI.h>
#include "RF24.h"
#include <MFRC522.h>
#include <string>
#include <sstream>
#include <iomanip>

#define RST_PIN         D4           // Configurable, see typical pin layout above
#define SS_PIN          D3          // Configurable, see typical pin layout above

#  define LED_PIN D0
#  define BUTTON_PIN D1
#  define LED_ON digitalWrite(LED_PIN, HIGH)
#  define LED_OFF digitalWrite(LED_PIN, LOW)

/****************** User Config ***************************/
int radioNumber = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
// radio(ce_pin, cs_pin)
// apparently the ce pin turns on/off the board whereas the cs_pin is the chip select for the spi
RF24 radio(D8, D2);

const uint8_t message_length = 255;
char message[message_length];
/**********************************************************/

// so technically the first 32 bits need to be the same for multiple reading pipes, but here it looks like they did the opposite
// We think that the arduino may have little/big endian that makes this true (we're assumming the library example did it correctly)
byte addresses[][6] = {"0HanH", "1HanH"};

typedef enum Modes{READ, WRITE, CLEAR};
Modes modes;

bool currentMode = READ;
bool haveWrittenOrReadWhilePressed = false;

const uint8_t data_length = 255;
byte data[data_length];
String data_string;
byte lastBlock[]    = { //00 00 00 00  00 00 FF 07  80 69 FF FF  FF FF FF FF
  0x00, 0x00, 0x00, 0x00, //  1,  2,   3,  4,
  0x00, 0x00, 0xFF, 0x07, //  5,  6,   7,  8,
  0x80, 0x69, 0xFF, 0xFF, //  9, 10, 255, 11,
  0xFF, 0xFF, 0xFF, 0xFF  // 12, 13, 14, 15
};
String lastBlockString = "00 00 00 00  00 00 ff 07  80 69";
byte dataBlock[16];

struct PACKET_struct {
  uint16_t x;
  uint16_t y;
  float a;
  char message;
};
typedef union PACKET{
  PACKET_struct packet;
  byte bytes[sizeof(PACKET_struct)];
};

// a c string that will hold the 
char packet_string[sizeof(PACKET)+1];

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

String PICC_DumpMifareClassicSectorToString(MFRC522::Uid *uid,      ///< Pointer to Uid struct returned from a successful PICC_Select().
    MFRC522::MIFARE_Key *key,  ///< Key A for the sector.
    byte sector     ///< The sector to dump, 0..39.
                                           );
char *float2s(float f, unsigned int digits=2);
template<typename TInputIter>
std::string make_hex_string(TInputIter first, TInputIter last, bool use_uppercase = true, bool insert_spaces = false);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("Receiver is booting up"));
  SPI.begin();
  radio.begin();
  mfrc522.PCD_Init(); // Init MFRC522 card

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  if (radioNumber) {
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1, addresses[0]);
  } else {
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
  }

  //  radio.openReadingPipe(1,addresses[radioNumber]);

  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Start the radio listening for data
  radio.startListening();

  // set the mrfrc522 one less than max gain, 0x0110000
  // tried to do the max gain, 0x07, but that didn't work with battery, probably because it drew too much current at once
  mfrc522.PCD_SetAntennaGain(0x06<<4);

  pinMode(LED_PIN, OUTPUT);
  LED_OFF;
  pinMode(BUTTON_PIN, INPUT);
  Serial.println("Receiver is ready");

}

void loop() {
  //Serial.println("Looping");

  /****************** Pong Back Role ***************************/


  //    unsigned long got_time;


//  if ( radio.available()) {
//    // Variable for the received timestamp
//    //      while (radio.available()) {                                   // While there is data ready
//    //        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
//    //      }
//    while (radio.available()) {                                   // While there is data ready
//      radio.read( &message, message_length * sizeof(char) );             // Get the payload
//    }
//
//    radio.stopListening();                                        // First, stop listening so we can talk
//    //      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.
//    radio.write( &message, message_length * sizeof(char) );              // Send the final one back.
//    radio.startListening();                                       // Now, resume listening so we catch the next packets.
//    Serial.print(F("Sent response "));
//    Serial.println(message);
//    //      Serial.println(got_time);
//    Serial.print("Got the message: ");
//    Serial.println(message);
//    //      Serial.print("And I am node number ");
//    //      Serial.println(radioNumber);
//
//    // check if it's a read or write message
//    // message will either be "r" or "w XXX" where XXX is the data to write
//    if (message[0] == 'r') {
//      // put into read mode
//      Serial.println("Put into read mode");
//      currentMode = READ;
//    } else if (message[0] == 'w') {
//      //put into write mode and set the message to write
//      Serial.println("Put into write mode");
//      data_string = String(message);
//      data_string = data_string.substring(2, data_string.length());// just get the data to write
//      //        data_string.replace(" ", "");
//      //        data_string.replace("\n", "");
//      Serial.print("Will write: ");
//      Serial.println(data_string);
//      currentMode = WRITE;
//    } else if (message[0] == 'c') {
//      //put into clear mode
//      Serial.println("Put into clear mode");
//      currentMode = CLEAR;
//      data_string = "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00";
//    }
//  }

  if ( radio.available()) {
    // Variable for the received timestamp
    //      while (radio.available()) {                                   // While there is data ready
    //        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
    //      }
    PACKET data;
    while (radio.available()) {                                   // While there is data ready
      radio.read( &data, sizeof(data) );             // Get the payload
    }

    radio.startListening();                                       // Now, resume listening so we catch the next packets.
    Serial.print("Got the message: ");
    Serial.println(data.packet.message);
    Serial.print("x: ");
    Serial.println(data.packet.x);
    Serial.print("y: ");
    Serial.println(data.packet.y);
    Serial.print("a: ");
    Serial.println(float2s(data.packet.a));
    Serial.print("message: ");
    Serial.println(data.packet.message);
    //      Serial.print("And I am node number ");
    //      Serial.println(radioNumber);

    // check if it's a read or write message
    // message will either be "r" or "w XXX" where XXX is the data to write
    if (data.packet.message == 'r') {
      // put into read mode
      Serial.println("Put into read mode");
      currentMode = READ;
    } else if (data.packet.message == 'w') {
      //put into write mode and set the message to write
      Serial.println("Put into write mode");
      data_string = String(make_hex_string(std::begin(data.bytes), std::end(data.bytes), true, true).c_str());
      data_string = swapEndiannessForPacket(data_string);
      Serial.print("Will write: ");
      Serial.println(data_string);
      currentMode = WRITE;
    } else if (data.packet.message == 'c') {
      //put into clear mode
      Serial.println("Put into clear mode");
      currentMode = CLEAR;
      data_string = "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00";
    }

  }

  if (digitalRead(BUTTON_PIN) == HIGH) {
    if (haveWrittenOrReadWhilePressed) {
      return;
    }
    //    Serial.println("Button pressed");

    byte atqa_answer[2];
    byte atqa_size = 2;
    MFRC522::StatusCode result = mfrc522.PICC_WakeupA(atqa_answer, &atqa_size);
    if ( ! (result == MFRC522::STATUS_OK || result == MFRC522::STATUS_COLLISION))
      return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
      return;

    // Show some details of the PICC (that is: the tag/card)
          Serial.print(F("Card UID:"));
          dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    //      Serial.println();
    //      Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    //      Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
            &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("This sample only works with MIFARE Classic cards."));
      return;
    }

    // In this sample we use the second sector,
    // that is: sector #1, covering block #4 up to and including block #7
    byte sector         = 2;
    byte blockAddr      = 8;
    byte trailerBlock   = 11;
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }

    if (currentMode == READ) { //------------------------------------ READ MODE -------------------------------------------

      // Show the whole sector as it currently is
      Serial.println(F("Current data in sector:"));
      //mfrc522.PICC_DumpMifareClassicSectorToBuffer(&(mfrc522.uid), &key, sector);
      String temp = PICC_DumpMifareClassicSectorToString(&(mfrc522.uid), &key, sector);
//      Serial.println(temp);

      // Halt PICC
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();
      temp.trim();
      if(temp == "") { // if it's an empty string it failed
        return;
      } else if(temp == lastBlockString) {
        return;
      }
      
      int length_of_message = temp.length();
      temp = temp.substring(temp.lastIndexOf('\n', length_of_message-2)+1, length_of_message); // get the last block of the string(which is the first block in memory) (length - 2 to avoid the ending newline)
      temp.trim();
//      temp.replace("  ", " "); //get rid of extra spaces
//      temp = "% " + temp; //preappend the % for processing purposes at the transmitter
//      temp.getBytes(data, message_length);
      temp.replace(" ","");
      PACKET firstBlockData = stringToBlock(temp);
      Serial.print("x: ");
      Serial.println(firstBlockData.packet.x);
      Serial.print("y: ");
      Serial.println(firstBlockData.packet.y);
      Serial.print("a: ");
      Serial.println(float2s(firstBlockData.packet.a));
      Serial.println(firstBlockData.packet.message);
      
      radio.stopListening();                                        // First, stop listening so we can talk
//      if (!radio.write( &data, sizeof(char) * temp.length() )) {
//        Serial.println("Couldn't send message:");
//        Serial.print(temp);
//        Serial.println(" Is the receiver on?");
//      } else {
//        Serial.println(F("Sent: "));
//        Serial.println(temp);
//      }
      if (!radio.write( &firstBlockData, sizeof(firstBlockData) )) {
        Serial.println("Couldn't send the data:");
        Serial.println(" Is the receiver on?");
      } else {
        Serial.println(F("Sent: "));
        Serial.println(temp);
      }
      radio.startListening();                                       // Now, resume listening so we catch the next packets.
//      Serial.print(F("Sent response "));
//      Serial.println((char*)data);
    } else if(currentMode == WRITE || currentMode == CLEAR){

      //------------------------------------------------------- WRITE MODE ------------------------------------------------------------


      // Authenticate using key B
      Serial.println(F("Authenticating again using key B..."));
      status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
      }

      memset(data, 0, sizeof(data)); //clear the previous data, just in case
      data_string.toCharArray((char*)data, data_length);
      char* end_ptr = (char*)data;
      //      const char* end_ptr = data_string.c_str();
      //      int numOfNibbles = data_string.replace(" ", "");.replace("\n", "").length();
      for (byte j = (sector * 4); j < (sector * 4) + 3; j++) { //for each block (except for the trailer block)
        // Prep the data to write
        memset(buffer, 0, sizeof(buffer)); //clear the buffer

        for (int k = 0; k < sizeof(lastBlock); k++) { // for each nibble
          buffer[k] = (byte)strtol(end_ptr, &end_ptr, 16);
        }
        //        Serial.print("Trying to write the buffer: ");
        //        Serial.println(String(buffer));

        // Write data to the block
        //      Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
        //      Serial.println(F(" ..."));
        //      dump_byte_array(dataBlock, 16); Serial.println();
        status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(j, buffer, 16);
        if (status != MFRC522::STATUS_OK) {
          Serial.print(F("MIFARE_Write() failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
        }
      }


      //      Serial.println();

      //      // Read data from the block (again, should now be what we have written)
      //      Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
      //      Serial.println(F(" ..."));
      //      status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
      //      if (status != MFRC522::STATUS_OK) {
      //          Serial.print(F("MIFARE_Read() failed: "));
      //          Serial.println(mfrc522.GetStatusCodeName(status));
      //      }
      //      Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
      //      dump_byte_array(buffer, 16); Serial.println();
      //
      //      // Check that data in block is what we have written
      //      // by counting the number of bytes that are equal
      //      Serial.println(F("Checking result..."));
      //      byte count = 0;
      //      for (byte i = 0; i < 16; i++) {
      //          // Compare buffer (= what we've read) with dataBlock (= what we've written)
      //          if (buffer[i] == dataBlock[i])
      //              count++;
      //      }
      //      Serial.print(F("Number of bytes that match = ")); Serial.println(count);
      //      if (count == 16) {
      //          Serial.println(F("Success :-)"));
      //      } else {
      //          Serial.println(F("Failure, no match :-("));
      //          Serial.println(F("  perhaps the write didn't work properly..."));
      //      }
      //      Serial.println();

      // Dump the sector data
      Serial.println(F("Current data in sector:"));
      mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
      Serial.println();

      // Halt PICC
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();
    }
    if(currentMode != CLEAR) { // for clear we want it to keep clearing as long as button is pressed
      haveWrittenOrReadWhilePressed = true;
      LED_ON;
    }
  } else {
    LED_OFF;
    haveWrittenOrReadWhilePressed = false;
  }


  /****************** Change Roles via Serial Commands ***************************/

  //  if ( Serial.available() )
  //  {
  //    char c = toupper(Serial.read());
  //    if ( c == 'T' && role == 0 ){
  //      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
  //      role = 1;                  // Become the primary transmitter (ping out)
  //
  //   }else
  //    if ( c == 'R' && role == 1 ){
  //      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
  //       role = 0;                // Become the primary receiver (pong back)
  //       radio.startListening();
  //
  //    }
  //  }

} // Loop

/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// Function to copy 'len' elements from 'src' to 'dst'
void copy(byte* src, byte* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}

/**
   Dumps memory contents of a sector of a MIFARE Classic PICC.
   Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
   Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the sector trailer access bits.
   Returns an empty string if it fails
*/
String PICC_DumpMifareClassicSectorToString(MFRC522::Uid *uid,      ///< Pointer to Uid struct returned from a successful PICC_Select().
    MFRC522::MIFARE_Key *key,  ///< Key A for the sector.
    byte sector     ///< The sector to dump, 0..39.
                                           ) {
  String toReturn = "";
  MFRC522::StatusCode status;
  byte firstBlock;    // Address of lowest address to dump actually last block dumped)
  byte no_of_blocks;    // Number of blocks in sector
  bool isSectorTrailer; // Set to true while handling the "last" (ie highest address) in the sector.

  // The access bits are stored in a peculiar fashion.
  // There are four groups:
  //    g[3]  Access bits for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
  //    g[2]  Access bits for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
  //    g[1]  Access bits for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
  //    g[0]  Access bits for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
  // Each group has access bits [C1 C2 C3]. In this code C1 is MSB and C3 is LSB.
  // The four CX bits are stored together in a nible cx and an inverted nible cx_.
  byte c1, c2, c3;    // Nibbles
  byte c1_, c2_, c3_;   // Inverted nibbles
  bool invertedError;   // True if one of the inverted nibbles did not match
  byte g[4];        // Access bits for each of the four groups.
  byte group;       // 0-3 - active group for access bits
  bool firstInGroup;    // True for the first block dumped in the group

  // Determine position and size of sector.
  if (sector < 32) { // Sectors 0..31 has 4 blocks each
    no_of_blocks = 4;
    firstBlock = sector * no_of_blocks;
  }
  else if (sector < 40) { // Sectors 32-39 has 16 blocks each
    no_of_blocks = 16;
    firstBlock = 128 + (sector - 32) * no_of_blocks;
  }
  else { // Illegal input, no MIFARE Classic PICC has more than 40 sectors.
    return "";
  }

  // Dump blocks, highest address first.
  byte byteCount;
  byte buffer[18];
  byte blockAddr;
  isSectorTrailer = true;
  invertedError = false;  // Avoid "unused variable" warning.
  for (int8_t blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--) {
    blockAddr = firstBlock + blockOffset;
    // Sector number - only on first line
    //    if (isSectorTrailer) {
    ////      if(sector < 10)
    ////        Serial.print(F("   ")); // Pad with spaces
    ////      else
    ////        Serial.print(F("  ")); // Pad with spaces
    ////      Serial.print(sector);
    ////      Serial.print(F("   "));
    //    }
    //    else {
    //      Serial.print(F("       "));
    //    }
    // Block number
    //    if(blockAddr < 10)
    //      Serial.print(F("   ")); // Pad with spaces
    //    else {
    //      if(blockAddr < 100)
    //        Serial.print(F("  ")); // Pad with spaces
    //      else
    //        Serial.print(F(" ")); // Pad with spaces
    //    }
    //    Serial.print(blockAddr);
    //    Serial.print(F("  "));
    // Establish encrypted communications before reading the first block
    if (isSectorTrailer) {
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, firstBlock, key, uid);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(MFRC522::GetStatusCodeName(status));
        return "";
      }
    }
    // Read block
    byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(MFRC522::GetStatusCodeName(status));
      return "";
    }
    // Dump data
    for (byte index = 0; index < 16; index++) {
      if (buffer[index] < 0x10)
        toReturn += " 0";
      //        Serial.print(F(" 0"));
      else
        toReturn += " ";
      //        Serial.print(F(" "));
      //      Serial.print(buffer[index], HEX);
      toReturn += String(buffer[index], HEX);
      if ((index % 4) == 3) {
        //        Serial.print(F(" "));
        toReturn += " ";
      }
    }
    // Parse sector trailer data
    if (isSectorTrailer) {
      c1  = buffer[7] >> 4;
      c2  = buffer[8] & 0xF;
      c3  = buffer[8] >> 4;
      c1_ = buffer[6] & 0xF;
      c2_ = buffer[6] >> 4;
      c3_ = buffer[7] & 0xF;
      invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
      g[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
      g[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
      g[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
      g[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
      isSectorTrailer = false;
    }

    // Which access group is this block in?
    if (no_of_blocks == 4) {
      group = blockOffset;
      firstInGroup = true;
    }
    else {
      group = blockOffset / 5;
      firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
    }

    if (firstInGroup) {
      // Print access bits
      //      Serial.print(F(" [ "));
      //      Serial.print((g[group] >> 2) & 1, DEC); Serial.print(F(" "));
      //      Serial.print((g[group] >> 1) & 1, DEC); Serial.print(F(" "));
      //      Serial.print((g[group] >> 0) & 1, DEC);
      //      Serial.print(F(" ] "));
      if (invertedError) {
        Serial.print(F(" Inverted access bits did not match! "));
      }
    }

    if (group != 3 && (g[group] == 1 || g[group] == 6)) { // Not a sector trailer, a value block
      int32_t value = (int32_t(buffer[3]) << 24) | (int32_t(buffer[2]) << 16) | (int32_t(buffer[1]) << 8) | int32_t(buffer[0]);
      //      Serial.print(F(" Value=0x")); Serial.print(value, HEX);
      //      Serial.print(F(" Adr=0x")); Serial.print(buffer[12], HEX);
    }
    //    Serial.println();
    toReturn += '\n';
  }

  return toReturn;
} // End PICC_DumpMifareClassicSectorToSerial()

PACKET stringToBlock(String block) {
  PACKET toReturn;
  toReturn.packet.message = 'r';
  // first separate each part into smaller substrings
  int index = 0;
  String x = block.substring(0, sizeof(toReturn.packet.x) * 2); // multiply by 2 since two hex chars in one byte
  index += sizeof(toReturn.packet.x) * 2;
  String y = block.substring(index, index + (sizeof(toReturn.packet.y)* 2));
  index += sizeof(toReturn.packet.y) * 2;
  String a = block.substring(index, index + (sizeof(toReturn.packet.a)* 2));

//  Serial.print("String for a is: ");
//  Serial.println(a);

  // now convert each substring into it's corresponding data value
  char buffer[16];
  x.getBytes((byte*)buffer, sizeof(buffer));
  unsigned long long_x = strtoul( buffer, nullptr, 16); //base=16 since it's hexadecimal
  toReturn.packet.x = (uint16_t) long_x;
//  Serial.print("Value of x in datablock struct is: ");
//  Serial.println(toReturn.x);
  y.getBytes((byte*)buffer, sizeof(buffer));
  unsigned long long_y = strtoul( buffer, nullptr, 16); //base=16 since it's hexadecimal
  toReturn.packet.y = (uint16_t) long_y;
  // for a, we need to get the bytes into memory via strtoul, than convert those bytes to a float
  // couldn't use strtof since it doesn't let you parse the raw bytes
  // note that sizeof(long)=sizeof(float)=4 bytes
  a.getBytes((byte*)buffer, sizeof(buffer));
  unsigned long long_a = strtoul( buffer, nullptr, 16); //base=16 since it's hexadecimal
  memcpy(&toReturn.packet.a, &long_a, sizeof(float));
  
//  Serial.print("Value of a in datablock struct is: ");
//  Serial.println(float2s(toReturn.a));

  return toReturn;
}

char *float2s(float f, unsigned int digits /*=2*/)
{
   static char buf[16]; // Buffer to build string representation
   int index = 0;       // Position in buf to copy stuff to

   // For debugging: Uncomment the following line to see what the
   // function is working on.
   //Serial.print("In float2s: bytes of f are: ");printBytes(f);

   // Handle the sign here:
   if (f < 0.0) {
       buf[index++] = '-'; 
       f = -f;
   }
   // From here on, it's magnitude

   // Handle infinities 
   if (isinf(f)) {
       strcpy(buf+index, "INF");
       return buf;
   }
   
   // Handle NaNs
   if (isnan(f)) {
       strcpy(buf+index, "NAN");
       return buf;
   }
   
   //
   // Handle numbers.
   //
   
   // Six or seven significant decimal digits will have no more than
   // six digits after the decimal point.
   //
   if (digits > 6) {
       digits = 6;
   }
   
   // "Normalize" into integer part and fractional part
   int exponent = 0;
   if (f >= 10) {
       while (f >= 10) {
           f /= 10;
           ++exponent;
       }
   }
   else if ((f > 0) && (f < 1)) {
       while (f < 1) {
           f *= 10;
           --exponent;
       }
   }

   //
   // Now add 0.5 in to the least significant digit that will
   // be printed.

   //float rounder = 0.5/pow(10, digits);
   // Use special power-of-integer function instead of the
   // floating point library function.
   float rounder = 0.5 / ipow10(digits);
   f += rounder;

   //
   // Get the whole number part and the fractional part into integer
   // data variables.
   //
   unsigned intpart = (unsigned)f;
   unsigned long fracpart  = (unsigned long)((f - intpart) * 1.0e7);

   //
   // Divide by a power of 10 that zeros out the lower digits
   // so that the "%0.lu" format will give exactly the required number
   // of digits.
   //
   fracpart /= ipow10(6-digits+1);

   //
   // Create the format string and use it with sprintf to form
   // the print string.
   //
   char format[16];
   // If digits > 0, print
   //    int part decimal point fraction part and exponent.

   if (digits) {
     
       sprintf(format, "%%u.%%0%dlu E%%+d", digits);
       //
       // To make sure the format is what it is supposed to be, uncomment
       // the following line.
       //Serial.print("format: ");Serial.println(format);
       sprintf(buf+index, format, intpart, fracpart, exponent);
   }
   else { // digits == 0; just print the intpart and the exponent
       sprintf(format, "%%u E%%+d");
       sprintf(buf+index, format, intpart, exponent);
   }

   return buf;
} 
//
// Handy function to print hex values
// of the bytes of a float.  Sometimes
// helps you see why things don't
// get rounded to the values that you
// might think they should.
//
// You can print the actual byte values
// and compare with the floating point
// representation that is shown in a a
// reference like
//    [urlhttp://en.wikipedia.org/wiki/Floating_point[/url]
//
void printBytes(float f)
{
   unsigned char *chpt = (unsigned char *)&f;
   char buffer[5]; // Big enough to hold each printed byte 0x..\0
   //
   // It's little-endian: Get bytes from memory in reverse order
   // so that they show in "register order."
   //
   for (int i = sizeof(f)-1; i >= 0; i--) {
       sprintf(buffer, "%02x ", (unsigned char)chpt[i]);
       Serial.print(buffer);
   }
   Serial.println();
}

//
// Raise 10 to an unsigned integer power,
// It's used in this program for powers
// up to 6, so it must have a long return
// type, since in avr-gcc, an int can't hold
// 10 to the power 6.
//
// Since it's an integer function, negative
// powers wouldn't make much sense.
//
// If you want a more general function for raising
// an integer to an integer power, you could make 
// "base" a parameter.
unsigned long ipow10(unsigned power)
{
   const unsigned base = 10;
   unsigned long retval = 1;

   for (int i = 0; i < power; i++) {
       retval *= base;
   }
   return retval;
}

template<typename TInputIter>
std::string make_hex_string(TInputIter first, TInputIter last, bool use_uppercase /*= true*/, bool insert_spaces /*= false*/)
{
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    if (use_uppercase)
        ss << std::uppercase;
    while (first != last)
    {
        ss << std::setw(2) << static_cast<int>(*first++);
        if (insert_spaces && first != last)
            ss << " ";
    }
    return ss.str();
}

// changes the endianness for the data in this string, based upon the packet struct
// assumes string is in "XX XX XX ..." format
// if the packet changes, need to edit the code below
String swapEndiannessForPacket(String s){
  Serial.print("Swapping Endianness for: ");
  Serial.println(s);
  int index = 0; // points to first char of that element
  int num_of_bytes[] = {2, 2, 4, 1};
  for(int j = 0; j < sizeof(num_of_bytes)/sizeof(int); j++){
    for(int k = 0; k < num_of_bytes[j]/2; k++){
      String temp = s.substring(index + (k*3), index +(k*3)+2); // the first byte of the swap
      Serial.print("temp: ");
      Serial.println(temp);
      s[index + (k*3)] = s[index + ((num_of_bytes[j]-1-k)*3)]; // swap the two chars over
      s[index + (k*3) + 1] = s[index + ((num_of_bytes[j]-1-k)*3)+1];
      s[index + ((num_of_bytes[j]-1-k)*3)] = temp[0];
      s[index + ((num_of_bytes[j]-1-k)*3)+1] = temp[1];
    }
    index += num_of_bytes[j] * 3;
    Serial.print("Byte num: ");
    Serial.println(j);
    Serial.print("String looks like: ");
    Serial.println(s);
  }
  return s;
}
