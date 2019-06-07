
/*
  Getting Started example sketch for nRF24L01+ radios
  This is a very basic example of how to send data from one node to another
  Updated: Dec 2014 by TMRh20
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
RF24 radio(7, 8);
/**********************************************************/

const int message_length = 300;
char message[message_length];

// so technically the first 32 bits need to be the same for multiple reading pipes, but here it looks like they did the opposite
// We think that the arduino may have little/big endian that makes this true (we're assumming the library example did it correctly)
byte addresses[][6] = {"0Node", "1Node", "2Node", "3Node", "4Node", "5Node", "6Node"};

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
  char message = 0;
  DATABLOCK dataBlock;
};

struct COMMAND {
  char message; // either 'l', 'r', or 'f'
  char padding; // had to put this in since the NodeMCU pads and Arduino does not
  // and I have no idea why... perhaps a bug?
  uint16_t pwm;
};

typedef union PACKETUNION {
  PACKET packet = PACKET();
  COMMAND command;
  byte bytes[sizeof(PACKET)];
};

char *float2s(float f, unsigned int digits = 2);
void transmitPacket(PACKET* p);
void stringToPacket(String data, PACKET* p);
void stringToCommand(String data, COMMAND* c);
void printDataBlock(DATABLOCK &d);
void printCommand(COMMAND &c);
String threeLongInteger(int x);

void setup() {
  Serial.begin(115200);
  Serial.println(F("Setting up the transmitter"));
  Serial.setTimeout(5);

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
  radio.openReadingPipe(1, addresses[4]); // open up reading pipes, with the pipe number being the node number of that node
  radio.openReadingPipe(2, addresses[5]);
  radio.openReadingPipe(3, addresses[6]);
  radio.startListening();
}

void loop() {


  /****************** Change Roles via Serial Commands ***************************/

  if ( Serial.available() )
  {
    String serialMessage = Serial.readString();
    serialMessage.trim();
//    Serial.print("Got message at arduino: ");
//    Serial.println(serialMessage);
    int nodeNum = serialMessage.charAt(1) - '0';
    if ( nodeNum == 1 || nodeNum == 2 || nodeNum == 3) {
      radio.stopListening();                                    // First, stop listening so we can talk.

      Serial.print("Opening up the writing channel for node ");
      Serial.println(nodeNum);
      radio.openWritingPipe(addresses[nodeNum]);

      Serial.println(F("Now sending"));

      //unsigned long time = micros();                             // Take the time, and send it.  This will block until complete
      //     if (!radio.write( &time, sizeof(unsigned long) )){
      //       Serial.println(F("failed"));
      //     }

      //      String message_to_send = serialMessage.substring(2, serialMessage.length());
      String message_to_send = serialMessage;
//      Serial.print("Going to send: ");
//      Serial.println(message_to_send);
      //unsigned char message[message_length];
      //String personalized_message = message_to_send + nodeNum;
      //      message_to_send.getBytes((unsigned char*)message, message_length);
      //      if (!radio.write( &message, sizeof(char) * message_to_send.length() )) {
      //        Serial.println("Couldn't send message:");
      //        Serial.print(message_to_send);
      //        Serial.print(" to node number ");
      //        Serial.print(nodeNum);
      //        Serial.println(" Is this receiver on?");
      //      } else {
      //        Serial.println(F("Sent: "));
      //        Serial.print(message_to_send);
      //        Serial.print(" to node number ");
      //        Serial.println(nodeNum);
      //      }
      PACKETUNION packetUnion;
      PACKET p = packetUnion.packet;
      DATABLOCK data = p.dataBlock;
      COMMAND command = packetUnion.command;
      p.message = message_to_send[2];
      if (p.message != 'l' && p.message != 'r' && p.message != 'f' && p.message != 'w') {
        Serial.print("Unknown message: ");
        Serial.println(p.message);
        return;
      }
      if (p.message == 'w') {
        stringToPacket(message_to_send, &p);
        if (!radio.write( &p, sizeof(p) )) {
          Serial.println("Couldn't send message:");
//          Serial.print(message_to_send);
          Serial.println(" Is the receiver on?");
        } else {
//          Serial.println(F("Sent: "));
//          Serial.println(message_to_send);
          printDataBlock(p.dataBlock);

        }
      } else { //it's a command then
        stringToCommand(message_to_send, &(command));
        if (!radio.write( &command, sizeof(command) )) {
          Serial.println("Couldn't send message:");
//          Serial.print(message_to_send);
          Serial.println(" Is the receiver on?");
        } else {
//          Serial.println(F("Sent: "));
//          Serial.println(message_to_send);
          printCommand(command);

        }
      }


    } else {
      Serial.print("ERROR: Requested node number of ");
      Serial.print(nodeNum);
      Serial.println(" is unknown");
      Serial.print("Serial Message Received: ");
      Serial.println(serialMessage);
      Serial.print("q");
    }
    radio.startListening();
  }

  // now check to see if any messages from the nodes
  for ( uint8_t nodeNum = 1; nodeNum <= 3; nodeNum++) {
    if (radio.available(&nodeNum)) {
      PACKETUNION packetUnion;
      while (radio.available(&nodeNum)) {                                   // While there is data ready
        //        radio.read( &message, message_length * sizeof(char) );             // Get the payload
        radio.read( &packetUnion, sizeof(packetUnion));
      }
      PACKET p = packetUnion.packet;
      DATABLOCK data = p.dataBlock;
      if (p.message == 'r') {
        Serial.println("Got the following message: ");
        //      Serial.println(p.message);
        //      Serial.print("x: ");
        //      Serial.println(data.x);
        //      Serial.print("y: ");
        //      Serial.println(data.y);
        printDataBlock(data);
        //      Serial.print("a: ");
        //      Serial.println(float2s(data.a));
        transmitPacket(nodeNum, &p);
      } else {
        Serial.print(F("Got packet with unknown message: "));
        Serial.println(p.message);
      }

    }
  }


} // Loop

