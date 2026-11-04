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
#include "MiP_CEU.h"
#include "Wire.h"


#define MODULE_I2C_ADDRESS  0x62

#define CMD_START_BYTE      0xAA

#define ENABLE_PAIRING_CMD          0x40
#define DEVICE_PAIRING_IND          0x41
#define GET_NETWORK_TABLE_SIZE_CMD  0x42
#define GET_NETWORK_TABLE_ROW_CMD   0x43
#define DEL_EN_DEVICE_CMD           0x44
#define DEL_ALL_EN_DEVICE_CMD       0x45
#define PAIRING_REQ_CMD             0x48
#define PAIRING_CONFIRM_IND         0x49
#define GET_ACTIVATION_STATUS_CMD   0x4A
#define TX_MSG_CMD                  0x50
#define TX_MSG_CONFIRMED_IND        0x51
#define TX_MSG_IND                  0x52
#define RX_MSG_IND                  0x53
#define LINK_CHECK_REQ_CMD          0x56
#define LINK_CHECK_ANS_IND          0x57
#define SET_APP_KEY_CMD             0x58
#define TX_SESSION_ABORT_IND        0x59

#define MAX_PAYLOAD_LEN             26


//EEPROM Addresses
#define EE_ADDR_DEVICE_TYPE         0x00
#define EE_ADDR_UNCONFIRMED_TX_NUM  0x01
#define EE_ADDR_CONFIRMED_TX_NUM    0x02
#define EE_ADDR_END_NODE_PAIRING_REQ_PAYLOAD  0x03
#define EE_ADDR_END_NODE_PAIRING_MASTER_ADDR  0x04
#define EE_ADDR_END_NODE_MASTER_TABLE_INDEX   0x08
#define EE_ADDR_POWER                         0x10
#define EE_ADDR_FREQUENCY                     0x11
#define EE_ADDR_RSSI_THRESHOLD                0x12

#define EE_ADDR_DATA_IND_TIMEOUT              0x80
#define EE_ADDR_APP_AES_ENABLE                0x82


MipCEU::MipCEU(int nWakePin, int nDataIndicatePin)
    :MipBase(nWakePin, nDataIndicatePin, MODULE_I2C_ADDRESS)
{
    _nWakePin = nWakePin;
    _nDataIndicatePin = nDataIndicatePin;
}

