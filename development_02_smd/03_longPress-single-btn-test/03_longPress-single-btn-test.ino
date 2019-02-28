//https://www.instructables.com/id/Arduino-Dual-Function-Button-Long-PressShort-Press/

int btnState=0; //0->no press 1-> new press; 2-> short press; 3-> long press;
unsigned long btnTimer = 0;
int btn = 6;
int key = 0;
int longPressTime=800;

void setup() {
  Serial.begin(9600);
  pinMode(btn, INPUT_PULLUP);
}

void loop() {
  getKey();

  Serial.print(btnState);
  if(key){
    Serial.print(" || 1");
    if(btnState == 2){
      Serial.println(" || short Press || ");
    }else if(btnState == 3){
      Serial.println(" || long Press || ");
    }else{
      Serial.println(" || undefined || ");
    }
  }else{
    Serial.print(" || 0 || ");  
  }

  Serial.println(btnTimer);

  delay(50);
  key = 0;
}

void getKey(){
  if(digitalRead(btn) == 0){//btn actived!
    if(btnState == 0){ // new press
      btnState = 1;
      btnTimer = millis(); //start the timer
    }
    if(millis()-btnTimer > longPressTime){//long press for sure!
      btnState = 3;
      key = 1;
      btnTimer = millis();
      longPressTime = 500;
    }else{//long press debounce
      if(btnState == 3 ){
        key = 0; 
      }
    }
  }else{
    if(btnState == 1 and key == 0){
      key = 1;
      btnState = 2;
    }else{
      btnState = 0;
      btnTimer = 0;
    }
    longPressTime = 800;
  }
}
