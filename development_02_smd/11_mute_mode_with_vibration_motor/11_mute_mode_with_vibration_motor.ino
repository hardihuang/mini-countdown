/*
[update log]
------------------------------------------------
  V1.1.0 Mar/1/2019 19:38 battery logo display battery level
  V1.2.0 Mar/1/2019 13:14 added countdown animation mode, with blinking last dot
  V1.2.1 Mar/2/2019 13:50 digite mode second countdown small font
  V1.2.2 Mar/2/2019 16:05 auto check battery  when boot up, and alert user if the product battery is low
  V1.2.3 Mar/2/2019 18:23 countdown animation blinking dot change speed based on the time length
  V1.2.4 Mar/2/2019 21:15 connect/disconnect charging cable charging animation alart //hardware update: A1 pull-down to usb port Vcc
  V1.2.5 Mar/3/2019 14:06 user can check batt level while pausing
  V1.2.6 Mar/4/2019 08:16 toggle mute mode while puasing


*/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <EEPROM.h>

//matrix display 
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

// small numbers
static const unsigned char snum0[] =
{B000,
B000,
B000,
B111,
B101,
B101,
B101,
B111};

static const unsigned char snum1[] =
{B000,
B000, 
B000,
B010,
B010,
B010,
B010,
B010};

static const unsigned char snum2[] =
{B000,
B000,
B000,
B111,
B001,
B111,
B100,
B111};

static const unsigned char snum3[] =
{B000,
B000,
B000,
B111,
B001,
B111,
B001,
B111};

static const unsigned char snum4[] =
{B000,
B000,
B000,
B101,
B101,
B111,
B001,
B001};

static const unsigned char snum5[] =
{B000,
B000,
B000,
B111,
B100,
B111,
B001,
B111};

static const unsigned char snum6[] =
{B000,
B000,
B000,
B111,
B100,
B111,
B101,
B111};

static const unsigned char snum7[] =
{B000,
B000,
B000,
B111,
B001,
B001,
B001,
B001};

static const unsigned char snum8[] =
{B000,
B000,
B000,
B111,
B101,
B111,
B101,
B111};

static const unsigned char snum9[] =
{B000,
B000,
B000,
B111,
B101,
B111,
B001,
B111};

//logo
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

const unsigned char PROGMEM speaker_bitmap[] =
{B00000000,
B00100010,
B01101001,
B11100101,
B11100101,
B01101001,
B00100010,
B00000000};

const unsigned char PROGMEM mute_bitmap[] =
{B00100010,
B01000100,
B10001000,
B01000100,
B00100010,
B00010001,
B00100010,
B01000100};

//data
static unsigned char*  numbers[10] = {num0, num1, num2, num3, num4,num5,num6,num7,num8,num9};
static unsigned char*  snumbers[10] = {snum0, snum1, snum2, snum3, snum4,snum5,snum6,snum7,snum8,snum9};
int digitArr[4]={0,0,0,0};
int state = 0;//0=set time; 1=countdown; 2=alart;
int countDownData[6] = {0, 0, 0, 0, 0}; //minute,second,off/on/pause,current minute, current second
bool mode = 1;// 0=animation| 1=digit
bool mute=0;
unsigned long countDownTimer = millis();
unsigned long aniTimer = millis();
int aniState=0;
int tempA;
int tempB;
int buzzPin = 3;
int vibMotor = A5;
int brightness = 1;
int shortCut[2]={25,5};
int autoPowerOffTime = 5;//10 minutes;
unsigned long idleTimer = millis();
//key
int btnSet = 2;
int btnLeft = 4;
int btnRight = 8;
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

unsigned long battlevelTimer = millis()-300000;//offset to check when boot-up, if power low, alart the user when they turn on the machine
int pcnt;//battery percentage


//countdown animation
double intervalSpeed;
unsigned int pn;
int row;
int left;
unsigned long blinkDotTimer = millis();
bool blinkDot=1;
bool chargingState = 0;// 0=unplug;1=charging
bool chargingFlag = 0;


