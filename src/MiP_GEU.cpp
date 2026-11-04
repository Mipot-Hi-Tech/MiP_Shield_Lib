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
#include "MiP_GEU.h"
#include "Wire.h"

#define MODULE_I2C_ADDRESS          0x62

#define CMD_START_BYTE      0xAA

#define CHANGE_STACK_CMD    0x2A
#define GET_STACK_CMD       0x2B

#define EE_ADDR_STACK       0x90

MipGEU::MipGEU(int nWakePin, int nDataIndicatePin)
  : MipBase(nWakePin, nDataIndicatePin, MODULE_I2C_ADDRESS),
  beu(nWakePin, nDataIndicatePin),
  wMBus(nWakePin, nDataIndicatePin)
{
    _nWakePin = nWakePin;
    _nDataIndicatePin = nDataIndicatePin;
}

void MipGEU::begin(){
  mipSelectedStack stack;
  MipBase::begin();
  delay(200);
  MipBase::wakeUp();
  delay(500);

  if(getCurrentStack(&stack) == 0){
    _stack = stack;
  }
  if(_stack == MIP_WMBUS){
    wMBus.begin();
  }
}

int MipGEU::changeStack(mipSelectedStack newStack){
  uint8_t txBuf[5] = { CMD_START_BYTE, CHANGE_STACK_CMD, 0x01, (uint8_t)newStack, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    _stack = newStack;
    if(_stack == MIP_WMBUS){
      wMBus.begin();
    }
    return 0;
  }
  return -1;
}

int MipGEU::getCurrentStack(mipSelectedStack* stack){
  uint8_t txBuf[5] = { CMD_START_BYTE, GET_STACK_CMD, 0x00, 0x00 };
  txBuf[3] = calculateChecksum(txBuf, 3);
  uint8_t rxBuf[6];

  int ret = writeCommandAndReadReply(txBuf, 4, rxBuf, 6);
  if(ret == 6 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    *stack = (mipSelectedStack) rxBuf[4];
    _stack = (mipSelectedStack) rxBuf[4];
    return 0;
  }
  return -1;  
}

int MipGEU::changeStackAndSave(mipSelectedStack newStack){
  int ret;

  ret = setEepromValue(EE_ADDR_STACK, (uint8_t*) &newStack, 1);
  if(ret == 0){
    _stack = newStack;
  }
  return ret;
}

int MipGEU::getSavedStack(mipSelectedStack* stack){
  uint8_t stackVal;
  int ret;

  ret = getEepromValue(EE_ADDR_STACK, &stackVal, 1);
  if(ret == 0){
    *stack = (mipSelectedStack) stackVal;
    _stack = (mipSelectedStack) stackVal;
  }
  return ret;
}

//Common methods
int MipGEU::setDataIndicateTimeout(int timeoutMs){
  if(_stack == MIP_WMBUS){
    return wMBus.setDataIndicateTimeout(timeoutMs);
  }else{
    return beu.setDataIndicateTimeout(timeoutMs);
  }
}

int16_t MipGEU::getRSSI(){
  if(_stack == MIP_WMBUS){
    return wMBus.getRSSI();
  }else{
    return beu.getRSSI();
  }
}

int8_t MipGEU::getSNR(){
  if(_stack == MIP_WMBUS){
    return -0x0F;
  }else{
    return beu.getSNR();
  }
}

//wMBus methods
int MipGEU::getTemperature()
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.getTemperature();
}

int MipGEU::setLseCalibrationPeriod(uint16_t periodS, bool save)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setLseCalibrationPeriod(periodS, save);
}

int MipGEU::getLseCalibrationPeriod()
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.getLseCalibrationPeriod();
}

int MipGEU::setMode(mipWMBusMode mode, bool save)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setMode(mode, save);
}

int MipGEU::setCField(uint8_t cField, bool save)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setCField(cField, save);
}

int MipGEU::setEncryptionMode(mipWMBusEncryptionMode mode, bool save)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setEncryptionMode(mode, save);
}

int MipGEU::setEncryptionKey(uint8_t keyVersion, uint8_t key[], uint8_t keyID)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setEncryptionKey(keyVersion, key, keyID);
}

