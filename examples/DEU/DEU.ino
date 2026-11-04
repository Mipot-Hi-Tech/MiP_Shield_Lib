
#include "MiP_DEU.h"

#define N_WAKE_PIN          4
#define N_DATA_INDICATE_PIN 3
#define N_RESET_PIN         2

MipDEU deu(N_WAKE_PIN, N_DATA_INDICATE_PIN);

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

  Serial.println("Setup");
  deu.begin();

  delay(200);

  int ret = deu.softwareReset();
  Serial.print("reset: ");
  Serial.println(ret);

  delay(800);

  struct mipVersion_s fwVersion;
  fwVersion = deu.getFwVersion();
  msgBufLen = snprintf(msgBuf, msgBufSize, "FW version:%u.%u.%u.%u\r\n", fwVersion.major, fwVersion.minor, fwVersion.patch, fwVersion.build);
  Serial.print(msgBuf);

  
  uint32_t serialNumber;
  serialNumber = deu.getSerialNumber();

  msgBufLen = snprintf(msgBuf, msgBufSize, "SN: %u\r\n", serialNumber);
  Serial.print(msgBuf);

  uint8_t aesKey[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
  ret = deu.setAesKey(aesKey);
  Serial.print("set AES key: ");
  Serial.println(ret);

  uint8_t iv[16] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
  ret = deu.setIV(iv);
  Serial.print("set IV: ");
  Serial.println(ret);

  ret = deu.setPower(10);
  Serial.print("setPower: ");
  Serial.println(ret);

  uint8_t msg[10] = {0x11, 0x12, 0x13, 0x14, 0x15};
  ret = deu.sendMsg(msg, 5);
  Serial.print("sendMsg: ");
  Serial.println(ret);
  
  deu.startRx();
  
}

void loop() {
  const int rxBufSize = 100;
  uint8_t rxBuf[rxBufSize];
  int ret;

  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  if(deu.isDataAvailable()){
    Serial.println("dataAvailable!");
  }


  ret = deu.readMsg(rxBuf, rxBufSize);
  if(ret > 0){
    
    msgBufLen = snprintf(msgBuf, msgBufSize, "rx msg (%i): ", ret);
    for(int i = 0; i < ret; i++){
      msgBufLen += snprintf(msgBuf + msgBufLen, msgBufSize - msgBufLen, "%02X ", rxBuf[i]);
    }
    Serial.println(msgBuf);
    msgBufLen = snprintf(msgBuf, msgBufSize, "RSSI: %i, SNR: %i", deu.getRSSI(), deu.getSNR());
    Serial.println(msgBuf);
  }
}
