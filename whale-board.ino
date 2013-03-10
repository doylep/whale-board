//** Modified Version 
// Sensors needed to be added 
// I.T
/*
 * MultiRX sketch
 * Receive data from two software serial ports
 */
#include <SoftwareSerial.h>
#define WAIT_5 5000
#define WAIT_1 1000
// Initialize RX and TX pins

const int rxAero = 10;
const int txAero = 7;
const int rxXbee = 9;
const int txXbee = 6;
const int rxGPS = 8;
const int txGPS = 5;
const int rxSD = 11;
const int txSD = 3;
const int H_input = A0; // analog pin 0
const int T1_pin = A1;  // analog pin 1
const int z_input = A2; // analog pin 2
const int y_input = A3; // analog pin 3
const int x_input = A4; // analog pin 4
const int T2_pin = A5;  // analog pin 5 
const int prsPin = A6; // digital pin 4

// Variables
int state =1;
long int start =0;
String CUT = "cut";
String SAFE_MOD = "safe";
boolean flag = false;

int rawPr = 0;
int H_Value = 0;
int x = 0;
int y = 0;
int z = 0;
int temp1 = 0;
int temp2 = 0;

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
    data.reserve(400);
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

  delay(WAIT_1);  //delay to make sure command mode is entered

  openLog.println("new dataFile.txt"); //makes new file named "dataFile.txt"
 
  delay(WAIT_1); //delay to avoid sadness

  openLog.println("append dataFile.txt");  //says we want to write stuff to "dataFile.txt"
  delay(WAIT_1); //delay to avoid sadness
  
  aero.listen(); // Set “AeroComm” to be the active device
}

void loop()
{
  aerocom:
  start = millis();
  // Listen to the aerocomm for 5 seconds and write received data to xBee  
  while ( (start + WAIT_5) > millis() ) {
    aero.listen(); // first listen then for to serial event
    if (aero.available() > 0) 
    {
      Serial.println("Getting data from AeroComm");
      serialEvent(aero, '@');
     
      // Check if recieved data is "cut" command otherwise do nothing!
      if(data.equalsIgnoreCase(CUT)) {
        xbee.print(data);
        data = "";
        goto printData; // Save data first after cut command
      }
      xbee.listen(); // any point in listening to xbee?
      delay(2);
      

      // If we switch to safemode after cut the wires
      // here is the part we only listen to gps and print data into data logger   ---   ??
      
      if(data.equalsIgnoreCase(SAFE_MOD))
      {
        flag = true; // only safe mode
        //go to GPS and print data from GPS
        goto gps;
        // do not need to read from sensors anymore  --- WHY NOT GET MORE DATA ON THE DECENT REGARDLESS?
      }
      //** Why do you listen after serialevent?
      //aero.listen();
      //delay(2);
    }
  }
  
  
  sensors:
  int analogcount = 0;
  do
  {
  switch (analogcount){
        case 1:    // Presure Sensor Reading/Writting
        rawPr = analogRead(prsPin);
        data += String(rawPr); // Add to string
        data += String(",");
        break;
        
        case 2:   // HUMIDITY SENSOR Reading/Writting
        H_Value = analogRead(H_input); // read the value from the sensor:
        data += String(H_Value); // Add to string
        data += String(",");
        break;
           
        case 3: // ACCELERATION (X,Y,Z) Reading/Writting
        x = analogRead(x_input); 
        y = analogRead(y_input); 
        z = analogRead(z_input); 
        data += String(x); // Add to string
        data += String(",");
        data += String(y); // Add to string
        data += String(",");
        data += String(z); // Add to string
        data += String(","); 
        break;
       
        case 4:
        temp1 = analogRead(T1_pin);
        temp1 = int(temp1);
        data += String(temp1);
        data +=(",");
        temp2 = analogRead(T2_pin);
        temp2 = int(temp2);
        data += String(temp2);
        data +=(",");
        break;
        
        default:
        // Do nothing
        break;
        
      }// End of reading from Sensors
      analogcount++; 
    }while(analogcount < 5);// End of for loop
    goto printData;
  

  gps:
  // Listen to the GPS for 1 second and write received data to variable data
  start = millis();
  while ( (start + WAIT_1) > millis() ) {
    gps.listen();
    delay(2);
    if(gps.available() > 0)
    {
      Serial.println("Getting GPS data");
      serialEvent(gps, '\n');
      Serial.println(data);
      //if(flag) goto 
      goto printData;
     // break; // ** you wanna listen to gps for 1 sec why do you break?
    }
  }
 
  
  printData:
  // Print GPS data to AeroComm
  Serial.println("Writing data to AeroComm");
  // aero.listen();//**** Why do you have so many aero.listen?  Remove this
  delay(2);  
  if(aero.available()) aero.print(data);// print to write in exact form it received
  
  // Print data to SD Card
  Serial.println("Writing data to SD Card");
  // openLog.listen(); // Do not need to listen to print data !!
  delay(2);
  openLog.print(data); 
  data = "";// reset data string? 
  goto aerocom;
  //aero.listen();
        
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