int MipGEU::sendMsg(uint8_t payload[], uint8_t payloadLen)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.sendMsg(payload, payloadLen);
}

int MipGEU::sendEncryptedMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[])
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.sendEncryptedMsg(payload, payloadLen, encryptionOffset, keyID, encryptionType, IV);
}

int MipGEU::readMsg(uint8_t block1[], uint8_t* CIField, uint8_t payload[], uint8_t payloadMaxLen, int8_t* RSSI)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.readMsg(block1, CIField, payload, payloadMaxLen, RSSI);
}

int MipGEU::sendSyncMsg(uint8_t payload[], uint8_t payloadLen, uint32_t interval)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.sendSyncMsg(payload, payloadLen, interval);
}

int MipGEU::sendEncryptedSyncMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[], uint32_t interval)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.sendEncryptedSyncMsg(payload, payloadLen, encryptionOffset, keyID, encryptionType, IV, interval);
}

int MipGEU::changeSyncMsg(uint8_t payload[], uint8_t payloadLen)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.changeSyncMsg(payload, payloadLen);
}
int MipGEU::changeEncryptedSyncMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[])
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.changeEncryptedSyncMsg(payload, payloadLen, encryptionOffset, keyID, encryptionType, IV);
}

int MipGEU::getMode(mipWMBusMode* mode)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.getMode(mode);
}

int MipGEU::setChannel(mipWMBusRfChannel channel)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setChannel(channel);
}

int MipGEU::setPower(mipWMBusPower power)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setPower(power);
}

int MipGEU::disableSleep()
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.disableSleep();
}

int MipGEU::enableSleep()
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.enableSleep();
}

int MipGEU::enableDeepStop()
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.enableDeepStop();
}

int MipGEU::enableShutdown()
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.enableShutdown();
}

int MipGEU::setRxWindow(uint8_t windowMs)
{
    if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setRxWindow(windowMs);
}

int MipGEU::getCField(uint8_t* cField)
{
  if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.getCField(cField);
}

int MipGEU::setManufacturerID(uint16_t manID)
{
  if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setManufacturerID(manID);
}

int MipGEU::getManufacturerID(uint16_t* manID)
{
  if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.getManufacturerID(manID);
}

int MipGEU::setDeviceID(uint32_t devID)
{
  if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setDeviceID(devID);
}

int MipGEU::getDeviceID(uint32_t* devID)
{
  if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.getDeviceID(devID);
}

int MipGEU::setVersion(uint8_t version)
{
  if(_stack != MIP_WMBUS){
        return -0xFF;
    }
    return wMBus.setVersion(version);
}

int MipGEU::getVersion(uint8_t* version)
{
  if(_stack != MIP_WMBUS){
      return -0xFF;
  }
  return wMBus.getVersion(version);
}

int MipGEU::setDeviceType(uint8_t deviceType)
{
  if(_stack != MIP_WMBUS){
      return -0xFF;
  }
  return wMBus.setDeviceType(deviceType);
}

int MipGEU::getDeviceType(uint8_t* deviceType)
{
  if(_stack != MIP_WMBUS){
      return -0xFF;
  }
  return wMBus.getDeviceType(deviceType);
}

int MipGEU::enableBlock1FromModule()
{
  if(_stack != MIP_WMBUS){
      return -0xFF;
  }
  return wMBus.enableBlock1FromModule();
}

int MipGEU::disableBlock1FromModule()
{
  if(_stack != MIP_WMBUS){
      return -0xFF;
  }
  return wMBus.disableBlock1FromModule();
}

int MipGEU::enableRSSI()
{
  if(_stack != MIP_WMBUS){
      return -0xFF;
  }
  return wMBus.enableRSSI();
}

int MipGEU::disableRSSI()
{
  if(_stack != MIP_WMBUS){
      return -0xFF;
  }
  return wMBus.disableRSSI();
}

// BEU methods
int MipGEU::getDevEUI(uint8_t buf[8]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getDevEUI(buf);
}

int MipGEU::resumeOtaaSession() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.resumeOtaaSession();
}

int MipGEU::join(mipActivationMode mode) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.join(mode);
}

