#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <EEPROM.h>

#define VMIN 3.6  // Battery Level 0%
#define VMAX 4.2  // Batter Level 100%
#define VBAT_PER_BITS 0.0041056  // Volts per bit on the A0 pin, VMAX / 1023 = 3.44 / 1023 = 0.0033625
#define BATTERY_SENSE_PIN A0


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

const unsigned char PROGMEM setAlarm_bitmap[] =
{ B00011000,
  B00111100,
  B01100110,
  B01000010,
  B01000010,
  B11000011,
  B11111111,
  B00011000
};

const unsigned char PROGMEM countDown_bitmap[] =
{ B00000000,
  B01111100,
  B01000100,
  B00101000,
  B00010000,
  B00101000,
  B01000100,
  B01111100
};

const unsigned char PROGMEM pause_bitmap[] =
{B00000000,
B01100110,
B01100110,
B01100110,
B01100110,
B01100110,
B01100110,
B00000000};

const unsigned char PROGMEM resume_bitmap[] =
{B00000000,
B00010000,
B00011000,
B00011100,
B00011110,
B00011100,
B00011000,
B00010000};


//data
static unsigned char*  numbers[10] = {num0, num1, num2, num3, num4,num5,num6,num7,num8,num9};
int digitArr[4]={0,0,0,0};
int state = 0;//0=set time; 1=countdown; 2=alart;
int countDownData[6] = {0, 0, 0, 0, 0}; //minute,second,off/on/pause,current minute, current second
unsigned long countDownTimer = millis();
unsigned long aniTimer = millis();
int aniState=0;
int tempA;
int tempB;
int buzzPin = 9;
int brightness = 1;
int shortCut[2]={25,5};
int autoPowerOffTime = 10;//10 minutes;
unsigned long idleTimer = millis();
//key
int btnSet = 2;
int btnLeft = 4;
int btnRight = 3;
int btnA = 5;//5 minute shortcut
int btnB = 6;//25 minute shortcut
int power = 7;

String key = "0";
char hexaKeys[] = {'L', 'R', 'S', 'A', 'B'};
int keyArray[] = {btnLeft, btnRight, btnSet, btnA, btnB}; //start, -, +
int lastKey;
unsigned long btnTimer[5];

int btnState[5]={0,0,0,0,0}; //0->no press 1-> new press; 2-> short press; 3-> long press;
int longPressTime[5]={800,800,2000,800,800};


 void setup() { 
  pinMode(A0, INPUT);
  analogReference(INTERNAL);
  pinMode(power,OUTPUT);
  digitalWrite(power, HIGH);
  Serial.begin(9600);
  matrix.setIntensity(brightness);
  matrix.setRotation(0, 2);
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  pinMode(btnSet, INPUT_PULLUP);
  pinMode(btnA, INPUT_PULLUP);
  pinMode(btnB, INPUT_PULLUP);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);  
  delay(1000);
 } 

 void loop() { 
  drawDisplay();
  delay(500);
}


void drawDisplay(){


  int sensorValue = analogRead(A0);    // Read Analog Value               // Calculate Battery Level (Percent)
  int pcnt = map(sensorValue, 900,1023,0,100);
  if(pcnt>=100){
    countDownData[3]=1;
    countDownData[4]=0;
  }else{
    countDownData[3]=0;
    countDownData[4] = pcnt;
    }
  countDownData[4] = pcnt;
  /*
  Serial.print("raw data: ");
  Serial.print(sensorValue);
  Serial.print("  new data: ");
  Serial.println(pcnt);
  */
  digitFormate();
  matrix.fillScreen(LOW);
  
  if(1){//digit mode
    if(countDownData[3]==0){//less than 1 minute,display second
      for(int i=0;i<3;i++){
        for(int n=0;n<8;n++){
          matrix.drawPixel(i+tempA,n,bitRead(numbers[digitArr[2]][n],2-i));  
        }
      }
      for(int i=0;i<3;i++){
        for(int n=0;n<8;n++){
          matrix.drawPixel(i+5+tempB,n,bitRead(numbers[digitArr[3]][n],2-i));  
        }
      }  
    }else{//greater than 1 minute,display minute
      for(int i=0;i<3;i++){
        for(int n=0;n<8;n++){
          matrix.drawPixel(i+tempA,n,bitRead(numbers[digitArr[0]][n],2-i));  
        }
      }
      for(int i=0;i<3;i++){
        for(int n=0;n<8;n++){
          matrix.drawPixel(i+5+tempB,n,bitRead(numbers[digitArr[1]][n],2-i));  
        }
      }  
    }
    
    matrix.write(); // Send bitmap to display  
  }else{//animation mode
    
  }
}

void digitFormate(){
  if(countDownData[3]<10){
    digitArr[0]=0;
    digitArr[1]=countDownData[3];
  }else{
    digitArr[1]=countDownData[3]%10;
    digitArr[0]=(countDownData[3]/10)%10;
  }
  
  if(countDownData[4]<10){
    digitArr[2]=0;
    digitArr[3]=countDownData[4];
  }else{
    digitArr[3]=countDownData[4]%10;
    digitArr[2]=(countDownData[4]/10)%10;
  }
}
