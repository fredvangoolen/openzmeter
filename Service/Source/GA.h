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
class GaPoTNumSinglePhaseBivectorTerm final {
  public :
    uint8_t TermId1;
    uint8_t TermId2;
    double  Value;
  public :
    bool IsScalar();
    GaPoTNumSinglePhaseBivectorTerm(uint8_t id1, uint8_t id2, double value);
    string ToText();
    string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class GaPoTNumSinglePhaseVectorTerm final {
  public :
    uint8_t TermId;
    double  Value;
  public :
    GaPoTNumSinglePhaseVectorTerm(uint8_t id, double value);
    string ToText();
    string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class GaPoTNumRectPhasor final {
  public :
    uint8_t Id;
    double  XValue;
    double  YValue;
  public :
    GaPoTNumRectPhasor operator +(const GaPoTNumRectPhasor &p2);
    GaPoTNumRectPhasor(uint8_t id, double x, double y);
    vector<GaPoTNumSinglePhaseVectorTerm> GetTerms();
    string ToText();
    string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class GaPoTNumSinglePhaseBivector final {
  private :
    map<uint16_t, GaPoTNumSinglePhaseBivectorTerm> _termsDictionary;
  public :
  GaPoTNumSinglePhaseBivector();
  void AddTerm(GaPoTNumSinglePhaseBivectorTerm term);
  void AddTerm(uint8_t id1, uint8_t id2, double value);
  vector<GaPoTNumSinglePhaseBivectorTerm> GetTerms();
  string TermsToText();
  string ToString();
};
//----------------------------------------------------------------------------------------------------------------------
class GAVector final {
  private :
    map<uint8_t, GaPoTNumRectPhasor> _phasorsDictionary;
  public :
    GaPoTNumSinglePhaseBivector operator *(GAVector v2);
    void AddRectPhasor(uint8_t id, double x, double y);
    vector<GaPoTNumSinglePhaseVectorTerm> GetTerms();
//        public IEnumerable<GaPoTNumRectPhasor> GeTRectPhasors()
//        {
//            return _phasorsDictionary.Values;
//        }
  string ToText();
  string RectPhasorsToText();
  string ToString();
};
//----------------------------------------------------------------------------------------------------------------------