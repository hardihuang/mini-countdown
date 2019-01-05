
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

static const unsigned char num0[] =
{B000,
B111,
B101,
B101,
B101,
B101,
B101,
B111};

static const unsigned char num1[] =
{B000,
B010,
B010,
B010,
B010,
B010,
B010,
B010};

static const unsigned char num2[] =
{B000,
B111,
B001,
B001,
B111,
B100,
B100,
B111};

static const unsigned char num3[] =
{B000,
B111,
B001,
B001,
B111,
B001,
B001,
B111};

static const unsigned char num4[] =
{B000,
B101,
B101,
B101,
B111,
B001,
B001,
B001};

static const unsigned char num5[] =
{B000,
B111,
B100,
B100,
B111,
B001,
B001,
B111};

static const unsigned char num6[] =
{B000,
B111,
B100,
B100,
B111,
B101,
B101,
B111};

static const unsigned char num7[] =
{B000,
B111,
B001,
B001,
B001,
B001,
B001,
B001};

static const unsigned char num8[] =
{B000,
B111,
B101,
B101,
B111,
B101,
B101,
B111};

static const unsigned char num9[] =
{B000,
B111,
B101,
B101,
B111,
B001,
B001,
B111};

static unsigned char*  numbers[10] = {num0, num1, num2, num3, num4,num5,num6,num7,num8,num9};

 void setup() { 
  matrix.setIntensity(1);
  matrix.setRotation(0, 1);
 } 

 void loop() { 
  
   matrix.fillScreen(LOW);
   for(int i=0;i<3;i++){
    for(int n=0;n<8;n++){
      matrix.drawPixel(i,n,bitRead(numbers[0][n],2-i));  
    }
   }
   for(int i=0;i<3;i++){
    for(int n=0;n<8;n++){
      matrix.drawPixel(i+5,n,bitRead(numbers[1][n],2-i));  
    }
   }
   matrix.write(); // Send bitmap to display
 }

 String formateDigit(int digitTemp) {
  String strTemp = String(digitTemp);
  if (digitTemp < 10) {
    strTemp = "0" + strTemp;
  }
  return strTemp;
}
