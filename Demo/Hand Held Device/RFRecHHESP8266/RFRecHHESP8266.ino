
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
// Choose the sector and block to read from
byte sector         = 2;
byte blockAddr      = 8;
byte trailerBlock   = 11;
/**********************************************************/

// so technically the first 32 bits need to be the same for multiple reading pipes, but here it looks like they did the opposite
// We think that the arduino may have little/big endian that makes this true (we're assumming the library example did it correctly)
byte addresses[][6] = {"0HanH", "1HanH"};

typedef enum Modes {READ, WRITE, CLEAR, ERASE};
//Modes modes;

Modes currentMode = READ;
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
//byte dataBlock[16];

struct NODE {
  uint8_t x = 0;
  uint8_t y = 0;
  bool isLeaf = false;
  bool doesExist = false;
};

struct DATABLOCK {
  // data about the tag itself
  uint8_t x = 0;
  uint8_t y = 0;
  NODE otherNodes[4];
};

struct PACKET {
  char message;
  DATABLOCK dataBlock;
};

typedef union PACKETUNION {
  PACKET packet = PACKET();
  byte bytes[sizeof(PACKET)] ;
};

// a c string that will hold the
char packet_string[sizeof(PACKET) + 1];

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

// variables for the led-sequences
bool isFlashing = false;
int num_of_flashes = 0;
int max_num_of_flashes = 1;
unsigned long time_per_flash_cycle = 0;
unsigned long start_time = 0;

String PICC_DumpMifareClassicSectorToString(MFRC522::Uid *uid,      ///< Pointer to Uid struct returned from a successful PICC_Select().
    MFRC522::MIFARE_Key *key,  ///< Key A for the sector.
    byte sector     ///< The sector to dump, 0..39.
                                           );
char *float2s(float f, unsigned int digits = 2);
template<typename TInputIter>
std::string make_hex_string(TInputIter first, TInputIter last, bool use_uppercase = true, bool insert_spaces = false);
void startFlashSequence(int numOfFlashes, unsigned long lengthOfFlashCycle);
void updateFlashSequence();

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
  mfrc522.PCD_SetAntennaGain(0x06 << 4);

  pinMode(LED_PIN, OUTPUT);
  LED_OFF;
  pinMode(BUTTON_PIN, INPUT);
  Serial.println("Receiver is ready");
  startFlashSequence(2, 500);

}

