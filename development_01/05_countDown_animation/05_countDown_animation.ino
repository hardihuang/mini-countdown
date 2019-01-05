#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>

 bool btnA=0;
 bool btnB=0;
 int count = 0; 
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

//data
static unsigned char*  numbers[10] = {num0, num1, num2, num3, num4,num5,num6,num7,num8,num9};
int digitArr[2]={0,0};
int state = 0;//0=set time; 1=countdown; 2=alart;
int countDownData[6] = {25, 0, 0, 25, 0}; //minute,second,off/on/pause,current minute, current second
unsigned long countDownTimer = millis();
unsigned long aniTimer = millis();
int aniState=0;
int tempA;
int tempB;
  
 void setup() { 
  Serial.begin(9600);
  matrix.setIntensity(15);
  matrix.setRotation(0, 1);
  pinMode (6,INPUT);//A
  pinMode (7,INPUT);//B
  pinMode (2,INPUT_PULLUP);
  pinMode (3,INPUT_PULLUP);
  aLastState = digitalRead(6);   
 } 

 void loop() { 
  getKey();

  Serial.print(state);
  Serial.print(" || ");
  Serial.print(countDownData[3]);
  Serial.print(" || ");
  Serial.println(countDownData[4]);
  
  //control flow
  drawDisplay();
  switch(state){
    case 0://set time mode
      if(btnA == 0 and countDownData[2] == 0){
        state = 1;  
        countDownData[2]=1;
      }
      countDownData[3]=countDownData[0];
      countDownData[4]=countDownData[1];
    break;
    case 1://start countdown mode
      if(countDownData[2]==1){
        if(millis() - countDownTimer >= 1000){
          if(countDownData[4]>0){
            countDownData[4]--;
          }else{
            if(countDownData[3]>0){
              countDownData[3]--;
              countDownData[4]=59;
            }else{//time's up
              matrix.fillScreen(LOW);
              matrix.setIntensity(15);
              scrollMessage("time's up");
              state = 0;
              countDownData[2]=0;
              countDownData[3]=countDownData[0];
              countDownData[4]=countDownData[1];
            }
          }
          countDownTimer = millis();
        }
      }
      
    break;
    case 2://alart mode

    break;
  }
}

void getKey(){
  btnA = digitalRead(2);
  btnB = digitalRead(3);
  aState = digitalRead(6);
   if (aState != aLastState){     
     if (digitalRead(7) != aState) { 
      if(countDownData[0]<90){
        countDownData[0]++;
      }  
     } else {
      if(countDownData[0]>0){
        countDownData[0] --;
       }
     }
   } 
   aLastState = aState;
   
}
void drawDisplay(){
  digitFormate();
  matrix.fillScreen(LOW);
  if(countDownData[2] == 1){
    
    if(millis()-aniTimer>=500){
      switch(aniState){
        case 0:
          tempA = 0;
          tempB = 0;
          aniState++;
        break;
        case 1:
          tempA = 1;
          tempB = 0;
          aniState++;
        break;
        case 2:
          tempA = 0;
          tempB = 0;
          aniState++;
        break;
        case 3:
          tempA = 0;
          tempB = -1;
          aniState=0;
        break;
      }
      aniTimer = millis();
    }
    
  }else{
    tempA = 0;
    tempB = 0;  
  }
  
  if(1){//digit mode
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
    matrix.write(); // Send bitmap to display  
  }else{//animation mode
    
  }
  
  
  
}

void digitFormate(){
  if(countDownData[0]<10){
    digitArr[0]=0;
    digitArr[1]=countDownData[3];
  }else{
    digitArr[1]=countDownData[3]%10;
    digitArr[0]=(countDownData[3]/10)%10;
  }
}


void scrollMessage(String msg) {
  msg += " "; // add a space at the end
  for ( int i = 0 ; i < 6 * msg.length() + matrix.width() - 1 - 1; i++ ) {
    int letter = i / 6;
    int x = (matrix.width() - 1) - i % 6;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + 6 - 1 >= 0 && letter >= 0 ) {
      if ( letter < msg.length() ) {
        matrix.drawChar(x, y, msg[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= 6;
    }
    matrix.write(); // Send bitmap to display
    delay(35);
  }
  matrix.setCursor(0, 0);
}
