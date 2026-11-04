/**
* Copyright (c) Mipot S.p.A. All rights reserved.
*
* BSD-3-Clause
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its
*    contributors may be used to endorse or promote products derived from
*    this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
*/

#include "api/Common.h"


#include "Arduino.h"
#include "MiP_base.h"
#include "Wire.h"


#define CMD_START_BYTE      0xAA
#define RESET_CMD           0x30
#define FACTORY_RESET_CMD   0x31
#define EEPROM_WRITE_CMD    0x32
#define EEPROM_READ_CMD     0x33
#define GET_FW_VERSION_CMD  0x34
#define GET_SERIALNO_CMD    0x35


#define MSG_MIN_SIZE        0x04

void printBuffer(uint8_t* buf, uint32_t bufLen)
{
  Serial.print("buf (");
  Serial.print(bufLen);
  Serial.print("): ");
	for (int i=0; i<bufLen; i++){
    Serial.print(buf[i], HEX);
    Serial.print(' ');
	}
  Serial.println(".");
}

int MipBase::sendCmd(uint8_t *buf, int len)
{
  Wire.beginTransmission(_i2cAddr);
  delay(5);
  for(int i = 0; i < len; i++)
  {
    Wire.write(buf[i]);
  }
  delay(10);

  byte ret = Wire.endTransmission();

  if(ret > 0){
    return ret;
  }
  return 0;
}

int MipBase::read(uint8_t *buf, int len, int timeout, uint8_t dataIndicateDelay)
{
  long startTime = millis();
  uint32_t moduleWaitTimeout = 0;
  while (digitalRead(_nDataIndicatePin) == HIGH) {
    long elapsedTime = millis() - startTime;
    if (elapsedTime > timeout) {
      return -1;
    }
  }
  if(dataIndicateDelay){
    delay(dataIndicateDelay);
  }

  uint32_t nBytesRead = Wire.requestFrom(_i2cAddr, len, true);
  int i = 0;
  // Serial.print("nBytesRead: ");
  // Serial.println(nBytesRead, HEX);
  // Serial.print("While available: ");
  while(Wire.available()){
    buf[i] = Wire.read();
    // Serial.print(buf[i], HEX);
    // Serial.print(" ");
    //For some reason, this delay is necessary on the Arduino UNO R4, otherwise Wire.read() returns wrong data.
    delayMicroseconds(100);   
    i++;
  }
  // Serial.println(".");
  return i;
}

int MipBase::readVariableLength(uint8_t* buf, int bufSize, uint8_t dataIndicateDelay)
{
  if(bufSize < MSG_MIN_SIZE){
    return -1;
  }
  if(dataIndicateDelay){
    delay(dataIndicateDelay);
  }
  // Serial.println("readVariableLength");
  uint32_t nBytesRead = Wire.requestFrom(_i2cAddr, 3, false);
  int i = 0;
  // Serial.print("nBytesRead 1: ");
  // Serial.println(nBytesRead);
  while(Wire.available()){
    buf[i] = Wire.read();
    //Serial.print(buf[i], HEX);
    //Serial.print(" ");
    //For some reason, this delay is necessary on the Arduino UNO R4, otherwise Wire.read() returns wrong data.
    delayMicroseconds(100);
    i++;
  }
  // printBuffer(buf, i);
  uint32_t msgLen = buf[2];
  nBytesRead = Wire.requestFrom(_i2cAddr, msgLen + 1, true);
  // Serial.print("nBytesRead 2: ");
  // Serial.println(nBytesRead);
  while(Wire.available()){
    buf[i] = Wire.read();
    //Serial.print(i, HEX);
    //Serial.print(".");
    //For some reason, this delay is necessary on the Arduino UNO R4, otherwise Wire.read() returns wrong data.
    delayMicroseconds(100);
    i++;
  }
  // printBuffer(buf, i);
  return i;
}

int MipBase::writeCommandAndReadReply(uint8_t* cmdBuf, int cmdBufLen, uint8_t* rpyBuf, int rpyBufLen)
{
  int ret = -1;
  int nWakeStatus;
  nWakeStatus = digitalRead(_nWakePin);
  // Serial.print("nWakeStatus: ");
  // Serial.println(nWakeStatus);
  if(nWakeStatus == HIGH){    //wake up if needed
    digitalWrite(_nWakePin, LOW);
    delay(200);
  }

  // Serial.println("Sending cmd");
  ret = sendCmd(cmdBuf, cmdBufLen);
  if(ret == 0){
    //Serial.println("reading reply");
    ret = read(rpyBuf, rpyBufLen, 4000, 5);
    //Serial.print("read ret: ");
    //Serial.println(ret);
    // if(ret > 0){
    //   printBuffer(rpyBuf, ret);
    // }
  }

  if(nWakeStatus == HIGH){   //return to previous state
    digitalWrite(_nWakePin, HIGH);
    delay(100);
  }
  return ret;
}

uint8_t MipBase::calculateChecksum(uint8_t* buf, int bufLen)
{
  uint8_t n, crc;
	crc = 0;
  Serial.print("cksum (");
  Serial.print(bufLen);
  Serial.print("): ");
	for (n=0; n<bufLen; n++){
		crc += buf[n];
    Serial.print(buf[n], HEX);
    Serial.print(' ');
	}
	crc = (crc ^ 0xFF) + 1;
  Serial.print("= ");
  Serial.println(crc, HEX);
	return crc;
}

int MipBase::isChecksumValid(uint8_t* buf, int bufLen)
{
  uint8_t crc;

  crc = calculateChecksum(buf, bufLen - 1);
  if(crc == buf[bufLen - 1]){
    return 1;
  }
  return 0;
}

