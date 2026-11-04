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
#include "MiP_wMBus.h"
#include "Wire.h"

#define MODULE_I2C_ADDRESS          0x5A    //but only for 32001605*

#define CMD_START_BYTE      0xAA

#define GET_TEMPERATURE_CMD         0x2C
#define GET_RSSI_CMD                0x39
#define LSE_CALIBRATION_CMD         0x3C
#define GET_LSE_CALIBRATION_INT_CMD 0x3D
#define SET_MODE_CMD                0x40
#define SET_C_FIELD_CMD             0x41
#define ENABLE_ENC_KEY_CMD          0x45
#define SET_ENC_KEY_CMD             0x46
#define TX_MSG_CMD                  0x50
#define TX_ENC_MSG_CMD              0x51
#define RX_MSG_CMD                  0x53
#define TX_SYNC_MSG_CMD             0x5A
#define TX_SYNC_ENC_MSG_CMD         0x5B
#define CHG_SYNC_MSG_CMD            0x5C
#define CHG_SYNC_ENC_MSG_CMD        0x5D



#define MAX_PAYLOAD_LEN             251
#define MAX_ENCRYPTED_PAYLOAD_LEN   236

#define KEY_LEN                     16
#define IV_LEN                      16

#define BLOCK_1_LEN                 9


//EEPROM Addresses
#define EE_ADDR_WMBUS_MODE          0x00
#define EE_ADDR_RF_CHANNEL          0x01
#define EE_ADDR_RF_POWER            0x02
#define EE_ADDR_AUTO_SLEEP          0x03
#define EE_ADDR_RX_WINDOW           0x04
#define EE_ADDR_C_FIELD             0x10
#define EE_ADDR_MANUFACTURER_ID     0x11
#define EE_ADDR_DEVICE_ID           0x13
#define EE_ADDR_WMBUS_VERSION       0x17
#define EE_ADDR_DEVICE_TYPE         0x18
#define EE_ADDR_BLOCK1_FROM_MODULE  0x20
#define EE_ADDR_RSSI_ENABLE         0x21
#define EE_ADDR_DATA_IND_TIMEOUT    0x22

#define SLEEP_DISABLE         0x00
#define SLEEP_ENABLE          0x01
#define SLEEP_DEEP_STOP       0x01
#define SLEEP_SHUTDOWN        0x02


MipWMBus::MipWMBus(int nWakePin, int nDataIndicatePin)
    :MipBase(nWakePin, nDataIndicatePin, MODULE_I2C_ADDRESS)
{
  _nWakePin = nWakePin;
  _nDataIndicatePin = nDataIndicatePin;
}

void MipWMBus::begin()
{
  MipBase::begin();

  bool rssiEn;
  getRssiEnabled(&rssiEn);
}

int MipWMBus::getTemperature()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_TEMPERATURE_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[7];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 7);
  if(ret == 7 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    int16_t temperature;
    temperature = rxBuf[4];
    temperature |= rxBuf[5] << 8;
    return temperature;
  }
  return 0xFFFF;
}

int MipWMBus::getRSSI()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_RSSI_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return rxBuf[3];
  }
  return 0xFFFF;
}

int MipWMBus::setLseCalibrationPeriod(uint16_t periodS, bool save)
{
  uint8_t txBuf[7];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = LSE_CALIBRATION_CMD;
  txBuf[2] = 0x03;
  txBuf[3] = save ? 0xFF : 0x00;
  txBuf[4] = periodS & 0xFF;
  txBuf[5] = (periodS >> 8) & 0xFF;
  txBuf[6] = calculateChecksum(txBuf, 6);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 7, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::getLseCalibrationPeriod()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_LSE_CALIBRATION_INT_CMD, 0x00, 0x00};
  txBuf[3] = calculateChecksum(txBuf, 3);

  uint8_t rxBuf[6];
  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 6);
  if(ret == 6 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    int period;
    period =  rxBuf[3];
    period |= rxBuf[4] << 8;
  }
  return -1;
}

