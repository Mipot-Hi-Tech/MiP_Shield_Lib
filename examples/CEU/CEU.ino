
#include "MiP_CEU.h"


#define N_WAKE_PIN          4
#define N_DATA_INDICATE_PIN 3
#define N_RESET_PIN         2

MipCEU ceu(N_WAKE_PIN, N_DATA_INDICATE_PIN);

const int msgBufSize = 100;
char msgBuf[msgBufSize];
int msgBufLen;


void setup() {
  int ret;

  Serial.begin(9600);
  // while the serial stream is not open, do nothing:
  while (!Serial) ;
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(N_RESET_PIN, OUTPUT_OPENDRAIN);
  digitalWrite(N_RESET_PIN, HIGH);

  Serial.println("======================");
  ceu.begin();
  ceu.wakeUp();

  delay(800);
  uint8_t rxBuf[100];
  uint32_t srcID;
  Serial.println("is there something to read?");
  ret = ceu.readMsg(rxBuf, 100, &srcID);
  Serial.print("readMsg: ");
  Serial.println(ret);

  delay(200);

  ret = ceu.factoryReset();
  Serial.print("factoryReset: ");
  Serial.println(ret);

  delay(2000);
  Serial.println("late reply?");
  ret = ceu.readMsg(rxBuf, 100, &srcID);
  Serial.print("readMsg: ");
  Serial.println(ret);

  ret = ceu.softwareReset();
  Serial.print("reset: ");
  Serial.println(ret);

  delay(800);

  struct mipVersion_s fwVersion;
  fwVersion = ceu.getFwVersion();
  msgBufLen = snprintf(msgBuf, msgBufSize, "FW version:%u.%u.%u.%u\r\n", fwVersion.major, fwVersion.minor, fwVersion.patch, fwVersion.build);
  Serial.print(msgBuf);

  
  uint32_t serialNumber;
  serialNumber = ceu.getSerialNumber();

  msgBufLen = snprintf(msgBuf, msgBufSize, "SN: %u\r\n", serialNumber);
  Serial.print(msgBuf);

  ret = ceu.setDeviceType(MIP_CEU_END_NODE);
  Serial.print("setDeviceType: ");
  Serial.println(ret);

  uint32_t masterAddress = 0;
  int activationStatus;
  activationStatus = ceu.getActivationStatus(&masterAddress);
  Serial.print("getActivationStatus: ");
  Serial.println(activationStatus);
  if(activationStatus > 0){
    Serial.print("masterAddress: ");
    Serial.println(masterAddress, HEX);
  }

  if(activationStatus == 0){
    uint8_t idx;
    ret = ceu.pairingRequest(&masterAddress, &idx);
    Serial.print("pairingRequest: ");
    Serial.println(ret);
    msgBufLen = snprintf(msgBuf, msgBufSize, "masterAddress: %08X, idx:%u\r\n", masterAddress, idx);
    Serial.print(msgBuf);

    delay(300);
    activationStatus = ceu.getActivationStatus(&masterAddress);
    Serial.print("getActivationStatus: ");
    Serial.println(activationStatus);
    if(activationStatus > 0){
      Serial.print("masterAddress: ");
      Serial.println(masterAddress, HEX);
    }
  }


}

void loop() {
  const int rxBufSize = 100;
  uint8_t rxBuf[rxBufSize];
  int ret;
  static int i = 0;

  digitalWrite(LED_BUILTIN, HIGH);
  
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  if(i == 30){
    static uint8_t msg[10] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x00};
    i = 0;
    msg[5]++;
  }
  i++;

  if(ceu.isDataAvailable()){
    Serial.println("dataAvailable!");
  }
  
  uint32_t srcID;
  ret = ceu.readMsg(rxBuf, 100, &srcID);
  if(ret > 0){
    msgBufLen = snprintf(msgBuf, msgBufSize, "rx msg (%i): ", ret);
    for(int i = 0; i < ret; i++){
      msgBufLen += snprintf(msgBuf + msgBufLen, msgBufSize - msgBufLen, "%02X ", rxBuf[i]);
    }
    Serial.println(msgBuf);
    
  }
  
}
