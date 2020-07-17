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
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftEvents {

      Event_t                    pEvent;                   //Running event
    list<Event_t>              pEvents;                  //Close pending events

  private : //Events related (only acceded inside analyzer thread)
    bool                       pPrevStationary;          //Steady state flag
    uint16_t                   pPrevSamplesLen;          //Previous samples count
    uint16_t                   pPrevSamplesPos;          //Previous samples write pos
    float                     *pPrevSamples;             //Previous samples
    uint16_t                   pPrevCyclesLen;           //Previous cycles count
    uint16_t                   pPrevCyclesPos;           //Previous cycles write pos
    float                     *pPrevCycles;              //Previous cycles

  private :
    void GetEvents(const json &params, Response &response) override;
    void GetEventsRange(const json &params, Response &response) override;
    void DelEventsRange(const json &params, Response &response) override;
    void GetEvent(const json &params, Response &response) override;
    void SetEvent(const json &params, Response &response) override;
    void DelEvent(const json &params, Response &response) override;

    void Events_Func();
    bool FindRunningEvent(const uint32_t mask);

};
// ---------------------------------------------------------------------------------------------------------------------