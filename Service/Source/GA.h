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
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class GaPoTNumBivectorTerm final {
  public :
    uint8_t TermId1;
    uint8_t TermId2;
    float  Value;
  public :
    bool IsScalar();
    GaPoTNumBivectorTerm(uint8_t id1, uint8_t id2, float value);
    string ToText();
    string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class GaPoTNumVectorTerm final {
  public :
    uint8_t TermId;
    float  Value;
  public :
    GaPoTNumVectorTerm(uint8_t id, float value);
    GaPoTNumVectorTerm operator +(const GaPoTNumVectorTerm &t2);
    string ToText();
    string ToString();
};
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
class GaPoTNumBivector final {
  public :
    map<uint16_t, GaPoTNumBivectorTerm> _termsDictionary;
  public :
    GaPoTNumBivector();
    float GetActiveTotal();
    float Norm();
    GaPoTNumBivector GetNonActivePart();
    void AddTerm(GaPoTNumBivectorTerm term);
    void AddTerm(uint8_t id1, uint8_t id2, float value);
    vector<GaPoTNumBivectorTerm> GetTerms();
//    GaPoTNumBivector Inverse();
    string TermsToText();
    string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class GAVector final {
  private :
    map<uint8_t, GaPoTNumVectorTerm> _termsDictionary;
  public :
    GaPoTNumBivector operator *(GAVector v2);
    GAVector operator *(float v2);
    GAVector operator *(GaPoTNumBivector &bv2);
    void AddRectPhasor(uint8_t id, float x, float y);
    GAVector Inverse();
    float Norm();
    void AddTerm(int id, float value);
  string ToText();
  string RectPhasorsToText();
  string ToString();
};
//----------------------------------------------------------------------------------------------------------------------