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
#include <vector>
#include <stdint.h>
#include <string>
#include "nlohmann/json.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class PriceSourceESIOS;
class ESIOSFetch final {
  friend class PriceSourceESIOS;
  private :
    typedef struct {
      uint32_t   IndicatorNum;
      uint32_t   GeoID;
      string     TimeTrunc;
      string     VarName;
      string     Description;
    } Variable_t;
  private :
    static string              pToken;
    static uint64_t            pSyncDate;
    static pthread_t           pThread;
    static pthread_mutex_t     pMutex;
    volatile static bool       pTerminate;
  public :
    static vector<Variable_t>  pVariables;
  private :
    static void     GetESIOSCallback(const json &params, Response &response);
    static void     SetESIOSCallback(const json &params, Response &response);
    static void     Configure(const json &config);
    static json     CheckConfig(const json &config);
    static void    *Thread_Func(void *params);
    static bool     FetchIndicator(SSLClient &client, const string &token, const Variable_t &var, uint64_t to);
  public :
    ESIOSFetch();
    ~ESIOSFetch();
};
// ---------------------------------------------------------------------------------------------------------------------