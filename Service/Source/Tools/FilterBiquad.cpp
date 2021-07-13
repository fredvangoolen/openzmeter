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
#include <stdlib.h>
#include <math.h>
#include "FilterBiquad.h"
// ---------------------------------------------------------------------------------------------------------------------
FilterBiquad::FilterBiquad(const uint8_t channels, const uint8_t order, const float samplerate, const float freq, const float bandwidth) : pChannelsCount(channels), pOrder(order) {
  pA = (float*)calloc(order * 3, sizeof(float));
  pB = (float*)calloc(order * 3, sizeof(float));
  pD = (float*)calloc(channels * (order + 1) * 2, sizeof(float));
  SetBandPass(samplerate, freq, bandwidth);
}
FilterBiquad::FilterBiquad(const uint8_t channels, const uint8_t order, const float samplerate, const float freq) : pChannelsCount(channels), pOrder(order) {
  pA = (float*)calloc(order * 3, sizeof(float));
  pB = (float*)calloc(order * 3, sizeof(float));
  pD = (float*)calloc(channels * (order + 1) * 2, sizeof(float));
  SetLowPass(samplerate, freq);
}
void FilterBiquad::SetBandPass(const float samplerate, const float freq, const float bandwidth) {
  float f1  = freq - bandwidth / 2.0;
  float f2  = freq + bandwidth / 2.0;
  float ts  = 1.0 / samplerate;
  float f   = sqrt(f1 * f2);
  float w   = 2.0 * M_PI * f / samplerate;
  float wa1 = 2.0 * samplerate * tan(M_PI * f1 * ts);
  float wa2 = 2.0 * samplerate * tan(M_PI * f2 * ts);
  float bw  = wa2 - wa1;
  float wa  = sqrt(wa1 * wa2);
  if((f1 <= 0) || (f2 >= (samplerate / 2.0))) return;
  float *A = pA;
  float *B = pB;
  for(uint8_t i = 0; i < pOrder; i++) {
    float          phi  = M_PI / 2.0 + M_PI * (2.0 * i + 1) / (pOrder * 2.0);
    complex<float> _z = complex<float>(cos(w), -sin(w));
    complex<float> p_lp = complex<float>(cos(phi) * bw / (wa * 2.0), sin(phi) * bw / (wa * 2.0));
    complex<float> s    = p_lp * p_lp;
    s.real(1.0 - s.real());
    s.imag(0.0 - s.imag());
    s = sqrt(s);
    complex<float> p_bp = complex<float>((p_lp.real() - s.imag()) * wa, (p_lp.imag() + s.real()) * wa);
    complex<float> z    = BilinearTransform(p_bp, ts);
    A[0] = 1.0;
    A[1] = -2.0 * z.real();
    A[2] = pow(z.real(), 2.0) + pow(z.imag(), 2.0);
    complex<float> p  = (complex<float>(-1.0, 0.0) * _z * _z + (float)1.0) / ((complex<float>(A[2], 0.0) * _z + A[1]) * _z + 1.0f);
    float k = 1.0 / sqrt(pow(p.real(), 2.0) + pow(p.imag(), 2.0));
    B[0] =  1.0 * k;
    B[1] =  0.0 * k;
    B[2] = -1.0 * k;
    A += 3;
    B += 3;
  }
}
void FilterBiquad::SetLowPass(const float samplerate, const float freq) {
  float w   = 2.0 * M_PI * freq / samplerate;
  float *A = pA;
  float *B = pB;
  for(uint8_t i = 0; i < pOrder; i++) {
    float phi   = M_PI / (4.0 * pOrder) * ((pOrder - i - 1) * 2 + 1);
    float alpha = sin(w) * cos(phi);
    B[0] = (1.0 - cos(w)) / (2.0 * (1.0 + alpha));
    B[1] = (1.0 - cos(w)) / (1.0 + alpha);
    B[2] = (1.0 - cos(w)) / (2.0 * (1.0 + alpha));
    A[0] = 1.0;
    A[1] = -2.0 * cos(w) / (1.0 + alpha);
    A[2] = (1.0 - alpha) / (1.0 + alpha);
    A += 3;
    B += 3;
  }
}
FilterBiquad::~FilterBiquad() {
  free(pA);
  free(pB);
  free(pD);
}
complex<float> FilterBiquad::BilinearTransform(complex<float> &s, float ts) {
  complex<float> p, q;
  float x = s.real();
  float y = s.imag();
  x *= ts / 2.0;
  y *= ts / 2.0;
  p.real(1.0 + x);
  p.imag(y);
  q.real(1.0 - x);
  q.imag(-y);
  return p / q;
}
void FilterBiquad::Push(float *values) {
  float *D = pD;
  for(uint8_t ch = 0; ch < pChannelsCount; ch++) {
    float *A = pA;
    float *B = pB;
    for(uint8_t k = 0; k < pOrder; k++) {
      float y = values[ch] * B[0];
      y += D[0] * B[1] + D[1] * B[2];
      y -= D[2] * A[1] + D[3] * A[2];
      D[1] = D[0];
      D[0] = values[ch];
      values[ch] = y;
      D += 2;
      A += 3;
      B += 3;
    }
    D[1] = D[0];
    D[0] = values[ch];
    D += 2;
  }
}
// ---------------------------------------------------------------------------------------------------------------------