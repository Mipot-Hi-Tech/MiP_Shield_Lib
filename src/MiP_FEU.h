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


#ifndef MIP_FEU_H
#define MIP_FEU_H

#include "Arduino.h"

#include "MiP_base.h"
#include "MiP_BEU.h"
#include "MiP_Deu.h"


enum mipSelectedStack {
    MIP_LORA_MODEM  = 0,
    MIP_LORAWAN     = 1
};

class MipFEU : public MipBase
{
    public:
    MipFEU(int nWakePin, int nDataIndicatePin);

    void begin();

    //FEU methods
    int changeStack(mipSelectedStack newStack);
    int getCurrentStack(mipSelectedStack* stack);
    int changeStackAndSave(mipSelectedStack newStack);
    int getSavedStack(mipSelectedStack* stack);

    //Common methods
    int setDataIndicateTimeout(int timeoutMs);
    int16_t getRSSI();
    int8_t getSNR();

    //DEU methods
    int sendMsg(uint8_t buf[], int bufLen);
    int readMsg(uint8_t buf[], int bufLen);
    
    int setAesKey(uint8_t key[]);
    int setIV(uint8_t buf[]);

    int setPower(int powerLevel);
    int setFrequency(mipFrequencyChannelIndex frequency);
    int setBandwidth(mipBandwidthIndex bandwidth);
    int setSpreadingFactor(mipSpreadingFactorIndex spreadingFactor);
    int setCodeRate(mipCodeRateIndex codeRate);

    int enableAES();
    int disableAES();

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
    MipDEU deu;
    MipBEU beu;

};



#endif
