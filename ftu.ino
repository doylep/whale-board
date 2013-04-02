/*  Flight Termination Unit
Based on the Software Serial Example
by the Space Whale team   */

#include <SoftwareSerial.h>

// Global constants
const int WAIT_5 = 5000;
const int WAIT_10 = 20000;
const int cutPin = 8;
const String cutCommand = "cut@";
const String CutCommand = "Cut@";
const char STR_END = '@';
SoftwareSerial xBeeNano(2, 3); // RX, TX

// Global variables
boolean stringComplete = false;
String data="";
unsigned long start;


// Setup loop
void setup()  
{
  // Open serial communications
  Serial.begin(9600);
  
  // Wait for serial port to connect
  while (!Serial) {
    ;  
  }
  
  // Set the data rate for the SoftwareSerial port
  xBeeNano.begin(9600);

  // Set cut pin
  pinMode(cutPin, OUTPUT);
}


// Main loop
void loop()
{
  
  // Check for data
  serialEvent(); 
  if (stringComplete) 
  {  
    // Send to computer and check for cut
    // Compare lower and upper cases
    if (data.equals(cutCommand)||data.equalsIgnoreCase(cutCommand))
    {
      // Cutdown balloon
      start = millis();
      while (start + WAIT_10 > millis()) {
        digitalWrite(cutPin, HIGH);
      }
      digitalWrite(cutPin, LOW);
    }
  }

  // Wait
  digitalWrite(cutPin, LOW);
  delay(1000);
  data = "";
}


// Gather serial data
void serialEvent() 
{
  // Note start time
  start = millis();

  // Reset flag
  stringComplete=false;
  if (xBeeNano.available())
  {
    // Wait for complete string or WAIT_1
    while(!stringComplete && (start + WAIT_5 > millis()))
    {
      // Get the new byte:
      char inChar = (char)xBeeNano.read(); 
      delay(2);
      
      // Add it to the data:
      if(inChar > 0)
        data += inChar;
      
      // Check for terminating character
      if (inChar == STR_END)
        stringComplete = true;
    }
  }
}
