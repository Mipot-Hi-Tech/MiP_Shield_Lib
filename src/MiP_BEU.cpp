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
#include "MiP_BEU.h"


#define MODULE_I2C_ADDRESS          0x62

#define CMD_START_BYTE              0xAA

#define GET_DEV_EUI_CMD             0x36
#define RESUME_OTAA_SESSION_CMD     0x3F
#define JOIN_CMD                    0x40
#define JOIN_IND                    0x41
#define GET_ACTIVATION_STATUS_CMD   0x42
#define SET_APP_KEY_CMD             0x43
#define SET_APP_SESSION_KEY_CMD     0x44
#define SET_NWK_SESSION_KEY_CMD     0x45
#define TX_MSG_CMD                  0x46
#define TX_MSG_CONFIRMED_IND        0x47
#define TX_MSG_UNCONFIRMED_IND      0x48
#define RX_MSG_IND                  0x49
#define GET_SESSION_STATUS_CMD      0x4A
#define SET_NEXT_DR_CMD             0x4B
#define SET_BATTERY_LVL_CMD         0x50
#define GET_BATTERY_LVL_CMD         0x51
#define SET_UPLINK_CNT_CMD          0x52
#define GET_UPLINK_CNT_CMD          0x53
#define SET_DOWNLINK_CNT_CMD        0x54
#define GET_DOWNLINK_CNT_CMD        0x55
#define SET_CH_PARAMETERS_CMD       0x57
#define GET_CH_PARAMETERS_CMD       0x58
#define LINK_CHECK_REQUEST_CMD      0x60
#define LINK_CHECK_REQUEST_IND      0x61
#define DEVICE_TIME_REQUEST_CMD     0x62
#define DEVICE_TIME_REQUEST_IND     0x63
#define RESET_ABP_CMD               0x64
#define RESET_ABP_IND               0x65
#define REKEY_OTAA_CMD              0x66
#define REKEY_OTAA_IND              0x67

//EEPROM Addresses
#define EE_ADDR_CUSTOMER_DEV_EUI    0x00
#define EE_ADDR_APP_EUI             0x08
#define EE_ADDR_DEV_ADDR            0x10
#define EE_ADDR_CLASS               0x20
#define EE_ADDR_DR                  0x21
#define EE_ADDR_TX_POWER            0x22
#define EE_ADDR_ADR                 0x23
#define EE_ADDR_DUTY_CYCLE          0x24
#define EE_ADDR_TX_REPETITION       0x25
#define EE_ADDR_ENABLE_CUSTOMER_EUI 0x26
#define EE_ADDR_RX2_DR              0x27
#define EE_ADDR_RX2_FREQ            0x28
#define EE_ADDR_PUBLIC_NWK          0x2E
#define EE_ADDR_DATA_IND_TIMEOUT    0x80

MipBEU::MipBEU(int nWakePin, int nDataIndicatePin)
  : MipBase(nWakePin, nDataIndicatePin, MODULE_I2C_ADDRESS)
{
  _nWakePin = nWakePin;
  _nDataIndicatePin = nDataIndicatePin;
  _lastRSSI = -999;
  _lastSNR = -999;
}

int MipBEU::getDevEUI(uint8_t devEuiBuf[8])
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_DEV_EUI_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[12];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 12);
  if (ret == 12 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    for (int i = 0; i < 8; i++) {
      devEuiBuf[i] = rxBuf[3 + i];
    }
    return 0;
  }
  return -1;
}

int MipBEU::resumeOtaaSession()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, RESUME_OTAA_SESSION_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if (ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)) {
    return 0;
  }
  return -1;
}

int MipBEU::join(mipActivationMode mode)
{
  uint8_t txBuf[5] = { CMD_START_BYTE, JOIN_CMD, 0x01, (uint8_t)mode, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 5);
  if (ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)) {
    if (mode == MIP_ACTIVATION_ABP) {
      return 0; // ABP doesn't have to wait for JOIN_IND
    }
    // Serial.println("Waiting for JOIN_IND");
    // OTAA need to wait for JOIN_IND
    ret = read(rxBuf, 5, 10000, 10);
    if (ret == 5 && rxBuf[1] == JOIN_IND && rxBuf[3] == 0x00 && isChecksumValid(rxBuf, 5)) {
      return 0;
    }
  }
  return -1;
}

