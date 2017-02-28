#include "Arduino.h"

namespace function_test {

#define BTS01_RESET 43
#define LED_MULTI_FET 18
#define TEST_PINS { 0,1,2,3,6,7,8,9,10,11,12,13,16,17,18,20,21,30,31 }

#define ARRAYCOUNT(a) (sizeof(a) / sizeof((a)[0]))

const byte testPin[] = TEST_PINS;

static void
resetBTS01(void)
{
  pinMode(BTS01_RESET,OUTPUT);
  digitalWrite(BTS01_RESET, LOW);
  delay(20);
  digitalWrite(BTS01_RESET, HIGH);
  delay(20);
}

static void
pinInit(void)
{
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  
  for (int i = 0; i < ARRAYCOUNT(testPin); i++){
    pinMode(testPin[i], OUTPUT);
    digitalWrite(testPin[i], HIGH);
  }
  for (int i = 0; i < 6; i++){
    pinMode(24 + i,INPUT_PULLUP);  /*  A0 = 24 */
  }
}

static void
indicate_function_test_Mode()
{
  digitalWrite(0, LOW);  // LED Live
  delay(500);
  digitalWrite(0, HIGH);

}

static void
dcmotorPinInit(void)
{
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(12, OUTPUT);
}

static void serialCheck(boolean, boolean);

static void
setFilterPolicy(void)
{
  /*  Setting "0" ,filter policy */
  Serial.print("AT+CAM=0,04\r");
  serialCheck(true, false);
}

static void
setDeviceName(void)
{
  Serial.print("AT+CDN=KOOV Core\r");
  serialCheck(true, false);
}

static void
getDeviceInfo(void)
{
  delay(100);
  Serial.print("AT+CDN\r");
  serialCheck(true, true);
  delay(100);
  Serial.print("AT+CAM\r");
  serialCheck(true, true);
  delay(100);
  Serial.print("AT+RVN\r");
  serialCheck(true, true);
  delay(100);
  Serial.print("AT+RMN\r");
  serialCheck(true, true);
  delay(100);
  Serial.print("AT+RBA\r");
  serialCheck(true, true);
  delay(100);
  Serial.print("AT+RBI\r");
  serialCheck(true, true);
}

static void
voltageCheck(void)
{
  if ((digitalRead(A4) == LOW) && (digitalRead(A5) == LOW)){
    for (int i = 0; i < ARRAYCOUNT(testPin); i++){
      digitalWrite(testPin[i],LOW);
    }
  }
  if ((digitalRead(A4) == HIGH) && (digitalRead(A5) == HIGH)){
    for (int i = 0; i < ARRAYCOUNT(testPin); i++){
      digitalWrite(testPin[i],HIGH);
    }
  }
}

static void dcMotorForward(void);
static void dcMotorBackward(void);
static void dcMotorBrake(void);
static void dcMotorStop(void);

static void
dcmotorCheck(void)
{
  if (digitalRead(A0) == LOW){
    digitalWrite(LED_MULTI_FET,HIGH);
    dcmotorPinInit();
    dcMotorForward();
    delay(500);
    digitalWrite(LED_MULTI_FET,LOW);
    dcMotorStop();
  }

  if (digitalRead(A2) == LOW){
    digitalWrite(LED_MULTI_FET,HIGH);
    dcmotorPinInit();
    dcMotorBackward();
    delay(500);
    digitalWrite(LED_MULTI_FET,LOW);
    dcMotorStop();
  }
}

static void
dcMotorForward(void)
{
  digitalWrite(5,LOW);
  digitalWrite(4,HIGH);
  digitalWrite(10,LOW);
  digitalWrite(12,HIGH);
}

static void
dcMotorBackward(void)
{
  digitalWrite(5,HIGH);
  digitalWrite(4,LOW);
  digitalWrite(10,HIGH);
  digitalWrite(12,LOW);
}

static void
dcMotorBrake(void)
{
  digitalWrite(5,HIGH);
  digitalWrite(4,HIGH);
  digitalWrite(10,HIGH);
  digitalWrite(12,HIGH);
}

static void
dcMotorStop(void)
{
  digitalWrite(5,LOW);
  digitalWrite(4,LOW);
  digitalWrite(10,LOW);
  digitalWrite(12,LOW);
}

static void
serialCheck(boolean wait, boolean usb)
{
  if(wait){
    unsigned long timeout = millis();
    while(!Serial.available()){
      if (millis() > (timeout + 500)){
        break;
      }
    }
  }
  
  while(Serial.available()){
    char inByte = Serial.read();

    if (usb){
      SerialUSB.print(inByte);
    }

    if (inByte == '1' ){
      Serial.print("A\r");
    }else if (inByte == 'd' ){
      resetBTS01();
      Serial.print("AT+DBI=ALL\r");
      delay(100);
    }
  }
}

static void
serialUSBCheck(void)
{
  while(SerialUSB.available()){
    char inByte = SerialUSB.read();

    if (inByte == '1' ){
      SerialUSB.println("OK");
      SerialUSB.println();
    }
    if (inByte == 'f' ){
      resetBTS01();
      getDeviceInfo();
      // without this delay, following AT command does not work.
      delay(100);
      Serial.print("AT+SBO\r");
      serialCheck(true, false);
    }
  }
}

static void
function_test_setup()
{
  SerialUSB.begin(38400);  /*  USB */
  Serial.begin(38400);  /* BT module */
  pinInit();

  resetBTS01();
  delay(100);
  setFilterPolicy();
  delay(100);
  setDeviceName();
  delay(500);

  Serial.print("AT+SBO\r");  /* to operation mode */
  serialCheck(true, false);
  analogReference(AR_DEFAULT);

  indicate_function_test_Mode();
}  //  setup

static void
function_test_loop()
{
  while(true){
    voltageCheck();
    dcmotorCheck();
  
    serialCheck(false, false);
    serialUSBCheck();
    // with this delay enabled, TX/RX led decrease brightness.
//    delay(10);
  }
}  //  loop


#undef BTS01_RESET
#undef LED_MULTI_FET
#undef TEST_PINS
#undef ARRAYCOUNT

};
