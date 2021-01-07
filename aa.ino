#include <SoftwareSerial.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "LedControl.h"
#define RX 10
#define TX 11
#define dht_dpin 7
#define multiplier 0.185

static const uint8_t PIN_MP3_TX = 2; // Connects to module's RX 
static const uint8_t PIN_MP3_RX = 3; // Connects to module's TX

DHT dht(dht_dpin, DHT11);
LedControl lc=LedControl(12,8,9,1); 
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;

String AP = "laugh";
String PASS = "hahahaha";
String API = "7PFIYGZ50G17N4T6";
String HOST = "api.thingspeak.com";
String PORT = "80";
String field1 = "field1";
String field2 = "field2";
String field3 = "field3";
int countTrueCommand;
int countTimeCommand;
boolean found = false; 
int x,y,count = 0;
float temp = 0.0;
float flag = 0.0;
SoftwareSerial esp8266(RX,TX); 

void setup() {
  Serial.begin(9600);
  softwareSerial.begin(9600);
  esp8266.begin(115200);
  
  player.begin(softwareSerial);
  player.volume(50);

  lc.shutdown(0,false);// turn off power saving, enables display
  lc.setIntensity(0,8);// sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}

void loop() {
  x = dht.readHumidity();
  y = dht.readTemperature();
  
  float SensorRead = analogRead(A0)*(5.0 / 1023.0);     //We read the sensor output  
  float Current = (SensorRead-2.5)/multiplier;          //Calculate the current value

  int index = random()%3 + 1;
  Current = abs(Current);
  Serial.print("Current: ");
  Serial.println(Current); 
  flag = Current - temp; 
  Serial.print("flag: ");
  Serial.println(flag); 
  temp = Current;
 
  String getData = "GET /update?api_key="+ API +"&"+ field1 +"="+ String(x) +"&"+ field2 +"="+ String(y)+"&"+ field3 +"="+ String(Current);
  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
  esp8266.println(getData);delay(1500);countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0",5,"OK");
 
  if(abs(flag) > 1.5 || count == 5){
    lc.clearDisplay(0);// clear screen
    lc.setLed(0,1,1,true); 
    lc.setLed(0,1,2,true);
    lc.setLed(0,0,7,true);
    lc.setLed(0,1,6,true);
    lc.setLed(0,2,1,true);
    lc.setLed(0,2,2,true);
    lc.setLed(0,2,5,true);
    lc.setLed(0,3,5,true);
    lc.setLed(0,4,5,true);
    lc.setLed(0,5,5,true);
    lc.setLed(0,5,1,true);
    lc.setLed(0,5,2,true);
    lc.setLed(0,6,1,true);
    lc.setLed(0,6,2,true);
    lc.setLed(0,6,6,true);
    lc.setLed(0,7,7,true);
    player.play(1);
    delay(50000);
    Serial.println("bigger");
    if(count == 5){
      count = 0;
    }
  }
  else{
    Serial.println("less"); 
    player.pause();
    lc.clearDisplay(0);// clear screen
    lc.setLed(0,1,1,true); // turns on LED at col, row
    lc.setLed(0,1,2,true);
    lc.setLed(0,0,4,true);
    lc.setLed(0,1,5,true);
    lc.setLed(0,2,1,true);
    lc.setLed(0,2,2,true);
    lc.setLed(0,2,6,true);
    lc.setLed(0,3,6,true);
    lc.setLed(0,4,6,true);
    lc.setLed(0,5,6,true);
    lc.setLed(0,5,1,true);
    lc.setLed(0,5,2,true);
    lc.setLed(0,6,1,true);
    lc.setLed(0,6,2,true);
    lc.setLed(0,6,5,true);
    lc.setLed(0,7,4,true);
    count++;
  }
}
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1)){
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay)){
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if(found == true){
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if(found == false){
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
 }
