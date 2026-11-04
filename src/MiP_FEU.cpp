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
#include "MiP_FEU.h"
#include "Wire.h"

#define MODULE_I2C_ADDRESS          0x62

#define CMD_START_BYTE      0xAA

#define CHANGE_STACK_CMD    0x2A
#define GET_STACK_CMD       0x2B

#define EE_ADDR_STACK       0x90

MipFEU::MipFEU(int nWakePin, int nDataIndicatePin)
  : MipBase(nWakePin, nDataIndicatePin, MODULE_I2C_ADDRESS),
  beu(nWakePin, nDataIndicatePin),
  deu(nWakePin, nDataIndicatePin)
{
    _nWakePin = nWakePin;
    _nDataIndicatePin = nDataIndicatePin;
}

void MipFEU::begin(){
  mipSelectedStack stack;
  MipBase::begin();
  delay(200);
  MipBase::wakeUp();
  delay(500);

  if(getCurrentStack(&stack) == 0){
    _stack = stack;
  }
  
}

int MipFEU::changeStack(mipSelectedStack newStack){
  uint8_t txBuf[5] = { CMD_START_BYTE, CHANGE_STACK_CMD, 0x01, (uint8_t)newStack, 0x00 };
  txBuf[4] = calculateChecksum(txBuf, 4);
  uint8_t rxBuf[5];

  int ret = writeCommandAndReadReply(txBuf, 5, rxBuf, 5);
  if(ret == 5 && isReplySuccess(txBuf[1], rxBuf, ret, false)){
    _stack = newStack;
    return 0;
  }
  return -1;
}

int MipFEU::getCurrentStack(mipSelectedStack* stack){
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

int MipFEU::changeStackAndSave(mipSelectedStack newStack){
  int ret;

  ret = setEepromValue(EE_ADDR_STACK, (uint8_t*) &newStack, 1);
  if(ret == 0){
    _stack = newStack;
  }
  return ret;
}

int MipFEU::getSavedStack(mipSelectedStack* stack){
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
int MipFEU::setDataIndicateTimeout(int timeoutMs){
  if(_stack == MIP_LORA_MODEM){
    return deu.setDataIndicateTimeout(timeoutMs);
  }else{
    return beu.setDataIndicateTimeout(timeoutMs);
  }
}

int16_t MipFEU::getRSSI(){
  if(_stack == MIP_LORA_MODEM){
    return deu.getRSSI();
  }else{
    return beu.getRSSI();
  }
}

int8_t MipFEU::getSNR(){
  if(_stack == MIP_LORA_MODEM){
    return deu.getSNR();
  }else{
    return beu.getSNR();
  }
}

//DEU methods
int MipFEU::sendMsg(uint8_t buf[], int bufLen){
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.sendMsg(buf, bufLen);
}

int MipFEU::readMsg(uint8_t buf[], int bufLen){
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.readMsg(buf, bufLen);
}

int MipFEU::setAesKey(uint8_t key[]) {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.setAesKey(key);
}

int MipFEU::setIV(uint8_t buf[]) {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.setIV(buf);
}

int MipFEU::setPower(int powerLevel) {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.setPower(powerLevel);
}

int MipFEU::setFrequency(mipFrequencyChannelIndex frequency) {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.setFrequency(frequency);
}

int MipFEU::setBandwidth(mipBandwidthIndex bandwidth) {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.setBandwidth(bandwidth);
}

int MipFEU::setSpreadingFactor(mipSpreadingFactorIndex spreadingFactor) {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.setSpreadingFactor(spreadingFactor);
}

int MipFEU::setCodeRate(mipCodeRateIndex codeRate) {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.setCodeRate(codeRate);
}

int MipFEU::enableAES() {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.enableAES();
}

int MipFEU::disableAES() {
  if(_stack != MIP_LORA_MODEM){
    return -0xFF;
  }
  return deu.disableAES();
}

// BEU methods
int MipFEU::getDevEUI(uint8_t buf[8]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getDevEUI(buf);
}

int MipFEU::resumeOtaaSession() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.resumeOtaaSession();
}

int MipFEU::join(mipActivationMode mode) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.join(mode);
}

int MipFEU::getActivationStatus() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getActivationStatus();
}

