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
#include <pthread.h>
#include <stdarg.h>
// ---------------------------------------------------------------------------------------------------------------------
class LogFile final {
  private :
    enum Type { ERROR = 0, WARNING = 1, INFO = 2, DEBUG = 3 };
  private :
    static pthread_mutex_t  pMutex;
    static bool             pConsole;
    static int              pFile;
    static Type             pLevel;
    static bool             pDebug;
  private :
    static void Msg(Type type, const char *msg, va_list params);
  public :
    LogFile();
    virtual ~LogFile();
  public :
    static void EnableDebug();
    static void CloseConsole();
    static void Info(const char *msg, ...);
    static void Warning(const char *msg, ...);
    static void Error(const char *msg, ...);
    static void Debug(const char *msg, ...);
};