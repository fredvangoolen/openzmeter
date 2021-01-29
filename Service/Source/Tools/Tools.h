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
#include <termios.h>
#include <string>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static const string Base64;
  private :
    static termios pConsoleAttr;
  private :
    static void ResetConsole();
    static bool isBase64(const uint8_t c);
  public :
    static void      DisableEcho();
    static void      RunDaemon();
    static uint64_t  GetTime64();
    static string    GetUUID();
    static string    DecodeBase64(const string &input);
    static string    EncodeBase64(const string &input);
    static bool      ValidUUID(const string &uuid);
};
//----------------------------------------------------------------------------------------------------------------------