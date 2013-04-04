/*
 * MultiRX sketch
 * Receive data from two software serial ports
 */
#include <SoftwareSerial.h>

#define WAIT_flight 3000000
#define WAIT_com 600000
#define WAIT_20 20000
#define WAIT_10 10000
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

// Initialize sensor parameters
int rawPr = 0;
int H_Value = 0;
int x = 0;
int y = 0;
int z = 0;
int temp1 = 0;
int temp2 = 0;
int count;

// Initialize parameters for SerialEvent
String data="";
boolean stringComplete;

// Initialize other parameters
unsigned long start = 0;
unsigned long startEvent = 0;
unsigned long lastCom;
unsigned long flightTime;
const String CUT = "cut@";
int idx = 0;
int idx2;
String temp;
boolean validAlt = false;
float alt;

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
  flightTime = millis();
  lastCom = millis();
}

void loop()
{  
    aerocomm();
    data += (millis()-flightTime)/1000;
    data += '#';
    sensors();
    gpsData();
    // Check to see if we've communicated with the ground within the last 10 minutes
    if ((millis() - lastCom) > WAIT_com)
    {
      Serial.println("No communication from ground");
      getAltitude();
      checkCut();
    }
    Serial.print(data);
    printData();
}

void aerocomm() {
  // Listen to the aerocomm for 5 seconds and write received data to xBee  
  start = millis();
  aero.listen();

  while ( (start + WAIT_5) > millis() ) {
    if (aero.available() > 0) 
    {
      Serial.println("Getting data from AeroComm...");
      serialEvent(aero, '@');    
      lastCom = millis();  // Reset the communication timer 
      if(data.equalsIgnoreCase(CUT)) {
        xbee.print(data);  // Print data to the xBee
        Serial.println("Cut command sent.");
      }
    }
  }
  data += '#'; // Add deliminator for data packeting
}

void sensors() {
   count = 1; 
    Serial.println("Getting data from Sensors...");
    do
    {
    switch (count){
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
      count++; 
    }while(count < 5);// End of for loop
    data+='#'; // Add deliminator for data packeting
  }
  
void gpsData() {
  // Listen to the GPS for 1 second and write received data to variable data
  gps.listen();
  delay(200);
  
    if(gps.available() > 0)
    {
      Serial.println("Getting data from GPS...");
      serialEvent(gps, '\n');
    }  
  data += '#';
  }
  
 void printData() {

  // Print GPS data to AeroComm
  Serial.println("Writing data to AeroComm");
  aero.print(data);
  
  // Print GPS data to SD Card
  Serial.println("Writing data to SD Card");
  openLog.print(data);  
  
  data = "";
        
}

void serialEvent(SoftwareSerial &Port, char endCommand) 
{
  startEvent = millis();
  stringComplete=false;
  if (Port.available()>0)
  {
    while(!stringComplete && ((startEvent + WAIT_20) > millis())){
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

void getAltitude()
{
  // Remove Sensor Data
  idx = data.indexOf('#');
  temp = data.substring(idx+1);
  idx = temp.indexOf('#');
  temp = temp.substring(idx+1);
  idx = temp.indexOf('#');
  temp = temp.substring(idx+1);
  if (temp.startsWith("$GPGGA")) // Check to see if gps string is normal
  {
    for (count = 0; count < 9; count ++) // Change this back to 9 after testing
    {
      idx = temp.indexOf(',');
      if (idx == -1)
      {
        validAlt = false;
        Serial.println("Invalid GPS String: not enough fields");
        break;
      }
      temp = temp.substring(idx+1);
    }
    idx = temp.indexOf(',');

    if ( idx > 0 )
    {
      temp = temp.substring(0, idx);
      validAlt = true;
    }
    else
    {
      Serial.println("Invalid Altitude: no value");
      validAlt = false;
    }
  }
  
  if(validAlt)
  {
    char buf[temp.length()];
    temp.toCharArray(buf,temp.length());
    alt = atof(buf); 
    if (alt < 200 || alt > 35000)
    {
      Serial.println("Invalid Altitude: not within expected range");
      validAlt = false;
    }
  }
}

void checkCut()
{
  Serial.println("Checking altitude...");
  Serial.print("The altitude is:");
  Serial.println(alt);
  if(validAlt)
  {
    // Check to see if balloon is above 50000 ft or 15240 m
    if (alt > 15420)
    {
      xbee.print(CUT);
      data += "Above 50000 ft";
      Serial.println("Cut command sent");
    }
  }
  else
  {
    Serial.println("Checking time of flight...");
    if ((millis() - flightTime) > WAIT_flight)
    {
      xbee.print(CUT);
      data += "Flight is longer than 50 min";    
      Serial.println("Cut command sent");  
    }
  }
}