// Transmits the message from the node to the computer via serial
// $nodeNum x y [x1 y1 iL1]...        where x,y are 3 numbers long and iL is either 1 or 0
// if the node does not exist, it will not transmit it
void transmitPacket(int nodeNum, PACKET* p) {
  DATABLOCK* d = &((*p).dataBlock);
  Serial.print("$");
  Serial.print(nodeNum);
  Serial.print(" ");
  Serial.print(threeLongInteger((*d).x));
  Serial.print(" ");
  Serial.print(threeLongInteger((*d).y));
  Serial.print(" ");
  for (int j = 0; j < 4; j++) {
    if ((*d).otherNodes[j].doesExist) {
      Serial.print("[");
      Serial.print(threeLongInteger((*d).otherNodes[j].x));
      Serial.print(" ");
      Serial.print(threeLongInteger((*d).otherNodes[j].y));
      Serial.print(" ");
      if ((*d).otherNodes[j].isLeaf) {
        Serial.print("1");
      } else {
        Serial.print("0");
      }
      Serial.print("] ");
    }
  }
  Serial.println();
} //end transmitPacket

// Preps from the serial line to the handheld device. Assumes string is as below
// $nodeNumMessage x y [x1 y1 iL1] [x2 y2 iL2]...        where x,y are 3 numbers long and iL is either 1 or 0
// where nodeNum is a digit and Message is a single char
void stringToPacket(String data, PACKET* p) {
  DATABLOCK* d = &((*p).dataBlock);
  (*p).message = data[2];
  (*d).x = (uint8_t) data.substring(4, 4 + 3).toInt();
  (*d).y = (uint8_t) data.substring(8, 8 + 3).toInt();
  int nodeNum = 0;
  int index = 12;
  // this loop assumes index is at the the open bracket
  while (nodeNum < 4 && (index + 11) <= data.length()) { // 11 is the size of [x1 y1 iL1]
    index += 1; //skip past first open bracket
    (*d).otherNodes[nodeNum].x = data.substring(index, index + 3).toInt();
    index += 4;
    (*d).otherNodes[nodeNum].y = data.substring(index, index + 3).toInt();
    index += 4;
    if (data[index] == '0') {
      (*d).otherNodes[nodeNum].isLeaf = false;
    } else {
      (*d).otherNodes[nodeNum].isLeaf = true;
    }
    (*d).otherNodes[nodeNum].doesExist = true;
    index += 3; // to get to the next open bracket
    nodeNum++;
  }
  //  packet.a = data.substring(11, 11+5).toFloat();
}

// Preps from the serial line to the handheld device. Assumes string is as below
// $nodeNumMessage pwm...        where pwm is a 3-4 digits long
// where nodeNum is a digit and Message is a single char
void stringToCommand(String data, COMMAND* c) {
  (*c).message = data[2];
  (*c).pwm = (uint16_t) data.substring(4).toInt();
  Serial.print("Here is the substring: ");
  Serial.println(data.substring(4).toInt());
}

// converts an int to a three char long string
// outputs ERR if the integer can't be converted
String threeLongInteger(int x) {
  String toReturn = String(x);
  if (toReturn.length() > 3) {
    return "ERR";
  } else {
    while (toReturn.length() < 3) {
      toReturn = "0" + toReturn;
    }
    return toReturn;
  }
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
    strcpy(buf + index, "INF");
    return buf;
  }

  // Handle NaNs
  if (isnan(f)) {
    strcpy(buf + index, "NAN");
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
  for (int i = sizeof(f) - 1; i >= 0; i--) {
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

void printDataBlock(DATABLOCK &d) {
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

void printCommand(COMMAND &c) {
  Serial.print("command: ");
  Serial.println(c.message);
  Serial.print("pwm: ");
  Serial.println(c.pwm);
}