int MipBase::isReplySuccess(uint8_t cmd, uint8_t* rpyBuf, int rpyBufLen, bool skipStatus)
{
  if(rpyBuf[1] != (cmd | 0x80)){
    return 0;
  }
  if(!skipStatus){
    if(rpyBufLen < 5){
      return 0;
    }
    if(rpyBuf[3] != 0){
      return 0;
    }
  }
  if(isChecksumValid(rpyBuf, rpyBufLen)){
    return 1;
  }
  return 0;
}

int MipBase::setEepromValue(uint8_t address, uint8_t* value, uint8_t valueLen){
  int ret = -1;
  uint8_t txBuf[20];
  uint8_t rxBuf[5];
  int i;

  if(valueLen + MSG_MIN_SIZE > 20){
    return -1;
  }
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = EEPROM_WRITE_CMD;
  txBuf[2] = 1 + valueLen;  //len
  txBuf[3] = address;  //start
  for(i = 0; i < valueLen; i++){
    txBuf[i+4] = value[i];
  }
  txBuf[i+4] = calculateChecksum(txBuf, i+4);

  ret = writeCommandAndReadReply(txBuf, i+5, rxBuf, 5);
  if(ret == 5){
    if(isReplySuccess(txBuf[1], rxBuf, ret, false)){
      ret = 0;
    }else{
      ret = -1;
    }
  }
  return ret;
}

int MipBase::getEepromValue(uint8_t address, uint8_t* value, uint8_t valueLen){
  int ret = -1;
  uint8_t txBuf[6];
  uint8_t rxBuf[21];

  if(valueLen > (21 - 5)){    //max amount in rx buffer
    return -1;
  }
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = EEPROM_READ_CMD;
  txBuf[2] = 2;
  txBuf[3] = address;
  txBuf[4] = valueLen;
  txBuf[5] = calculateChecksum(txBuf, 5);

  ret = writeCommandAndReadReply(txBuf, 6, rxBuf, 5 + valueLen);
  if(ret == valueLen + 5){
    if(isReplySuccess(txBuf[1], rxBuf, ret, false)){
      for(int i = 0; i < valueLen; i++){
        value[i] = rxBuf[i+4];
        return 0;
      }
    }
  }
  return -1;
}

MipBase::MipBase(int nWakePin, int nDataIndicatePin, uint8_t i2cAddr)
{
  _nWakePin = nWakePin;
  _nDataIndicatePin = nDataIndicatePin;
  _i2cAddr = i2cAddr;
}

void MipBase::begin()
{
  pinMode(_nWakePin, OUTPUT);
  pinMode(_nDataIndicatePin, INPUT);
  digitalWrite(_nWakePin, HIGH);

  // Setting for I2C Bus
  Wire.begin();
  Wire.setTimeout(4000);
}

void MipBase::wakeUp()
{
  digitalWrite(_nWakePin, LOW);
}

void MipBase::sleep()
{
  digitalWrite(_nWakePin, HIGH);
}

int MipBase::softwareReset()
{
  int ret = -1;
  uint8_t txBuf[4];
  uint8_t rxBuf[4];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = RESET_CMD;
  txBuf[2] = 0x00;
  txBuf[3] = 0x26;

  // Serial.println("softwareReset");
  ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 4);
  if(ret == 4){
    // printBuffer(rxBuf, ret);
    if(rxBuf[0] == 0xAA && rxBuf[1] == 0xB0 && rxBuf[2] == 0x00 && rxBuf[3] == 0xA6){ //this command has a reply that is a bit a special case (no status)
      ret = 0;
    }
  }
  Wire.end();
  delay(200);
  Wire.begin();
  return ret;
}

int MipBase::factoryReset()
{
  int ret = -1;
  uint8_t txBuf[4];
  uint8_t rxBuf[5];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = FACTORY_RESET_CMD;
  txBuf[2] = 0x00;
  txBuf[3] = 0x25;

  // Serial.println("factoryReset");

  ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if(ret == 5){
    if(isReplySuccess(txBuf[1], rxBuf, ret, false)){
      ret = 0;
    }else{
      ret = -1;
    }
  }
  Wire.end();
  delay(200);
  Wire.begin();
  return ret;
}


struct mipVersion_s MipBase::getFwVersion()
{
  int ret = -1;
  uint8_t txBuf[4];
  uint8_t rxBuf[8];
  struct mipVersion_s version = {0};

  Serial.println("getFwVersion");

  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = GET_FW_VERSION_CMD;
  txBuf[2] = 0x00;
  txBuf[3] = 0x22;

  ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 8);
  if(ret == 8){
    // printBuffer(rxBuf, ret);
    if(isChecksumValid(rxBuf, 8)){
      version.major = rxBuf[6];
      version.minor = rxBuf[5];
      version.patch = rxBuf[4];
      version.build = rxBuf[3];
    }
  }

  return version;
}

uint32_t MipBase::getSerialNumber()
{
  int ret = -1;
  uint8_t txBuf[4];
  uint8_t rxBuf[8];
  uint32_t serialNumber = 0;

  Serial.println("getSN");

  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = GET_SERIALNO_CMD;
  txBuf[2] = 0x00;
  txBuf[3] = 0x21;

  ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 8);
  if(ret == 8){
    if(isChecksumValid(rxBuf, 8)){
      serialNumber = rxBuf[3];
      serialNumber |= rxBuf[4] << 8;
      serialNumber |= rxBuf[5] << 16;
      serialNumber |= rxBuf[6] << 24;
    }
  }

  return serialNumber;
}

bool MipBase::isDataAvailable()
{
  if(digitalRead(_nDataIndicatePin) == HIGH)
  {
    return false;
  }
  return true;
}

