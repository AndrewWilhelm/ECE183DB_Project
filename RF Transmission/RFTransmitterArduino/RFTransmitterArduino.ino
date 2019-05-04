
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
//bool radioNumber = 0;

//int nodeNum = 1;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
// radio(ce_pin, cs_pin)
// apparently the ce pin turns on/off the board whereas the cs_pin is the chip select for the spi
RF24 radio(7,8);
/**********************************************************/

// so technically the first 32 bits need to be the same for multiple reading pipes, but here it looks like they did the opposite
// We think that the arduino may have little/big endian that makes this true (we're assumming the library example did it correctly)
byte addresses[][6] = {"0Node","1Node","2Node","3Node"};

// Used to control whether this node is sending or receiving
bool role = 0;
String message_to_send = "Hello there node number ";
String message_to_return = "New RF transmitter... who dis?";

void setup() {
  Serial.begin(115200);
  Serial.println(F("Setting up the transmitter"));
  
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
  
  // Start the radio listening for data
//  radio.startListening();
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
     int nodeNum = serialMessage.charAt(0) - '0';
     if ( nodeNum == 1 || nodeNum == 2 || nodeNum == 3) {
        radio.stopListening();                                    // First, stop listening so we can talk.

        Serial.print("Opening up the writing channel for node ");
        Serial.println(nodeNum);
        radio.openWritingPipe(addresses[nodeNum]);
        
        Serial.println(F("Now sending"));
    
        unsigned long time = micros();                             // Take the time, and send it.  This will block until complete
    //     if (!radio.write( &time, sizeof(unsigned long) )){
    //       Serial.println(F("failed"));
    //     }

        String message_to_send = serialMessage.substring(2,serialMessage.length());
        Serial.print("Going to send: ");
        Serial.println(message_to_send);
        unsigned char message[100];
            //String personalized_message = message_to_send + nodeNum;
       message_to_send.getBytes(message, 100);
       if (!radio.write( &message, sizeof(char) * message_to_send.length() )){
         Serial.println("Couldn't send message:");
         Serial.print(message_to_send);
         Serial.print(" to node number ");
         Serial.print(nodeNum);
         Serial.println(" Is this receiver on?");
       } else {
          Serial.println(F("Sent: "));
          Serial.print(message_to_send);
          Serial.print(" to node number ");
          Serial.println(nodeNum);
       }
     } else {
        Serial.print("ERROR: Requested node number of ");
        Serial.print(nodeNum);
        Serial.println(" is unknown");
        Serial.print("Serial Message Received: ");
        Serial.println(serialMessage);
        Serial.print("q");
     }
  }


} // Loop