int MipBEU::getActivationStatus()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_ACTIVATION_STATUS_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if (ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    return rxBuf[3]; // 0: Not activated, 1: Joining, 2: Joined, 3: MAC error
  }
  return -1;
}

int MipBEU::setAppKey(const uint8_t appKey[16])
{
  uint8_t txBuf[20];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_APP_KEY_CMD;
  txBuf[2] = 0x10;
  for (int i = 0; i < 16; i++) {
    txBuf[3 + i] = appKey[i];
  }
  txBuf[19] = calculateChecksum(txBuf, 19);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 20, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipBEU::setAppSessionKey(const uint8_t appSKey[16])
{
  uint8_t txBuf[20];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_APP_SESSION_KEY_CMD;
  txBuf[2] = 0x10;
  for (int i = 0; i < 16; i++) {
    txBuf[3 + i] = appSKey[i];
  }
  txBuf[19] = calculateChecksum(txBuf, 19);

  uint8_t rxBuf[4];
  int ret = writeCommandAndReadReply(txBuf, 20, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

int MipBEU::setNetworkSessionKey(const uint8_t nwkSKey[16])
{
  uint8_t txBuf[20];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_NWK_SESSION_KEY_CMD;
  txBuf[2] = 0x10;
  for (int i = 0; i < 16; i++) {
    txBuf[3 + i] = nwkSKey[i];
  }
  txBuf[19] = calculateChecksum(txBuf, 19);

  uint8_t rxBuf[4];
  int ret = writeCommandAndReadReply(txBuf, 20, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

int MipBEU::sendMsg(const uint8_t payload[], int payloadLen, bool confirmed, uint8_t port)
{
  uint8_t txBuf[260];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = TX_MSG_CMD;
  txBuf[2] = payloadLen + 2;
  txBuf[3] = confirmed ? 0x01 : 0x00;
  txBuf[4] = port;
  for (int i = 0; i < payloadLen; i++) {
    txBuf[5 + i] = payload[i];
  }
  int totalLen = 5 + payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if (ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)) {
    uint8_t indBuf[9];
    uint8_t expectedInd = confirmed ? TX_MSG_CONFIRMED_IND : TX_MSG_UNCONFIRMED_IND;
    int indLen = confirmed ? 9 : 7;
    ret = read(indBuf, indLen, 10000, 10);
    if (ret == indLen && indBuf[1] == expectedInd) {
      return 0;
    }
  }
  return -1;
}

int MipBEU::readMsg(uint8_t payloadBuf[], int maxLen, uint8_t *port, bool *isConfirmed)
{
  if (digitalRead(_nDataIndicatePin) == HIGH) {
    return 0;
  }
  
  uint8_t buf[260];
  int len = readVariableLength(buf, sizeof(buf), 10);
  if (len < 10 || buf[1] != RX_MSG_IND || buf[3] != 0x00 || !isChecksumValid(buf, len)) {
    return 0;
  }

  if (isConfirmed){
    *isConfirmed = (buf[4] == 1);
  }
  
  _lastRSSI = (int16_t)(buf[11] | (buf[12] << 8));
  _lastSNR = (int8_t)buf[13];
  
  if (port){
    *port = buf[14];
  }

  int dataLen = buf[2] - 12; // Lunghezza del payload
  if (dataLen > 0 && dataLen <= maxLen) {
    for (int i = 0; i < dataLen; i++) {
      payloadBuf[i] = buf[15 + i];
    }
    return dataLen;
  }
  return 0;
}

int16_t MipBEU::getRSSI()
{
  return _lastRSSI;
}

int8_t MipBEU::getSNR()
{
  return _lastSNR;
}

int MipBEU::getSessionStatus()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_SESSION_STATUS_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if (ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    return rxBuf[3]; // 0: Idle, 1: Busy, 2: Device not activated, 3: Delayed
  }
  return -1;
}

int MipBEU::setNextDataRate(mipDataRateEU dr)
{
  uint8_t txBuf[5] = { CMD_START_BYTE, SET_NEXT_DR_CMD, 0x01, (uint8_t)dr, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipBEU::setBatteryLevel(uint8_t level)
{
  uint8_t txBuf[5] = { CMD_START_BYTE, SET_BATTERY_LVL_CMD, 0x01, level, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[4];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

int MipBEU::getBatteryLevel()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_BATTERY_LVL_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if (ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    return rxBuf[3];
  }
  return -1;
}

int MipBEU::setUplinkCounter(uint32_t count)
{
  uint8_t txBuf[8];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_UPLINK_CNT_CMD;
  txBuf[2] = 0x04;
  txBuf[3] = count & 0xFF;
  txBuf[4] = (count >> 8) & 0xFF;
  txBuf[5] = (count >> 16) & 0xFF;
  txBuf[6] = (count >> 24) & 0xFF;
  txBuf[7] = calculateChecksum(txBuf, 7);

  uint8_t rxBuf[4];
  int ret = writeCommandAndReadReply(txBuf, 8, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

uint32_t MipBEU::getUplinkCounter()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_UPLINK_CNT_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[8];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 8);
  if (ret == 8 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    int counter;
    counter = (uint32_t)rxBuf[3] | ((uint32_t)rxBuf[4] << 8) | ((uint32_t)rxBuf[5] << 16) | ((uint32_t)rxBuf[6] << 24);
    return counter;
  }
  return -1;
}

int MipBEU::setDownlinkCounter(uint32_t count)
{
  uint8_t txBuf[8];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_DOWNLINK_CNT_CMD;
  txBuf[2] = 0x04;
  txBuf[3] = count & 0xFF;
  txBuf[4] = (count >> 8) & 0xFF;
  txBuf[5] = (count >> 16) & 0xFF;
  txBuf[6] = (count >> 24) & 0xFF;
  txBuf[7] = calculateChecksum(txBuf, 7);

  uint8_t rxBuf[4];
  int ret = writeCommandAndReadReply(txBuf, 8, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

uint32_t MipBEU::getDownlinkCounter()
{
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_DOWNLINK_CNT_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[8];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 8);
  if (ret == 8 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    int counter;
    counter = (uint32_t)rxBuf[3] | ((uint32_t)rxBuf[4] << 8) | ((uint32_t)rxBuf[5] << 16) | ((uint32_t)rxBuf[6] << 24);
    return counter;
  }
  return -1;
}

int MipBEU::setChannelParameters(uint8_t chIdx, uint32_t freqHz, uint8_t drRange, uint8_t status)
{
  uint8_t txBuf[11];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_CH_PARAMETERS_CMD;
  txBuf[2] = 0x07;
  txBuf[3] = chIdx;
  txBuf[4] = freqHz & 0xFF;
  txBuf[5] = (freqHz >> 8) & 0xFF;
  txBuf[6] = (freqHz >> 16) & 0xFF;
  txBuf[7] = (freqHz >> 24) & 0xFF;
  txBuf[8] = drRange;
  txBuf[9] = status;
  txBuf[10] = calculateChecksum(txBuf, 10);

  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 11, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipBEU::getChannelParameters(uint8_t chIdx, uint32_t *freqHz, uint8_t *drRange, uint8_t *status)
{
  uint8_t txBuf[5] = { CMD_START_BYTE, GET_CH_PARAMETERS_CMD, 0x01, chIdx, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[10];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 10);
  if (ret == 10 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    if (freqHz) {
      *freqHz = (uint32_t)rxBuf[3] | ((uint32_t)rxBuf[4] << 8) | ((uint32_t)rxBuf[5] << 16) | ((uint32_t)rxBuf[6] << 24);
    }
    if (drRange){
      *drRange = rxBuf[7];
    }
    if (status){
      *status  = rxBuf[8];
    }
    return 0;
  }
  return -1;
}

int MipBEU::linkCheckRequest(uint8_t *margin, uint8_t *gwCnt)
{
  uint8_t txBuf[5] = { CMD_START_BYTE, LINK_CHECK_REQUEST_CMD, 0x01, 0x01, 0xF4 };
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 5);
  if (ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)) {
    uint8_t indBuf[6];
    ret = read(indBuf, 6, 10000, 10);
    if (ret == 6 && indBuf[1] == LINK_CHECK_REQUEST_IND) {
      if (margin){
        *margin = indBuf[3];
      }
      if (gwCnt){
        *gwCnt  = indBuf[4];
      }
      return 0;
    }
  }
  return -1;
}

int MipBEU::deviceTimeRequest(uint32_t *epochSec, uint8_t *epochFrac)
{
  uint8_t txBuf[5] = { CMD_START_BYTE, DEVICE_TIME_REQUEST_CMD, 0x01, 0x01, 0xF2 };
  uint8_t rxBuf[4];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 4);
  if (ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)) {
    uint8_t indBuf[9];
    ret = read(indBuf, 9, 10000, 10);
    if (ret == 9 && indBuf[1] == DEVICE_TIME_REQUEST_IND) {
      if (epochSec) {
        *epochSec = (uint32_t)indBuf[3] | ((uint32_t)indBuf[4] << 8) | ((uint32_t)indBuf[5] << 16) | ((uint32_t)indBuf[6] << 24);
      }
      if (epochFrac){
        *epochFrac = indBuf[7];
      }
      return 0;
    }
  }
  return -1;
}

int MipBEU::resetABP()
{
  uint8_t txBuf[5] = { CMD_START_BYTE, RESET_ABP_CMD, 0x01, 0x01, 0xF0 };
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
  
}

int MipBEU::rekeyOTAA()
{
  uint8_t txBuf[5] = { CMD_START_BYTE, REKEY_OTAA_CMD, 0x01, 0x01, 0xEE };
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipBEU::setCustomerDevEUI(uint8_t customerEui[8])
{
  return setEepromValue(EE_ADDR_CUSTOMER_DEV_EUI, customerEui, 8);
}

int MipBEU::setAppEUI(uint8_t appEui[8])
{
  return setEepromValue(EE_ADDR_APP_EUI, appEui, 8);
}

int MipBEU::setDevAddr(uint32_t devAddr)
{
  return setEepromValue(EE_ADDR_DEV_ADDR, (uint8_t*)&devAddr, 4);
}

int MipBEU::setClass(mipClass classType)
{
  return setEepromValue(EE_ADDR_CLASS, (uint8_t*)&classType, 1);
}

int MipBEU::setDataRate(mipDataRateEU dr)
{
  return setEepromValue(EE_ADDR_DR, (uint8_t*)&dr, 1);
}

int MipBEU::setPower(mipTxPowerEU powerLevel)
{
  return setEepromValue(EE_ADDR_TX_POWER, (uint8_t*)&powerLevel, 1);
}

int MipBEU::enableAdaptiveDataRate()
{
  uint8_t v = 1;
  return setEepromValue(EE_ADDR_ADR, &v, 1);
}

int MipBEU::disableAdaptiveDataRate()
{
  uint8_t v = 0;
  return setEepromValue(EE_ADDR_ADR, &v, 1);
}

int MipBEU::enableDutyCycleControl()
{
  uint8_t v = 1;
  return setEepromValue(EE_ADDR_DUTY_CYCLE, &v, 1);
}

int MipBEU::disableDutyCycleControl()
{
  uint8_t v = 0;
  return setEepromValue(EE_ADDR_DUTY_CYCLE, &v, 1);
}

int MipBEU::setUnconfirmedTxRepetitions(uint8_t repetitions)
{
  return setEepromValue(EE_ADDR_TX_REPETITION, &repetitions, 1);
}

int MipBEU::enableCustomerDevEUI()
{
  uint8_t v = 1;
  return setEepromValue(EE_ADDR_ENABLE_CUSTOMER_EUI, &v, 1);
}

int MipBEU::disableCustomerDevEUI()
{
  uint8_t v = 0;
  return setEepromValue(EE_ADDR_ENABLE_CUSTOMER_EUI, &v, 1);
}

int MipBEU::setRX2DataRate(mipDataRateEU dr)
{
  return setEepromValue(EE_ADDR_RX2_DR, (uint8_t*)&dr, 1);
}

int MipBEU::setRX2Frequency(uint32_t freqHz)
{
  uint8_t freq[4];
  freq[0] = freqHz & 0xFF;
  freq[1] = (freqHz >> 8) & 0xFF;
  freq[2] = (freqHz >> 16) & 0xFF;
  freq[3] = (freqHz >> 24) & 0xFF;
  return setEepromValue(EE_ADDR_RX2_FREQ, freq, 4);
}

int MipBEU::enablePublicNetwork()
{
  uint8_t v = 1;
  return setEepromValue(EE_ADDR_PUBLIC_NWK, &v, 1);
}

int MipBEU::disablePublicNetwork()
{
  uint8_t v = 0;
  return setEepromValue(EE_ADDR_PUBLIC_NWK, &v, 1);
}

int MipBEU::setDataIndicateTimeout(uint8_t timeoutMs)
{
  return setEepromValue(EE_ADDR_DATA_IND_TIMEOUT, &timeoutMs, 1);
}

