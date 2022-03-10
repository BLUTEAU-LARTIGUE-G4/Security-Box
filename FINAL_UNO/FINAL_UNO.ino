#include <SoftwareSerial.h>
#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LIGNES 4
#define COLONNES 4
#define CODE 4

SoftwareSerial ArduinoMaster(0,1);

String msg="",boite="",code="";
int num1=-1,num2=-1;
int sep;
int acces=0;

// variables led
const int led_VERTE = 13;
const int led_ROUGE = 12;
const int led_ORANGE = 11;

const char kp4x4Keys[LIGNES][COLONNES]  = {
  {'1', '2', '3', 'A'}, {'4', '5', '6', 'B'}, {'7', '8', '9', 'C'}, {'*', '0', '#', 'D'}};
byte lignePin [4] = {9, 8, 7, 6};
byte colonnePin [4] = {5, 4, 3, 2};
char tab[CODE];
//char nouveau_code[]="";
char codebon[]= "1234";
char codeferme[]= "0000";
char codemanager[]= "ABCD";
int i=0;
int j=0;
int incomingByte;
int tentative=0;
int chance= 3;
LiquidCrystal_I2C lcd(0x27,16,2);
Servo servo_10;//servo branché sur l'entrée 10

//Variable keypad
Keypad kp4x4  = Keypad(makeKeymap(kp4x4Keys), lignePin, colonnePin, LIGNES, COLONNES);

void setup(){
  Serial.begin(9600);
  ArduinoMaster.begin(9600);

  servo_10.attach(10);

  pinMode(led_ORANGE, OUTPUT);
  pinMode(led_ROUGE, OUTPUT);
  pinMode(led_VERTE, OUTPUT);

  digitalWrite(led_ORANGE,LOW);
  digitalWrite(led_VERTE,LOW);
  digitalWrite(led_ROUGE,LOW);  

  lcd.init();
  lcd.cursor_on();
  lcd.blink_on();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("LOZA BOX");
  lcd.setCursor(0,1);
  lcd.print("Bienvenue");
  delay(3000);  
  lcd.init();
  lcd.cursor_on();
  lcd.blink_on();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Veuillez entrer");
  lcd.setCursor(0,1);
  lcd.print("votre code");
  lcd.cursor_off();
  lcd.blink_off();
  
}

void loop(){
  
  readMasterPort();
  convertMsgToMultiCmd();
  lecturechiffre();

  
  // Send answer to master
  if(num1!=-1 && num2!=-1){
    Serial.print("boite : " );
    Serial.println(num1);

    Serial.print("Code : " );
    Serial.println(num2);

    ArduinoMaster.print("Sensor 1 : " );
    ArduinoMaster.println(num1);

    ArduinoMaster.print("Sensor 2 : " );
    ArduinoMaster.println(num2);
    //nouveau_code = num2;
    Serial.println(boite);
    Serial.println(code);
    
    
    num1=-1;
    num2=-1;
  }

  
}


void lecturechiffre(){
  char transformechiffre = kp4x4.getKey(); //récupère le chiffre du keypad
  if (transformechiffre) {
    tab[i]=transformechiffre;//mets les chiffres à la suite des autres
    i=i+1;
    if(i==CODE){
      lcd.clear(); 
      lcd.setCursor(0,0);
      lcd.print("****");
      delay(2000);
      i=0;

      if((strcmp(tab,codebon)==0)){ //compare le code entré et celui attendu
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print("Code bon");
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print("En attente de ");
        lcd.setCursor(0,1);
        lcd.print("validation");
        digitalWrite(led_ORANGE,HIGH);
        delay(10000);
        readMasterPort();
        convertMsgToMultiCmd();
        if (acces==1){
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Acces autorise");
          servo_10.write(-90); //ouvre le loquet
          digitalWrite(led_ORANGE,LOW);
          digitalWrite(led_ROUGE,LOW);
          digitalWrite(led_VERTE,HIGH);
          delay(3000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Fermeture boite : ");
          lcd.setCursor(0,1);
          lcd.print("Entrez OOOO");
        }
        else{
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Acces rejete");
          digitalWrite(led_ORANGE,LOW);
          digitalWrite(led_ROUGE,HIGH);
          digitalWrite(led_VERTE,LOW);
          delay(3000);
          digitalWrite(led_ROUGE,LOW);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Veuillez entrer");
          lcd.setCursor(0,1);
          lcd.print("votre code");
          
        }
      }

      else if(strcmp(tab,codeferme)==0){ //compare le code entré et celui attendu (code de fermeture : ici 0000)
        digitalWrite(led_ORANGE,LOW);
        digitalWrite(led_VERTE,LOW);
        digitalWrite(led_ROUGE,LOW);
        servo_10.write(90); //ouvre le loquet
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print("Boite fermee");
        delay(3000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Veuillez entrer");
        lcd.setCursor(0,1);
        lcd.print("votre code");
      }

      else if(strcmp(tab,codemanager)==0){ //compare le code entré et celui attendu (code de fermeture : ici 0000)
        digitalWrite(led_ORANGE,LOW);
        digitalWrite(led_VERTE,LOW);
        digitalWrite(led_ROUGE,LOW);
        servo_10.write(-90); //ouvre le loquet
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print("Code manager");
        lcd.setCursor(0,1);
        lcd.print("Boite ouverte");
        delay(4000);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Fermeture boite : ");
        lcd.setCursor(0,1);
        lcd.print("Entrez OOOO");
      }
      
      else{
        tentative ++;
        chance --;
        lcd.clear(); 
        lcd.setCursor(0,0);
        lcd.print("Code faux, reste :");
        lcd.setCursor(0,1);
        lcd.print(chance);
        lcd.print(" essai(s)");
        digitalWrite(led_ROUGE, HIGH);
        delay(3000);
        digitalWrite(led_ROUGE, LOW);
        digitalWrite(led_VERTE, LOW);
        digitalWrite(led_ORANGE,LOW);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Veuillez entrer");
        lcd.setCursor(0,1);
        lcd.print("votre code");
        if(chance==0){
          lcd.clear(); 
          lcd.setCursor(0,0);
          lcd.print("Veuillez patien-");
          lcd.setCursor(0,1);
          lcd.print("-ter 20sec");
          delay(20000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Veuillez entrer");
          lcd.setCursor(0,1);
          lcd.print("votre code");
          chance = 3;
      }
   }
  }
  }
}


void readMasterPort(){
 while (ArduinoMaster.available()) {
   delay(10); 
   if (ArduinoMaster.available() >0) {
     char c = ArduinoMaster.read();  
     
     msg += c;
   }
 }
 ArduinoMaster.flush();
}


void convertMsgToMultiCmd(){
   if (msg.length() >0) {
     sep = msg.indexOf('x');
     boite = msg.substring(0, sep); 
     code = msg.substring(sep+1, msg.length()+1);
     
     char carray1[6]; //magic needed to convert string to a number
     boite.toCharArray(carray1, sizeof(carray1));
     num1 = atoi(carray1);
     acces = num1;

     char carray2[5];
     code.toCharArray(carray2, sizeof(carray2));
     num2 = atoi(carray2);
     const char* foo = code.c_str();
     strcpy(codebon, foo);
     Serial.println(code);
     Serial.println(codebon);
     //strcpy(codebon,code);
     
      
     msg="";
 }
}
