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
#include "TariffScript.h"
// ---------------------------------------------------------------------------------------------------------------------
string TariffScript::CheckConfig(const string &script, set<string> &variables) {
  typedef struct {
    set<string> *Variables;
    float        TmpVar;
  } Context_t;
  Context_t Context;
  Context.TmpVar = 0;
  Context.Variables = &variables;
  Parser   TmpParser;
  auto VariableFactory = [](const char_type *name, void *params) -> float* {
    Context_t *Tmp = (Context_t*)params;
    Tmp->Variables->insert(name);
    return &Tmp->TmpVar;
  };
  TmpParser.SetVarFactory(VariableFactory, &Context);
  for(auto &Var : variables) TmpParser.DefineVar(Var,  &Context.TmpVar);
  try {
    TmpParser.SetExpr(script);
    TmpParser.Eval();
  } catch(Parser::exception_type &e) {
    return "'Script' parse failed (" + e.GetMsg() + ")";
  }
  return "";
}
// ---------------------------------------------------------------------------------------------------------------------