void setup() { 
  pinMode(power,OUTPUT);
  digitalWrite(power, HIGH);
    
  pinMode(vibMotor, OUTPUT);
  digitalWrite(vibMotor,LOW);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, LOW);  

  fetchCountDownData();
    
  matrix.setIntensity(brightness);
  matrix.setRotation(0, 0);
  greating();
  
  pinMode(A2, INPUT);//battery level
  pinMode(A1, INPUT);//charging state
  analogReference(INTERNAL);

  //Serial.begin(9600);
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  pinMode(btnSet, INPUT_PULLUP);
  pinMode(btnA, INPUT_PULLUP);
  pinMode(btnB, INPUT_PULLUP);
  pinMode(btnC, INPUT_PULLUP);


  

  //solve boot-up display 00 second first and then switch to real data bug
  countDownData[2]=0;
  countDownData[3]=countDownData[0];
  countDownData[4]=countDownData[1];
 } 

 void loop() { 
  getKey();
  autoPowerOff();
  checkBatt();

  //control flow
  drawDisplay();
  switch(state){
    case 0://set time mode
      if(key == "S"){
        if(btnState[lastKey]==2){//short press
          if(countDownData[2] == 0){
            state = 1;
            countDownData[2]=1;
            
            matrix.fillScreen(LOW);
            matrix.drawBitmap(0,0,resume_bitmap,8,8,1);
            matrix.write();
            delay(200);
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
          if(countDownData[0]>1){
            countDownData[0]--;
            writeCountDownData();
          }else{
            countDownData[0] = 90;
          }
        }else if(btnState[lastKey]==3){
          if(countDownData[0]>6){
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
            countDownData[0]=1;  
          }
        }else if(btnState[lastKey]==3){
          if(countDownData[0]<85){
            countDownData[0]+=5; 
          }else{
            countDownData[0]=1;  
          }
        }
        writeCountDownData();
      }else if(key == "A"){
        if(btnState[lastKey]==2){//fetch data
          countDownData[0]=shortCut[0];       
          writeCountDownData();
        }else if(btnState[lastKey]==3){//write new shortCut data
          shortCut[0] = countDownData[0];
          writeCountDownData();
          blink3();
        }
        
      }else if(key == "B"){
        if(btnState[lastKey]==2){//fetch data
          countDownData[0]=shortCut[1];      
          writeCountDownData(); 
        }else if(btnState[lastKey]==3){//write new shortCut data
          shortCut[1] = countDownData[0];
          writeCountDownData();
          blink3();
        }
      }else if(key == "C"){
        if(btnState[lastKey]==2){//short press check battery info
          batteryInfo();
          battlevelTimer = millis();//if user checked battery menually, then we reset the timer for auto-checking battery level
        }else if(btnState[lastKey]==3){//long press to toggle mute mode
          muteToggle();
        }
        
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
          battlevelTimer = millis();//reset the auto-checking batt timer
        }
      }else if(key == "L"){
        if(countDownData[2] == 0){
          state = 0;  
          countDownData[3]=countDownData[0];
          countDownData[4]=countDownData[1];
          delay(100);
        }
      }else if(key == "C"){
        if(countDownData[2] == 1){//toggle the mode while counting down
          mode = !mode;
          writeCountDownData();
        }else{
          if(btnState[lastKey]==2){//short press
            batteryInfo();
            battlevelTimer = millis();
          }else if(btnState[lastKey]==3){//long press
            muteToggle();  
          }
          
        }
        
      }
      
      if(countDownData[2]==1){      
        if( millis() - countDownTimer>= 1000){//offset some extra commant time
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
                
                if(mute){
                  digitalWrite(vibMotor,HIGH);
                  delay(200);
                  digitalWrite(vibMotor,LOW);
                }else{
                  tone(buzzPin, 415, 500);
                  delay(500);
                  noTone(buzzPin);
                }

                matrix.fillScreen(LOW);
                matrix.drawBitmap(0,0,setAlarm_bitmap,8,8,1);
                matrix.write();
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
    case 2:

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
        if(lastKey != 5){//do not vib or buzz is we are toggling the mute mode
          if(mute){
            digitalWrite(vibMotor,HIGH);
            delay(30);
            digitalWrite(vibMotor,LOW);
          }else{
            digitalWrite(buzzPin, HIGH);
            delay(1);
            digitalWrite(buzzPin, LOW);
          }
        }
        
        
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
        if(mute){
          digitalWrite(vibMotor,HIGH);
          delay(40);
          digitalWrite(vibMotor,LOW);
        }else{
          digitalWrite(buzzPin, HIGH);
          delay(1);
          digitalWrite(buzzPin, LOW);
        }
              
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
    intervalSpeed = countDownData[0]*(60000/64.0)+countDownData[1]*(1000/64.0);
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
  
  if(mode == 0 and countDownData[2]!=0){//if we are not stop and in animation mode
    matrix.fillScreen(LOW);
    
    pn = countDownData[3]*(60000/intervalSpeed) + countDownData[4]*(1000/intervalSpeed);//current pixel number, mind the order, change that might cause arithmetic overflow and get a negative number
    row = pn/8;
    left = pn % 8;
    if(row>0){
      matrix.fillRect(0,8-row,8,row,1);  
    }
    if(left>0){
      matrix.fillRect(0,7-row,left,1,1);   
    }
    
    //the blinking dot will change speed based on the time length
    //200 is the 1 minute blinking speed( top speed), 1000 is the 90 minutes blinking speed(slowest speed)
    if(millis()-blinkDotTimer>map(intervalSpeed,937,84375,200,1000)){ 
      blinkDot = !blinkDot;  
      blinkDotTimer = millis();
    }
    
    if(left+1>0){
      matrix.drawPixel(left,7-row,blinkDot);//blink last pixel
    }else{
      matrix.drawPixel(7,8-row,blinkDot);
    }
    matrix.write();
    
  }else{//digit mode
    if(countDownData[3]==0){//less than 1 minute,display second
      for(int i=0;i<3;i++){
        for(int n=0;n<8;n++){
          matrix.drawPixel(i+tempA,n,bitRead(snumbers[digitArr[2]][n],2-i));  
        }
      }
      for(int i=0;i<3;i++){
        for(int n=0;n<8;n++){
          matrix.drawPixel(i+5+tempB,n,bitRead(snumbers[digitArr[3]][n],2-i));  
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
    delay(40);
  }
  matrix.setCursor(0, 0);
}

void greating() {
  matrix.fillScreen(LOW);
  matrix.drawBitmap(0,0,countDown_bitmap,8,8,1);
  matrix.write();

   if(mute){
    digitalWrite(vibMotor,HIGH);
    delay(200);
    digitalWrite(vibMotor,LOW);
  }else{
    tone(buzzPin, 415, 100);
    delay(100 * 1.3);
    tone(buzzPin, 466, 100);
    delay(100 * 1.3);
    tone(buzzPin, 370, 200);
    delay(200 * 1.3);
    noTone(buzzPin);
  }
  matrix.fillScreen(LOW); // show black
  matrix.write();
}

void writeCountDownData() {
  EEPROM.write(0, countDownData[0]);
  EEPROM.write(1, shortCut[0]);
  EEPROM.write(2, shortCut[1]);
  EEPROM.write(3, mode);
  EEPROM.write(4, mute);
}

void fetchCountDownData() {
  countDownData[0] = EEPROM.read(0);
  countDownData[1] = 0;
  shortCut[0] = EEPROM.read(1);
  shortCut[1] = EEPROM.read(2);
  mode = EEPROM.read(3);
  if(mode>1){
    mode = 1;
  }
  mute = EEPROM.read(4);
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

void batteryReading(){
  int sensorValue = analogRead(A2);    // Read Analog Value               // Calculate Battery Level (Percent)
  int sum;
  for(int i=0;i<5;i++){
    sum += map(sensorValue, 900,1023,0,100);
    delay(90);
  }
  pcnt = sum/5;
}

void lowBattAlart(){
  matrix.fillScreen(LOW);
  matrix.drawRect(0,3,1,2,1);
  matrix.drawRect(1,2,7,4,1);
  matrix.write();
  delay(500);
  
  for(int i=0; i<2; i++){
    matrix.fillScreen(LOW);
    matrix.write();
    if(mute){
      digitalWrite(vibMotor,HIGH);
      delay(40);
      digitalWrite(vibMotor,LOW);
    }else{
      tone(buzzPin, 200, 50);
      delay(50);
    }
    
    delay(500);
    
    matrix.drawRect(0,3,1,2,1);
    matrix.drawRect(1,2,7,4,1);
    matrix.write();
    noTone(buzzPin);
    delay(500);
  }  
  delay(100);
  
  matrix.fillScreen(LOW);
  matrix.write();
  scrollMessage("Low battery");
  matrix.fillScreen(LOW);  
}

void batteryInfo(){
  
  batteryReading();
  matrix.fillScreen(LOW); 
  if(chargingState == 0){//if we are not charging with usb
    if(pcnt>0){
      matrix.drawRect(0,3,1,2,1);
      matrix.drawRect(1,2,7,4,1);
    
      if(pcnt>0 and pcnt<=20){
        matrix.drawRect(6,3,1,2,1);
      }else if(pcnt>20 and pcnt <=40){
        matrix.drawRect(5,3,2,2,1);
      }else if(pcnt>40 and pcnt <=60){
        matrix.drawRect(4,3,3,2,1);
      }else if(pcnt>60 and pcnt <=80){
        matrix.drawRect(3,3,4,2,1);
      }else if(pcnt >80 and pcnt <=100){
        matrix.drawRect(2,3,5,2,1);
      }
      
      matrix.write(); // Send bitmap to display  
      delay(1500);
    }else{//low power
      lowBattAlart();
    }
  }else{
    chargingAnimation();  
  }
}

void chargingAnimation(){
    matrix.fillScreen(LOW); 
    for(int i=0;i<2;i++){
      matrix.drawRect(0,3,1,2,1);
      matrix.drawRect(1,2,7,4,1);
      
      for(int j=0;j<6;j++){
        if(j==1){
          matrix.drawRect(6,3,1,2,1);
        }else if(j==2){
          matrix.drawRect(5,3,2,2,1);
        }else if(j==3){
          matrix.drawRect(4,3,3,2,1);
        }else if(j==4){
          matrix.drawRect(3,3,4,2,1);
        }else if(j==5){
          matrix.drawRect(2,3,5,2,1);
        }
        matrix.write(); 
        delay(200);
      }
      matrix.fillScreen(LOW); 
      matrix.write();
    }
    matrix.fillScreen(LOW); 
    matrix.write();
    delay(250);
}

void checkBatt(){
  chargingState=digitalRead(A1);
  if(chargingState){
    if(chargingFlag == 0)  {//just pluged in
      if(countDownData[2]==1){
        matrix.fillScreen(LOW);
        matrix.drawBitmap(0,0,pause_bitmap,8,8,1);
        matrix.write();
        delay(500);
        countDownData[2] = 0;
      }

      if(mute){
        digitalWrite(vibMotor,HIGH);
        delay(40);
        digitalWrite(vibMotor,LOW);
      }else{
        tone(buzzPin, 200, 100);//sound effect
        delay(100);
        tone(buzzPin, 600, 100);
        delay(100);
      }

      chargingAnimation();
      chargingFlag = 1;
    }
  }else{
    if(chargingFlag){//just unpluged
      if(countDownData[2]==1){
        matrix.fillScreen(LOW);
        matrix.drawBitmap(0,0,pause_bitmap,8,8,1);
        matrix.write();
        delay(500);
        countDownData[2] = 0;
      }

      if(mute){
        digitalWrite(vibMotor,HIGH);
        delay(40);
        digitalWrite(vibMotor,LOW);
      }else{
        tone(buzzPin, 600, 100);//sound effect
        delay(100);
        tone(buzzPin, 200, 100);
        delay(100);
      }
      
      batteryInfo();//display current battery level
      chargingFlag = 0;
    }else if(millis() - battlevelTimer>300000 and countDownData[2]!=1){//check battery status every 5 minutes
      batteryReading();
      if(pcnt<=0){
        lowBattAlart();
      }
      battlevelTimer = millis();
    }
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

void muteToggle(){
  mute = !mute;
  matrix.fillScreen(LOW);
  if(mute){
    matrix.drawBitmap(0,0,mute_bitmap,8,8,1);
    matrix.write();
    digitalWrite(vibMotor,HIGH);
    delay(40);
    digitalWrite(vibMotor,LOW);
    delay(100);
    digitalWrite(vibMotor,HIGH);
    delay(40);
    digitalWrite(vibMotor,LOW);
    delay(100);
    
  }else{
    matrix.drawBitmap(0,0,speaker_bitmap,8,8,1);
    matrix.write();
    digitalWrite(buzzPin, HIGH);
    delay(1);
    digitalWrite(buzzPin, LOW);
    delay(100);
    digitalWrite(buzzPin, HIGH);
    delay(1);
    digitalWrite(buzzPin, LOW);
    delay(100);
    
  }
  delay(1000);
  writeCountDownData();
  matrix.fillScreen(LOW);  
}
