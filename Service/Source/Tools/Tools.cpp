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
#include <sys/time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <unistd.h>
#include "Tools.h"
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
termios Tools::pConsoleAttr;
//----------------------------------------------------------------------------------------------------------------------
void Tools::ResetConsole() {
  tcsetattr (STDIN_FILENO, TCSANOW, &pConsoleAttr);
}
void Tools::DisableEcho() {
  struct termios tattr;
  if(!isatty(STDIN_FILENO)) return;
  if(tcgetattr(STDIN_FILENO, &pConsoleAttr) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(atexit(ResetConsole) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(tcgetattr (STDIN_FILENO, &tattr) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  tattr.c_lflag &= ~(ICANON | ECHO);
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
uint64_t Tools::GetTime64() {
  struct timespec Time;
  if(clock_gettime(CLOCK_REALTIME, &Time) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return 0;
  }
  return (uint64_t)Time.tv_sec * 1000 + Time.tv_nsec / 1000000;
}
string Tools::GetUUID() {
  uuid_t UUID;
  char   UUIDText[37];
  uuid_generate(UUID);
  uuid_unparse_upper(UUID, UUIDText);
  return UUIDText;
}
bool Tools::ValidUUID(const string &uuid) {
  uuid_t UUID;
  return (uuid_parse(uuid.c_str(), UUID) == 0);
}
void Tools::RunDaemon() {
  LogFile::Info("Move to background task");
  pid_t ProcID = fork();
  if(ProcID < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    exit(1);
  }
  if(ProcID > 0) exit(0);
  umask(0);
  pid_t SID = setsid();
  if(SID < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    exit(1);
  }
  LogFile::CloseConsole();
  if(close(STDIN_FILENO) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(close(STDOUT_FILENO) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(close(STDERR_FILENO) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}