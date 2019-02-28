//https://www.instructables.com/id/Arduino-Dual-Function-Button-Long-PressShort-Press/

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
  Serial.begin(9600);
  pinMode(btnLeft, INPUT_PULLUP);
  pinMode(btnRight, INPUT_PULLUP);
  pinMode(btnSet, INPUT_PULLUP);
  pinMode(btnA, INPUT_PULLUP);
  pinMode(btnB, INPUT_PULLUP);
}

void loop() {
  getKey();
  Serial.print("[Key]: ");
  Serial.print(key);
  Serial.print(" [state]: ");
  Serial.print(btnState[lastKey]);

  delay(100);
  key = "0";

  Serial.println(" ");
}


void getKey(){
  for(int i=0; i<5; i++){
    if(digitalRead(keyArray[i])==0){//btn actived!
      lastKey=i;
      if(btnState[i] == 0){//new press
        btnState[i] = 1;
        btnTimer[i] = millis();//start the timer 
        //Serial.print(" new press! "); 
      }
      if(millis()-btnTimer[i] > longPressTime[i]){//long press for sure
        //Serial.print(" long press "); 
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
