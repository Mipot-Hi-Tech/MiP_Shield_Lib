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


#ifndef MIP_CEU_H
#define MIP_CEU_H

#include "Arduino.h"

#include "MiP_base.h"


enum mipCeuMode {
  MIP_CEU_MASTER    = 0,
  MIP_CEU_END_NODE  = 1
};

enum mipCeuFrequencyIndex {
  MIP_CH0_868_1_MHZ = 0,
  MIP_CH1_868_3_MHZ = 1,
  MIP_CH2_868_5_MHZ = 2
};

class MipCEU : public MipBase
{
  public:
    MipCEU(int nWakePin, int nDataIndicatePin);
    
    void startRx();
    void stopRx();

    int enablePairing();
    int disablePairing();
    int getNetworkTableSize();
    int getNetworkTableRow(uint8_t row, uint32_t* endNodeID, uint8_t* pairingPayload);
    int deleteEndNode(uint32_t nodeID);
    int deleteAllEndNodes();
    int pairingRequest(uint32_t* masterID, uint8_t* idx);
    int getActivationStatus(uint32_t* masterID);
    int sendMsg(const uint8_t payload[], int payloadLen, bool confirmed, uint32_t destID);
    int readMsg(uint8_t payloadBuf[], int payloadMaxLen, uint32_t* sourceID);
    int linkCheckRequest(uint8_t power, uint8_t msgNumber, uint8_t threshold);
    int setAppKey(uint8_t key[]);

    int setDeviceType(mipCeuMode mode);
    int setUnconfirmedTxNumber(uint8_t num);
    int setConfirmedTxNumber(uint8_t num);
    int setEndNodePairingRequestPayload(uint8_t payload);
    int getMasterAddress(uint32_t* masterAddress);
    int getEndNodeMasterTableIndex(uint8_t* index);
    int setPower(uint8_t power);
    int setFrequency(mipCeuFrequencyIndex frequency);
    int setRssiThreshold(uint8_t rssiThreshold);
    int setDataIndicateTimeout(uint8_t timeoutMs);
    int enableAES();
    int disableAES();
    

  private:
    int16_t _lastRSSI;
    int8_t _lastSNR;

};

#endif