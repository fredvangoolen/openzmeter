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
#include <list>
#include <complex>
#include <sstream>
#include <inttypes.h>
#include <libpq-fe.h>
#include "nlohmann/json.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Database final {
  private :
    PGconn      *pDBConn;
    PGresult    *pResult;
  private :
    void     ClearAsync();
  public :
    string   Bind(const uint8_t param) const;
    string   Bind(const uint32_t param) const;
    string   Bind(const int64_t param) const;
    string   Bind(const uint64_t param) const;
    string   Bind(const float param) const;
    string   Bind(const float *params, const uint32_t len) const;
    string   Bind(const float *params, const uint32_t lenA, const uint32_t lenB) const;
    string   Bind(const uint16_t *params, const uint32_t len) const;
    string   Bind(const bool param) const;
    string   Bind(const string &string) const;
    string   BindID(const string &string) const;
  public :
    bool     ExecSentenceNoResult(const string &sql);
    bool     ExecSentenceAsyncResult(const string &sql);
    bool     ExecResource(const string &file);
    bool     FecthRow();
    string   ResultString(const string &name) const;
    float    ResultFloat(const string &name) const;
    uint64_t ResultUInt64(const string &name) const;
    json     ResultJSON(const string &name) const;
    Database(const string &schema = "public");
    ~Database();
};
class DatabasePool final {
  private :
    typedef struct {
      PGconn   *Connection;
      uint64_t  LastTime;
    } Connection_t;
  private :
    static string              pConnectionString;
    static pthread_mutex_t     pMutex;
    static list<Connection_t>  pConnectionPool;
  public :
    static PGconn *PopConnection();
    static void    PushConnection(PGconn *conn);
    static void    AdjustWorkers();
  public :
    DatabasePool();
    ~DatabasePool();
};