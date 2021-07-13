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
#include "ResourceFile.h"
#include <string.h>
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
extern char _binary_ResourceFiles_start[];
extern char _binary_ResourceFiles_end[];
map<string, ResourceFile::File_t> ResourceFile::pFiles;
//----------------------------------------------------------------------------------------------------------------------
ResourceFile::ResourceFile() {
  LogFile::Info("Starting ResourceFile...");
  char *Ptr = _binary_ResourceFiles_start;
  uint32_t FileCount = atoi(Ptr);
  Ptr = &Ptr[strlen(Ptr) + 1];
  for(uint32_t i = 0; i < FileCount; i++) {
    File_t File;
    string Name = Ptr;
    Ptr = &Ptr[strlen(Ptr) + 1];
    File.Size = atoi(Ptr);
    Ptr = &Ptr[strlen(Ptr) + 1];
    File.Ptr = (uint8_t*)Ptr;
    Ptr = &Ptr[File.Size];
    pFiles.insert({Name, File});
  }
  LogFile::Info("ResourceFile started");
}
ResourceFile::~ResourceFile() {
  LogFile::Info("Stoping ResourceFile...");
  pFiles.clear();
  LogFile::Info("ResourceFile stoped");
}
uint8_t *ResourceFile::GetFile(string name, size_t *size) {
  *size = 0;
  auto File = pFiles.find(name);
  if(File == pFiles.end()) return NULL;
  *size = File->second.Size;
  return File->second.Ptr;
}
string ResourceFile::GetFile(const string name) {
  size_t Size;
  uint8_t *Data = GetFile(name, &Size);
  return (Size == 0) ? string() : string((char*)Data, Size);
}
//----------------------------------------------------------------------------------------------------------------------