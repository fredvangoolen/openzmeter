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
#include <unistd.h>
#include "MessageLoop.h"
#include "Tools.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
pthread_mutex_t                      MessageLoop::pListenersMutex;
map<string, MessageLoop::Listener_t> MessageLoop::pListeners;
pthread_mutex_t                      MessageLoop::pMessagesMutex;
set<string>                          MessageLoop::pMessages;
// ---------------------------------------------------------------------------------------------------------------------
MessageLoop::MessageLoop() {
}
MessageLoop::~MessageLoop() {
  pthread_mutex_lock(&pListenersMutex);
  pListeners.clear();
  pthread_mutex_unlock(&pListenersMutex);
  pthread_mutex_lock(&pMessagesMutex);
  pMessages.clear();
  pthread_mutex_unlock(&pMessagesMutex);
}
void MessageLoop::SendMessage(const string &id) {
  pthread_mutex_lock(&pMessagesMutex);
  pMessages.insert(id);
  pthread_mutex_unlock(&pMessagesMutex);
}
void MessageLoop::HandleEvents() {
  uint64_t Time = Tools::GetTime64();
  pthread_mutex_lock(&pListenersMutex);
  for(auto &Item : pListeners) {
    bool Call = false;
    pthread_mutex_lock(&pMessagesMutex);
    auto Message = pMessages.find(Item.first);
    if(Message != pMessages.end()) {
      pMessages.erase(Message);
      Call = true;
    }
    pthread_mutex_unlock(&pMessagesMutex);
    if((Call == false) && (Item.second.Interval != 0) && ((Item.second.LastCall + Item.second.Interval) < Time)) Call = true;
    if(Call == false) continue;
    LogFile::Debug("Handling event '%s'", Item.first.c_str());
    Item.second.Function();
    Item.second.LastCall = Time;
  }
  pthread_mutex_unlock(&pListenersMutex);
  usleep(100000);
}
void MessageLoop::RegisterListener(const string &id, function<void()> func, uint32_t ms) {
  Listener_t Listener;
  Listener.Function = func;
  Listener.LastCall = 0;
  Listener.Interval = ms;
  pthread_mutex_lock(&pListenersMutex);
  pListeners.insert({ id, Listener });
  pthread_mutex_unlock(&pListenersMutex);
}
void MessageLoop::UnregisterListener(const string &id) {
  pthread_mutex_lock(&pListenersMutex);
  auto Item = pListeners.find(id);
  if(Item != pListeners.end()) pListeners.erase(Item);
  pthread_mutex_unlock(&pListenersMutex);
}
// ---------------------------------------------------------------------------------------------------------------------