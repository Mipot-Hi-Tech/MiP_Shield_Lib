
#include "MiP_wMBus.h"

#define N_WAKE_PIN          4
#define N_DATA_INDICATE_PIN 3
#define N_RESET_PIN         2

MipWMBus wMBus(N_WAKE_PIN, N_DATA_INDICATE_PIN);

const int msgBufSize = 100;
char msgBuf[msgBufSize];
int msgBufLen;


void setup() {
  

  Serial.begin(9600);
  // while the serial stream is not open, do nothing:
  while (!Serial) ;
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(N_RESET_PIN, OUTPUT_OPENDRAIN);
  digitalWrite(N_RESET_PIN, HIGH);

  Serial.println("======================");
  wMBus.begin();

  delay(200);

  int ret = wMBus.softwareReset();
  Serial.print("reset: ");
  Serial.println(ret);

  delay(800);

  struct mipVersion_s fwVersion;
  fwVersion = wMBus.getFwVersion();
  msgBufLen = snprintf(msgBuf, msgBufSize, "FW version:%u.%u.%u.%u\r\n", fwVersion.major, fwVersion.minor, fwVersion.patch, fwVersion.build);
  Serial.print(msgBuf);

  
  uint32_t serialNumber;
  serialNumber = wMBus.getSerialNumber();

  msgBufLen = snprintf(msgBuf, msgBufSize, "SN: %u\r\n", serialNumber);
  Serial.print(msgBuf);

  ret = wMBus.setMode(MIP_WMBUS_S2_SHORT_PREAMBLE_MODE, true);
  Serial.print("setMode: ");
  Serial.println(ret);

  ret = wMBus.enableBlock1FromModule();
  Serial.print("enableBlock1FromModule: ");
  Serial.println(ret);

  ret = wMBus.enableRSSI();
  Serial.print("enableRSSI: ");
  Serial.println(ret);

  uint8_t aesKey[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
  ret = wMBus.setEncryptionKey(1, aesKey, 0);
  Serial.print("setEncryptionKey: ");
  Serial.println(ret);

  ret = wMBus.setEncryptionMode(MIP_WMBUS_ENCRYPTION_MODE_5, true);
  Serial.print("setEncryptionMode: ");
  Serial.println(ret);

  uint8_t msg[10] = {0x11, 0x12, 0x13, 0x14, 0x15};
  ret = wMBus.sendMsg(msg, 5);
  Serial.print("sendMsg: ");
  Serial.println(ret);

  uint8_t IV[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
  uint8_t msg2[10] = {0x21, 0x22, 0x23, 0x24, 0x25};

  ret = wMBus.sendEncryptedMsg(msg2, 5, 1, 0, MIP_WMBUS_ENCRYPTION_MODE_5, IV);
  Serial.print("sendEncryptedMsg: ");
  Serial.println(ret);

  delay(500);

  uint8_t msg3[10] = {0x31, 0x32, 0x33, 0x34, 0x35};
  ret = wMBus.sendSyncMsg(msg3, 5, 20);
  Serial.print("sendSyncMsg: ");
  Serial.println(ret);
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
    ret = wMBus.changeSyncMsg(msg, 6);
    Serial.print("changeSyncMsg: ");
    Serial.println(ret);
  }
  i++;

  if(wMBus.isDataAvailable()){
    Serial.println("dataAvailable!");
  }
  
  uint8_t block1[10];
  uint8_t CIField;
  int8_t RSSI;
  
  ret = wMBus.readMsg(block1, &CIField, rxBuf, rxBufSize, &RSSI);
  if(ret > 0){
    
    msgBufLen = snprintf(msgBuf, msgBufSize, "rx msg (%i): ", ret);
    for(int i = 0; i < ret; i++){
      msgBufLen += snprintf(msgBuf + msgBufLen, msgBufSize - msgBufLen, "%02X ", rxBuf[i]);
    }
    Serial.println(msgBuf);
    msgBufLen = snprintf(msgBuf, msgBufSize, "CI Field: %i, RSSI:%i", CIField, RSSI);
    Serial.println(msgBuf);
  }
  
}
