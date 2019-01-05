
int btn=2;
int power = 3;
int timer = 0;
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
pinMode(btn,INPUT);
pinMode(power,OUTPUT);
digitalWrite(power, HIGH);
}

void loop() {
  Serial.println(timer);
  // put your main code here, to run repeatedly:
if(digitalRead(btn) == 0){
  timer++;
  if(timer>=300){
    
    timer = 0;
    digitalWrite(power, LOW);
    delay(4000);
  }
  }else{
    timer = 0;  
  }

}