void loop() {

  updateFlashSequence();

  if ( radio.available()) {

    PACKETUNION packetUnion;
    while (radio.available()) {                                   // While there is data ready
      radio.read( &packetUnion, sizeof(packetUnion) );             // Get the payload
    }

    PACKET packet = packetUnion.packet;
    DATABLOCK data = packet.dataBlock;

    radio.startListening();                                       // Now, resume listening so we catch the next packets.
    Serial.print("Got the message: ");
    Serial.println(packet.message);
    //      Serial.print("And I am node number ");
    //      Serial.println(radioNumber);

    // check if it's a read or write message
    // message will either be "r" or "w XXX" where XXX is the data to write
    if (packet.message == 'r') {
      // put into read mode
      Serial.println("Put into read mode");
      currentMode = READ;
      startFlashSequence(2, 500);
    } else if (packet.message == 'w') {
      //put into write mode and set the message to write
      Serial.println("Put into write mode");
      printDataBlock(data);
      data_string = String(make_hex_string(std::begin(packetUnion.bytes), std::end(packetUnion.bytes), true, true).c_str());
      data_string = swapEndiannessForPacket(data_string);
      Serial.print("Will write: ");
      Serial.println(data_string);
      currentMode = WRITE;
      startFlashSequence(4, 200);
    } else if (packet.message == 'c') {
      //put into clear mode
      Serial.println("Put into clear mode");
      currentMode = CLEAR;
      //data_string = "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00";
      // make it so that the message is zero so that it completely clears the tag
      packetUnion.packet.message = 0;
      data_string = String(make_hex_string(std::begin(packetUnion.bytes), std::end(packetUnion.bytes), true, true).c_str());
      data_string = swapEndiannessForPacket(data_string);
      startFlashSequence(7, 100);
    } else if (packet.message == 'e') {
      // put into read mode
      Serial.println("Put into erase mode");
      currentMode = ERASE;
      startFlashSequence(1, 1500);
    } else {
      Serial.print("Tried to put into an unknown mode");
      Serial.println(packet.message);
    }
  }

  if (digitalRead(BUTTON_PIN) == HIGH) {
    if (haveWrittenOrReadWhilePressed) {
      return;
    }
    //    Serial.println("Button pressed");

    if (currentMode == READ) { //------------------------------------ READ MODE -------------------------------------------

      if (!RFIDTagIsPresent()) {
        return;
      }

      String temp = readRFIDTag();
      if (temp == "") { // if it's an empty string it failed
        return;
      } else if (temp == lastBlockString) {
        return;
      }

      temp.replace(" ", "");
      PACKET firstBlockData;
      blockToPacket(temp, &firstBlockData);
      //      Serial.print("x: ");
      printDataBlock(firstBlockData.dataBlock);

      radio.stopListening();                                        // First, stop listening so we can talk

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
    } else if (currentMode == WRITE || currentMode == CLEAR) {
      Serial.println("Starting to write/clear");
      Serial.println(currentMode);
      //------------------------------------------------------- WRITE MODE ------------------------------------------------------------

      if (!RFIDTagIsPresent()) {
        return;
      }

      if (!writeRFIDTag(data_string)) { //had problems writing to it
        return;
      }

    } else if (currentMode == ERASE) {
      //------------------------------------------------------- ERASE MODE ------------------------------------------------------------

      Serial.println("Starting to erase");
      // first read the new tag
      if (!RFIDTagIsPresent()) {
        return;
      }
      Serial.println("A tag is present");
      String temp = readRFIDTag();
      if (temp == "") { // if it's an empty string it failed
        Serial.println("The reading failed for erase");
        return;
      } else if (temp == lastBlockString) {
        Serial.println("The last block string was detected");
        return;
      }
      Serial.println("Read succesfully");

      temp.replace(" ", "");
      PACKET firstBlockData;
      blockToPacket(temp, &firstBlockData);
      //      Serial.print("x: ");
      Serial.print("Found this data beforehand");
      printDataBlock(firstBlockData.dataBlock);

      // prep the string with the data
      PACKETUNION pu; // create blank packet union
      pu.packet.message = firstBlockData.message;
      pu.packet.dataBlock.x = firstBlockData.dataBlock.x;
      pu.packet.dataBlock.y = firstBlockData.dataBlock.y;
      Serial.println("About to write the following:");
      printDataBlock(pu.packet.dataBlock);
      String justXAndY = String(make_hex_string(std::begin(pu.bytes), std::end(pu.bytes), true, true).c_str());
      justXAndY = swapEndiannessForPacket(justXAndY);

      // now write to the tag
      if (!RFIDTagIsPresent()) {
        Serial.println("Couldn't find tag when trying to write (during erase mode)");
        return;
      }
      if (!writeRFIDTag(justXAndY)) { //had problems writing to it
        Serial.println("Failed to write the string while trying to erase");
        return;
      }
      startFlashSequence(4, 200);

    } else {
      Serial.println("### Unknown mode ###");
    }

    if (currentMode != CLEAR && currentMode != ERASE) { // for clear we want it to keep clearing as long as button is pressed
      haveWrittenOrReadWhilePressed = true;
      LED_ON;
    }
  } else {
    LED_OFF;
    haveWrittenOrReadWhilePressed = false;
  }

} // Loop


// Signals if an RFID tag is present. Note: if you want to see if just a new tag is present, use mfrc522.PICC_IsNewCardPresent()
bool RFIDTagIsPresent() {
  byte atqa_answer[2];
  byte atqa_size = 2;
  MFRC522::StatusCode result = mfrc522.PICC_WakeupA(atqa_answer, &atqa_size);
  if ( ! (result == MFRC522::STATUS_OK || result == MFRC522::STATUS_COLLISION)) {
    //    Serial.println("Status was not okay and there were no collisions");
    return false;
  }
  return true;
} // end isRFIDTag()


