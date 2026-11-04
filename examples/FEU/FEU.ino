
#include "MiP_FEU.h"

#define N_WAKE_PIN          4
#define N_DATA_INDICATE_PIN 3
#define N_RESET_PIN         2

MipFEU feu(N_WAKE_PIN, N_DATA_INDICATE_PIN);

const int msgBufSize = 100;
char msgBuf[msgBufSize];
int msgBufLen;

mipSelectedStack stack;

void setup() {
  

  Serial.begin(9600);
  // while the serial stream is not open, do nothing:
  while (!Serial) ;
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(N_RESET_PIN, OUTPUT_OPENDRAIN);
  digitalWrite(N_RESET_PIN, HIGH);

  Serial.println("======================");
  feu.begin();
  feu.wakeUp();

  delay(200);

  int ret = feu.softwareReset();
  Serial.print("reset: ");
  Serial.println(ret);

  delay(800);

  struct mipVersion_s fwVersion;
  fwVersion = feu.getFwVersion();
  msgBufLen = snprintf(msgBuf, msgBufSize, "FW version:%u.%u.%u.%u\r\n", fwVersion.major, fwVersion.minor, fwVersion.patch, fwVersion.build);
  Serial.print(msgBuf);

  
  uint32_t serialNumber;
  serialNumber = feu.getSerialNumber();

  msgBufLen = snprintf(msgBuf, msgBufSize, "SN: %u\r\n", serialNumber);
  Serial.print(msgBuf);

  //mipSelectedStack stack;
  ret = feu.getCurrentStack(&stack);
  msgBufLen = snprintf(msgBuf, msgBufSize, "ret getCurrentStack: %i, stack:%i\r\n", ret, stack);
  Serial.print(msgBuf);

  stack = MIP_LORAWAN;
  ret = feu.changeStack(stack);
  msgBufLen = snprintf(msgBuf, msgBufSize, "ret changeStack: %i\r\n", ret);
  Serial.print(msgBuf);

  if(stack == MIP_LORA_MODEM){

    uint8_t aesKey[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    ret = feu.setAesKey(aesKey);
    Serial.print("set AES key: ");
    Serial.println(ret);

    uint8_t iv[16] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    ret = feu.setIV(iv);
    Serial.print("set IV: ");
    Serial.println(ret);

    ret = feu.setPower(10);
    Serial.print("setPower: ");
    Serial.println(ret);

    ret = feu.disableAES();
    Serial.print("disableAES: ");
    Serial.println(ret);

    uint8_t msg[10] = {0x11, 0x12, 0x13, 0x14, 0x15};
    ret = feu.sendMsg(msg, 5);
    Serial.print("sendMsg: ");
    Serial.println(ret);
  }else{  //LoRaWAN
    int activationStatus;

    activationStatus = feu.getActivationStatus();
    msgBufLen = snprintf(msgBuf, msgBufSize, "Activation status: %i\r\n", activationStatus);
    Serial.print(msgBuf);

    uint8_t devEUI[8];

    ret = feu.getDevEUI(devEUI);
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
    ret = feu.setAppEUI(appEUI);
    msgBufLen = snprintf(msgBuf, msgBufSize, "setAppEUI: %i\r\n", ret);
    Serial.print(msgBuf);
    
    //appKey: FE 7F 53 C7 64 F8 90 44 6B A4 37 2C 60 4A 19 64
    uint8_t appKey[16] = {0x64, 0x19, 0x4A, 0x60, 0x2C, 0x37, 0xA4, 0x6B, 0x44, 0x90, 0xF8, 0x64, 0xC7, 0x53, 0x7F, 0xFE};
    ret = feu.setAppKey(appKey);
    msgBufLen = snprintf(msgBuf, msgBufSize, "setAppKey: %i\r\n", ret);
    Serial.print(msgBuf);
    
    ret = feu.enablePublicNetwork();
    msgBufLen = snprintf(msgBuf, msgBufSize, "enablePublicNetwork: %i\r\n", ret);
    Serial.print(msgBuf);

    delay(800);

    ret = feu.join(MIP_ACTIVATION_OTAA);
    msgBufLen = snprintf(msgBuf, msgBufSize, "join: %i\r\n", ret);
    Serial.print(msgBuf);

    feu.sleep();

    uint8_t msg[10] = {0x11, 0x12, 0x13, 0x14, 0x15};
    ret = feu.sendMsg(msg, 5, false, 10);
    msgBufLen = snprintf(msgBuf, msgBufSize, "sendMsg: %i\r\n", ret);
    Serial.print(msgBuf);
  }

  

  
}

void loop() {
  const int rxBufSize = 100;
  uint8_t rxBuf[rxBufSize];
  int ret;
  static int i = 0;
  bool shouldSend = false;

  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  if(feu.isDataAvailable()){
    Serial.println("dataAvailable!");
  }
  if(i == 60){
    i = 0;
    shouldSend = true;
  }
  i++;
  
  if(stack == MIP_LORA_MODEM){
    ret = feu.readMsg(rxBuf, rxBufSize);
    if(ret > 0){
      
      msgBufLen = snprintf(msgBuf, msgBufSize, "rx msg (%i): ", ret);
      for(int i = 0; i < ret; i++){
        msgBufLen += snprintf(msgBuf + msgBufLen, msgBufSize - msgBufLen, "%02X ", rxBuf[i]);
      }
      Serial.println(msgBuf);
      msgBufLen = snprintf(msgBuf, msgBufSize, "RSSI: %i, SNR: %i", feu.getRSSI(), feu.getSNR());
      Serial.println(msgBuf);
    }
  }else{
    if(shouldSend){
      uint8_t msg[10] = {0x11, 0x12, 0x13, 0x14, 0x15};
      ret = feu.sendMsg(msg, 5, false, 10);
    }
    ret = feu.readMsg(rxBuf, rxBufSize, NULL, NULL);
    if(ret > 0){
      
      msgBufLen = snprintf(msgBuf, msgBufSize, "rx msg (%i): ", ret);
      for(int i = 0; i < ret; i++){
        msgBufLen += snprintf(msgBuf + msgBufLen, msgBufSize - msgBufLen, "%02X ", rxBuf[i]);
      }
      Serial.println(msgBuf);
      msgBufLen = snprintf(msgBuf, msgBufSize, "RSSI: %i, SNR: %i", feu.getRSSI(), feu.getSNR());
      Serial.println(msgBuf);
    }
  }
  
}
