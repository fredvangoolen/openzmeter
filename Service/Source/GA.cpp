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
// Derived from GeometricAlgebraNumericsLib
// Copyright (c) 2019 Ahmad Hosny Eid
//----------------------------------------------------------------------------------------------------------------------
#include "GA.h"
#include <cmath>
#include <vector>
//----------------------------------------------------------------------------------------------------------------------
static double Epsilon = pow(2, -25);
bool IsNearZero(double d) {
  return (fabs(d) <= Epsilon);
}
//----------------------------------------------------------------------------------------------------------------------
//GaPoTNumRectPhasor GaPoTNumRectPhasor::operator +(const GaPoTNumRectPhasor &p2) {
//  if(Id != p2.Id) throw new exception();
//  return GaPoTNumRectPhasor(Id, XValue + p2.XValue, YValue + p2.YValue);
//}
//GaPoTNumRectPhasor::GaPoTNumRectPhasor(uint8_t id, double x, double y) {
//  if(id % 2 != 0) throw new exception();
//  Id     = id;
//  XValue = x;
//  YValue = y;
//}
//vector<GaPoTNumVectorTerm> GaPoTNumRectPhasor::GetTerms() {
//  vector<GaPoTNumVectorTerm> Tmp;
//  if(XValue != 0) Tmp.push_back(GaPoTNumVectorTerm(Id, XValue));
//  if(YValue != 0) Tmp.push_back(GaPoTNumVectorTerm(Id + 1, -YValue));
//  return Tmp;
//}
//string GaPoTNumRectPhasor::ToText() {
//  if((XValue == 0) && (YValue == 0)) return "0";
//  return "r(" + to_string(XValue) + ", " + to_string(YValue) + ") <" + to_string(Id + 1) + ", " + to_string(Id + 2) +">";
//}
//string GaPoTNumRectPhasor::ToString() {
//  return ToText();
//}
//----------------------------------------------------------------------------------------------------------------------
bool GaPoTNumBivectorTerm::IsScalar() {
  return TermId1 == TermId2;
}
GaPoTNumBivectorTerm::GaPoTNumBivectorTerm(uint8_t id1, uint8_t id2, float value) {
  if(id1 == id2) {
    TermId1 = 0;
    TermId2 = 0;
    Value = value;
  } else if(id1 < id2) {
    TermId1 = id1;
    TermId2 = id2;
    Value = value;
  } else {
    TermId1 = id2;
    TermId2 = id1;
    Value = -value;
  }
}
string GaPoTNumBivectorTerm::ToText() {
  if(Value == 0) return "0";
  return IsScalar() ? to_string(Value) : to_string(Value) + " <" + to_string(TermId1 + 1) + ", " + to_string(TermId2 + 1) + ">";
}
string GaPoTNumBivectorTerm::ToString() {
  return ToText();
}
//----------------------------------------------------------------------------------------------------------------------
GaPoTNumBivector::GaPoTNumBivector() {
}
float GaPoTNumBivector::GetActiveTotal() {
  float Tmp = 0.0;
  for(auto &Term : _termsDictionary) {
    if(Term.second.IsScalar()) Tmp += Term.second.Value;
  }
  return Tmp;
}
float GaPoTNumBivector::Norm() {
  float Tmp = 0.0;
  for(auto &Term : _termsDictionary) Tmp += Term.second.Value * Term.second.Value;
  return sqrt(Tmp);
}
void GaPoTNumBivector::AddTerm(GaPoTNumBivectorTerm term) {
  uint16_t Idx = (term.TermId1 << 8) | term.TermId2;
  auto oldTerm = _termsDictionary.find(Idx);
  if(oldTerm != _termsDictionary.end()) {
    oldTerm->second = GaPoTNumBivectorTerm(term.TermId1, term.TermId2, oldTerm->second.Value + term.Value);
  } else {
    _termsDictionary.insert({Idx, term});
  }
}
//GaPoTNumBivector GaPoTNumBivector::Inverse() {
//  GaPoTNumBivector Tmp;
//  float Scale = 1.0 / Norm();
//  vector<GaPoTNumBivectorTerm> Tmp;
//  for(auto &Term : _termsDictionary) {
//    Tmp.AddTerm(GaPoTNumBivectorTerm(Term.second.TermId1, Term.second.TermId2, (Term.second.IsScalar()) ? Term.second.Value * Scale : -Term.second.Value * Scale));
//  }
//  return Tmp;
//}
void GaPoTNumBivector::AddTerm(uint8_t id1, uint8_t id2, float value) {
  AddTerm(GaPoTNumBivectorTerm(id1, id2, value));
}
vector<GaPoTNumBivectorTerm> GaPoTNumBivector::GetTerms() {
  vector<GaPoTNumBivectorTerm> Tmp;
  for(auto &Term : _termsDictionary) {
    if(IsNearZero(Term.second.Value) == false) Tmp.push_back(Term.second);
  }
  return Tmp;
}
GaPoTNumBivector GaPoTNumBivector::GetNonActivePart() {
  GaPoTNumBivector Tmp;
  for(auto &Term : _termsDictionary) {
    if(Term.second.IsScalar() == false) Tmp.AddTerm(Term.second);
  }
  return Tmp;
}
string GaPoTNumBivector::TermsToText() {
  auto Terms = GetTerms();
  if(Terms.empty()) return "0";
  string Tmp;
  for(auto &Term : Terms) {
    Tmp += Term.ToString() + ", ";
  }
  Tmp.resize(Tmp.size() - 2);
  return Tmp;
}
string GaPoTNumBivector::ToString() {
  return TermsToText();
}
//----------------------------------------------------------------------------------------------------------------------
GaPoTNumVectorTerm::GaPoTNumVectorTerm(uint8_t id, float value) {
  TermId = id;
  Value = value;
}
GaPoTNumVectorTerm GaPoTNumVectorTerm::operator +(const GaPoTNumVectorTerm &t2) {
  return GaPoTNumVectorTerm(TermId, Value + t2.Value);
}
string GaPoTNumVectorTerm::ToText() {
  if(Value == 0) return "0";
  return to_string(Value) + " <" + to_string(TermId + 1) + "> ";
}
string GaPoTNumVectorTerm::ToString() {
  return ToText();
}
//----------------------------------------------------------------------------------------------------------------------
GaPoTNumBivector GAVector::operator *(GAVector v2) {
  GaPoTNumBivector bivector;
  for(auto &term1 : _termsDictionary) {
    for(auto &term2 : v2._termsDictionary) {
      bivector.AddTerm(term1.second.TermId, term2.second.TermId, term1.second.Value * term2.second.Value);
    }
  }
  return bivector;
}
void GAVector::AddTerm(int id, float value) {
  auto oldPhasor = _termsDictionary.find(id);
  if(oldPhasor != _termsDictionary.end()) {
    oldPhasor->second = (oldPhasor->second + GaPoTNumVectorTerm(id, value));
  } else {
    _termsDictionary.insert({id, GaPoTNumVectorTerm(id, value)});
  }
}
void GAVector::AddRectPhasor(uint8_t id, float x, float y) {
  AddTerm(id, x);
  AddTerm(id + 1, y);
}
GAVector GAVector::Inverse() {
  float Scale = 1.0 / Norm();
  GAVector Tmp;
  for(auto &Term : _termsDictionary) Tmp._termsDictionary.insert({Term.second.TermId, GaPoTNumVectorTerm(Term.second.TermId, Term.second.Value * Scale)});
  return Tmp;
}
GAVector GAVector::operator *(float v2) {
  GAVector Tmp;
  for(auto &Term : _termsDictionary) Tmp._termsDictionary.insert({Term.second.TermId, GaPoTNumVectorTerm(Term.second.TermId, Term.second.Value * v2)});
  return Tmp;
}
GAVector GAVector::operator *(GaPoTNumBivector &bv2) {
  GAVector Tmp;
  for(auto &Term1 : _termsDictionary) {
    for(auto &Term2 : bv2._termsDictionary) {
      auto scalarValue = Term1.second.Value * Term2.second.Value;
      if(Term2.second.IsScalar()) {
        Tmp.AddTerm(Term1.second.TermId, scalarValue);
      } else if(Term1.second.TermId == Term2.second.TermId1) {
        Tmp.AddTerm(Term2.second.TermId2, scalarValue);
      } else if(Term1.second.TermId == Term2.second.TermId2) {
        Tmp.AddTerm(Term2.second.TermId1, -scalarValue);
      }
    }
  }
  return Tmp;
}
string GAVector::ToString() {
  if(_termsDictionary.empty()) return "0";
  string Tmp;
  for(auto &Term : _termsDictionary) Tmp += Term.second.ToString() + ", ";
  Tmp.resize(Tmp.size() - 2);
  return Tmp;
}
float GAVector::Norm() {
  float Tmp = 0.0;
  for(auto &Term : _termsDictionary) Tmp += Term.second.Value * Term.second.Value;
  return sqrt(Tmp);
}