// used to make sure that we can select the tag to read/write to it
bool validateTag() {
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    //    Serial.println("Couldn't select a card");
    return false;
  }

  // Show some details of the PICC (that is: the tag/card)
  //      Serial.print(F("Card UID:"));
  //      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  //      Serial.println();
  //      Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  //      Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Check for compatibility
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return false;
  }
  return true;
} //end validateTag()

// returns the data of the RFID tag if it worked, otherwise it returns an empty string
// call RFIDTagIsPresent() or mfrc522.PICC_IsNewCardPresent() before hand to make sure there is a tag present to read/write to
String readRFIDTag() {

  if ( !validateTag() ) {
    //      Serial.println("Can't read or write to the tag");
    return "";
  }

  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return "";
  }

  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  //mfrc522.PICC_DumpMifareClassicSectorToBuffer(&(mfrc522.uid), &key, sector);
  String temp = PICC_DumpMifareClassicSectorToString(&(mfrc522.uid), &key, sector);
  Serial.println("temp");
  Serial.println(temp);

  // make it so that instead of 4 separate lines, we have just one long line of strings
  // we also need to put it in order though as if we get rid of \n, it's backwards
  String one_line = "";
  int string_index = 0;
  for (int j = 0; j < 4; j++) {
    one_line = temp.substring(string_index, temp.indexOf('\n', string_index)) + one_line;
    string_index = temp.indexOf('\n', string_index) + 2;
  }
  temp = one_line;

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();


  int length_of_message = temp.length();
  temp.trim();
  temp.replace("  ", " ");
  //  temp.getBytes(data, message_length);

  return temp;


} // end readRFIDTag()