int MipWMBus::setMode(mipWMBusMode mode, bool save)
{
  uint8_t txBuf[6];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_MODE_CMD;
  txBuf[2] = 0x02;
  txBuf[3] = save ? 0xFF : 0x00;
  txBuf[4] = mode & 0xFF;
  txBuf[5] = calculateChecksum(txBuf, 5);
  
  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 6, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::setCField(uint8_t cField, bool save)
{
  uint8_t txBuf[6];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_C_FIELD_CMD;
  txBuf[2] = 0x02;
  txBuf[3] = save ? 0xFF : 0x00;
  txBuf[4] = cField;
  txBuf[5] = calculateChecksum(txBuf, 5);
  
  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 6, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::setEncryptionMode(mipWMBusEncryptionMode mode, bool save)
{
  uint8_t txBuf[6];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = ENABLE_ENC_KEY_CMD;
  txBuf[2] = 0x02;
  txBuf[3] = save ? 0xFF : 0x00;
  txBuf[4] = mode;
  txBuf[5] = calculateChecksum(txBuf, 5);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 6, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::setEncryptionKey(uint8_t keyVersion, uint8_t key[], uint8_t keyID)
{
  uint8_t txBuf[22];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_ENC_KEY_CMD;
  txBuf[2] = 0x12;
  txBuf[3] = keyID;
  txBuf[4] = keyVersion;
  int i;
  for(i = 0; i < KEY_LEN; i++){
    txBuf[i + 5] = key[i];
  }
  txBuf[21] = calculateChecksum(txBuf, 21);
  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 22, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::sendMsg(uint8_t payload[], uint8_t payloadLen)
{
  if(payloadLen > MAX_PAYLOAD_LEN){
    return -1;
  }
  uint8_t txBuf[256];   //this may be too big for smaller MCUs
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = TX_MSG_CMD;
  txBuf[2] = payloadLen;
  for(int i = 0; i < payloadLen; i++){
    txBuf[3 + i] = payload[i];
  }
  int totalLen = 3 + payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::sendEncryptedMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[])
{
  if(payloadLen > MAX_ENCRYPTED_PAYLOAD_LEN){
    return -1;
  }
  uint8_t txBuf[260];   //this may be too big for smaller MCUs 
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = TX_ENC_MSG_CMD;
  txBuf[2] = payloadLen + 16 + 3;   //16 bytes for IV and 3 for Enc_Type, Enc_Offset and Key_Idx
  txBuf[3] = encryptionType;
  txBuf[4] = encryptionOffset;
  txBuf[5] = keyID;
  int i;
  for(i = 0; i < IV_LEN; i++){
    txBuf[6 + i] = IV[i];
  }
  int totalLen = 6 + IV_LEN;
  for(i = 0; i < payloadLen; i++){
    txBuf[totalLen + i] = payload[i];
  }
  totalLen += payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::readMsg(uint8_t block1[], uint8_t* CIField, uint8_t payload[], uint8_t payloadMaxLen, int8_t* RSSI)
{
  if (digitalRead(_nDataIndicatePin) == HIGH) {
    return 0;
  }
  uint8_t buf[270];
  int len = readVariableLength(buf, sizeof(buf), 10);
  if(len < 14 || buf[1] != RX_MSG_CMD || !isChecksumValid(buf, len)){
    return 0;
  }
  uint8_t payloadLen;
  if(_rssiEnabled){
    payloadLen = buf[2] - 11;
    *RSSI = buf[len - 2];
  }else{
    payloadLen = buf[2] - 10;
    *RSSI = INT8_MIN;
  }
  for(int i = 0; i < BLOCK_1_LEN; i++){
    block1[i] = buf[3 + i];
  }
  *CIField = buf[3 + BLOCK_1_LEN];
  for(int i = 0; i < payloadLen; i++){
    payload[i] = buf[4 + BLOCK_1_LEN + i];
  }
  return payloadLen;
}

int MipWMBus::sendSyncMsg(uint8_t payload[], uint8_t payloadLen, uint32_t interval)
{
  if(payloadLen > MAX_PAYLOAD_LEN){
    return -1;
  }
  uint8_t txBuf[260];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = TX_SYNC_MSG_CMD;
  txBuf[2] = payloadLen + 4;
  txBuf[3] = interval & 0xFF;
  txBuf[4] = (interval >> 8) & 0xFF;
  txBuf[5] = (interval >> 16) & 0xFF;
  txBuf[6] = (interval >> 24) & 0xFF;
  for(int i; i < payloadLen; i++){
    txBuf[7 + i] = payload[i];
  }
  int totalLen = 7 + payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::sendEncryptedSyncMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[], uint32_t interval)
{
  if(payloadLen > (MAX_ENCRYPTED_PAYLOAD_LEN - 4)){
    return -1;
  }
  uint8_t txBuf[260];   //this may be too big for smaller MCUs 
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = TX_SYNC_ENC_MSG_CMD;
  txBuf[2] = payloadLen + 16 + 3 + 4;   //16 bytes for IV, 3 for Enc_Type, Enc_Offset and Key_Idx and 4 bytes for interval
  txBuf[3] = interval & 0xFF;
  txBuf[4] = (interval >> 8) & 0xFF;
  txBuf[5] = (interval >> 16) & 0xFF;
  txBuf[6] = (interval >> 24) & 0xFF;
  txBuf[7] = encryptionType;
  txBuf[8] = encryptionOffset;
  txBuf[9] = keyID;
  int i;
  for(i = 0; i < IV_LEN; i++){
    txBuf[10 + i] = IV[i];
  }
  int totalLen = 10 + IV_LEN;
  for(i = 0; i < payloadLen; i++){
    txBuf[totalLen + i] = payload[i];
  }
  totalLen += payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::changeSyncMsg(uint8_t payload[], uint8_t payloadLen)
{
  uint8_t txBuf[260];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = CHG_SYNC_MSG_CMD;
  txBuf[2] = payloadLen;
  for(int i = 0; i < payloadLen; i++){
    txBuf[3 + i] = payload[i];
  }
  int totalLen = 3 + payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::changeEncryptedSyncMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[])
{
  if(payloadLen > MAX_ENCRYPTED_PAYLOAD_LEN){
    return -1;
  }
  uint8_t txBuf[260];   //this may be too big for smaller MCUs 
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = CHG_SYNC_ENC_MSG_CMD;
  txBuf[2] = payloadLen + 16 + 3;   //16 bytes for IV and 3 for Enc_Type, Enc_Offset and Key_Idx
  txBuf[3] = encryptionType;
  txBuf[4] = encryptionOffset;
  txBuf[5] = keyID;
  int i;
  for(i = 0; i < IV_LEN; i++){
    txBuf[6 + i] = IV[i];
  }
  int totalLen = 6 + IV_LEN;
  for(i = 0; i < payloadLen; i++){
    txBuf[totalLen + i] = payload[i];
  }
  totalLen += payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipWMBus::getMode(mipWMBusMode* mode)
{
  return getEepromValue(EE_ADDR_WMBUS_MODE, (uint8_t*)mode, 1);
}

int MipWMBus::setChannel(mipWMBusRfChannel channel)
{
  return setEepromValue(EE_ADDR_RF_CHANNEL, (uint8_t*)&channel, 1);
}

int MipWMBus::setPower(mipWMBusPower power)
{
  return setEepromValue(EE_ADDR_RF_POWER, (uint8_t*)&power, 1);
}

int MipWMBus::disableSleep()
{
  uint8_t val = SLEEP_DISABLE;
  return setEepromValue(EE_ADDR_AUTO_SLEEP, &val, 1);
}

int MipWMBus::enableSleep()
{
  uint8_t val = SLEEP_ENABLE;
  return setEepromValue(EE_ADDR_AUTO_SLEEP, &val, 1);
}

int MipWMBus::enableDeepStop()
{
  uint8_t val = SLEEP_DEEP_STOP;
  return setEepromValue(EE_ADDR_AUTO_SLEEP, &val, 1);
}

int MipWMBus::enableShutdown()
{
  uint8_t val = SLEEP_SHUTDOWN;
  return setEepromValue(EE_ADDR_AUTO_SLEEP, &val, 1);
}

int MipWMBus::setRxWindow(uint8_t windowMs)
{
  return setEepromValue(EE_ADDR_RX_WINDOW, &windowMs, 1);
}

int MipWMBus::getCField(uint8_t* cField)
{
  return getEepromValue(EE_ADDR_C_FIELD, cField, 1);
}

int MipWMBus::setManufacturerID(uint16_t manID)
{
  return setEepromValue(EE_ADDR_MANUFACTURER_ID, (uint8_t*)&manID, 2);
}
int MipWMBus::getManufacturerID(uint16_t* manID)
{
  return getEepromValue(EE_ADDR_MANUFACTURER_ID, (uint8_t*)&manID, 2);
}

int MipWMBus::setDeviceID(uint32_t devID)
{
  return setEepromValue(EE_ADDR_DEVICE_ID, (uint8_t*)&devID, 4);
}
int MipWMBus::getDeviceID(uint32_t* devID)
{
  return getEepromValue(EE_ADDR_DEVICE_ID, (uint8_t*)&devID, 4);
}

int MipWMBus::setVersion(uint8_t version)
{
  return setEepromValue(EE_ADDR_WMBUS_VERSION, &version, 1);
}

int MipWMBus::getVersion(uint8_t *version)
{
  return getEepromValue(EE_ADDR_WMBUS_VERSION, version, 1);
}

int MipWMBus::setDeviceType(uint8_t deviceType)
{
  return setEepromValue(EE_ADDR_DEVICE_TYPE, &deviceType, 1);
}

int MipWMBus::getDeviceType(uint8_t *deviceType)
{
  return getEepromValue(EE_ADDR_DEVICE_TYPE, deviceType, 1);
}

int MipWMBus::enableBlock1FromModule()
{
  uint8_t val = 1;
  return setEepromValue(EE_ADDR_BLOCK1_FROM_MODULE, &val, 1);
}

int MipWMBus::disableBlock1FromModule()
{
  uint8_t val = 0;
  return setEepromValue(EE_ADDR_BLOCK1_FROM_MODULE, &val, 1);
}

int MipWMBus::enableRSSI()
{
  uint8_t val = 1;
  int ret;
  ret = setEepromValue(EE_ADDR_RSSI_ENABLE, &val, 1);
  if(ret == 0){
    _rssiEnabled = true;
  }
  return ret;
}

int MipWMBus::disableRSSI()
{
  uint8_t val = 0;
  int ret;
  ret = setEepromValue(EE_ADDR_RSSI_ENABLE, &val, 1);
  if(ret == 0){
    _rssiEnabled = false;
  }
  return ret;
}

int MipWMBus::getRssiEnabled(bool* rssiEnabled)
{
  int ret;
  uint8_t rssiEn;
  ret = getEepromValue(EE_ADDR_RSSI_ENABLE, &rssiEn, 1);
  if(ret == 0){
    if(rssiEn){
      *rssiEnabled = true;
      _rssiEnabled = true;
    }else{
      *rssiEnabled = false;
      _rssiEnabled = false;
    }
    return 0;
  }
  return ret;
}

int MipWMBus::setDataIndicateTimeout(uint8_t timeoutMs)
{
  return setEepromValue(EE_ADDR_DATA_IND_TIMEOUT, &timeoutMs, 1);
}
