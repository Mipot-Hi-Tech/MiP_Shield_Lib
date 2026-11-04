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
* Base class for all MiP modules
*/


#ifndef MIP_BASE_H
#define MIP_BASE_H

#include "Arduino.h"

struct mipVersion_s{
  uint8_t major;
  uint8_t minor;
  uint8_t patch;
  uint8_t build;
};



class MipBase
{
  public:
    MipBase(int nWakePin, int nDataIndicatePin, uint8_t i2cAddr);
    void begin();

    void wakeUp();
    void sleep();
    int softwareReset();
    int factoryReset();
    
    struct mipVersion_s getFwVersion();
    uint32_t getSerialNumber();

    int setEepromValue(uint8_t address, uint8_t* value, uint8_t valueLen);
    int getEepromValue(uint8_t address, uint8_t* value, uint8_t valueLen);
    int isReplySuccess(uint8_t cmd, uint8_t* rpyBuf, int rpyBufLen, bool skipStatus);
    int isChecksumValid(uint8_t* buf, int bufLen);
    uint8_t calculateChecksum(uint8_t* buf, int bufLen);
    int writeCommandAndReadReply(uint8_t* cmdBuf, int cmdBufLen, uint8_t* rpyBuf, int rpyBufLen);
    int readVariableLength(uint8_t* buf, int bufSize, uint8_t dataIndicateDelay);
    int read(uint8_t *buf, int len, int timeout, uint8_t dataIndicateDelay);
    int sendCmd(uint8_t *buf, int len);

    bool isDataAvailable();

  protected:
    int _nWakePin;
    int _nDataIndicatePin;
    uint8_t _i2cAddr;
};

#endif