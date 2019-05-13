
/*
  Getting Started example sketch for nRF24L01+ radios
  This is a very basic example of how to send data from one node to another
  Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
// radio(ce_pin, cs_pin)
// apparently the ce pin turns on/off the board whereas the cs_pin is the chip select for the spi
RF24 radio(7, 8);
/**********************************************************/

// so technically the first 32 bits need to be the same for multiple reading pipes, but here it looks like they did the opposite
// We think that the arduino may have little/big endian that makes this true (we're assumming the library example did it correctly)
byte addresses[][6] = {"0HanH", "1HanH"};

const uint8_t message_length = 255;

// Used to control whether this node is sending or receiving
//bool role = 0;
char message[message_length]; // Note: had elsewhere that this was an unsigned char array

void setup() {
  Serial.begin(115200);
  Serial.println(F("Setting up the transmitter"));

  radio.begin();

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

  radio.setPayloadSize(255);

  // Start the radio listening for data
  radio.startListening();
  Serial.println("Transmitter is ready");
}

void loop() {


  /****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
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

    String serialMessage = Serial.readString();
    Serial.print("Got message at arduino: ");
    Serial.println(serialMessage);
    bool isValidMessage = true; //TODO: Change this so we actually check for a valid command
    if ( isValidMessage ) {
      radio.stopListening();                                    // First, stop listening so we can talk.

      Serial.print("Opening up the writing channel for node ");
      Serial.println((radioNumber + 1) % 2);
      //        radio.openWritingPipe(addresses[(radioNumber+1)%2]);

      Serial.println(F("Now sending"));

      //     if (!radio.write( &time, sizeof(unsigned long) )){
      //       Serial.println(F("failed"));
      //     }

      //        String message_to_send = serialMessage.substring(2,serialMessage.length());
      String message_to_send = serialMessage; // TODO: Place any code in here to parse the serial message before sending it
      Serial.print("Going to send: ");
      Serial.println(message_to_send);
      //String personalized_message = message_to_send + nodeNum;
      message_to_send.getBytes(message, message_length);
      //      bool failed = false;
      //      int index = 0;
      //      while (index < message_to_send.length() && index < message_length) {
      //        if (!radio.write( &(message[index]), sizeof(char) * 32 )) {
      //          Serial.println("Couldn't send message:");
      //          Serial.print(message_to_send);
      //          Serial.println(" Is the receiver on?");
      //          failed = true;
      //        }
      //        index += 32;
      //      }
      //      if(!failed){
      //        Serial.println(F("Sent: "));
      //        Serial.print(message_to_send);
      //      }

      if (!radio.write( &message, sizeof(char) * message_to_send.length() )) {
        Serial.println("Couldn't send message:");
        Serial.print(message_to_send);
        Serial.println(" Is the receiver on?");
      } else {
        Serial.println(F("Sent: "));
        Serial.print(message_to_send);
      }
    } else {
      Serial.println("ERROR: Didn't recognize the following command");
      Serial.print("Serial Message Received: ");
      Serial.println(serialMessage);
      Serial.print("q");
    }

    radio.startListening();
  }

  if ( radio.available()) {
    // Variable for the received timestamp
    while (radio.available()) {                                   // While there is data ready
      radio.read( &message, message_length * sizeof(char) );             // Get the payload
    }
    if(message[0] == '%') {
      Serial.println(message);
    } else {
    Serial.print(F("Got the message: "));
    Serial.println(message);
    }
  }


} // Loop
