//=== ACS712_30A ===
//精度改進版 programed by LIN
#define Ideal_mv_A  66.0  // ACS712_30A 對應類比量輸出66mV/A (理想值 at Vcc=5.0V)
#include <SoftwareSerial.h>
#define RX 10
#define TX 11
float ADCcount;           // 所讀出之類比轉數位值 (0-1023)
float I_ave;              // 所測電流值(採樣1000次再取平均值)
float Vcc;                // 實際(接近+5V)電壓值
float Zero_count;         // ACS712 無電流輸入時offset歸零值(理想=512)
float Real_mV_A;          // ACS712 實際對應類比量輸出??mV/A (理想值66mV/A at Vcc=5.0V)
String AP = "laugh";       // CHANGE ME
String PASS = "hahahaha"; // CHANGE ME
String API = "7MEJFIROH6JR31L0";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
String field1 = "field1";
String field2 = "field2";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int x,y;
SoftwareSerial esp8266(RX,TX); 
void setup() {

  Serial.begin( 9600 );  //設定serial port傳輸速率
  esp8266.begin(115200);
  delay(100);
  // 取得歸零值(無電流時之ADC值當作歸零值;以下取1000次之平均值)
  // 理想上ACS712無電流輸入時其輸出是Vcc/2(=ADC 512),而Arduino之ADC為10bit=0~1023
  Zero_count=0.0;
  // 因analogRead()每次讀取類比電壓存在著誤差,故取樣1000次採平均值
  for(int i = 0; i < 1000; i++) {
     Zero_count = Zero_count + analogRead(A0)/1000.0; //因要取1000次平均值再累加,故除1000
     delay(1); //等待1ms; 因 analogRead() 此函式每次讀取需要約0.1ms
  }
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
    // 印出歸零值當參考
  Serial.println("========= ACS712_30A by LIN =========");
  Serial.println("......... Zeroing !.........");
  Serial.print("Zero_count = ");
  Serial.println(Zero_count); // 看看ACS712無電流輸入時offset歸零值(理想=512)
  
  //利用內部參考1.1v電壓,來讀取實際之Vcc電壓
  Vcc = readVcc();
  Serial.print("Vcc = ");
  Serial.print(Vcc);          // 看看實際Vcc電壓(理想=5V=5000mV)
  Serial.println("mV  (ideal Vcc = 5000mV)"); 
 //ACS712 output sensitivity(理想)=185mV/A at Vcc=5.0V
  Real_mV_A=Vcc*Ideal_mv_A/5000.0;
  Serial.print("Real_mV_A = "); // ACS712實際對應類比量輸出=??mV/A
  Serial.print(Real_mV_A); 
  Serial.println("mV  (ideal Output_mV_A = 66mV)");
  Serial.println("-------- Start testing ! ----------");
}
    void loop() {
  // 讀取ADC值(取樣1000次,以便取得較穩定值)
  ADCcount=0.0;
  for(int i = 0; i < 1000; i++) {
    ADCcount = ADCcount +  analogRead(A0)/1000.0;
   //因要取1000次平均值再累加,故除1000
    delay(1); //等待1ms; 因 analogRead() 此函式每次讀取需要約0.1ms
  }
  // 將ADC換算成電流
  // 公式:I=(count-Zero_count)(Vcc/1024)mV/66mV
 // I_ave=(ADCcount-Zero_count)*(Vcc/1024.0)*1000.0/66; //(單位mA 取樣1000次之平均值) 
    I_ave=(ADCcount-Zero_count)*(Vcc/1024.0)*1000.0/Real_mV_A; 
  //(單位mA 取樣1000次之平均值) 
  // 印出
  Serial.print("ADC:");
  Serial.print((int)ADCcount,DEC);
  Serial.print(" = ");  
  Serial.print( (int)I_ave, DEC );         //印出所測電流
  Serial.print( " mA" );                   //(單位mA)  
  Serial.print( "  = " );  
  Serial.print((float)I_ave/1000.9, DEC);  //(單位A)
  Serial.println( " A" ); 
  x = (int)ADCcount;
  y = (int)I_ave;
  
   Serial.println();
   Serial.println(x);
   Serial.println(y);
   String getData = "GET /update?api_key="+ API +"&"+ field1 +"="+String(x)+"&"+ field2 +"="+String(y);
   sendCommand("AT+CIPMUX=1",5,"OK");
   sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
   sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
   esp8266.println(getData);delay(1500);countTrueCommand++;
   sendCommand("AT+CIPCLOSE=0",5,"OK");
}
//=========================================================
// 利用內部參考1.1v電壓,來讀取較精確Vcc +5V電壓
//=========================================================
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // 等Vref穩定
  ADCSRA |= _BV(ADSC); // 開始轉換
  while (bit_is_set(ADCSRA,ADSC)); // 測量
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // 反推算出Vcc(單位mV); 其中 1125300 = 1.1*1023*1000
  return result;  // Vcc 單位mv
}
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
