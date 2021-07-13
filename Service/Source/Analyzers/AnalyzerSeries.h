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
#include "Analyzer.h"
#include "HTTPRequest.h"
#include "nlohmann/json.hpp"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSeries {
  private :
    const string  pSerial;
    const uint8_t pVoltageChannels;
    const uint8_t pCurrentChannels;
  public :
    void GetSeries(HTTPRequest &req) const;
    void SetSeries(HTTPRequest &req) const;
    void GetSeriesRange(HTTPRequest &req) const;
    void DelSeriesRange(HTTPRequest &req) const;
  private :
    void CleanOldSamples() const;
  public :
    AnalyzerSeries(const string &id, const Analyzer::AnalyzerMode mode);
    virtual ~AnalyzerSeries();
};
//----------------------------------------------------------------------------------------------------------------------