// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Device.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class DeviceVirtual final : public Device {
  private :
    static bool                     pRegistered;
  private :
    pthread_t                       pThread;
    AnalyzerSoft                   *pAnalyzer;
    pthread_rwlock_t                pConfigLock;
    bool                            pTerminate;
    Analyzer::AnalyzerMode          pMode;
    float                           pSampleRate;
    float                           pVoltage;
    float                           pCurrent;
    float                           pCurrentPhase;
    float                           pFrequency;
  private :
    static bool            Register();
    static vector<Device*> GetInstances();
    static void           *Capture_Thread(void *args);
  private :
    DeviceVirtual();
    void Configure(const json &config);
    void GetDevice(const json &params, Response &response) override;
    void SetDevice(const json &params, Response &response) override;
  public :
    ~DeviceVirtual() override;
};
//----------------------------------------------------------------------------------------------------------------------
