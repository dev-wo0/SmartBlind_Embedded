#include "SPI.h"
#include "WiFi.h"
#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include "text_reader.c"

#define address 0x24

const int buttonPin = 1;
const int ledPin = 12;

//------------------Client API-------------//
char ssid[] = "RTEMD5G";       //와이파이 SSID

char host[] = "www.kma.go.kr";

WiFiServer server(80);
WiFiClient client;

String wt_hour;
String wt_temp;
String wt_twfEn;

String tom_twfEn;

int a = 0;

//------------------Client API-------------//

//------------------Main Server-------------//
const char * delimiter = "\n";

char * str;
char * pch;

LiquidCrystal_I2C lcd(0x20, 16, 2);
Servo myservo;                       //서브모터
int angle = 0;                       //서브모터 각
int sw = -1;                         //스위치 역할

bool ledStatus = false;
bool lastLedStatus = false;
bool stringIsOk = false;
bool AutoStatus = false;
bool AlarmStatus = false;
String remoteIp;
int timer;
//------------------Main Server-------------//
//------------------FND-------------//
int us = 300000;

int DIG2 = 2;
int DIG3 = 3;
int DIG4 = 4;

#define zero 0xc0
#define one 0xF9
#define two 0xA4
#define three 0xB0
#define four 0x99
#define five 0x92
#define six 0x82
#define seven 0xF8
#define eight 0x80
#define nine 0x90

int u_timer=10;
int a_digit, b_digit, c_digit = 0;
//------------------FND-------------//



int buttonState = 0;
int flag = -1;
void setup() {
  //각 변수에 정해진 공간 할당
  Serial.begin(9600);

  delay(10);
  //WiFi연결 시도
  Serial.println("Connecting to WiFi....");
  WiFi.begin(ssid);

  server.begin();
  Serial.println("Connect success!");
  Serial.println("Waiting for DHCP address");
  //DHCP주소를 기다린다

  Serial.println("\n");
  printWifiData();
  connectToServer();

  lcd.init();
  lcd.backlight();
  myservo.attach(9);

  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Remote IP:");

  delay (1000);

  Wire.begin();
  pinMode(DIG2, OUTPUT);
  pinMode(DIG3, OUTPUT);
  pinMode(DIG4, OUTPUT);
  config_out(0x00, 0x00);
  write_out(0x00);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
}

