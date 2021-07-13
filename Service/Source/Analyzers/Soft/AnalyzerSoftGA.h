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
#pragma once
#include <stdint.h>
#include <string>
#include <map>
#include <vector>
#include "nlohmann/json.hpp"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
//class GaPoTNumBivectorTerm final {
//  public :
//    uint8_t TermId1;
//    uint8_t TermId2;
//    float  Value;
//  public :
//    bool IsScalar();
//    GaPoTNumBivectorTerm(uint8_t id1, uint8_t id2, float value);
//    string ToText();
//    string ToString();
//};
//----------------------------------------------------------------------------------------------------------------------
//class GaPoTNumVectorTerm final {
//  public :
//    uint8_t TermId;
//    float   Value;
//  public :
//    GaPoTNumVectorTerm(uint8_t id, float value);
//    GaPoTNumVectorTerm operator +(const GaPoTNumVectorTerm &t2);
//    string ToText();
//    string ToString();
//};
//----------------------------------------------------------------------------------------------------------------------
//class GaPoTNumRectPhasor final {
//  public :
//    uint8_t Id;
//    float  XValue;
//    float  YValue;
//  public :
//    GaPoTNumRectPhasor operator +(const GaPoTNumRectPhasor &p2);
//    GaPoTNumRectPhasor(uint8_t id, float x, float y);
//    vector<GaPoTNumVectorTerm> GetTerms();
//    string ToText();
//    string ToString();
//};
//----------------------------------------------------------------------------------------------------------------------
class GABivector final {
  private :
    float        *pTerms;
    uint16_t      pDims;
  public :

//    map<uint16_t, GaPoTNumBivectorTerm> _termsDictionary;
  public :
    GABivector(uint16_t dims);
    ~GABivector();
  public :
    void AddTerm(const uint16_t id1, const uint16_t id2, float value);

  public :
    json ToJSON();

//    float GetActiveTotal();
//    float Norm();
//    GABivector GetNonActivePart();
//    void AddTerm(GaPoTNumBivectorTerm term);

//    vector<GaPoTNumBivectorTerm> GetTerms();
//    GaPoTNumBivector Inverse();
//    string TermsToText();
//    string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class GAVector final {
  private :
    float        *pTerms;
    uint16_t      pDims;
  public :
    GAVector(uint16_t dims);
    ~GAVector();
  public :
    GAVector   operator *(const float v2) const;
    GABivector operator *(const GAVector &v2) const;
    GAVector   operator *(const GABivector &bv2) const;
  public :
    void     AddRectPhasor(const uint16_t id, const float x, const float y);
    void     AddTerm(const uint16_t id, const float value);
//    float    Norm();
//    GAVector Inverse();
    json     ToJSON();


//    string ToText();
//    string RectPhasorsToText();
//    string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftGA {
  public :
  AnalyzerSoftGA();
  ~AnalyzerSoftGA();
  bool  GetSeriesNow(const string &serie, json &result) const;
  json  Calc();


};