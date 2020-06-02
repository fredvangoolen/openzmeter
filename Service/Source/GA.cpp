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
//
//     public sealed class Dictionary2Keys<TKey, TValue> : IReadOnlyCollection<KeyValuePair<Tuple<TKey, TKey>, TValue>>
//    {
//        private readonly Dictionary<Tuple<TKey, TKey>, TValue> _dictionary =
//            new Dictionary<Tuple<TKey, TKey>, TValue>();
//
//
//        public int Count => _dictionary.Count;
//
//        public ICollection<Tuple<TKey, TKey>> Keys => _dictionary.Keys;
//
//        public ICollection<TValue> Values => _dictionary.Values;
//
//        public TValue this[TKey key1, TKey key2]
//        {
//            get { return _dictionary[new Tuple<TKey, TKey>(key1, key2)]; }
//            set { _dictionary[new Tuple<TKey, TKey>(key1, key2)] = value; }
//        }
//
//
//        public Dictionary2Keys<TKey, TValue> Add(TKey key1, TKey key2, TValue value)
//        {
//            _dictionary.Add(new Tuple<TKey, TKey>(key1, key2), value);
//
//            return this;
//        }
//
//        public bool TryGetValue(TKey key1, TKey key2, out TValue value)
//        {
//            return _dictionary.TryGetValue(new Tuple<TKey, TKey>(key1, key2), out value);
//        }
//
//        public IEnumerator<KeyValuePair<Tuple<TKey, TKey>, TValue>> GetEnumerator()
//        {
//            return _dictionary.GetEnumerator();
//        }
//
//        IEnumerator IEnumerable.GetEnumerator()
//        {
//            return _dictionary.GetEnumerator();
//        }
//    }
//    public static class GaNumericsUtils
//    {
//
//
//        public static string Concatenate(this IEnumerable<string> items, string separator)
//        {
//            var s = new StringBuilder();
//
//            var itemSeparator = separator ?? String.Empty;
//
//            var flag = false;
//            foreach (var item in items)
//            {
//                if (flag)
//                    s.Append(itemSeparator);
//                else
//                    flag = true;
//
//                s.Append(item);
//            }
//
//            return s.ToString();
//        }
//
//
//    }
//----------------------------------------------------------------------------------------------------------------------
GaPoTNumRectPhasor GaPoTNumRectPhasor::operator +(const GaPoTNumRectPhasor &p2) {
  if(Id != p2.Id) throw new exception();
  return GaPoTNumRectPhasor(Id, XValue + p2.XValue, YValue + p2.YValue);
}
GaPoTNumRectPhasor::GaPoTNumRectPhasor(uint8_t id, double x, double y) {
  if(id % 2 != 0) throw new exception();
  Id     = id;
  XValue = x;
  YValue = y;
}
vector<GaPoTNumSinglePhaseVectorTerm> GaPoTNumRectPhasor::GetTerms() {
  vector<GaPoTNumSinglePhaseVectorTerm> Tmp;
  if(XValue != 0) Tmp.push_back(GaPoTNumSinglePhaseVectorTerm(Id, XValue));
  if(YValue != 0) Tmp.push_back(GaPoTNumSinglePhaseVectorTerm(Id + 1, -YValue));
  return Tmp;
}
string GaPoTNumRectPhasor::ToText() {
  if((XValue == 0) && (YValue == 0)) return "0";
  return "r(" + to_string(XValue) + ", " + to_string(YValue) + ") <" + to_string(Id + 1) + ", " + to_string(Id + 2) +">";
}
string GaPoTNumRectPhasor::ToString() {
  return ToText();
}
//----------------------------------------------------------------------------------------------------------------------
bool GaPoTNumSinglePhaseBivectorTerm::IsScalar() {
  return TermId1 == TermId2;
}
GaPoTNumSinglePhaseBivectorTerm::GaPoTNumSinglePhaseBivectorTerm(uint8_t id1, uint8_t id2, double value) {
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
string GaPoTNumSinglePhaseBivectorTerm::ToText() {
  if(Value == 0) return "0";
  return IsScalar() ? to_string(Value) : to_string(Value) + " <" + to_string(TermId1 + 1) + ", " + to_string(TermId2 + 1) + ">";
}
string GaPoTNumSinglePhaseBivectorTerm::ToString() {
  return ToText();
}
//----------------------------------------------------------------------------------------------------------------------
GaPoTNumSinglePhaseBivector::GaPoTNumSinglePhaseBivector() {
}
void GaPoTNumSinglePhaseBivector::AddTerm(GaPoTNumSinglePhaseBivectorTerm term) {
  uint16_t Idx = (term.TermId1 << 8) | term.TermId2;
  auto oldTerm = _termsDictionary.find(Idx);
  if(oldTerm != _termsDictionary.end()) {
    oldTerm->second = GaPoTNumSinglePhaseBivectorTerm(term.TermId1, term.TermId2, oldTerm->second.Value + term.Value);
  } else {
    _termsDictionary.insert({Idx, term});
  }
}
void GaPoTNumSinglePhaseBivector::AddTerm(uint8_t id1, uint8_t id2, double value) {
  AddTerm(GaPoTNumSinglePhaseBivectorTerm(id1, id2, value));
}
vector<GaPoTNumSinglePhaseBivectorTerm> GaPoTNumSinglePhaseBivector::GetTerms() {
  vector<GaPoTNumSinglePhaseBivectorTerm> Tmp;
  for(auto &Term : _termsDictionary) {
    if(IsNearZero(Term.second.Value) == false) Tmp.push_back(Term.second);
  }
  return Tmp;
}
string GaPoTNumSinglePhaseBivector::TermsToText() {
  auto Terms = GetTerms();
  if(Terms.empty()) return "0";
  string Tmp;
  for(auto &Term : Terms) {
    Tmp += Term.ToString() + ", ";
  }
  Tmp.resize(Tmp.size() - 2);
  return Tmp;
}
string GaPoTNumSinglePhaseBivector::ToString() {
  return TermsToText();
}
//----------------------------------------------------------------------------------------------------------------------
GaPoTNumSinglePhaseVectorTerm::GaPoTNumSinglePhaseVectorTerm(uint8_t id, double value) {
  TermId = id;
  Value = value;
}
string GaPoTNumSinglePhaseVectorTerm::ToText() {
  if(Value == 0) return "0";
  return to_string(Value) + " <" + to_string(TermId + 1) + "> ";
}
string GaPoTNumSinglePhaseVectorTerm::ToString() {
  return ToText();
}
//----------------------------------------------------------------------------------------------------------------------
GaPoTNumSinglePhaseBivector GAVector::operator *(GAVector v2) {
  GaPoTNumSinglePhaseBivector bivector;
  for(auto &term1 : GetTerms()) {
    for(auto &term2 : v2.GetTerms()) {
      bivector.AddTerm(term1.TermId, term2.TermId, term1.Value * term2.Value);
    }
  }
  return bivector;
}
void GAVector::AddRectPhasor(uint8_t id, double x, double y) {
  GaPoTNumRectPhasor newPhasor(id, x, y);
  auto oldPhasor = _phasorsDictionary.find(id);
  if(oldPhasor != _phasorsDictionary.end()) {
    oldPhasor->second = (oldPhasor->second + newPhasor);
  } else {
    _phasorsDictionary.insert({id, newPhasor});
  }
}
vector<GaPoTNumSinglePhaseVectorTerm> GAVector::GetTerms() {
  vector<GaPoTNumSinglePhaseVectorTerm> Terms;
  for(auto &Phasor : _phasorsDictionary) {
    for(auto &Term : Phasor.second.GetTerms()) Terms.push_back(Term);
  }
  return Terms;
}
string GAVector::ToString() {
  if(_phasorsDictionary.empty()) return "0";
  string Tmp;
  for(auto &Term : _phasorsDictionary) Tmp += Term.second.ToString() + ", ";
  Tmp.resize(Tmp.size() - 2);
  return Tmp;
}