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
#include "AnalyzerSoftGA.h"
#include <cmath>
#include <vector>
//----------------------------------------------------------------------------------------------------------------------
//static double Epsilon = pow(2, -25);
//bool IsNearZero(double d) {
//  return (fabs(d) <= Epsilon);
//}
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
//bool GaPoTNumBivectorTerm::IsScalar() {
//  return TermId1 == TermId2;
//}
//GaPoTNumBivectorTerm::GaPoTNumBivectorTerm(uint8_t id1, uint8_t id2, float value) {
//  if(id1 == id2) {
//    TermId1 = 0;
//    TermId2 = 0;
//    Value = value;
//  } else if(id1 < id2) {
//    TermId1 = id1;
//    TermId2 = id2;
//    Value = value;
//  } else {
//    TermId1 = id2;
//    TermId2 = id1;
//    Value = -value;
//  }
//}
//string GaPoTNumBivectorTerm::ToText() {
//  if(Value == 0) return "0";
//  return IsScalar() ? to_string(Value) : to_string(Value) + " <" + to_string(TermId1 + 1) + ", " + to_string(TermId2 + 1) + ">";
//}
//string GaPoTNumBivectorTerm::ToString() {
//  return ToText();
//}
//----------------------------------------------------------------------------------------------------------------------
GABivector::GABivector(uint16_t dims) {
  pTerms = (float*)calloc((dims * (dims - 1)) / 2 + 1, sizeof(float));
  pDims  = dims;
}
GABivector::~GABivector() {
  if(pTerms != NULL) free(pTerms);
}
void GABivector::AddTerm(uint16_t id1, uint16_t id2, float value) {
  if(id1 == id2) {
    pTerms[0] += value;
  } else if(id1 < id2) {
    pTerms[(id2 * (id2 - 1)) / 2 + id1 + 1] += value;
  } else {
    pTerms[(id1 * (id1 - 1)) / 2 + id2 + 1] -= value;
  }
}


//float GABivector::GetActiveTotal() {
//  float Tmp = 0.0;
//  for(auto &Term : _termsDictionary) {
//    if(Term.second.IsScalar()) Tmp += Term.second.Value;
//  }
//  return Tmp;
//}
//float GABivector::Norm() {
//  float Tmp = 0.0;
//  for(auto &Term : _termsDictionary) Tmp += Term.second.Value * Term.second.Value;
//  return sqrt(Tmp);
//}
//void GABivector::AddTerm(GaPoTNumBivectorTerm term) {
//  uint16_t Idx = (term.TermId1 << 8) | term.TermId2;
//  auto oldTerm = _termsDictionary.find(Idx);
//  if(oldTerm != _termsDictionary.end()) {
//    oldTerm->second = GaPoTNumBivectorTerm(term.TermId1, term.TermId2, oldTerm->second.Value + term.Value);
//  } else {
//    _termsDictionary.insert({Idx, term});
//  }
//}
//GaPoTNumBivector GaPoTNumBivector::Inverse() {
//  GaPoTNumBivector Tmp;
//  float Scale = 1.0 / Norm();
//  vector<GaPoTNumBivectorTerm> Tmp;
//  for(auto &Term : _termsDictionary) {
//    Tmp.AddTerm(GaPoTNumBivectorTerm(Term.second.TermId1, Term.second.TermId2, (Term.second.IsScalar()) ? Term.second.Value * Scale : -Term.second.Value * Scale));
//  }
//  return Tmp;
//}
//vector<GaPoTNumBivectorTerm> GaPoTNumBivector::GetTerms() {
//  vector<GaPoTNumBivectorTerm> Tmp;
//  for(auto &Term : _termsDictionary) {
//    if(IsNearZero(Term.second.Value) == false) Tmp.push_back(Term.second);
//  }
//  return Tmp;
//}
//GaPoTNumBivector GaPoTNumBivector::GetNonActivePart() {
//  GaPoTNumBivector Tmp;
//  for(auto &Term : _termsDictionary) {
//    if(Term.second.IsScalar() == false) Tmp.AddTerm(Term.second);
//  }
//  return Tmp;
//}
//string GaPoTNumBivector::TermsToText() {
//  auto Terms = GetTerms();
//  if(Terms.empty()) return "0";
//  string Tmp;
//  for(auto &Term : Terms) {
//    Tmp += Term.ToString() + ", ";
//  }
//  Tmp.resize(Tmp.size() - 2);
//  return Tmp;
//}
//string GaPoTNumBivector::ToString() {
//  return TermsToText();
//}
json GABivector::ToJSON() {
  //returns and array of bivector in form: [230, [100, [1, 2]], ... ]
  json Tmp = json::array();
  Tmp.push_back(pTerms[0]);
  for(uint16_t id1 = 0; id1 < pDims; id1++) {
    for(uint16_t id2 = (id1 + 1); id2 < pDims; id2++) {
      json Term  = json::array();
      json Index = json::array();
      Term.push_back(pTerms[(id2 * (id2 - 1)) / 2 + id1 + 1]);
      Index.push_back(id1 + 1);
      Index.push_back(id2 + 1);
      Term.push_back(Index);
      Tmp.push_back(Term);
    }
  }
  return Tmp;
}
//----------------------------------------------------------------------------------------------------------------------
GAVector::GAVector(uint16_t dims) {
  pTerms = (float*)calloc(dims, sizeof(float));
  pDims  = dims;
}
GAVector::~GAVector() {
  if(pTerms != NULL) free(pTerms);
}
GAVector GAVector::operator *(const float v2) const {
  GAVector Tmp(pDims);
  for(uint16_t n = 0; n < pDims; n++) Tmp.AddTerm(n, pTerms[n] * v2);
  return Tmp;
}
GABivector GAVector::operator *(const GAVector &v2) const {
  GABivector Tmp((v2.pDims > pDims) ? v2.pDims : pDims);
  for(uint16_t x = 0; x < pDims; x++) {
    for(uint16_t y = 0; y < v2.pDims; y++) Tmp.AddTerm(x, y, pTerms[x] * v2.pTerms[y]);
  }
  return Tmp;
}
void GAVector::AddRectPhasor(const uint16_t id, const float x, const float y) {
  AddTerm(id, x);
  AddTerm(id + 1, -y);
}
void GAVector::AddTerm(const uint16_t id, const float value) {
  pTerms[id] += value;
}
//GAVector GAVector::operator *(GaPoTNumBivector &bv2) {
//  GAVector Tmp;
//  for(auto &Term1 : _termsDictionary) {
//    for(auto &Term2 : bv2._termsDictionary) {
//      auto scalarValue = Term1.second.Value * Term2.second.Value;
//      if(Term2.second.IsScalar()) {
//        Tmp.AddTerm(Term1.second.TermId, scalarValue);
//      } else if(Term1.second.TermId == Term2.second.TermId1) {
//        Tmp.AddTerm(Term2.second.TermId2, scalarValue);
//      } else if(Term1.second.TermId == Term2.second.TermId2) {
//        Tmp.AddTerm(Term2.second.TermId1, -scalarValue);
//      }
//    }
//  }
//  return Tmp;
//}
//GAVector GAVector::Inverse() {
//  float Scale = 1.0 / Norm();
//  GAVector Tmp;
//  for(auto &Term : _termsDictionary) Tmp._termsDictionary.insert({Term.second.TermId, GaPoTNumVectorTerm(Term.second.TermId, Term.second.Value * Scale)});
//  return Tmp;
//}
//string GAVector::ToString() {
//  if(_termsDictionary.empty()) return "0";
//  string Tmp;
//  for(auto &Term : _termsDictionary) Tmp += Term.second.ToString() + ", ";
//  Tmp.resize(Tmp.size() - 2);
//  return Tmp;
//}
//float GAVector::Norm() {
//  float Tmp = 0.0;
//  for(auto &Term : _termsDictionary) Tmp += Term.second.Value * Term.second.Value;
//  return sqrt(Tmp);
//}
json GAVector::ToJSON() {
  //returns and array of phasors in form: [230, 1], [0.1, 2], ... ]
  json Tmp = json::array();
  for(uint16_t n = 0; n < pDims; n++) {
    json Term  = json::array();
    Term.push_back(pTerms[n]);
    Term.push_back(n + 1);
    Tmp.push_back(Term);
  }
  return Tmp;
}

  AnalyzerSoftGA::AnalyzerSoftGA() {}
