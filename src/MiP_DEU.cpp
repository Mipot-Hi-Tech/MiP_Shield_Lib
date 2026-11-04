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

#include "Arduino.h"
#include "MiP_Deu.h"
#include "Wire.h"

#define MODULE_I2C_ADDRESS  0x62

#define CMD_START_BYTE      0xAA

#define TX_MSG_CMD          0x50
#define TX_MSG_IND          0x52
#define RX_MSG_IND          0x53
#define SET_AES_KEY_CMD     0x58
#define SET_IV_CMD          0x59


// #define MSG_MIN_SIZE        0x04


MipDEU::MipDEU(int nWakePin, int nDataIndicatePin)
  : MipBase(nWakePin, nDataIndicatePin, MODULE_I2C_ADDRESS)
{
  _nWakePin = nWakePin;
  _nDataIndicatePin = nDataIndicatePin;
  _lastRSSI = -999;
  _lastSNR = -999;
}

void MipDEU::startRx()
{
  wakeUp();
}

void MipDEU::stopRx()
{
  sleep();
}

int MipDEU::sendMsg(uint8_t buf[], int bufLen)
{
  int ret = -1;
  int timeOnAir = 0;
  uint8_t txBuf[20];
  uint8_t rxBuf[9];
  int i;

  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = TX_MSG_CMD;
  txBuf[2] = bufLen;
  for(i=0; i < bufLen; i++){
    txBuf[i+3] = buf[i];
  }
  txBuf[i+3] = calculateChecksum(txBuf, i+3);

  ret = writeCommandAndReadReply(txBuf, i+4, rxBuf, 5);
  if(ret == 5){
    if(isReplySuccess(txBuf[1], rxBuf, ret, false)){
      Serial.println("waiting for TX_IND");
      ret = read(rxBuf, 9, 9000, 10);
      Serial.print("ret: ");
      Serial.println(ret);
      if(ret == 9){
        if(rxBuf[1] == TX_MSG_IND && rxBuf[2] == 0x05 && rxBuf[3] == 0 && isChecksumValid(rxBuf, 9)){
          timeOnAir = rxBuf[4];
          timeOnAir |= rxBuf[5] << 8;
          timeOnAir |= rxBuf[6] << 16;
          timeOnAir |= rxBuf[7] << 24;
          return timeOnAir;
        }
      }
    }
  }
  return -1;
}

int MipDEU::readMsg(uint8_t* payload, int payloadMaxLen)
{
  int len;
  if(digitalRead(_nDataIndicatePin) == HIGH){
    return 0;
  }
  uint8_t buf[250];
  len = readVariableLength(buf, sizeof(buf), 10);
  if(buf[3] != 0x00){ //status byte
    return 0;
  }
  if(!isChecksumValid(buf, len)){
    return 0;
  }
  _lastRSSI = buf[4];
  _lastRSSI |= buf[5] << 8;
  _lastSNR = buf[6];

  int dataLen = buf[2] - 4;
  if (dataLen > 0 && dataLen <= payloadMaxLen) {
    for (int i = 0; i < dataLen; i++) {
      payload[i] = buf[7 + i];
    }
    return dataLen;
  }
  return len - 8;
}

int16_t MipDEU::getRSSI()
{
  return _lastRSSI;
}

int8_t MipDEU::getSNR()
{
  return _lastSNR;
}

int MipDEU::setAesKey(uint8_t key[])
{
  int ret = -1;
  uint8_t txBuf[20];
  uint8_t rxBuf[5];
  int i;

  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_AES_KEY_CMD;
  txBuf[2] = 0x10;
  for(i=0; i < 16; i++){
    txBuf[i+3] = key[i];
  }
  txBuf[i+3] = calculateChecksum(txBuf, 19);

  ret = writeCommandAndReadReply(txBuf, 20, rxBuf, 5);
  if(ret == 5){
    if(isReplySuccess(txBuf[1], rxBuf, ret, false)){
      ret = 0;
    }else{
      ret = -1;
    }
  }
  return ret;
}

int MipDEU::setIV(uint8_t iv[])
{
  int ret = -1;
  uint8_t txBuf[20];
  uint8_t rxBuf[5];
  int i;

  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_IV_CMD;
  txBuf[2] = 0x10;
  for(i=0; i < 16; i++){
    txBuf[i+3] = iv[i];
  }
  txBuf[i+3] = calculateChecksum(txBuf, 19);

  ret = writeCommandAndReadReply(txBuf, 20, rxBuf, 5);
  if(ret == 5){
    if(isReplySuccess(txBuf[1], rxBuf, ret, false)){
      ret = 0;
    }else{
      ret = -1;
    }
  }
  return ret;
}

int MipDEU::setPower(int powerLevel)
{
  return setEepromValue(0, (uint8_t*)&powerLevel, 1);
}

int MipDEU::setFrequency(mipFrequencyChannelIndex frequency)
{
  return setEepromValue(1, (uint8_t*)&frequency, 1);
}

int MipDEU::setBandwidth(mipBandwidthIndex bandwidth)
{
  return setEepromValue(2, (uint8_t*)&bandwidth, 1);
}

int MipDEU::setSpreadingFactor(mipSpreadingFactorIndex spreadingFactor)
{
  return setEepromValue(3, (uint8_t*)&spreadingFactor, 1);
}

int MipDEU::setCodeRate(mipCodeRateIndex codeRate)
{
  return setEepromValue(4, (uint8_t*)&codeRate, 1);
}

int MipDEU::setDataIndicateTimeout(int dataIndicateTimeoutMs)
{
  return setEepromValue(5, (uint8_t*)&dataIndicateTimeoutMs, 1);
}

int MipDEU::enableAES()
{
  uint8_t v = 1;
  return setEepromValue(7, &v, 1);
}

int MipDEU::disableAES()
{
  uint8_t v = 0;
  return setEepromValue(7, &v, 1);
}
