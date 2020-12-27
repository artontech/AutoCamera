#include "AccelStepper.h"
#include <SoftwareSerial.h>
// #include <Stepper.h>

// WIFI
SoftwareSerial wifiSerial(8, 9); // RX(ESP8266-TX), TX(ESP8266-RX)
bool needReconnect = false;

// 电机步进方式定义
#define FULLSTEP 4    //全步进参数
#define HALFSTEP 8    //半步进参数

// 一号28BYJ48连接的ULN2003电机驱动板引脚
#define stepper1Pin1 4
#define stepper1Pin2 5
#define stepper1Pin3 6
#define stepper1Pin4 7

// 二号28BYJ48连接的ULN2003电机驱动板引脚
#define stepper2Pin1 10
#define stepper2Pin2 11
#define stepper2Pin3 12
#define stepper2Pin4 13

AccelStepper stepper1(FULLSTEP, stepper1Pin1, stepper1Pin3, stepper1Pin2, stepper1Pin4);
AccelStepper stepper2(FULLSTEP, stepper2Pin1, stepper2Pin3, stepper2Pin2, stepper2Pin4);

long stepper1Step = 0;
bool stepper1Work = false;
long stepper2Step = 0;
bool stepper2Work = false;

void setup() {
  Serial.begin(9600);
  
  // Init motor
  pinMode(stepper1Pin1, OUTPUT);
  pinMode(stepper1Pin2, OUTPUT);
  pinMode(stepper1Pin3, OUTPUT);
  pinMode(stepper1Pin4, OUTPUT);
  pinMode(stepper2Pin1, OUTPUT);
  pinMode(stepper2Pin2, OUTPUT);
  pinMode(stepper2Pin3, OUTPUT);
  pinMode(stepper2Pin4, OUTPUT);
  stepper1.setMaxSpeed(500.0); // 1号电机最大速度500
  stepper1.setAcceleration(50.0); // 1号电机加速度50.0
  stepper2.setMaxSpeed(500.0);
  stepper2.setAcceleration(50.0);

  // Init wifi
  Serial.println("Init wifi");
  wifiSerial.begin(9600);
  wifiSerial.print("+++"); // 退出tcp透传模式，用println会出错
  wifiSerial.end();
  delay(500);
  wifiSerial.begin(115200);
  wifiSerial.print("+++");
  delay(1000);
  wifiSerial.println("AT+RST");   // 初始化重启一次esp8266
  echo();
  delay(1000);

  // Reset BOUD rate
  wifiSerial.println("AT+RST");   // 初始化重启一次esp8266
  echo();
  delay(1500);
  wifiSerial.println("AT+UART_CUR=9600,8,1,0,1");
  echo();
  delay(100);
  wifiSerial.end();
  wifiSerial.begin(9600);
  wifiSerial.println("AT+UART_CUR?");
  echo();
  delay(100);

  // 连接
  wifiSerial.println("AT");
  echo();
  delay(500);
  wifiSerial.println("AT+CWMODE=1");  // 设置Wi-Fi模式
  echo();
  wifiSerial.println("AT+CWJAP=\"Arton_2.4G\",\"lfy1331YFL\"");  // 连接Wi-Fi
  echo();
  delay(5000);

  // 进入TCP透传模式
  wifiSerial.println("AT+CIPMODE=1");
  echo();
  delay(100);
  wifiSerial.println("AT+CIPSTART=\"TCP\",\"192.168.2.100\",17888");  // 连接服务器的端口
  delay(1000);
  echo();
  wifiSerial.println("AT+CIPSEND"); // 进入TCP透传模式，接下来发送的所有消息都会发送给服务器
  echo();
  delay(1000);

  // Send init msg
  wifiSerial.println("Arton");
}

// 读取WIFI消息
void echo() {
  delay(50);
  while (wifiSerial.available()) {
    Serial.write(wifiSerial.read());
  }
}

// 读取TCP透传消息
String readWIFISerial() {
  delay(50);
  String data = "";
  while (wifiSerial.available()) {
    data += char(wifiSerial.read());
  }
  return data;
}

byte getCRC8(String str) {
  byte crc = 0x00;
  for (int i = 0; i < str.length(); i++) {
    byte extract = str.charAt(i);
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

// 处理WIFI消息
void processWIFI() {
  // 断线重连
  if (needReconnect) {
    needReconnect = false;
    wifiSerial.print("+++"); // 退出tcp透传模式，用println会出错
    delay(1000);
  }
  
  // 处理消息
  if (wifiSerial.available()) {
    String msg = readWIFISerial();
    long msgLength = msg.length();
    long dataLength = msgLength - 4;
    if (msg.startsWith("Car+")) {
      int crc1 = msg.substring(dataLength + 1, msgLength).toInt();
      byte crc2 = getCRC8(msg.substring(0, dataLength));
      if (crc1 != crc2) {
        Serial.println("E2," + msg);
        return;
      }
      
      char order1 = msg.charAt(4), order2 = msg.charAt(5);
      String tmp = String(order1) + String(order2) + ", Crc:" + String(crc2);
      Serial.println(tmp);

      int v = 0;
      switch (order1) {
        // Motor
        case 'M':
          switch (order2) {
            case 'F': // Forward
              break;
            case 'B': // Backward
              break;
            case 'L': // Left
              break;
            case 'R': // Right
              break;
            case 'S': // Stop
              break;
          }
          break;
        
        // Stepper
        case 'S':
          switch (order2) {
            case '1': // Stepper1
              stepper1Step = msg.substring(6, dataLength).toInt();
              stepper1Work = true;
              stepper1.moveTo(stepper1Step); // 2048 for one revolution
              Serial.println("1 From " + String(stepper1.currentPosition()) + ", To:" + String(stepper1Step) + ", Crc:" + String(crc2));
              break;
            case '2': // Stepper2
              stepper2Step = msg.substring(6, dataLength).toInt();
              stepper2Work = true;
              stepper2.moveTo(stepper2Step); // 2048 for one revolution
              Serial.println("2 From " + String(stepper2.currentPosition()) + ", To:" + String(stepper2Step) + ", Crc:" + String(crc2));
              break;
          }
          break;
      }
    } else {
      Serial.println(msg);
    }
  }
}

void processStepper() {
  if (stepper1Work) {
    if (stepper1Step != stepper1.currentPosition()) {
      stepper1.run();
    } else {
      stepper1Work = false;
      
      // 强制拉低减少损耗
      digitalWrite(stepper1Pin1, LOW);
      digitalWrite(stepper1Pin2, LOW);
      digitalWrite(stepper1Pin3, LOW);
      digitalWrite(stepper1Pin4, LOW);
    }
  }
  if (stepper2Work) {
    if (stepper2Step != stepper2.currentPosition()) {
      stepper2.run();
    } else {
      stepper2Work = false;
      
      // 强制拉低减少损耗
      digitalWrite(stepper2Pin1, LOW);
      digitalWrite(stepper2Pin2, LOW);
      digitalWrite(stepper2Pin3, LOW);
      digitalWrite(stepper2Pin4, LOW);
    }
  }
}

void loop() {
  // WIFI read
  processWIFI();
  
  // WIFI write
  if (Serial.available()) {
    wifiSerial.write(Serial.read());
  }

  processStepper();
}
