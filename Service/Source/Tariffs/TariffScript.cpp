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
TariffScript::TariffScript(const string &expr) {
  pInitialized = false;
  pExpression = expr;
  pParser.SetVarFactory(VariableFactory, this);
}
string TariffScript::CheckConfig(const string &script, set<string> &variables) {
  typedef struct {
    set<string> *Variables;
    float        TmpVar;
  } Context_t;
  Context_t Context;
  Parser    TmpParser;
  Context.TmpVar = 0;
  Context.Variables = &variables;
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
float TariffScript::Eval() {
  if(pInitialized == false) {
    pParser.SetExpr(pExpression);
    pInitialized = true;
  }
  return pParser.Eval();
}
void TariffScript::Var(const string &name, float *val) {
  pParser.DefineVar(name, val);
}
float TariffScript::Var(const string &name) {
  auto Vars = pParser.GetVar();
  auto Item = Vars.find(name);
  return (Item == Vars.end()) ? 0.0 : *(Item->second);
}
float *TariffScript::VariableFactory(const char_type *name, void *params) {
  TariffScript *Tmp = (TariffScript*)params;
  Tmp->pDefVariables.push_back(0.0);
  return &Tmp->pDefVariables.back();
};
// ---------------------------------------------------------------------------------------------------------------------