void loop() {
  buttonState = digitalRead(buttonPin);
    a_digit = u_timer / 100;
    b_digit = u_timer / 10 - u_timer / 100 * 10;
    c_digit = u_timer - u_timer / 10 * 10;
  if (client.connected()) {
    while (client.available()) {
      //라인을 기준으로 문자열을 저장한다.
      String line = client.readStringUntil('\n');
      //Serial.println(line);

      //시간
      int temp11 = line.indexOf("</hour>");
      if (temp11 > 0) {
        String tmp_str = "<hour>";
        wt_hour = line.substring(line.indexOf(tmp_str) + tmp_str.length(), temp11);
        //Serial.print("hour is ");
        //Serial.println(wt_hour + ":00");
      }

      //온도
      int temp = line.indexOf("</temp>");
      if (temp > 0) {
        String tmp_str = "<temp>";
        wt_temp = line.substring(line.indexOf(tmp_str) + tmp_str.length(), temp);
        //Serial.print("temperature is ");
        //Serial.println(wt_temp);
      }

      //날씨 정보
      int wfEn = line.indexOf("</wfEn>");
      if (wfEn > 0) {
        String tmp_str = "<wfEn>";
        wt_twfEn = line.substring(line.indexOf(tmp_str) + tmp_str.length(), wfEn);
        //Serial.print("weather is ");
        //Serial.println(wt_twfEn);
        //Serial.println();
      }

      int data_seq= line.indexOf("<data seq=\"6\">");
      if (data_seq > 0 && a == 0) {
        a++;

        tom_twfEn = wt_twfEn;
        
        printInfo();
        break;
      }
    }
  }

  // put your setup code here, to run once:
  int light;
  light = analogRead(3);                                          //light val

  str = readFile();
  pch = strtok (str, delimiter);
  if (pch != NULL) {
    ledStatus =  (String(pch) == "true");
    pch = strtok (NULL, delimiter);
    AutoStatus =  (String(pch) == "true");
    pch = strtok (NULL, delimiter);
    AlarmStatus =  (String(pch) == "true");
    pch = strtok (NULL, delimiter);
    timer = int (pch);
    pch = strtok (NULL, delimiter);
    remoteIp = String (pch);
    pch = strtok (NULL, delimiter);
    stringIsOk  = String (pch) == "OK";
    //read the rest of the string, you can omit this
    while ((pch != NULL))
    {
      pch = strtok (NULL, delimiter);
    }
  }

  if(buttonState == HIGH){
    digitalWrite(ledPin, LOW);
    u_timer = 10;

    flag = 1;
    delay(100);
  }
  
  if (stringIsOk && (ledStatus != lastLedStatus)) {
    Serial.print ("HandMode : ");
    Serial.println (ledStatus);
    Serial.print ("AutoMode : ");
    Serial.println (AutoStatus);
    Serial.print ("SleepMode : ");
    Serial.println (AlarmStatus);
    
    //first, empty the screen from the old address
    lcd.setCursor(0, 1);
    lcd.print("                ");
    if (ledStatus == 0 && AutoStatus == 0) {    //수동으로 - 블라인드 내림
      lcd.noBacklight();
      for (angle = 180; angle >= 1; angle--)
      {
        myservo.write(angle);
        break;
      }
    }

    if (ledStatus == 1 && AutoStatus == 0) { //수동으로 - 블라인드 올림
      lcd.backlight();
      for (angle = 0; angle < 180; angle++) {
        myservo.write(angle);
        break;
      }
    }
    
    lcd.setCursor(0, 1);
    lcd.print(remoteIp);
    Serial.println(remoteIp);
    Serial.println("-------------------");
    lastLedStatus = ledStatus;
    flag = -1;
  }

  else if (stringIsOk && AutoStatus == 1) {
    Serial.println("AutoMode");
    if (light > 900)    //light val 900초과일때(햇빛이 셀때) - 블라인드 내려옴
    {
      for (angle = 180; angle >= 1; angle--)
      {
        myservo.write(angle);
        break;
      }
    }

    else               //light val 900이하일때(햇빛이 약할때) - 블라인드 올라감
    {
      for (angle = 0; angle < 180; angle++)
      {
        myservo.write(angle);
        break;
      }
    }
    flag = -1;
  }

  else if (stringIsOk && AlarmStatus == 1 && flag == -1) {
    Serial.println("Sleep Mode");
    for (int i = 1; i < 4; i++) {
      int val;
      if (i == 1) {
        val = c_digit;
      }
      else if (i == 2) {
        val = b_digit;
      }
      else if (i == 3) {
        val = a_digit;
      }

      if (val == 0) {
        DIG_sel(0);
        write_out(zero);
        DIG_sel(i);
      }
      else if (val == 1) {
        DIG_sel(0);
        write_out(one);
        DIG_sel(i);
      }
      else if (val == 2) {
        DIG_sel(0);
        write_out(two);
        DIG_sel(i);
      }
      else if (val == 3) {
        DIG_sel(0);
        write_out(three);
        DIG_sel(i);
      }
      else if (val == 4) {
        DIG_sel(0);
        write_out(four);
        DIG_sel(i);
      }
      else if (val == 5) {
        DIG_sel(0);
        write_out(five);
        DIG_sel(i);
      }
      else if (val == 6) {
        DIG_sel(0);
        write_out(six);
        DIG_sel(i);
      }
      else if (val == 7) {
        DIG_sel(0);
        write_out(seven);
        DIG_sel(i);
      }
      else if (val == 8) {
        DIG_sel(0);
        write_out(eight);
        DIG_sel(i);
      }
      else if (val == 9) {
        DIG_sel(0);
        write_out(nine);
        DIG_sel(i);
      }
    }
    u_timer--;
  }
  
  if (u_timer == 0 && flag == -1)
  {
    Serial.println("Time out");
    digitalWrite(ledPin, HIGH);
    if(tom_twfEn == "Clear" || tom_twfEn == "Partly Cloudy"){
      Serial.println("It's a nice day");
      for (angle = 0; angle < 180; angle++)
        {
          myservo.write(angle);
          break;
        }
      }
    else{
      Serial.println("It's a bad day");
      digitalWrite(ledPin, HIGH);
    }
  }
  delay(100);
}

//서버와 연결
void connectToServer() {
  Serial.println("connecting to server...");
  String content = "";
  if (client.connect(host, 80)) {
    Serial.println("Connected! Making HTTP request to www.kma.go.kr");
    //Serial.println("GET /data/2.5/weather?q="+location+"&mode=xml");
    client.println("GET /wid/queryDFSRSS.jsp?zone=4159025300 HTTP/1.1");
    //위에 지정된 주소와 연결한다.
    client.print("HOST: www.kma.go.kr\n");
    client.println("User-Agent: launchpad-wifi");
    client.println("Connection: close");

    client.println();
    Serial.println("Weather information for ");
    Serial.println("");
  }
  //마지막으로 연결에 성공한 시간을 기록
}

void printWifiData() {
  // Wifi쉴드의 IP주소를 출력
  Serial.println();
  Serial.println("IP Address Information:");
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.println("");
}

void printInfo() {
  Serial.print("hour is 2017. 06. 14 ");
  Serial.println(wt_hour + ":00");
  Serial.print("temperature is ");
  Serial.println(wt_temp);
  Serial.print("weather is ");
  Serial.println(wt_twfEn);
}

void write_out(int value) {
  Wire.beginTransmission(address);
  Wire.write(0x02);
  Wire.write(value);
  Wire.endTransmission();
  delayMicroseconds(us);
}

void config_out(int PORT0, int PORT1) {
  Wire.beginTransmission(address);
  Wire.write(0x06);
  Wire.write(PORT0);
  Wire.write(PORT1);
  Wire.endTransmission();
}

void DIG_sel(int val) {
  if (val == 0) {
    digitalWrite(DIG2, LOW);
    digitalWrite(DIG3, LOW);
    digitalWrite(DIG4, LOW);
  }
  else if (val == 1) {
    digitalWrite(DIG2, HIGH);
    digitalWrite(DIG3, LOW);
    digitalWrite(DIG4, LOW);
  }
  else if (val == 2) {
    digitalWrite(DIG2, LOW);
    digitalWrite(DIG3, HIGH);
    digitalWrite(DIG4, LOW);
  }
  else if (val == 3) {
    digitalWrite(DIG2, LOW);
    digitalWrite(DIG3, LOW);
    digitalWrite(DIG4, HIGH);
  }
}
