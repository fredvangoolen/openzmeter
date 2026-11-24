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
#include <math.h>
#include "DeviceS1Rate.h"
// ---------------------------------------------------------------------------------------------------------------------
DeviceS1Rate::DeviceS1Rate() {
  Config(0.0, 0.0, MIN_POINTS);
}
void DeviceS1Rate::Config(const double nominal, const double tolerance, const uint16_t maxPoints) {
  pNominal   = nominal;
  pTolerance = tolerance;
  pMaxPoints = maxPoints;
  pRate      = 0.0;
  Restart();
}
void DeviceS1Rate::Restart() {
  pPoints.clear();
  pStart    = -1.0;
  pLastTime = 0.0;
  pCount    = 0;
}
void DeviceS1Rate::Push(const double time, const uint32_t samples) {
  if((pStart >= 0.0) && ((time < pLastTime) || ((time - pLastTime) > MAX_SILENCE))) Restart();
  if(pStart < 0.0) pStart = time;
  pLastTime = time;
  pCount   += samples;
  if((time - pStart) < SETTLE) return;
  if((pPoints.empty() == false) && ((time - pPoints.back().first) < 1.0)) return;
  pPoints.push_back({ time, pCount });
  if(pPoints.size() > pMaxPoints) pPoints.pop_front();
  if(pPoints.size() < MIN_POINTS) return;
  double T0 = pPoints.front().first, C0 = pPoints.front().second, N = pPoints.size();
  double ST = 0.0, SC = 0.0, STT = 0.0, STC = 0.0;
  for(auto &Point : pPoints) {
    double T = Point.first - T0, C = Point.second - C0;
    ST  += T;
    SC  += C;
    STT += T * T;
    STC += T * C;
  }
  double Slope = (N * STC - ST * SC) / (N * STT - ST * ST);
  if(fabs(Slope - pNominal) <= pTolerance) {
    pRate = Slope;
  } else {                                                 //Not what the meter announces: samples were missed
    Restart();
  }
}
bool DeviceS1Rate::Valid() const {
  return pRate > 0.0;
}
double DeviceS1Rate::Rate() const {
  return (pRate > 0.0) ? pRate : pNominal;
}
// ---------------------------------------------------------------------------------------------------------------------
