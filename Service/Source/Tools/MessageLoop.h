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
#include <string>
#include <map>
#include <set>
#include <functional>
#include <pthread.h>
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class MessageLoop final {
  private :
    typedef struct {
      uint64_t         LastCall;
      function<void()> Function;
      uint32_t         Interval;
    } Listener_t;
  private :
    static pthread_mutex_t         pListenersMutex;
    static map<string, Listener_t> pListeners;
    static pthread_mutex_t         pMessagesMutex;
    static set<string>             pMessages;
  public :
    static void SendMessage(const string &id);
    static void RegisterListener(const string &id, function<void()> func, uint32_t ms = 0);
    static void UnregisterListener(const string &id);
    static void HandleEvents();
  public :
    MessageLoop();
    ~MessageLoop();
};
// ---------------------------------------------------------------------------------------------------------------------