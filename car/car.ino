#include <SoftwareSerial.h>
#include <Wire.h>
#include "PS2X_lib.h"
#include "Adafruit_MotorShield.h"
#include "Adafruit_MS_PWMServoDriver.h"

// WIFI
SoftwareSerial wifiSerial(2, 3); // RX(ESP8266-TX), TX(ESP8266-RX)
bool needReconnect = false;

// Motor
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *DCMotor_LF = AFMS.getMotor(1); // 左前
Adafruit_DCMotor *DCMotor_RF = AFMS.getMotor(4); // 右前
Adafruit_DCMotor *DCMotor_LB = AFMS.getMotor(2); // 左后
Adafruit_DCMotor *DCMotor_RB = AFMS.getMotor(3); // 右后

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  
  AFMS.begin(50);

  initWIFI();
}

void loop() {
  // WIFI read
  processWIFI();
  
  // WIFI write
  if (Serial.available()) {
    wifiSerial.write(Serial.read());
  }
}

// 初始化WIFI
void initWIFI() {
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
  wifiSerial.println("AT+CWJAP=\"liu_wifi\",\"lql13312999730\"");  // 连接Wi-Fi
  echo();
  delay(5000);

  // 进入TCP透传模式
  while(true) {
    wifiSerial.println("AT+CIPMODE?");
    String result = readWIFISerial();
    Serial.println(result);
    if (!result.endsWith("busy p...")) {
      break;
    }
    delay(1000);
  }
  wifiSerial.println("AT+CIPMODE=1");
  echo();
  delay(100);
  wifiSerial.println("AT+CIPSTART=\"TCP\",\"192.168.0.110\",17888");  // 连接服务器的端口
  delay(1000);
  String result = readWIFISerial();
  Serial.println(result);
  if (!result.endsWith("OK")) {
    needReconnect = true;
    Serial.println("need reconnect");
  }
  
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
  if (data.endsWith("\r\n")) data.remove(data.length() - 2, 2);
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
    delay(5000);
    initWIFI();
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

      switch (order1) {
        // Motor
        case 'M':
          int v = msg.substring(6, dataLength).toInt();
          if (v > 255) v = 255;
          
          switch (order2) {
            case 'F': // Forward
              forward(v);
              break;
            case 'B': // Backward
              backward(v);
              break;
            case 'L': // Left
              moveLeft(v);
              break;
            case 'R': // Right
              moveRight(v);
              break;
            case 'C': // Clockwise
              turnRight(v);
              break;
            case 'A': // Anticlockwise
              turnLeft(v);
              break;
            case 'S': // Stop
              stopMoving();
              break;
          }
          break;
        
        // Stepper
        case 'S':
          switch (order2) {
            case '1': // Stepper1
              break;
            case '2': // Stepper2
              break;
          }
          break;
      }
    } else {
      Serial.println(msg);
    }
  }
}

// 前进
void forward(int v) {
  DCMotor_LF->setSpeed(v);
  DCMotor_LF->run(FORWARD);
  DCMotor_RF->setSpeed(v);
  DCMotor_RF->run(FORWARD);
  DCMotor_LB->setSpeed(v);
  DCMotor_LB->run(FORWARD);
  DCMotor_RB->setSpeed(v);
  DCMotor_RB->run(FORWARD);
}

// 后退
void backward(int v) {
  DCMotor_LF->setSpeed(v);
  DCMotor_LF->run(BACKWARD);
  DCMotor_RF->setSpeed(v);
  DCMotor_RF->run(BACKWARD);
  DCMotor_LB->setSpeed(v);
  DCMotor_LB->run(BACKWARD);
  DCMotor_RB->setSpeed(v);
  DCMotor_RB->run(BACKWARD);
}

// 左平移
void moveLeft(int v) {
  DCMotor_LF->setSpeed(v);
  DCMotor_LF->run(BACKWARD);
  DCMotor_RF->setSpeed(v);
  DCMotor_RF->run(FORWARD);
  DCMotor_LB->setSpeed(v);
  DCMotor_LB->run(FORWARD);
  DCMotor_RB->setSpeed(v);
  DCMotor_RB->run(BACKWARD);
}

// 右平移
void moveRight(int v) {
  DCMotor_LF->setSpeed(v);
  DCMotor_LF->run(FORWARD);
  DCMotor_RF->setSpeed(v);
  DCMotor_RF->run(BACKWARD);
  DCMotor_LB->setSpeed(v);
  DCMotor_LB->run(BACKWARD);
  DCMotor_RB->setSpeed(v);
  DCMotor_RB->run(FORWARD);
}

// 左旋转
void turnLeft(int v) {
  DCMotor_LF->setSpeed(v);
  DCMotor_LF->run(BACKWARD);
  DCMotor_RF->setSpeed(v);
  DCMotor_RF->run(FORWARD);
  DCMotor_LB->setSpeed(v);
  DCMotor_LB->run(BACKWARD);
  DCMotor_RB->setSpeed(v);
  DCMotor_RB->run(FORWARD);
}

// 右旋转
void turnRight(int v) {
  DCMotor_LF->setSpeed(v);
  DCMotor_LF->run(FORWARD);
  DCMotor_RF->setSpeed(v);
  DCMotor_RF->run(BACKWARD);
  DCMotor_LB->setSpeed(v);
  DCMotor_LB->run(FORWARD);
  DCMotor_RB->setSpeed(v);
  DCMotor_RB->run(BACKWARD);
}

// 停止
void stopMoving() {
  DCMotor_LF->setSpeed(0);
  DCMotor_LF->run(RELEASE);
  DCMotor_RF->setSpeed(0);
  DCMotor_RF->run(RELEASE);
  DCMotor_LB->setSpeed(0);
  DCMotor_LB->run(RELEASE);
  DCMotor_RB->setSpeed(0);
  DCMotor_RB->run(RELEASE);
}
