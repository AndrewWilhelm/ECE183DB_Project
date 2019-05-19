
int PWMA=D1;//Right side 
int PWMB=D2;//Left side 
int DA=D3;//Right reverse 
int DB=D4;//Left reverse 


void turnRight();
void turnLeft();
void moveF();
void stopMove();
const int rightPWM = 800;//The analogWrite uses values from 0 to 1024
const int leftPWM = 800;
 
void setup(){ 
/*
for(int i = 0; i < pinsLength; i++) {
  pins[i] = i;
  pinMode(i, OUTPUT);
}*/
 // Debug console 
 Serial.begin(9600); 
 pinMode(PWMA, OUTPUT); 
 pinMode(PWMB, OUTPUT); 
 pinMode(DA, OUTPUT); 
 pinMode(DB, OUTPUT); 

delay(3000); 
} 
 
 
 
void loop(){ 
  moveF();
  delay(1500);
  stopMove();
  delay(1000);
  turnRight();
  delay(1500);
  stopMove();
  delay(1000);
  turnLeft();
  delay(1500);
  stopMove();
  delay(1000);
  //Forward 
/*    digitalWrite(PWMA, HIGH); 
    digitalWrite(DA, HIGH); 
      
    digitalWrite(PWMB, HIGH); 
    digitalWrite(DB, LOW);   */

 
} 
 
void moveF(){
    analogWrite(PWMA, rightPWM); 
    digitalWrite(DA, HIGH); 
      
    analogWrite(PWMB, leftPWM); 
    digitalWrite(DB, LOW);  
}

void turnRight() {
    analogWrite(PWMA, rightPWM); 
    digitalWrite(DA, LOW);//go in reverse 
      
    analogWrite(PWMB, leftPWM); 
    digitalWrite(DB, LOW); 
}

 void turnLeft() {
    analogWrite(PWMA, rightPWM); 
    digitalWrite(DA, HIGH); 
      
    analogWrite(PWMB, leftPWM); 
    digitalWrite(DB, HIGH);//go in reverse 
}

void stopMove() {
    analogWrite(PWMA, 0); 
    digitalWrite(DA, HIGH); 
      
    analogWrite(PWMB, 0); 
    digitalWrite(DB, LOW);}

