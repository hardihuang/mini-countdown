#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <EEPROM.h>

//matrix display 
int pinCS = 10; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

//battery level
#define VMIN 3.6  // Battery Level 0%
#define VMAX 4.2  // Batter Level 100%
#define VBAT_PER_BITS 0.0041056  // Volts per bit on the A0 pin, VMAX / 1023 = 3.44 / 1023 = 0.0033625
#define BATTERY_SENSE_PIN A2

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
int buzzPin = 3;
int brightness = 1;
int shortCut[2]={25,5};
int autoPowerOffTime = 10;//10 minutes;
unsigned long idleTimer = millis();
//key
int btnSet = 2;
int btnLeft = 8;
int btnRight = 4;
int btnA = 5;//5 minute shortcut
int btnB = 7;//25 minute shortcut
int btnC = 6;//battery percentage and mode change
int power = 9;

String key = "0";
char hexaKeys[] = {'L', 'R', 'S', 'A', 'B', 'C'};
int keyArray[] = {btnLeft, btnRight, btnSet, btnA, btnB, btnC}; //start, -, +
int lastKey;
unsigned long btnTimer[6];

int btnState[6]={0,0,0,0,0,0}; //0->no press 1-> new press; 2-> short press; 3-> long press;
int longPressTime[6]={800,800,2000,800,800,800};


 void setup() { 
  pinMode(power,OUTPUT);
  pinMode(A2, INPUT);
  analogReference(INTERNAL);
  digitalWrite(power, HIGH);
  Serial.begin(9600);
  matrix.setIntensity(brightness);
  matrix.setRotation(0, 2);
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  pinMode(btnSet, INPUT_PULLUP);
  pinMode(btnA, INPUT_PULLUP);
  pinMode(btnB, INPUT_PULLUP);
  pinMode(btnC, INPUT_PULLUP);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);  
  //greating();
  fetchCountDownData();
  delay(1000);
 } 

 void loop() { 
  getKey();
  autoPowerOff();
  /*
  Serial.print("[Key]: ");
  Serial.print(key);
  Serial.print(" [state]: ");
  Serial.print(btnState[lastKey]);
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
  */
  
  //control flow
  drawDisplay();
  switch(state){
    case 0://set time mode
      if(key == "S"){
        if(btnState[lastKey]==2){//short press
          if(countDownData[2] == 0){
            state = 1;
            countDownData[2]=1;
          }
        }else if(btnState[lastKey]==3){//power off
          //Serial.print(" power off ");
          matrix.fillScreen(LOW);
          matrix.write();
          delay(3000);
          digitalWrite(power, LOW);
        }
      }else if(key == "L"){
        if(btnState[lastKey]==2){
          if(countDownData[0]>0){
            countDownData[0]--;
            writeCountDownData();
          }else{
            countDownData[0] = 90;
          }
        }else if(btnState[lastKey]==3){
          if(countDownData[0]>5){
            countDownData[0]-=5;
            writeCountDownData();
          }else{
            countDownData[0] = 90;
          }
        }
        writeCountDownData();
      }else if(key == "R"){
        if(btnState[lastKey]==2){
          if(countDownData[0]<90){
            countDownData[0]++; 
          }else{
            countDownData[0]=0;  
          }
        }else if(btnState[lastKey]==3){
          if(countDownData[0]<85){
            countDownData[0]+=5; 
          }else{
            countDownData[0]=0;  
          }
        }
        writeCountDownData();
      }else if(key == "A"){
        if(btnState[lastKey]==2){//fetch data
          countDownData[0]=shortCut[0];       
        }else if(btnState[lastKey]==3){//write new shortCut data
          shortCut[0] = countDownData[0];
          writeCountDownData();
          blink3();
        }
        
      }else if(key == "B"){
        if(btnState[lastKey]==2){//fetch data
          countDownData[0]=shortCut[1];       
        }else if(btnState[lastKey]==3){//write new shortCut data
          shortCut[1] = countDownData[0];
          writeCountDownData();
          blink3();
        }
      }else if(key == "C"){
        batteryInfo();
      }
      
      countDownData[1]=0;
      countDownData[3]=countDownData[0];
      countDownData[4]=countDownData[1];
    break;
    case 1://start countdown mode
      if(key == "S"){
        if(countDownData[2] == 0){//resume
          matrix.fillScreen(LOW);
          matrix.drawBitmap(0,0,resume_bitmap,8,8,1);
          matrix.write();
          delay(500);
          countDownData[2] = 1;
        }else{//pause
          matrix.fillScreen(LOW);
          matrix.drawBitmap(0,0,pause_bitmap,8,8,1);
          matrix.write();
          delay(500);
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
              idleTimer = millis();
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
  delay(50);
  //Serial.println(" ");
}

void getKey(){
  for(int i=0; i<6; i++){
    if(digitalRead(keyArray[i])==0){//btn actived!
      lastKey=i;
      if(btnState[i] == 0){//new press
        btnState[i] = 1;
        btnTimer[i] = millis();//start the timer 
        //Serial.print(" new press! "); 
      }
      if(millis()-btnTimer[i] > longPressTime[i]){//long press for sure
        //Serial.print(" long press "); 
        digitalWrite(buzzPin, HIGH);
        delay(1);
        digitalWrite(buzzPin, LOW);
        btnState[i] = 3;
        key = hexaKeys[i];
        btnTimer[i] = millis();
        longPressTime[i] = 300;
      }else{
        if(btnState[i] == 3)  {
          key = "0"  ;
        }
      }
    }else{
      if(btnState[i] == 1 and key == "0")  {
        //Serial.print(" short press "); 
        digitalWrite(buzzPin, HIGH);
        delay(1);
        digitalWrite(buzzPin, LOW);
        key = hexaKeys[lastKey];
        btnState[i] = 2;
      }else{
        btnState[i] = 0;
        btnTimer[i] = 0;  
      }
      if(i==2){
        longPressTime[i] = 2000;//power off long press delay
      }else{
        longPressTime[i] = 800;
      }
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
  EEPROM.write(1, shortCut[0]);
  EEPROM.write(2, shortCut[1]);
}

void fetchCountDownData() {
  countDownData[0] = EEPROM.read(0);
  countDownData[1] = 0;
  shortCut[0] = EEPROM.read(1);
  shortCut[1] = EEPROM.read(2);
}

void blink3(){
  for(int i=0; i<3; i++){
    matrix.fillScreen(LOW);
    matrix.write();
    delay(200);
    drawDisplay();
    delay(200);
  }  
}

void autoPowerOff(){
  if(key!="0"){
    idleTimer = millis();  
  }else if(millis()-idleTimer > autoPowerOffTime*60000){
    if(state == 0 || countDownData[2]==0){
      digitalWrite(power,LOW)  ;
    }
  }
}

void batteryInfo(){
  matrix.fillScreen(LOW);
  matrix.write();
  int sensorValue = analogRead(A2);    // Read Analog Value               // Calculate Battery Level (Percent)
  int sum;
  for(int i=0;i<5;i++){
    sum += map(sensorValue, 900,1023,0,100);
    delay(100);
  }

  int pcnt = sum/5;
  
  if(pcnt>=100){
    countDownData[3]=1;
    countDownData[4]=0;
  }else{
    countDownData[3]=0;
    countDownData[4] = pcnt;
    }
  countDownData[4] = pcnt;
  digitFormate();
  matrix.fillScreen(LOW);
  

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
  delay(2000);

}
