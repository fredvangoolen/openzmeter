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
#include <complex>
#include <stdlib.h>
#include <math.h>
#include "FilterBandPass.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
FilterBandPass::FilterBandPass(const uint8_t channels, const uint8_t order, const float samplerate, const float freq, const float bandwidth) : pChannels(channels), pStageCount(order) {
  pStages = (Stage_t*)calloc(sizeof(Stage_t), order);
  for(uint8_t n = 0; n < order; n++) {
    pStages[n].Z1 = (float*)calloc(sizeof(float), channels);
    pStages[n].Z2 = (float*)calloc(sizeof(float), channels);
  }
  float f1 = freq - bandwidth / 2.0;
  float f2 = freq + bandwidth / 2.0;
  float f0 = sqrt(f1 * f2);
  if((f1 <= 0) || (f2 >= (samplerate / 2.0))) return;
  float F1 = samplerate / M_PI * tan(M_PI * f1 / samplerate);
  float F2 = samplerate / M_PI * tan(M_PI * f2 / samplerate);
  float F0 = sqrt(F1 * F2);
  for(uint8_t i = 0; i < order; i++) {
    float theta = (2.0 * (i + 1.0) - 1.0) * M_PI / (2.0 * order);
    complex<float> p_lp = complex<float>(0, 1) * cos(theta) - sin(theta);
    complex<float> alpha = (F2 - F1) / F0 * 0.5 * p_lp;
    complex<float> beta = sqrt(1.0 - pow((F2 - F1) / F0 * p_lp / 2.0, 2.0));
    complex<float> pa = 2.0 * M_PI * F0 * (alpha + complex<float>(0, 1) * beta);
    complex<float> p = (1.0 + pa / (2.0 * samplerate)) / (1.0 - pa / (2.0 * samplerate));
    pStages[i].CA0 = 1.0;
    pStages[i].CA1 = -2.0 * p.real();
    pStages[i].CA2 = pow(abs(p), 2.0);

    float w = 2.0 * M_PI * f0 / samplerate;
    complex<float> hc = exp(complex<float>(0, 1) * w);
    complex<float> hb = pow(hc, 2.0) - 1.0;
    complex<float> ha = pow(hc, 2.0) + pStages[i].CA1 * hc + pStages[i].CA2;
    complex<float> h = hb / ha;
    pStages[i].CB0 = 1.0 / abs(h);
    pStages[i].CB1 = 0.0;
    pStages[i].CB2 = -1.0 / abs(h);
  }
}
FilterBandPass::~FilterBandPass() {

}
// ---------------------------------------------------------------------------------------------------------------------