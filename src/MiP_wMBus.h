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
* Library for using Mipot's MiP series 32001505AEU or 32001605BE8SW wMBus module
*/


#ifndef MIP_CEU_H
#define MIP_CEU_H

#include "Arduino.h"

#include "MiP_base.h"

enum mipWMBusMode {
    MIP_WMBUS_S2_SHORT_PREAMBLE_MODE = 0x00,
    MIP_WMBUS_S2_LONG_PREAMBLE_MODE  = 0x01,
    MIP_WMBUS_S1_MODE                = 0x02,
    MIP_WMBUS_S1M_MODE               = 0x03,
    MIP_WMBUS_T1_METER_MODE          = 0x04,
    MIP_WMBUS_T2_METER_MODE          = 0x05,
    MIP_WMBUS_T2_OTHER_MODE          = 0x06,
    MIP_WMBUS_R2_METER_MODE          = 0x07,
    MIP_WMBUS_R2_OTHER_MODE          = 0x08,
    MIP_WMBUS_C1_METER_FRAME_A_MODE  = 0x09,
    MIP_WMBUS_C1_METER_FRAME_B_MODE  = 0x0A,
    MIP_WMBUS_C2_METER_FRAME_A_MODE  = 0x0B,
    MIP_WMBUS_C2_METER_FRAME_B_MODE  = 0x0C,
    MIP_WMBUS_C2_OTHER_FRAME_A_MODE  = 0x0D,
    MIP_WMBUS_C2_OTHER_FRAME_B_MODE  = 0x0E,
    MIP_WMBUS_T2_C2_OTHER_MODE       = 0x0F
};

enum mipWMBusRfChannel {
    MIP_WMBUS_CH0_868_03_MHZ = 0x00,
    MIP_WMBUS_CH1_868_09_MHZ = 0x01,
    MIP_WMBUS_CH2_868_15_MHZ = 0x02,
    MIP_WMBUS_CH3_868_21_MHZ = 0x03,
    MIP_WMBUS_CH4_868_27_MHZ = 0x04,
    MIP_WMBUS_CH5_868_33_MHZ = 0x05,
    MIP_WMBUS_CH6_868_39_MHZ = 0x06,
    MIP_WMBUS_CH7_868_45_MHZ = 0x07,
    MIP_WMBUS_CH8_868_51_MHZ = 0x08,
    MIP_WMBUS_CH9_868_57_MHZ = 0x09
};

enum mipWMBusPower {
    MIP_WMBUS_0_DBM  = 0x00,
    MIP_WMBUS_5_DBM  = 0x01,
    MIP_WMBUS_7_DBM  = 0x02,
    MIP_WMBUS_10_DBM = 0x03,
    MIP_WMBUS_12_DBM = 0x04
};

enum mipWMBusEncryptionMode {
    MIP_WMBUS_ENCRYPTION_DISABLED = 0x00,
    MIP_WMBUS_ENCRYPTION_MODE_5   = 0x05
};

class MipWMBus : public MipBase
{
    public:
    MipWMBus(int nWakePin, int nDataIndicatePin);

    void begin();
    int getTemperature();
    int getRSSI();
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
    int getRssiEnabled(bool* rssiEnabled);
    int setDataIndicateTimeout(uint8_t timeoutMs);

    private:
    bool _rssiEnabled;
};



#endif
