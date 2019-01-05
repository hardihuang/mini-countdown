
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

 #define outputA 6
 #define outputB 7
 int counter = 0; 
 int aState;
 int aLastState;  



int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);


 void setup() { 
  matrix.setIntensity(1);
   pinMode (outputA,INPUT);
   pinMode (outputB,INPUT);
   
   Serial.begin (9600);
   // Reads the initial state of the outputA
   aLastState = digitalRead(outputA);   
 } 

 void loop() { 
   aState = digitalRead(outputA); 
   if (aState != aLastState){     
     if (digitalRead(outputB) != aState) { 
      if(counter<90){
        counter ++;
      }  
     } else {
      if(counter>0){
        counter --;
       }
     }
   } 
   aLastState = aState; // Updates the previous state of the outputA with the current state
   String temp = formateDigit(counter);
   matrix.fillScreen(LOW);
   matrix.drawChar(0,0,temp.charAt(1),1,0,1);
   matrix.write(); // Send bitmap to display
   
   
   
 }

 String formateDigit(int digitTemp) {
  String strTemp = String(digitTemp);
  if (digitTemp < 10) {
    strTemp = "0" + strTemp;
  }
  return strTemp;
}