int MipFEU::setAppKey(const uint8_t appKey[16]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setAppKey(appKey);
}

int MipFEU::setAppSessionKey(const uint8_t appSKey[16]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setAppSessionKey(appSKey);
}

int MipFEU::setNetworkSessionKey(const uint8_t nwkSKey[16]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setNetworkSessionKey(nwkSKey);
}

int MipFEU::sendMsg(const uint8_t payload[], int payloadLen, bool confirmed, uint8_t port) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.sendMsg(payload, payloadLen, confirmed, port);
}

int MipFEU::readMsg(uint8_t payloadBuf[], int maxLen, uint8_t *port, bool *isConfirmed) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.readMsg(payloadBuf, maxLen, port, isConfirmed);
}

int MipFEU::getSessionStatus() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getSessionStatus();
}

int MipFEU::setNextDataRate(mipDataRateEU dr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setNextDataRate(dr);
}

int MipFEU::setBatteryLevel(uint8_t level) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setBatteryLevel(level);
}

int MipFEU::getBatteryLevel() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getBatteryLevel();
}

int MipFEU::setUplinkCounter(uint32_t count) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setUplinkCounter(count);
}

uint32_t MipFEU::getUplinkCounter() {
  if(_stack != MIP_LORAWAN){ 
    return 0;
  }
  return beu.getUplinkCounter();
}

int MipFEU::setDownlinkCounter(uint32_t count) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setDownlinkCounter(count);
}

uint32_t MipFEU::getDownlinkCounter() {
  if(_stack != MIP_LORAWAN) {
    return 0;
  }
  return beu.getDownlinkCounter();
}

int MipFEU::setChannelParameters(uint8_t chIdx, uint32_t freqHz, uint8_t drRange, uint8_t status) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setChannelParameters(chIdx, freqHz, drRange, status);
}

int MipFEU::getChannelParameters(uint8_t chIdx, uint32_t *freqHz, uint8_t *drRange, uint8_t *status) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.getChannelParameters(chIdx, freqHz, drRange, status);
}

int MipFEU::linkCheckRequest(uint8_t *margin, uint8_t *gwCnt) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.linkCheckRequest(margin, gwCnt);
}

int MipFEU::deviceTimeRequest(uint32_t *epochSec, uint8_t *epochFrac) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.deviceTimeRequest(epochSec, epochFrac);
}

int MipFEU::resetABP() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.resetABP();
}

int MipFEU::rekeyOTAA() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.rekeyOTAA();
}

int MipFEU::setCustomerDevEUI(uint8_t customerEui[8]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setCustomerDevEUI(customerEui);
}

int MipFEU::setAppEUI(uint8_t appEui[8]) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setAppEUI(appEui);
}

int MipFEU::setDevAddr(uint32_t devAddr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setDevAddr(devAddr);
}

int MipFEU::setClass(mipClass classType) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setClass(classType);
}

int MipFEU::setDataRate(mipDataRateEU dr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setDataRate(dr);
}

int MipFEU::setPower(mipTxPowerEU powerLevel) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setPower(powerLevel);
}

int MipFEU::enableAdaptiveDataRate() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enableAdaptiveDataRate();
}

int MipFEU::disableAdaptiveDataRate() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disableAdaptiveDataRate();
}

int MipFEU::enableDutyCycleControl() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enableDutyCycleControl();
}

int MipFEU::disableDutyCycleControl() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disableDutyCycleControl();
}

int MipFEU::setUnconfirmedTxRepetitions(uint8_t repetitions) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setUnconfirmedTxRepetitions(repetitions);
}

int MipFEU::enableCustomerDevEUI() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enableCustomerDevEUI();
}

int MipFEU::disableCustomerDevEUI() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disableCustomerDevEUI();
}

int MipFEU::setRX2DataRate(mipDataRateEU dr) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setRX2DataRate(dr);
}

int MipFEU::setRX2Frequency(uint32_t freqHz) {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.setRX2Frequency(freqHz);
}

int MipFEU::enablePublicNetwork() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.enablePublicNetwork();
}

int MipFEU::disablePublicNetwork() {
  if(_stack != MIP_LORAWAN){
    return -0xFF;
  }
  return beu.disablePublicNetwork();
}