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
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
class UserManager final {
  private :
    static void GetUsersCallback(const json &params, Response &response);
    static void GetUserCallback(const json &params, Response &response);
    static void SetUserPasswordCallback(const json &params, Response &response);
    static void SetUserCallback(const json &params, Response &response);
    static void AddUserCallback(const json &params, Response &response);
    static void DelUserCallback(const json &params, Response &response);
  public :
    UserManager();
    ~UserManager();
};
// ---------------------------------------------------------------------------------------------------------------------