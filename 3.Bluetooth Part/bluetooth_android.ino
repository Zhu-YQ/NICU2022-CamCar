uint state=0;
#define STATEOUT D2                   
/*
  BLE_Peripheral.ino从机代码

  This program uses the ArduinoBLE library to set-up an Arduino Nano 33 BLE 
  as a peripheral device and specifies a service and a characteristic. Depending 
  of the value of the specified characteristic, an on-board LED gets on. 

  The circuit:
  - Arduino Nano 33 BLE. 
*/

#include <ArduinoBLE.h>
#include <SPI.h>
//#include <Wire.h>

enum {
  START  = 1,
  STOP   = 0,
  ELSE   =-1,
};/*定义指令的值*/

const char* deviceServiceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";   /*定义从机的ID*/
const char* deviceServiceCharacteristicUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";/*定义主机的ID*/

BLEService stateService(deviceServiceUuid); 
BLEByteCharacteristic stateCharacteristic(deviceServiceCharacteristicUuid, BLERead | BLEWrite);

void setup() {
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  Serial.begin(9600);      /*初始化波特率*/
  //while (!Serial);         /*初始化串口监视器,如需使用串口监视器则取消本行注释*/

  pinMode(STATEOUT,OUTPUT);      /*初始化输出端口*/
  
  pinMode(LEDR, OUTPUT);   /*初始化LED红*/
  pinMode(LEDG, OUTPUT);   /*初始化LED绿*/
  pinMode(LEDB, OUTPUT);   /*初始化LED蓝*/
  pinMode(LED_BUILTIN, OUTPUT);   /*初始化LED按键*/

  digitalWrite(STATEOUT,LOW);         /*初始化输出为低*/
  //LED均为低电平有效
  digitalWrite(LEDR, HIGH);      /*初始化LED红为低电平*/
  digitalWrite(LEDG, HIGH);      /*初始化LED绿为低电平*/
  digitalWrite(LEDB, LOW);      /*初始化LED蓝为高电平*/
  digitalWrite(LED_BUILTIN, LOW);  /*初始化LED按键为低电平*/

  if (!BLE.begin()) {
    digitalWrite(LEDR,LOW);       /*若蓝牙未被打开则点亮红灯*/
    digitalWrite(LEDB,HIGH);
    while (1);
  }

    BLE.setLocalName("Arduino Nano 33 BLE ");
    BLE.setAdvertisedService(stateService);
    stateService.addCharacteristic(stateCharacteristic);
    BLE.addService(stateService);
    stateCharacteristic.writeValue(0);
    BLE.advertise();

    Serial.println("Nano 33 BLE Setup Complete");
    Serial.println(" ");
}

void loop() {
//  Serial.println(state);

  BLEDevice central = BLE.central();
  Serial.println("- Discovering central device..."); 
  delay(1000);   /*循环打印搜索中央设备中*/

  if (central) {
    Serial.println("* Connected to central device!");
    Serial.print("* Device MAC address: ");
    Serial.println(central.address());
    Serial.println(" "); /*如果搜索成功，则打印中央设备的地址码*/

    while (central.connected()) {
        if (stateCharacteristic.written()) {
           state = stateCharacteristic.value();
           writeState(state);  /*获取中央设备的身份码*/
         }
      }
    
      Serial.println("* Disconnected to central device!");       /*打印链接成功*/
    }
  
 
}

void writeState(int state) 
{
  switch (state) {
      case START:                                              /*如果开始跟踪，则使得LED灯绿灯亮，D2端口为高电平，并从串口返回信息*/
        Serial.println("* Actual value: START (green LED on)");
        Serial.println(" ");
        digitalWrite(LEDR, HIGH);
        digitalWrite(LEDG, LOW);
        digitalWrite(LEDB, HIGH);
        digitalWrite(STATEOUT,HIGH);
        Serial.println("START (output HIGH)");
        Serial.println(" ");

        
        break;
        
      case STOP:                                            /*如果结束跟踪，则使得LED灯蓝灯亮，D2端口为低电平，并从串口返回信息*/
        Serial.println("* Actual value: STOP (blue LED on)");
        Serial.println(" ");
        digitalWrite(LEDR, HIGH);
        digitalWrite(LEDG, HIGH);
        digitalWrite(LEDB, LOW);
        digitalWrite(STATEOUT,LOW);
        Serial.println("STOP (output LOW)");
        Serial.println(" ");


        
        break;

       default:                                               /*如果输入不为START/STOP，则不改变LED等状态和D2端口输出状态*/
        Serial.println("* Actual value: ELSE ");
        Serial.println(" ");
        
        Serial.println("ELSE (output remain)");
        Serial.println(" ");

        break;
  }  
}
