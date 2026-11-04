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
*
* Library for using Mipot's Mip Series 32001505FEU LoRa Module
*/


#ifndef MIP_GEU_H
#define MIP_GEU_H

#include "Arduino.h"

#include "MiP_base.h"
#include "MiP_BEU.h"
#include "MiP_wMBus.h"


enum mipSelectedStack {
    MIP_LORAWAN = 1,
    MIP_WMBUS   = 3
};

class MipGEU : public MipBase
{
    public:
    MipGEU(int nWakePin, int nDataIndicatePin);

    void begin();

    //GEU methods
    int changeStack(mipSelectedStack newStack);
    int getCurrentStack(mipSelectedStack* stack);
    int changeStackAndSave(mipSelectedStack newStack);
    int getSavedStack(mipSelectedStack* stack);

    //Common methods
    int setDataIndicateTimeout(int timeoutMs);
    int16_t getRSSI();
    int8_t getSNR();

    //wMBus methods
    int getTemperature();
    int setLseCalibrationPeriod(uint16_t periodS, bool save);
    int getLseCalibrationPeriod();
    int setMode(mipWMBusMode mode, bool save);
    int setCField(uint8_t cField, bool save);
    int setEncryptionMode(mipWMBusEncryptionMode mode, bool save);
    int setEncryptionKey(uint8_t keyVersion, uint8_t key[], uint8_t keyID);
    int sendMsg(uint8_t payload[], uint8_t payloadLen);
    int sendEncryptedMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[]);
    int readMsg(uint8_t block1[], uint8_t* CIField, uint8_t payload[], uint8_t payloadMaxLen, int8_t* RSSI);
    int sendSyncMsg(uint8_t payload[], uint8_t payloadLen, uint32_t interval);
    int sendEncryptedSyncMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[], uint32_t interval);
    int changeSyncMsg(uint8_t payload[], uint8_t payloadLen);
    int changeEncryptedSyncMsg(uint8_t payload[], uint8_t payloadLen, uint8_t encryptionOffset, uint8_t keyID, uint8_t encryptionType, uint8_t IV[]);

    int getMode(mipWMBusMode* mode);
    int setChannel(mipWMBusRfChannel channel);
    int setPower(mipWMBusPower power);
    int disableSleep();
    int enableSleep();
    int enableDeepStop();
    int enableShutdown();
    int setRxWindow(uint8_t windowMs);

    int getCField(uint8_t* cField);
    int setManufacturerID(uint16_t manID);
    int getManufacturerID(uint16_t* manID);
    int setDeviceID(uint32_t devID);
    int getDeviceID(uint32_t* devID);
    int setVersion(uint8_t version);
    int getVersion(uint8_t* version);
    int setDeviceType(uint8_t deviceType);
    int getDeviceType(uint8_t* deviceType);

    int enableBlock1FromModule();
    int disableBlock1FromModule();
    int enableRSSI();
    int disableRSSI();

    //BEU methods
    int getDevEUI(uint8_t buf[8]);
    int resumeOtaaSession();
    int join(mipActivationMode mode);
    int getActivationStatus();
    int setAppKey(const uint8_t appKey[16]);
    int setAppSessionKey(const uint8_t appSKey[16]);
    int setNetworkSessionKey(const uint8_t nwkSKey[16]);

    int sendMsg(const uint8_t payload[], int payloadLen, bool confirmed, uint8_t port);
    int readMsg(uint8_t payloadBuf[], int maxLen, uint8_t *port, bool *isConfirmed);

    int getSessionStatus();
    int setNextDataRate(mipDataRateEU dr);
    int setBatteryLevel(uint8_t level);
    int getBatteryLevel();
    int setUplinkCounter(uint32_t count);
    uint32_t getUplinkCounter();
    int setDownlinkCounter(uint32_t count);
    uint32_t getDownlinkCounter();
    int setChannelParameters(uint8_t chIdx, uint32_t freqHz, uint8_t drRange, uint8_t status);
    int getChannelParameters(uint8_t chIdx, uint32_t *freqHz, uint8_t *drRange, uint8_t *status);
    int linkCheckRequest(uint8_t *margin, uint8_t *gwCnt);
    int deviceTimeRequest(uint32_t *epochSec, uint8_t *epochFrac);

    int resetABP();
    int rekeyOTAA();

    int setCustomerDevEUI(uint8_t customerEui[8]);
    int setAppEUI(uint8_t appEui[8]);
    int setDevAddr(uint32_t devAddr);
    int setClass(mipClass classType);
    int setDataRate(mipDataRateEU dr);
    int setPower(mipTxPowerEU powerLevel);
    int enableAdaptiveDataRate();
    int disableAdaptiveDataRate();
    int enableDutyCycleControl();
    int disableDutyCycleControl();
    int setUnconfirmedTxRepetitions(uint8_t repetitions);
    int enableCustomerDevEUI();
    int disableCustomerDevEUI();
    int setRX2DataRate(mipDataRateEU dr);
    int setRX2Frequency(uint32_t freqHz);
    int enablePublicNetwork();
    int disablePublicNetwork();


    private:

    mipSelectedStack _stack;
    MipWMBus wMBus;
    MipBEU beu;

};



#endif
