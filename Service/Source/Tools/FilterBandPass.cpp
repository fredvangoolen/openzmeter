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
#include "FilterBandPass.h"
// ---------------------------------------------------------------------------------------------------------------------
FilterBandPass::FilterBandPass(const uint8_t channels, const uint8_t order, const float samplerate, const float freq, const float bandwidth) : pChannels(channels) {

/*function [a,K]= biquad_bp(N,fcenter,bw,fs)
  f1= fcenter- bw/2;                % Hz lower -3 dB frequency
  f2= fcenter+ bw/2;                % Hz upper -3 dB frequency
  if f2>=fs/2;
      error('fcenter+ bw/2 must be less than fs/2')
  end
  if f1<=0
      error('fcenter- bw/2 must be greater than 0')
  end

  % pre-warp f0, f1, and f2 (uppercase == continuous frequency variables)
  F1= fs/pi * tan(pi*f1/fs);
  F2= fs/pi * tan(pi*f2/fs);
  BW_hz= F2-F1;                % Hz -3 dB bandwidth
  F0= sqrt(F1*F2);             % Hz geometric mean frequency

  % transform poles for bpf centered at W0
  % pa contains N poles of the total 2N -- the other N poles not computed
  % are conjugates of these.
  for i= 1:N;
    theta= (2*i -1)*pi/(2*N);
    p_lp= -sin(theta) + j*cos(theta);
    alpha= BW_hz/F0 * 1/2*p_lp;
    beta= sqrt(1- (BW_hz/F0*p_lp/2).^2);
    pa(i)= 2*pi*F0*(alpha +j*beta);
    p= (1 + pa(i)/(2*fs))./(1 - pa(i)/(2*fs));    % bilinear transform
    a1= -2*real(p);
    a2= abs(p)^2;
    a(i,:)= [1 a1 a2];                 % denominator coeffs of biquad k
  end
  b= [1 0 -1];                           % biquad numerator coeffs
  % compute biquad gain K for response amplitude = 1.0 at f0
  f0= sqrt(f1*f2)                        % geometric mean frequency

  for k= 1:N;
    aa= a(k,:);
    h= freqzEdu(b,aa,[f0 f0],fs);         % frequency response at f = f0
    K(k)= 1/abs(h(1));
  end

endfunction
*/
}
FilterBandPass::~FilterBandPass() {

}
// ---------------------------------------------------------------------------------------------------------------------