int MipCEU::enablePairing()
{
  //if master
  uint8_t txBuf[5] = { CMD_START_BYTE, ENABLE_PAIRING_CMD, 0x01, 0x01, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

int MipCEU::disablePairing()
{
  //if master
  uint8_t txBuf[5] = { CMD_START_BYTE, ENABLE_PAIRING_CMD, 0x01, 0x00, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

int MipCEU::getNetworkTableSize()
{
  //if master
  uint8_t txBuf[5] = { CMD_START_BYTE, GET_NETWORK_TABLE_SIZE_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return rxBuf[3];
  }
  return -1;
}

int MipCEU::getNetworkTableRow(uint8_t row, uint32_t* endNodeID, uint8_t* pairingPayload)
{
  //if master
  uint8_t txBuf[5] = { CMD_START_BYTE, GET_NETWORK_TABLE_ROW_CMD, 0x01, row, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[9];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 9);
  if(ret == 9 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    *endNodeID = rxBuf[3];
    *endNodeID |= rxBuf[4] << 8;
    *endNodeID |= rxBuf[5] << 16;
    *endNodeID |= rxBuf[6] << 24;
    *pairingPayload = rxBuf[7];
    return 0;
  }
  return -1;
}

int MipCEU::deleteEndNode(uint32_t nodeID)
{
  //if master
  uint8_t txBuf[8];

  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = DEL_EN_DEVICE_CMD;
  txBuf[2] = 4;
  txBuf[3] = nodeID && 0xFF;
  txBuf[4] = nodeID >> 8;
  txBuf[5] = nodeID >> 16;
  txBuf[6] = nodeID >> 24;
  txBuf[7] = calculateChecksum(txBuf, 7);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 7, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipCEU::deleteAllEndNodes()
{
  //if master
  uint8_t txBuf[4] = { CMD_START_BYTE, DEL_ALL_EN_DEVICE_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  
  uint8_t rxBuf[5];
  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    return 0;
  }
  return -1;
}

int MipCEU::pairingRequest(uint32_t* masterID, uint8_t* idx)
{
  //if end node
  uint8_t txBuf[4] = { CMD_START_BYTE, PAIRING_REQ_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  
  uint8_t rxBuf[10];
  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    //Wait for PAIRING_CONFIRM_IND
    ret = read(rxBuf, 10, 10000, 10);
    if(ret == 10 && rxBuf[1] == PAIRING_CONFIRM_IND && rxBuf[3] == 0x00 && isChecksumValid(rxBuf, 10)){
      *masterID = rxBuf[4];
      *masterID |= rxBuf[5] << 8;
      *masterID |= rxBuf[6] << 16;
      *masterID |= rxBuf[7] << 24;
      *idx = rxBuf[8];
      return 0;
    }
  }
  return -1;
}

int MipCEU::getActivationStatus(uint32_t* masterID)
{
  //if end node
  uint8_t txBuf[4] = { CMD_START_BYTE, GET_ACTIVATION_STATUS_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  
  uint8_t rxBuf[9];
  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 9);
  if(ret == 9 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    if(rxBuf[3] != 0x00){
      *masterID = rxBuf[4];
      *masterID |= rxBuf[5] << 8;
      *masterID |= rxBuf[6] << 16;
      *masterID |= rxBuf[7] << 24;
    }
    return rxBuf[3];
  }
  return -1;
}

int MipCEU::sendMsg(const uint8_t payload[], int payloadLen, bool confirmed, uint32_t destID)
{
  if(payloadLen > MAX_PAYLOAD_LEN){
    return -1;
  }
  uint8_t txBuf[35];
  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = TX_MSG_CMD;
  txBuf[2] = payloadLen + 5; //4 dstID + 1 options
  txBuf[3] = confirmed ? 1 : 0;
  txBuf[4] = destID && 0xFF;
  txBuf[5] = (destID >> 8) && 0xFF;
  txBuf[6] = (destID >> 16) && 0xFF;
  txBuf[7] = (destID >> 24) && 0xFF;

  for(int i = 0; i < payloadLen; i++){
    txBuf[8 + i] = payload[i];
  }
  int totalLen = 8 + payloadLen;
  txBuf[totalLen] = calculateChecksum(txBuf, totalLen);

  uint8_t rxBuf[11];
  int ret = writeCommandAndReadReply(txBuf, totalLen + 1, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    if(rxBuf[3] == 0){
      uint8_t expectedInd = confirmed ? TX_MSG_CONFIRMED_IND : TX_MSG_IND;
      uint8_t indLen = confirmed ? 11 : 9;
      ret = read(rxBuf, indLen, 10000, 10);
      if(ret == indLen && rxBuf[1] == expectedInd){
        if(rxBuf[3] != 0){
          return 0;
        }
        uint32_t sessionTime;
        sessionTime = rxBuf[4];
        sessionTime = rxBuf[5] << 8;
        sessionTime = rxBuf[6] << 16;
        sessionTime = rxBuf[7] << 24;
        return sessionTime;
      }
    }
    return -rxBuf[3];
  }
  return -1;
}

int MipCEU::readMsg(uint8_t payloadBuf[], int payloadMaxLen, uint32_t* sourceID)
{
  int len;
  if(digitalRead(_nDataIndicatePin) == HIGH){
    return 0;
  }
  uint8_t rxBuf[38];
  len = readVariableLength(rxBuf, 38, 6);
  if(len < 12){
    return 0;
  }
  if(rxBuf[3] != 0x00){
    return 0;
  }
  if(!isChecksumValid(rxBuf, len)){
    return 0;
  }
  _lastRSSI = rxBuf[4];
  _lastRSSI |= rxBuf[5] << 8;
  _lastSNR = rxBuf[6];
  *sourceID = rxBuf[7];
  *sourceID |= rxBuf[8] << 8;
  *sourceID |= rxBuf[9] << 16;
  *sourceID |= rxBuf[10] << 24;

  int payloadLen = rxBuf[2] - 8;
  for(int i = 0; i < payloadLen && i < payloadMaxLen; i++){
    payloadBuf[i] = rxBuf[11 + i];
  }
  return payloadLen;
}

int MipCEU::linkCheckRequest(uint8_t power, uint8_t msgNumber, uint8_t threshold)
{
  //if end node
  uint8_t txBuf[7] = { CMD_START_BYTE, LINK_CHECK_REQ_CMD, 0x03, power, msgNumber, threshold, 0x00 };
  txBuf[6] = calculateChecksum(txBuf, 6);
  
  uint8_t rxBuf[6];
  int ret = writeCommandAndReadReply(txBuf, 7, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    //should report error status?
    //Wait for link check answer ind
    ret = read(rxBuf, 6, 10000, 10);
    if(ret == 6 && rxBuf[1] == LINK_CHECK_ANS_IND){
      if(rxBuf[3] == 0){
        return 0;
      }else{
        return rxBuf[4];
      }
    }
  }
  return -1;
}

int MipCEU::setAppKey(uint8_t key[])
{
  int ret = -1;
  uint8_t txBuf[20];
  uint8_t rxBuf[4];
  int i;

  txBuf[0] = CMD_START_BYTE;
  txBuf[1] = SET_APP_KEY_CMD;
  txBuf[2] = 0x10;
  for(i=0; i < 16; i++){
    txBuf[i+3] = key[i];
  }
  txBuf[i+3] = calculateChecksum(txBuf, 19);
  ret = writeCommandAndReadReply(txBuf, 20, rxBuf, 4);
  if(ret == 4 && isReplySuccess(txBuf[1], rxBuf, ret, true)){
    return 0;
  }
  return -1;
}

int MipCEU::setDeviceType(mipCeuMode mode)
{
  return setEepromValue(EE_ADDR_DEVICE_TYPE, (uint8_t*)&mode, 1);
}

int MipCEU::setUnconfirmedTxNumber(uint8_t num)
{
  return setEepromValue(EE_ADDR_UNCONFIRMED_TX_NUM, &num, 1);
}

int MipCEU::setConfirmedTxNumber(uint8_t num)
{
  return setEepromValue(EE_ADDR_CONFIRMED_TX_NUM, &num, 1);
}

int MipCEU::setEndNodePairingRequestPayload(uint8_t payload)
{
  return setEepromValue(EE_ADDR_END_NODE_PAIRING_REQ_PAYLOAD, &payload, 1);
}

int MipCEU::getMasterAddress(uint32_t* masterAddress)
{
  return getEepromValue(EE_ADDR_END_NODE_PAIRING_MASTER_ADDR, (uint8_t*)masterAddress, 4);
}

int MipCEU::getEndNodeMasterTableIndex(uint8_t* index)
{
  return getEepromValue(EE_ADDR_END_NODE_MASTER_TABLE_INDEX, index, 1);
}

int MipCEU::setPower(uint8_t power)
{
  return setEepromValue(EE_ADDR_POWER, &power, 1);
}

int MipCEU::setFrequency(mipCeuFrequencyIndex frequency)
{
  return setEepromValue(EE_ADDR_FREQUENCY, (uint8_t*)&frequency, 1);
}

int MipCEU::setRssiThreshold(uint8_t rssiThreshold)
{
  return setEepromValue(EE_ADDR_RSSI_THRESHOLD, &rssiThreshold, 1);
}

int MipCEU::setDataIndicateTimeout(uint8_t timeoutMs)
{
  return setEepromValue(EE_ADDR_DATA_IND_TIMEOUT, &timeoutMs, 1);
}

int MipCEU::enableAES()
{
  uint8_t aes = 1;
  return setEepromValue(EE_ADDR_APP_AES_ENABLE, &aes, 1);
}

int MipCEU::disableAES()
{
  uint8_t aes = 0;
  return setEepromValue(EE_ADDR_APP_AES_ENABLE, &aes, 1);
}
