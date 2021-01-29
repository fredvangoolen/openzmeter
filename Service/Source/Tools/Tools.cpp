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
termios       Tools::pConsoleAttr;
const string  Tools::Base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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
bool Tools::isBase64(const uint8_t c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}
string Tools::DecodeBase64(const string &encoded_string) {
  uint32_t i = 0, j = 0, in_ = 0, in_len = encoded_string.size();
  uint8_t char_array_4[4], char_array_3[3];
  string ret;
  while(in_len-- && (encoded_string[in_] != '=') && isBase64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if(i ==4) {
      for(i = 0; i <4; i++) char_array_4[i] = Base64.find(char_array_4[i]);
      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
      for (i = 0; (i < 3); i++) ret += char_array_3[i];
      i = 0;
    }
  }
  if(i) {
    for (j = i; j <4; j++) char_array_4[j] = 0;
    for (j = 0; j <4; j++) char_array_4[j] = Base64.find(char_array_4[j]);
    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }
  return ret;
}
string Tools::EncodeBase64(const string &input) {
  int i = 0, j = 0, in_len = input.size();
  uint8_t char_array_3[3], char_array_4[4];
  const char *bytes_to_encode = input.c_str();
  string ret;
  while(in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;
      for(i = 0; (i <4) ; i++) ret += Base64[char_array_4[i]];
      i = 0;
    }
  }
  if(i) {
    for(j = i; j < 3; j++) char_array_3[j] = '\0';
    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;
    for(j = 0; (j < i + 1); j++) ret += Base64[char_array_4[j]];
    while((i++ < 3)) ret += '=';
  }
  return ret;
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