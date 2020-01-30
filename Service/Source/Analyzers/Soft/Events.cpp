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
#include "Events.h"
#include "Database.h"
#include "AnalyzerSoft.h"
AnalyzerSoft::Events::Events(AnalyzerSoft *parent) {
//  pParent         = parent;
//  pTerminate      = false;
//  pEventThread    = 0;
//  pPrevStationary = false;
//  if(pthread_create(&pEventThread, NULL, Events_Thread, (void*)this) != 0) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    pEventThread = 0;
//    return;
//  }
//  pthread_setname_np(pEventThread, "EventsDet");
}
void AnalyzerSoft::Events::Start() {
  pWorkerLocks.WakeWorker();
}
void AnalyzerSoft::Events::Wait() {
  pWorkerLocks.WaitWorker();
}
AnalyzerSoft::Events::~Events() {
  pTerminate = true;
  pWorkerLocks.Abort(&pEventThread);
}



