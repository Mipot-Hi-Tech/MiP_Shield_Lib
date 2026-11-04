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


#ifndef MIP_DEU_H
#define MIP_DEU_H

#include "Arduino.h"

#include "MiP_base.h"



enum mipBandwidthIndex {
	MIP_TX_BANDWIDTH_125_KHZ = 0x00U,
	MIP_TX_BANDWIDTH_250_KHZ = 0x01U,
	MIP_TX_BANDWIDTH_500_KHZ = 0x02U,
};

enum mipSpreadingFactorIndex {
	MIP_SF_6_64_CHIPS   	= 0x06U,
	MIP_SF_7_128_CHIPS  	= 0x07U,
	MIP_SF_8_256_CHIPS  	= 0x08U,
	MIP_SF_9_512_CHIPS  	= 0x09U,
	MIP_SF_10_1024_CHIPS 	= 0x0AU,
	MIP_SF_11_2048_CHIPS 	= 0x0BU,
	MIP_SF_12_4096_CHIPS 	= 0x0CU
};


enum mipCodeRateIndex {
	MIP_CODE_RATE_4_5  = 0x01U,
	MIP_CODE_RATE_4_6  = 0x02U,
	MIP_CODE_RATE_4_7  = 0x03U,
	MIP_CODE_RATE_4_8  = 0x04U
};

enum mipFrequencyChannelIndex {
	MIP_CH0_868_075_MHZ  = 0x00U,
	MIP_CH1_868_100_MHZ  = 0x01U,
	MIP_CH2_868_125_MHZ  = 0x02U,
	MIP_CH3_868_150_MHZ  = 0x03U,
	MIP_CH4_868_175_MHZ  = 0x04U,
	MIP_CH5_868_200_MHZ  = 0x05U,
	MIP_CH6_868_225_MHZ  = 0x06U,
	MIP_CH7_868_250_MHZ  = 0x07U,
	MIP_CH8_868_275_MHZ  = 0x08U,
	MIP_CH9_868_300_MHZ  = 0x09U,
	MIP_CH10_868_325_MHZ = 0x0AU,
	MIP_CH11_868_350_MHZ = 0x0BU,
	MIP_CH12_868_375_MHZ = 0x0CU,
	MIP_CH13_868_400_MHZ = 0x0DU,
	MIP_CH14_868_425_MHZ = 0x0EU,
	MIP_CH15_868_450_MHZ = 0x0FU,
	MIP_CH16_868_475_MHZ = 0x10U,
	MIP_CH17_868_500_MHZ = 0x11U,
	MIP_CH18_868_525_MHZ = 0x12U,
	MIP_CH19_868_550_MHZ = 0x13U,
	MIP_CH20_868_575_MHZ = 0x14U,
	MIP_CH21_868_600_MHZ = 0x15U,
	MIP_CH22_868_625_MHZ = 0x16U,
	MIP_CH23_868_650_MHZ = 0x17U,
	MIP_CH24_868_675_MHZ = 0x18U,
	MIP_CH25_868_700_MHZ = 0x19U,
	MIP_CH26_868_725_MHZ = 0x1AU,
	MIP_CH27_868_750_MHZ = 0x1BU,
	MIP_CH28_868_775_MHZ = 0x1CU,
	MIP_CH29_868_800_MHZ = 0x1DU,
	MIP_CH30_868_825_MHZ = 0x1EU,
	MIP_CH31_868_850_MHZ = 0x1FU,
	MIP_CH32_868_875_MHZ = 0x20U,
	MIP_CH33_868_900_MHZ = 0x21U,
	MIP_CH34_868_925_MHZ = 0x22U,
	MIP_CH35_868_950_MHZ = 0x23U,
	MIP_CH36_868_975_MHZ = 0x24U,
	MIP_CH37_869_000_MHZ = 0x25U,
	MIP_CH38_869_025_MHZ = 0x26U,
	MIP_CH39_869_050_MHZ = 0x27U,
	MIP_CH40_869_075_MHZ = 0x28U,
	MIP_CH41_869_100_MHZ = 0x29U,
	MIP_CH42_869_125_MHZ = 0x2AU,
	MIP_CH43_869_150_MHZ = 0x2BU,
	MIP_CH44_869_175_MHZ = 0x2CU,
	MIP_CH45_869_200_MHZ = 0x2DU,
	MIP_CH46_869_225_MHZ = 0x2EU,
	MIP_CH47_869_250_MHZ = 0x2FU,
	MIP_CH48_869_275_MHZ = 0x30U,
	MIP_CH49_869_300_MHZ = 0x31U,
	MIP_CH50_869_325_MHZ = 0x32U,
	MIP_CH51_869_350_MHZ = 0x33U,
	MIP_CH52_869_375_MHZ = 0x34U,
	MIP_CH53_869_400_MHZ = 0x35U,
	MIP_CH54_869_425_MHZ = 0x36U,
	MIP_CH55_869_450_MHZ = 0x37U,
	MIP_CH56_869_475_MHZ = 0x38U,
	MIP_CH57_869_500_MHZ = 0x39U,
	MIP_CH58_869_525_MHZ = 0x3AU,
	MIP_CH59_869_550_MHZ = 0x3BU,
	MIP_CH60_869_575_MHZ = 0x3CU,
	MIP_CH61_869_600_MHZ = 0x3DU,
	MIP_CH62_869_625_MHZ = 0x3EU,
	MIP_CH63_869_650_MHZ = 0x3FU,
	MIP_CH64_869_675_MHZ = 0x40U,
	MIP_CH65_869_700_MHZ = 0x41U,
	MIP_CH66_869_725_MHZ = 0x42U,
	MIP_CH67_869_750_MHZ = 0x43U,
	MIP_CH68_869_775_MHZ = 0x44U,
	MIP_CH69_869_800_MHZ = 0x45U,
	MIP_CH70_869_825_MHZ = 0x46U,
	MIP_CH71_869_850_MHZ = 0x47U,
	MIP_CH72_869_875_MHZ = 0x48U,
	MIP_CH73_869_900_MHZ = 0x49U,
	MIP_CH74_869_925_MHZ = 0x4AU
};

class MipDEU : public MipBase
{
  public:
    MipDEU(int nWakePin, int nDataIndicatePin);
    
    void startRx();
    void stopRx();

    int sendMsg(uint8_t buf[], int bufLen);
    int readMsg(uint8_t payload[], int payloadMaxLen);
    int16_t getRSSI();
    int8_t getSNR();
    int setAesKey(uint8_t key[]);
    int setIV(uint8_t buf[]);

    int setPower(int powerLevel);
    int setFrequency(mipFrequencyChannelIndex frequency);
    int setBandwidth(mipBandwidthIndex bandwidth);
    int setSpreadingFactor(mipSpreadingFactorIndex spreadingFactor);
    int setCodeRate(mipCodeRateIndex codeRate);

    int setDataIndicateTimeout(int timeoutMs);
    int enableAES();
    int disableAES();
    

  private:
    int16_t _lastRSSI;
    int8_t _lastSNR;
};

#endif