
/*
  Getting Started example sketch for nRF24L01+ radios
  This is a very basic example of how to send data from one node to another
  Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

#define SD3            10

#define CE_PIN_RADIO    D8          // was D8 NOTE: D9 is SD2
#define CS_PIN_RADIO    SD3          // was D2 NOTE: D10 is SD3

/****************** User Config ***************************/

// Can't choose 0 (that's the transmitter). Valid values are 1-3
int radioNumber = 2;

int role = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
// radio(ce_pin, cs_pin)
// apparently the ce pin turns on/off the board whereas the cs_pin is the chip select for the spi
RF24 radio(CE_PIN_RADIO, CS_PIN_RADIO);

const int message_length = 300;
String message_to_send = "I got your message";
char message[message_length];
/**********************************************************/

// so technically the first 32 bits need to be the same for multiple reading pipes, but here it looks like they did the opposite
// We think that the arduino may have little/big endian that makes this true (we're assumming the library example did it correctly)
byte addresses[][6] = {"0Node", "1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

void setup() {
  Serial.begin(115200);
  Serial.println(F("RF24/examples/GettingStarted"));
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);

  // Open a writing and reading pipe on each radio, with opposite addresses
  //  if(radioNumber){
  //    radio.openWritingPipe(addresses[1]);
  //    radio.openReadingPipe(1,addresses[0]);
  //  }else{
  //    radio.openWritingPipe(addresses[0]);
  //    radio.openReadingPipe(1,addresses[1]);
  //  }

  radio.openReadingPipe(1, addresses[radioNumber]);

  // Start the radio listening for data
  radio.startListening();
  Serial.print("I am node number ");
  Serial.println(radioNumber);
}

void loop() {


  /****************** Pong Back Role ***************************/

  if ( role == 0 )
  {
    unsigned long got_time;
    //    Serial.println("Checking if radio is available");
    if ( radio.available()) {
      // Variable for the received timestamp
      //      while (radio.available()) {                                   // While there is data ready
      //        radio.read( &got_time, sizeof(unsigned long) );             // Get the payload
      //      }
      while (radio.available()) {                                   // While there is data ready
        radio.read( &message, message_length * sizeof(char) );             // Get the payload
      }

      //      radio.stopListening();                                        // First, stop listening so we can talk
      ////      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.
      //      radio.write( &message, message_length * sizeof(char) );              // Send the final one back.
      //      radio.startListening();                                       // Now, resume listening so we catch the next packets.
      //Serial.print(F("Sent response "));
      //      Serial.println(got_time);
      Serial.print("Got the message: ");
      Serial.println(message);
      //      Serial.print("And I am node number ");
      //      Serial.println(radioNumber);

      // send a message back
      radio.stopListening();
      radio.openWritingPipe(addresses[radioNumber + 3]);
      message_to_send.getBytes((unsigned char*)message, message_length);
      if (!radio.write( &message, sizeof(char) * message_to_send.length() )) {
        Serial.println("Couldn't send message:");
        Serial.print(message_to_send);
      } else {
        Serial.println(F("Sent: "));
        Serial.print(message_to_send);
      }
      radio.startListening();
    }
  }




  /****************** Change Roles via Serial Commands ***************************/
//  Serial.println("Checking if serial is available");
  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == 0 ) {
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      role = 1;                  // Become the primary transmitter (ping out)

    } else if ( c == 'R' && role == 1 ) {
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      role = 0;                // Become the primary receiver (pong back)
      radio.startListening();

    }
  }


} // Loop
