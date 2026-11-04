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
* Library for using Mipot's Mip Series 32001505DEU LoRa Module
*/



#ifndef MIP_BEU_H
#define MIP_BEU_H

#include "Arduino.h"

#include "MiP_base.h"


// Tipi di attivazione rete
enum mipActivationMode {
  MIP_ACTIVATION_ABP  = 0x00U,
  MIP_ACTIVATION_OTAA = 0x01U
};

// Data Rate / Spreading Factor per la regione EU
enum mipDataRateEU {
  MIP_DR_SF12_125KHZ = 0x00U,
  MIP_DR_SF11_125KHZ = 0x01U,
  MIP_DR_SF10_125KHZ = 0x02U,
  MIP_DR_SF9_125KHZ  = 0x03U,
  MIP_DR_SF8_125KHZ  = 0x04U,
  MIP_DR_SF7_125KHZ  = 0x05U,
  MIP_DR_SF7_250KHZ  = 0x06U,
  MIP_DR_FSK         = 0x07U
};

// Livelli di potenza di trasmissione per la regione EU
enum mipTxPowerEU {
  MIP_POWER_14_DBM = 0x00U,
  MIP_POWER_12_DBM = 0x01U,
  MIP_POWER_10_DBM = 0x02U,
  MIP_POWER_8_DBM  = 0x03U,
  MIP_POWER_6_DBM  = 0x04U,
  MIP_POWER_4_DBM  = 0x05U,
  MIP_POWER_2_DBM  = 0x06U,
  MIP_POWER_0_DBM  = 0x07U
};

// Classi LoRaWAN
enum mipClass {
  MIP_CLASS_A = 0x00U,
  MIP_CLASS_B = 0x01U,
  MIP_CLASS_C = 0x02U
};


class MipBEU : public MipBase
{
  public:
    MipBEU(int nWakePin, int nDataIndicatePin);
    
    void startRx();
    void stopRx();

    int getDevEUI(uint8_t buf[8]);
    int resumeOtaaSession();
    int join(mipActivationMode mode);
    int getActivationStatus();
    int setAppKey(const uint8_t appKey[16]);
    int setAppSessionKey(const uint8_t appSKey[16]);
    int setNetworkSessionKey(const uint8_t nwkSKey[16]);

    int sendMsg(const uint8_t payload[], int payloadLen, bool confirmed, uint8_t port);
    int readMsg(uint8_t payloadBuf[], int maxLen, uint8_t *port, bool *isConfirmed);
    int16_t getRSSI();
    int8_t getSNR();

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

    int setDataIndicateTimeout(uint8_t timeoutMs);
    

  private:
    int16_t _lastRSSI;
    int8_t _lastSNR;
};

#endif