AnalyzerSoftGA::~AnalyzerSoftGA() {}
bool AnalyzerSoftGA::GetSeriesNow(const string &serie, json &result) const {
  return true;
}
json AnalyzerSoftGA::Calc() {
  json Result = json::object();
//  GAVector vGA(50 * 2 * pVoltageChannels);
//  GAVector iGA(50 * 2 * pVoltageChannels);
//  for(uint8_t j = 0; j < pVoltageChannels; j++) {
//    for(uint8_t i = 0; i < 50; i++) {
//      vGA.AddRectPhasor(i * 2, pVoltageFFT[j]->Harmonic(i).Re, pVoltageFFT[j]->Harmonic(i).Im);
//      iGA.AddRectPhasor(i * 2, pCurrentFFT[j]->Harmonic(i).Re, pCurrentFFT[j]->Harmonic(i).Im);
//    }
//  }
//  Result["Voltage"] = vGA.ToJSON();
//  Result["Current"] = iGA.ToJSON();
//  GABivector M = vGA * iGA;
//  Result["Power"] = M.ToJSON();
//  float Ma = M.GetActiveTotal();
//  Result["Ma"] = Ma;
//  GaPoTNumBivector Mn = M.GetNonActivePart();
//  Result["Mn"] = Mn.ToString();
//  float Norm_M = M.Norm();
//  Result["Norm(M)"] = Norm_M;
//  float Norm_iGA = iGA.Norm();
//  Result["Norm(iGA)"] = Norm_iGA;
//  float Norm_vGA = vGA.Norm();
//  Result["Norm(vGA)"] = Norm_vGA;
//  float Norm_Ia = (vGA.Inverse() * Ma).Norm();
//  Result["Ia"] = Norm_Ia;
//  float Norm_In = (vGA.Inverse() * Mn).Norm();
//  Result["In"] = Norm_In;
//  auto PF = Ma / Norm_M;
//  Result["PF"] = PF;
  return Result;
}
