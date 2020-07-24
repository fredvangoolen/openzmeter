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
#include <list>
#include <inttypes.h>
#include "AnalyzerSoftEvent.h"
#include "AnalyzerSoftBlock.h"
#include "HTTPChunkedWriter.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftEvents final {
  private :
    string                     pSerial;

    AnalyzerSoftEvent         *pEvent;                   //Running event
    list<AnalyzerSoftEvent*>   pEvents;                  //Close pending events
    bool                       pPrevStationary;          //Steady state flag
    uint16_t                   pPrevSamplesLen;          //Previous samples count
    uint16_t                   pPrevSamplesPos;          //Previous samples write pos
    float                     *pPrevSamples;             //Previous samples
    uint16_t                   pPrevCyclesLen;           //Previous cycles count
    uint16_t                   pPrevCyclesPos;           //Previous cycles write pos
    float                     *pPrevCycles;              //Previous cycles

  private :
    void Events_Func();
    bool FindRunningEvent(const uint32_t mask);
  public :
    AnalyzerSoftEvents(const string &serial);
    ~AnalyzerSoftEvents();
        void Config(const bool enalbed);

    void Run(const AnalyzerSoftBlock *block);
    void Wait();
    void GetEvents(const json &params, Response &response);
    void GetEventsRange(const json &params, Response &response);
    void DelEventsRange(const json &params, Response &response);
    void GetEvent(const json &params, Response &response);
    void SetEvent(const json &params, Response &response);
    void DelEvent(const json &params, Response &response);
};
// ---------------------------------------------------------------------------------------------------------------------