// returns true if it was able to write to the tag, false otherwise
// call RFIDTagIsPresent() or mfrc522.PICC_IsNewCardPresent() before hand to make sure there is a tag present to read/write to
bool writeRFIDTag(String toWrite) {
  byte toWrite_bytes[data_length];

  if ( !validateTag() ) {
    //      Serial.println("Can't read or write to the tag");
    return false;
  }

  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  // Authenticate using key B
  Serial.println(F("Authenticating again using key B..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }

  memset(toWrite_bytes, 0, sizeof(toWrite_bytes)); //clear the previous data, just in case
  toWrite.toCharArray((char*)toWrite_bytes, data_length);
  char* end_ptr = (char*)toWrite_bytes;
  //      const char* end_ptr = data_string.c_str();
  //      int numOfNibbles = data_string.replace(" ", "");.replace("\n", "").length();
  for (byte j = (sector * 4); j < (sector * 4) + 3; j++) { //for each block (except for the trailer block)
    // Prep the data to write
    memset(buffer, 0, sizeof(buffer)); //clear the buffer

    for (int k = 0; k < sizeof(lastBlock); k++) { // for each nibble
      buffer[k] = (byte)strtol(end_ptr, &end_ptr, 16);
    }

    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(j, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
  }

  // Dump the sector data
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  return true;
} //writeRFIDTag()


/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void dump_byte_array(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// Function to copy 'len' elements from 'src' to 'dst'
void copy(byte * src, byte * dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}

/**
   Dumps memory contents of a sector of a MIFARE Classic PICC.
   Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
   Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the sector trailer access bits.
   Returns an empty string if it fails
*/
String PICC_DumpMifareClassicSectorToString(MFRC522::Uid * uid,     ///< Pointer to Uid struct returned from a successful PICC_Select().
    MFRC522::MIFARE_Key * key, ///< Key A for the sector.
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

// converts string of bytes (with no spaces) to a packet
void blockToPacket(String block, PACKET * p) {
  DATABLOCK* toReturn = &((*p).dataBlock);
  (*p).message = 'r';
  // first separate each part into smaller substrings
  int index = sizeof((*p).message) * 2; //skip the char that was written to the tag
  String x = block.substring(index, index + (sizeof((*toReturn).x) * 2)); // multiply by 2 since two hex chars in one byte
  index += sizeof((*toReturn).x) * 2;
  String y = block.substring(index, index + (sizeof((*toReturn).y) * 2));
  index += sizeof((*toReturn).y) * 2;
  String nodeData[4 * 4]; // 4*4 since 4 nodes with 4 bytes per node
  for (int j = 0; j < 4; j++) {
    nodeData[(4 * j) + 0] = block.substring(index, index + (sizeof((*toReturn).otherNodes[j].x) * 2));
    index += sizeof((*toReturn).otherNodes[j].x) * 2;
    nodeData[(4 * j) + 1] = block.substring(index, index + (sizeof((*toReturn).otherNodes[j].y) * 2));
    index += sizeof((*toReturn).otherNodes[j].y) * 2;
    nodeData[(4 * j) + 2] = block.substring(index, index + (sizeof((*toReturn).otherNodes[j].isLeaf) * 2));
    index += sizeof((*toReturn).otherNodes[j].isLeaf) * 2;
    nodeData[(4 * j) + 3] = block.substring(index, index + (sizeof((*toReturn).otherNodes[j].doesExist) * 2));
    index += sizeof((*toReturn).otherNodes[j].doesExist) * 2;
  }

  //  for(int k=0; k < 16; k++){
  //    if(k %4 == 1){
  //      Serial.println("Node");
  //    }
  //    Serial.println(nodeData[k]);
  //  }

  //  Serial.print("String for a is: ");
  //  Serial.println(a);

  // now convert each substring into it's corresponding data value
  char buffer[16];
  x.getBytes((byte*)buffer, sizeof(buffer));
  unsigned long long_x = strtoul( buffer, nullptr, 16); //base=16 since it's hexadecimal
  (*toReturn).x = (uint8_t) long_x;
  //  Serial.print("Value of x in datablock struct is: ");
  //  Serial.println((*toReturn).x);
  y.getBytes((byte*)buffer, sizeof(buffer));
  unsigned long long_y = strtoul( buffer, nullptr, 16); //base=16 since it's hexadecimal
  (*toReturn).y = (uint8_t) long_y;

  for (int j = 0; j < 4; j++) {
    nodeData[(4 * j) + 0].getBytes((byte*)buffer, sizeof(buffer));
    unsigned long long_x = strtoul( buffer, nullptr, 16); //base=16 since it's hexadecimal
    (*toReturn).otherNodes[j].x = (uint8_t) long_x;
    nodeData[(4 * j) + 1].getBytes((byte*)buffer, sizeof(buffer));
    unsigned long long_y = strtoul( buffer, nullptr, 16); //base=16 since it's hexadecimal
    (*toReturn).otherNodes[j].y = (uint8_t) long_y;
    if (nodeData[(4 * j) + 2] != "00") {
      (*toReturn).otherNodes[j].isLeaf = true;
    } else {
      (*toReturn).otherNodes[j].isLeaf = false;
    }
    if (nodeData[(4 * j) + 3] != "00") {
      (*toReturn).otherNodes[j].doesExist = true;
    } else {
      (*toReturn).otherNodes[j].doesExist = false;
    }
  }

} // end blockToPacket

char *float2s(float f, unsigned int digits /*=2*/)
{
  static char buf[16]; // Buffer to build string representation
  int index = 0;       // Position in buf to copy stuff to

  // Handle the sign here:
  if (f < 0.0) {
    buf[index++] = '-';
    f = -f;
  }
  // From here on, it's magnitude

  // Handle infinities
  if (isinf(f)) {
    strcpy(buf + index, "INF");
    return buf;
  }

  // Handle NaNs
  if (isnan(f)) {
    strcpy(buf + index, "NAN");
    return buf;
  }

  // Handle numbers.

  // Six or seven significant decimal digits will have no more than
  // six digits after the decimal point.
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
  fracpart /= ipow10(6 - digits + 1);

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
    sprintf(buf + index, format, intpart, fracpart, exponent);
  }
  else { // digits == 0; just print the intpart and the exponent
    sprintf(format, "%%u E%%+d");
    sprintf(buf + index, format, intpart, exponent);
  }

  return buf;
}

// Raise 10 to an unsigned integer power
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
String swapEndiannessForPacket(String s) {
  Serial.print("Swapping Endianness for: ");
  Serial.println(s);
  int index = 0; // points to first char of that element
  int num_of_bytes[] = {1, 1, 1};
  int num_of_bytes_per_node[] = {1, 1, 1, 1};
  // first do the message, and then the x, and y of the node
  for (int j = 0; j < sizeof(num_of_bytes) / sizeof(int); j++) {
    for (int k = 0; k < num_of_bytes[j] / 2; k++) {
      String temp = s.substring(index + (k * 3), index + (k * 3) + 2); // the first byte of the swap
      Serial.print("temp: ");
      Serial.println(temp);
      s[index + (k * 3)] = s[index + ((num_of_bytes[j] - 1 - k) * 3)]; // swap the two chars over
      s[index + (k * 3) + 1] = s[index + ((num_of_bytes[j] - 1 - k) * 3) + 1];
      s[index + ((num_of_bytes[j] - 1 - k) * 3)] = temp[0];
      s[index + ((num_of_bytes[j] - 1 - k) * 3) + 1] = temp[1];
    }
    index += num_of_bytes[j] * 3;
    Serial.print("Byte num: ");
    Serial.println(j);
    Serial.print("String looks like: ");
    Serial.println(s);
  }
  // then do all of the nodes
  Serial.println("Now working on the nodes");
  for (int node = 0; node < 4; node++) {
    for (int j = 0; j < sizeof(num_of_bytes) / sizeof(int); j++) {
      for (int k = 0; k < num_of_bytes[j] / 2; k++) {
        String temp = s.substring(index + (k * 3), index + (k * 3) + 2); // the first byte of the swap
        Serial.print("temp: ");
        Serial.println(temp);
        s[index + (k * 3)] = s[index + ((num_of_bytes[j] - 1 - k) * 3)]; // swap the two chars over
        s[index + (k * 3) + 1] = s[index + ((num_of_bytes[j] - 1 - k) * 3) + 1];
        s[index + ((num_of_bytes[j] - 1 - k) * 3)] = temp[0];
        s[index + ((num_of_bytes[j] - 1 - k) * 3) + 1] = temp[1];
      }
      index += num_of_bytes[j] * 3;
      Serial.print("Byte num: ");
      Serial.println(j);
      Serial.print("String looks like: ");
      Serial.println(s);
    }
  }
  return s;
}

void updateFlashSequence() {
  if (isFlashing) {
    unsigned long elapsedTime = millis() - start_time; // elapsed time for this cycle
    if (elapsedTime < time_per_flash_cycle / 2) { // first half of cycle were on
      LED_ON;
    } else if (elapsedTime < time_per_flash_cycle) {
      LED_OFF;
    } else {
      num_of_flashes += 1;
      start_time = millis();
      if (num_of_flashes >= max_num_of_flashes) {
        isFlashing = false;
      }
    }
  }
}

void startFlashSequence(int numOfFlashes, unsigned long lengthOfFlashCycle) {
  isFlashing = true;
  num_of_flashes = 0;
  max_num_of_flashes = numOfFlashes;
  time_per_flash_cycle = lengthOfFlashCycle;
  start_time = millis();
  updateFlashSequence();
}

void printDataBlock(DATABLOCK & d) {
  Serial.print("x: ");
  Serial.println(d.x);
  Serial.print("y: ");
  Serial.println(d.y);
  for (int j = 0; j < 4; j++) {
    if (d.otherNodes[j].doesExist) {
      Serial.print("Node ");
      Serial.println(j);
      Serial.print("x: ");
      Serial.println(d.otherNodes[j].x);
      Serial.print("y: ");
      Serial.println(d.otherNodes[j].y);
      Serial.print("isLeaf: ");
      if (d.otherNodes[j].isLeaf) {
        Serial.println("true");
      } else {
        Serial.println("false");
      }
    }
  }
}
