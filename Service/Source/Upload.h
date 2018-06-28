#pragma once
#include <json-c/json.h>
class Upload {
  public :
    Upload(char *id, json_object *config);
};