/*
 * MultiRX sketch
 * Receive data from two software serial ports
 */
#include <SoftwareSerial.h>

// Initialize RX and TX pins

const int rxAero = 10;
const int txAero = 7;
const int rxXbee = 9;
const int txXbee = 6;
const int rxGPS = 8;
const int txGPS = 5;
const int rxSD = 11;
const int txSD = 3;

// Initialize parameters for SerialEvent
String data="";
boolean stringComplete;

SoftwareSerial aero(rxAero, txAero); // aerocomm device connected to pins 10 and 7
SoftwareSerial xbee(rxXbee, txXbee); // xbee device connected to pins 9 and 6
SoftwareSerial gps(rxGPS, txGPS); // gps device connected to pins 8 and 5
SoftwareSerial openLog(rxSD, txSD); // sd device connected to pins 11 and 3

void setup()
{
  // Initialize serial monitor on computer (for diagnostic purposes)  
  Serial.begin(9600);
  while (!Serial){
  }
  
  // Initialize baud rates for all serial devices
  xbee.begin(9600);
  aero.begin(9600);
  openLog.begin(9600);
  gps.begin(4800);
  
  // Initialize Open Logger
  openLog.listen();
  delay(2);
  
  //write 26 (ctrl-z in ascii) three times to enter command mode
  openLog.write(26);
  openLog.write(26);
  openLog.write(26);

  delay(1000);  //delay to make sure command mode is entered

  openLog.println("new dataFile.txt"); //makes new file named "dataFile.txt"
 
  delay(1000); //delay to avoid sadness

  openLog.println("append dataFile.txt");  //says we want to write stuff to "dataFile.txt"
  delay(1000); //delay to avoid sadness
  
  aero.listen(); // Set “AeroComm” to be the active device
}

void loop()
{
  unsigned long start = millis();
  
  // Listen to the aerocomm for 5 seconds and write received data to xBee  
  while ( (start + 5000) > millis() ) {
    if (aero.available() > 0) 
    {
      Serial.println("Getting data from AeroComm");
      serialEvent(aero, '@');
      xbee.listen();
      delay(2);
      xbee.print(data);
      data = "";
      aero.listen();
      delay(2);
    }
  }
  
  // Listen to the GPS for 1 second and write received data to variable data
  start = millis();
  while ( (start + 1000) > millis() ) {
    gps.listen();
    delay(2);
    if(gps.available() > 0)
    {
      Serial.println("Getting GPS data");
      serialEvent(gps, '\n');
      Serial.println(data);
      break;
    }
  }
  
  // Print GPS data to AeroComm
  Serial.println("Writing GPS data to AeroComm");
  aero.listen();
  delay(2);  
  aero.print(data);
  
  // Print GPS data to SD Card
  Serial.println("Writing GPS data to SD Card");
  openLog.listen();
  delay(2);
  openLog.print(data);  
  
  data = "";
  
  aero.listen();
        
}

void serialEvent(SoftwareSerial &Port, char endCommand) 
{
  stringComplete=false;
  if (Port.available()>0)
  {
    Serial.println("Getting Data...");
  while(!stringComplete){
    // get the new byte:
    char inChar = (char)Port.read(); 
    delay(100);
    // add it to the inputString:
    if(inChar>0)
      data += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == endCommand) {
      stringComplete = true;
    }
  }
}
}
