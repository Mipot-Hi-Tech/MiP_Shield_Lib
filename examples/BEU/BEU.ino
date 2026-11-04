
#include "MiP_BEU.h"

#define N_WAKE_PIN          4
#define N_DATA_INDICATE_PIN 3
#define N_RESET_PIN         2

MipBEU beu(N_WAKE_PIN, N_DATA_INDICATE_PIN);

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
  beu.begin();

  delay(200);

  int ret = beu.softwareReset();
  Serial.print("reset: ");
  Serial.println(ret);

  delay(800);

  struct mipVersion_s fwVersion;
  fwVersion = beu.getFwVersion();
  msgBufLen = snprintf(msgBuf, msgBufSize, "FW version:%u.%u.%u.%u\r\n", fwVersion.major, fwVersion.minor, fwVersion.patch, fwVersion.build);
  Serial.print(msgBuf);

  
  uint32_t serialNumber;
  serialNumber = beu.getSerialNumber();

  msgBufLen = snprintf(msgBuf, msgBufSize, "SN: %u\r\n", serialNumber);
  Serial.print(msgBuf);

  int activationStatus;

  activationStatus = beu.getActivationStatus();
  msgBufLen = snprintf(msgBuf, msgBufSize, "Activation status: %i\r\n", activationStatus);
  Serial.print(msgBuf);

  uint8_t devEUI[8];

  ret = beu.getDevEUI(devEUI);
  msgBufLen = snprintf(msgBuf, msgBufSize, "ret devEUI: %i\r\n", ret);
  Serial.print(msgBuf);

  msgBufLen = snprintf(msgBuf, msgBufSize, "devEUI: ", ret);
  for(int i = 0; i < 8; i++){
    msgBufLen += snprintf(msgBuf + msgBufLen, msgBufSize - msgBufLen, "%02X ", devEUI[i]);
  }
  Serial.println(msgBuf);

  //The appEUI and the app Key are written to the module in little endian format
  //appEUI: 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
  uint8_t appEUI[8] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
  ret = beu.setAppEUI(appEUI);
  msgBufLen = snprintf(msgBuf, msgBufSize, "setAppEUI: %i\r\n", ret);
  Serial.print(msgBuf);
  
  //appKey: FE 7F 53 C7 64 F8 90 44 6B A4 37 2C 60 4A 19 64
  uint8_t appKey[16] = {0x64, 0x19, 0x4A, 0x60, 0x2C, 0x37, 0xA4, 0x6B, 0x44, 0x90, 0xF8, 0x64, 0xC7, 0x53, 0x7F, 0xFE};
  ret = beu.setAppKey(appKey);
  msgBufLen = snprintf(msgBuf, msgBufSize, "setAppKey: %i\r\n", ret);
  Serial.print(msgBuf);
  
  ret = beu.enablePublicNetwork();
  msgBufLen = snprintf(msgBuf, msgBufSize, "enablePublicNetwork: %i\r\n", ret);
  Serial.print(msgBuf);

  ret = geu.softwareReset();
  Serial.print("reset: ");
  Serial.println(ret);

  delay(800);

  ret = beu.join(MIP_ACTIVATION_OTAA);
  msgBufLen = snprintf(msgBuf, msgBufSize, "join: %i\r\n", ret);
  Serial.print(msgBuf);

  beu.sleep();

  uint8_t msg[10] = {0x11, 0x12, 0x13, 0x14, 0x15};
  ret = beu.sendMsg(msg, 5, false, 10);
  msgBufLen = snprintf(msgBuf, msgBufSize, "sendMsg: %i\r\n", ret);
  Serial.print(msgBuf);

  
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
  
  if(i == 300){
    i = 0;
    uint8_t msg[10] = {0x11, 0x12, 0x13, 0x14, 0x15};
    ret = beu.sendMsg(msg, 5, false, 10);
  }
  i++;

  if(beu.isDataAvailable()){
    Serial.println("dataAvailable!");
  }
  ret = beu.readMsg(rxBuf, rxBufSize, NULL, NULL);
  if(ret > 0){
    
    msgBufLen = snprintf(msgBuf, msgBufSize, "rx msg (%i): ", ret);
    for(int i = 0; i < ret; i++){
      msgBufLen += snprintf(msgBuf + msgBufLen, msgBufSize - msgBufLen, "%02X ", rxBuf[i]);
    }
    Serial.println(msgBuf);
    msgBufLen = snprintf(msgBuf, msgBufSize, "RSSI: %i, SNR: %i", beu.getRSSI(), beu.getSNR());
    Serial.println(msgBuf);
  }
}
