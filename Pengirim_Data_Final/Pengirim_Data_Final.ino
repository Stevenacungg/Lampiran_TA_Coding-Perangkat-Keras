/*test
  Reading multiple RFID tags, simultaneously!
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 3rd, 2016
  https://github.com/sparkfun/Simultaneous_RFID_Tag_Reader

  Constantly reads and outputs any tags heard

  If using the Simultaneous RFID Tag Reader (SRTR) shield, make sure the serial slide
  switch is in the 'SW-UART' position
*/

//Used for transmitting to the device
//If you run into compilation errors regarding this include, see the README

//RX, TX
//Library for controlling the M6E Nano module


//Wait for the serial port to come online
//Configure nano to run at 38400bps
//Freeze!
 //Set to North America
 //5.00 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling
  //Throw away the user's character
  //Begin scanning for tags
#include <SoftwareSerial.h>
SoftwareSerial softSerial(2, 3); // RX, TX

#include "SparkFun_UHF_RFID_Reader.h" 
RFID nano; 

//unsigned long readInterval = 30000;     // Interval waktu membaca (30 detik)
//unsigned long noReadInterval = 30000;   // Interval waktu tidak membaca (30 detik)
//unsigned long previousTime = 0;
//bool readingEnabled = true;

void setup()
{
  Serial.begin(115200);
  while (!Serial); 

  if (setupNano(38400) == false) 
  {
    Serial.println(F("Module failed to respond. Please check wiring."));
    while (1); 
  }

  nano.setRegion(REGION_OPEN);

  nano.setReadPower(2700); 

  nano.startReading(); 

  Serial.println("Sensor Bekerja");
}

void loop()
{
  if (nano.check() == true) 
  {
    byte responseType = nano.parseResponse(); 
    if (responseType == RESPONSE_IS_KEEPALIVE)
    {
    }
    else if (responseType == RESPONSE_IS_TAGFOUND)
    {
      byte tagEPCBytes = nano.getTagEPCBytes(); 
      Serial.print(F(" epc["));
      for (byte x = 0 ; x < tagEPCBytes ; x++)
      {
        if (nano.msg[31 + x] < 0x10) Serial.print(F("0")); 
        Serial.print(nano.msg[31 + x], HEX);
        //Serial.print(F(" "));
      }
      Serial.print(F("]"));
      Serial.println();
    }
    else if (responseType == ERROR_CORRUPT_RESPONSE)
    {
      //Serial.println("");
    }
    else
    {
      //Serial.println("");
    }
  }
}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while (softSerial.isListening() == false); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (softSerial.available()) softSerial.read();

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate

    delay(250);
  }

  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right

  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2

  nano.setAntennaPort(); //Set TX/RX antenna ports to 1

  return (true); //We are ready to rock
}
