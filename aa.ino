 #include <SoftwareSerial.h>
#include <LiquidCrystal_PCF8574.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>
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
LiquidCrystal_PCF8574 lcd(0x27);  // 設定i2c位址，一般情況就是0x27和0x3F兩種
DFRobotDFPlayerMini player;

String AP = "laugh";
String PASS = "hahahaha";
String API = "7PFIYGZ50G17N4T6";
String HOST = "api.thingspeak.com";
String PORT = "80";
String field1 = "field1";
String field2 = "field2";
String field3 = "field3";
String field4 = "field4";
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
  player.volume(30);

  dht.begin();  //初始化DHT
  lcd.begin(16, 2); // 初始化LCD
  lcd.setBacklight(255);
  lcd.clear();
  
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

  lcd.clear();
  lcd.setCursor(0, 0);  //設定游標位置 (字,行)
  lcd.print("RH  :");  //Relative Humidity 相對濕度簡寫
  lcd.setCursor(7, 0);  
  lcd.print(x);
  lcd.setCursor(9, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);  //設定游標位置 (字,行)
  lcd.print("Temp:");
  lcd.setCursor(7, 1);  
  lcd.print(y);
  lcd.setCursor(9, 1);
  lcd.print((char)223); //用特殊字元顯示符號的"度"
  lcd.setCursor(10, 1);
  lcd.print("C");
  
  float SensorRead = analogRead(A0)*(5.0 / 1023.0);     //We read the sensor output  
  float Current = (SensorRead-2.5)/multiplier;          //Calculate the current value

  int index = random()%3 + 1;
  Serial.print("濕度: ");
  Serial.println(x);
  Serial.print("溫度: ");
  Serial.println(y);  
  Current = abs(Current);
  Serial.print("Current: ");
  Serial.println(Current); 
  flag = Current - temp; 
  Serial.print("flag: ");
  Serial.println(flag); 
  temp = Current;
 
  String getData = "GET /update?api_key="+ API +"&"+ field1 +"="+ String(x) +"&"+ field2 +"="+ String(y)+"&"+ field3 +"="+ String(Current)+"&"+ field4 +"="+ String(flag);
  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
  esp8266.println(getData);delay(2000);countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0",5,"OK");
 
  if(abs(flag) > 0.2/* || count == 5*/){
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
    /*if(count == 5){
      count = 0;
    }*/
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
    /*count++;*/
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
    lcd.setCursor(13, 1);
    lcd.print("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if(found == false){
    Serial.println("Fail");
    lcd.setCursor(12, 1);
    lcd.print("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
 }
