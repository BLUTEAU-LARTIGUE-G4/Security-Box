#include <SoftwareSerial.h>
#include "SPIFFS.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "HardwareSerial.h"


SoftwareSerial ArduinoSlave(16,17);

// Replace with your network credentials
const char* ssid = "WIFI-LOZA";
const char* password = "";

// variable pour la reinitialisation du code
String code = String(1234);

// variables   
String etat;
String answer;
String msg;
int code_int=1234;
int boite=0;
int intVal1=0,oldIntVal1=0;
int intVal2=0,oldIntVal2=1234;


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String processor(const String& var){
  if(var == "STATE"){
    if(boite == 1){
      etat = "OUVERT";}
    
    else{
      etat = "FERME";}
    
    return etat;}

  if(var == "CODE"){
    return code;
  }
  return String();
}
 


void setup(){
  Serial.begin(9600);
  ArduinoSlave.begin(9600);

  SPIFFS.begin();

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  

  // Print ESP32 Local IP Address
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set the opening of the box
  server.on("/oui", HTTP_GET, [](AsyncWebServerRequest *request){
    boite = 1;

    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set the closing of the box
  server.on("/non", HTTP_GET, [](AsyncWebServerRequest *request){  
    boite = 0;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/code", HTTP_GET, [](AsyncWebServerRequest *request){
    String Code;
    Code = request->getParam("password")->value();
    char carray1[6]; //magic needed to convert string to a number
    Code.toCharArray(carray1, sizeof(carray1));
    code_int = atoi(carray1);
    code = Code;
    
    request->send(SPIFFS, "/index.html", String(), false, processor);
});

  // Start server
  server.begin();
                
}

void loop(){

  intVal1 = boite;
  intVal2 = code_int;

  //Send data to slave
  if(oldIntVal1!=intVal1 || oldIntVal2!=intVal2 ){
    Serial.print("Master sent : ");
    Serial.print(intVal1);
    Serial.print("x");
    Serial.println(intVal2);
    
    ArduinoSlave.print(intVal1);
    ArduinoSlave.print("x");
    ArduinoSlave.print(intVal2);
    oldIntVal1=intVal1;
    oldIntVal2=intVal2;
  }
  delay(1000);
  //Read answer from slave
  readSlavePort();
  
  //Send answer to monitor
  if(answer!=""){
    Serial.println("Slave received : ");
    Serial.println(answer);
    answer="";
  }
  
}

void readSerialPort(){
 while (Serial.available()) {
   delay(10);  
   if (Serial.available() >0) {
     char c = Serial.read();  //gets one byte from serial buffer
     msg += c; //makes the string readString
   }
 }
 Serial.flush();
}

void readSlavePort(){
  while (ArduinoSlave.available()) {
   delay(10);  
   if (ArduinoSlave.available() >0) {
     char c = ArduinoSlave.read();  //gets one byte from serial buffer
     answer += c; //makes the string readString
   }
 }
}