int MipGEU::getActivationStatus() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getActivationStatus();
}

int MipGEU::setAppKey(const uint8_t appKey[16]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setAppKey(appKey);
}

int MipGEU::setAppSessionKey(const uint8_t appSKey[16]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setAppSessionKey(appSKey);
}

int MipGEU::setNetworkSessionKey(const uint8_t nwkSKey[16]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setNetworkSessionKey(nwkSKey);
}

int MipGEU::sendMsg(const uint8_t payload[], int payloadLen, bool confirmed, uint8_t port) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.sendMsg(payload, payloadLen, confirmed, port);
}

int MipGEU::readMsg(uint8_t payloadBuf[], int maxLen, uint8_t *port, bool *isConfirmed) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.readMsg(payloadBuf, maxLen, port, isConfirmed);
}

int MipGEU::getSessionStatus() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getSessionStatus();
}

int MipGEU::setNextDataRate(mipDataRateEU dr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setNextDataRate(dr);
}

int MipGEU::setBatteryLevel(uint8_t level) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setBatteryLevel(level);
}

int MipGEU::getBatteryLevel() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getBatteryLevel();
}

int MipGEU::setUplinkCounter(uint32_t count) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setUplinkCounter(count);
}

uint32_t MipGEU::getUplinkCounter() {
  if(_stack != MIP_LORAWAN){ 
    return 0;
  }
  return beu.getUplinkCounter();
}

int MipGEU::setDownlinkCounter(uint32_t count) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setDownlinkCounter(count);
}

uint32_t MipGEU::getDownlinkCounter() {
  if(_stack != MIP_LORAWAN) {
    return 0;
  }
  return beu.getDownlinkCounter();
}

int MipGEU::setChannelParameters(uint8_t chIdx, uint32_t freqHz, uint8_t drRange, uint8_t status) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setChannelParameters(chIdx, freqHz, drRange, status);
}

int MipGEU::getChannelParameters(uint8_t chIdx, uint32_t *freqHz, uint8_t *drRange, uint8_t *status) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getChannelParameters(chIdx, freqHz, drRange, status);
}

int MipGEU::linkCheckRequest(uint8_t *margin, uint8_t *gwCnt) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.linkCheckRequest(margin, gwCnt);
}

int MipGEU::deviceTimeRequest(uint32_t *epochSec, uint8_t *epochFrac) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.deviceTimeRequest(epochSec, epochFrac);
}

int MipGEU::resetABP() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.resetABP();
}

int MipGEU::rekeyOTAA() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.rekeyOTAA();
}

int MipGEU::setCustomerDevEUI(uint8_t customerEui[8]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setCustomerDevEUI(customerEui);
}

int MipGEU::setAppEUI(uint8_t appEui[8]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setAppEUI(appEui);
}

int MipGEU::setDevAddr(uint32_t devAddr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setDevAddr(devAddr);
}

int MipGEU::setClass(mipClass classType) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setClass(classType);
}

int MipGEU::setDataRate(mipDataRateEU dr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setDataRate(dr);
}

int MipGEU::setPower(mipTxPowerEU powerLevel) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setPower(powerLevel);
}

int MipGEU::enableAdaptiveDataRate() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enableAdaptiveDataRate();
}

int MipGEU::disableAdaptiveDataRate() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disableAdaptiveDataRate();
}

int MipGEU::enableDutyCycleControl() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enableDutyCycleControl();
}

int MipGEU::disableDutyCycleControl() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disableDutyCycleControl();
}

int MipGEU::setUnconfirmedTxRepetitions(uint8_t repetitions) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setUnconfirmedTxRepetitions(repetitions);
}

int MipGEU::enableCustomerDevEUI() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enableCustomerDevEUI();
}

int MipGEU::disableCustomerDevEUI() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disableCustomerDevEUI();
}

int MipGEU::setRX2DataRate(mipDataRateEU dr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setRX2DataRate(dr);
}

int MipGEU::setRX2Frequency(uint32_t freqHz) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setRX2Frequency(freqHz);
}

int MipGEU::enablePublicNetwork() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enablePublicNetwork();
}

int MipGEU::disablePublicNetwork() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disablePublicNetwork();
}