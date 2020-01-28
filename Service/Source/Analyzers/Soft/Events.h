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
#include <deque>
#include <list>
#include "HTTPServer.h"
#include "Tools.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft::Events final {
  private :

    bool                       pTerminate;               //Signal to terminate
  private :
    static void *Events_Thread(void *args);
  private :

  public :
    Events(AnalyzerSoft *parent);
    ~Events();
    void Start();
    void Wait();
};