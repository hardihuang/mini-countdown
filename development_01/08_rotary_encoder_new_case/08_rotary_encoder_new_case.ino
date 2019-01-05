#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <EEPROM.h>
#include <RotaryEncoder.h>

RotaryEncoder encoder(A2, A1);
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
int buzzPin = 7;
int brightness = 1;

//key
int btnSet = 2;
int btnLeft = 4;
int btnRight = 3;
int btnA = 5;//5 minute shortcut
int btnB = 6;//25 minute shortcut

int debounce[5];
String key = "0";
char hexaKeys[] = {'L', 'R', 'S', 'A', 'B'};
int keyArray[] = {btnLeft, btnRight, btnSet, btnA, btnB}; //start, -, +

 void setup() { 
  Serial.begin(9600);
  matrix.setIntensity(brightness);
  matrix.setRotation(0, 1);
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  pinMode(btnSet, INPUT_PULLUP);
  pinMode(btnA, INPUT_PULLUP);
  pinMode(btnB, INPUT_PULLUP);
  //greating();
  fetchCountDownData();
 } 

 void loop() { 
  static int pos = 0;
  encoder.tick();
  int newPos = encoder.getPosition();
  if (pos != newPos) {
    Serial.print(newPos);
    Serial.println();
    pos = newPos;
  }
  
  getKey();
  Serial.print(" || CD3: ");
  Serial.print(countDownData[3]);
  Serial.print(" || CD4: ");
  Serial.print(countDownData[4]);
  Serial.print(" || DR0: ");
  Serial.print(digitArr[0]);
  Serial.print(" || DR1: ");
  Serial.print(digitArr[1]);
  Serial.print(" || DR2: ");
  Serial.print(digitArr[2]);
  Serial.print(" || DR3: ");
  Serial.println(digitArr[3]);
  
  //control flow
  drawDisplay();
  switch(state){
    case 0://set time mode
      if(key == "S"){
        if(countDownData[2] == 0){
          state = 1;  
          countDownData[2]=1;
        }
        
      }else if(key == "L"){
        if(countDownData[0]>0){
          countDownData[0]--;
          writeCountDownData();
        }
      }else if(key == "R"){
        if(countDownData[0]<90){
          countDownData[0]++; 
        }else{
          countDownData[0]=0;  
        }
        writeCountDownData();
      }else if(key == "A"){
        countDownData[0]=5;  
        
      }else if(key == "B"){
        countDownData[0]=25;  
      }
      countDownData[1]=0;
      countDownData[3]=countDownData[0];
      countDownData[4]=countDownData[1];
    break;
    case 1://start countdown mode
      if(key == "S"){
        if(countDownData[2] == 0){
          countDownData[2] = 1;
        }else{
          countDownData[2] = 0;
        }
      }else if(key == "L"){
        if(countDownData[2] == 0){
          state = 0;  
          countDownData[3]=countDownData[0];
          countDownData[4]=countDownData[1];
          delay(100);
        }
      }
      
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
              matrix.setIntensity(10);

              for(int i=0;i<3;i++){
                matrix.fillScreen(LOW);
                //matrix.drawBitmap(0,0,setAlarm_bitmap,8,8,0);
                matrix.write();
                tone(buzzPin, 415, 500);
                delay(500);
                
                matrix.fillScreen(LOW);
                matrix.drawBitmap(0,0,setAlarm_bitmap,8,8,1);
                matrix.write();
                noTone(buzzPin);
                delay(500);
              }
              matrix.fillScreen(LOW);
              matrix.write();
              matrix.setIntensity(brightness);

              
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
  key = "0";
  delay(100);
}

void getKey() {
  for (int i = 0; i < 5; i++) {
    if (digitalRead(keyArray[i]) == 0) {
      if (debounce[i] == 0) {
        key = hexaKeys[i];
        digitalWrite(buzzPin, HIGH);
        delay(1);
        digitalWrite(buzzPin, LOW);
        debounce[i] = 1;
      } else {
        debounce[i] -= 1;
      }
    } else {
      debounce[i] = 0;
    }
  }
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



void greating() {
  matrix.fillScreen(LOW);
  matrix.drawBitmap(0,0,countDown_bitmap,8,8,1);
  matrix.write();
  tone(buzzPin, 415, 500);
  tone(buzzPin, 415, 500);
  delay(500 * 1.3);
  tone(buzzPin, 466, 500);
  delay(500 * 1.3);
  tone(buzzPin, 370, 1000);
  delay(1000 * 1.3);
  noTone(buzzPin);

  matrix.fillScreen(LOW); // show black
  matrix.write();
  delay(500);
}

void writeCountDownData() {
  EEPROM.write(0, countDownData[0]);
}
void fetchCountDownData() {
  countDownData[0] = EEPROM.read(0);
  countDownData[1] = 0;
}


