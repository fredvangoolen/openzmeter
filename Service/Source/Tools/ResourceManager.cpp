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
#include "ResourceManager.h"
#include <string.h>
//----------------------------------------------------------------------------------------------------------------------
extern char _binary_ResourceFiles_start[];
extern char _binary_ResourceFiles_end[];
map<string, ResourceManager::File_t> ResourceManager::pFiles;
//----------------------------------------------------------------------------------------------------------------------
uint8_t *ResourceManager::GetFile(string name, size_t *size) {
  *size = 0;
  if(pFiles.size() == 0) {
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
  }
  auto File = pFiles.find(name);
  if(File == pFiles.end()) return NULL;
  *size = File->second.Size;
  return File->second.Ptr;
}
//----------------------------------------------------------------------------------------------------------------------