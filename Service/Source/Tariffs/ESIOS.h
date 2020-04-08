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
//#include "Tariff.h"
#include "nlohmann/json.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
class ESIOS final {
  private :
    static string              pToken;
    static pthread_t           pThread;
    static pthread_mutex_t     pMutex;
    static bool                pTerminate;
  private :
    static void  GetESIOSCallback(const json &params, Response &response);
    static void  SetESIOSCallback(const json &params, Response &response);
    static void  Configure(const json &config);
    static json  CheckConfig(const json &config);
    static void *Thread_Func(void *params);

//  private :
//    static bool    pClassRegistered;
//  private :
//    const char *pRate;
//    CURL       *pCurl;
//    CurlBuff_t  pCurlBuff;
//  private :
//    static size_t Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp);
//    static PricesInterface *CreateInstance(const char *name, json_object *config);
//    static void XMLError(void *ctx, const char *msg,...);
//  private :
//    void CheckPrices(uint64_t from);
  public :
    ESIOS();
    ~ESIOS();
};
