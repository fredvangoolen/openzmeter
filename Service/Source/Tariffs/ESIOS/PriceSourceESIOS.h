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
#include <map>
#include "Tariff.h"
#include "HTTPServer.h"
#include "PriceSource.h"
// ---------------------------------------------------------------------------------------------------------------------
class PriceSourceESIOS final : public PriceSource {
  private :
    static bool         pRegistered;
    static string       pQueryColumns;
  private :
    Database            pDB;
    uint64_t            pLoadedTime;
    map<string, float>  pLoaded;
  private :
    PriceSourceESIOS();
    static bool         Register();
    static PriceSource *CreateInstance();
    static json         GetVarsInfo();
  public :
    void PrepareRange(const uint64_t from, const uint64_t to);
    void GetParams(const uint64_t sampletime, map<string, float> &vars);
};
// ---------------------------------------------------------------------------------------------------------------------