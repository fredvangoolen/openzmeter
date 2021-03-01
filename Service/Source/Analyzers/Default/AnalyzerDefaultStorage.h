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
#include <pthread.h>
#include <inttypes.h>
#include "cpp-httplib/httplib.h"
#include "Analyzer.h"
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace httplib;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerDefaultStorage {
  private :
    string  pSerial;
  public :
    void GetSeries(const json &params, Response &response) const;
    void SetSeries(const json &params, Response &response) const;
    void GetSeriesRange(const json &params, Response &response) const;
    void GetEvents(const json &params, Response &response) const;
    void SetEvents(const json &params, Response &response) const;
    void GetEventsRange(const json &params, Response &response) const;
    void GetEvent(const json &params, Response &response) const;
    void GetAlarms(const json &params, Response &response) const;
    void SetAlarms(const json &params, Response &response) const;
    void GetAlarmsRange(const json &params, Response &response) const;
  public :
    AnalyzerDefaultStorage(const string &id);
    ~AnalyzerDefaultStorage();
    void CleanOldSamples() const;
};
//----------------------------------------------------------------------------------------------------------------------