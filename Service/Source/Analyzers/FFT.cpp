/* ================================== zMeter ===========================================================================
 * File:     FFT.cpp
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
//#include <fftw3.h>
#include <sys/types.h>
#include "FFT.h"
#include <errno.h>
#include <string.h>
#include <math.h>
#include <zlib.h>

#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
//pthread_mutex_t FFT::pPlannerMutex = PTHREAD_MUTEX_INITIALIZER;
FFT::FFT(char *name) {
  pTerminate = false;
  pWorking = false;
  pthread_cond_init(&pThreadCond, NULL);
  pthread_mutex_init(&pThreadMutex, NULL);
  pThread = 0;
  if(pthread_create(&pThread, NULL, FFT_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, name);
}
FFT::~FFT() {
  if(pThread == 0) return;
  pTerminate = true;
  pthread_cond_signal(&pThreadCond);
  if(pthread_join(pThread, NULL) != 0) LOGFile::Error("%s:%d -> (Unknown)\n", __func__, __LINE__);
}
void FFT::Start(vector<float> &in) {
  pthread_mutex_lock(&pThreadMutex);
  if(pWorking == true) {
    pthread_mutex_unlock(&pThreadMutex);
    return;
  }
  Output.clear();
  Output.reserve(in.size());
  for(uint n = 0; n < in.size(); n++) Output.push_back(complex<float>(in[n], 0));
  pWorking = true;
  pthread_mutex_unlock(&pThreadMutex);
  pthread_cond_signal(&pThreadCond);
}
void *FFT::FFT_Thread(void *args) {
  FFT *This = (FFT*)args;
  while(true) {
    pthread_mutex_lock(&This->pThreadMutex);
    while(This->pWorking == false) {
      if(This->pTerminate == true) {
        pthread_mutex_unlock(&This->pThreadMutex);
        return NULL;
      }
      pthread_cond_wait(&This->pThreadCond, &This->pThreadMutex);
    }
    pthread_mutex_unlock(&This->pThreadMutex);
    This->Transform(This->Output);
    for(uint i = 0; i < ((This->Output.size() / 2) + 1); i++) This->Output[i] = (This->Output[i] / (float)(This->Output.size() / M_SQRT2));
    This->Output.resize((This->Output.size() / 2) + 1);
    pthread_mutex_lock(&This->pThreadMutex);
    This->pWorking = false;
    pthread_mutex_unlock(&This->pThreadMutex);
    pthread_cond_signal(&This->pThreadCond);
  }
  return NULL;
}
void FFT::Wait() {
  pthread_mutex_lock(&pThreadMutex);
  while(pWorking == true) {
    if(pTerminate == true) return;
    pthread_cond_wait(&pThreadCond, &pThreadMutex);
  } 
  pthread_mutex_unlock(&pThreadMutex);
}
float FFT::CalcAngle(complex<float> a, complex<float> b) {
  float Tmp;
  if((a.real() == 0) && (a.imag() == 0)) 
    Tmp = 0;
  else if(a.real() < 0)
    Tmp = atan(a.imag() / a.real());
  else
    Tmp = atan(a.imag() / a.real()) + M_PI;
  if((b.real() == 0) && (b.imag() == 0)) 
    return Tmp;
  else if(b.real() < 0)
    return Tmp - atan(b.imag() / b.real());
  else
    return Tmp - atan(b.imag() / b.real()) + M_PI;
}
float FFT::BinPower(complex<float> bin) {
  return sqrt(norm(bin)) / M_SQRT2 * 2.0;
}
void FFT::Transform(vector<complex<float> > &vec) {
	size_t n = vec.size();
	if(n == 0) {
    return;
	} else if((n & (n - 1)) == 0) {
		TransformRadix2(vec);
	} else {
		TransformBluestein(vec);
  }
}
void FFT::InverseTransform(vector<complex<float> > &vec) {
	std::transform(vec.cbegin(), vec.cend(), vec.begin(), static_cast<complex<float> (*)(const complex<float> &)>(std::conj));
	Transform(vec);
	std::transform(vec.cbegin(), vec.cend(), vec.begin(), static_cast<complex<float> (*)(const complex<float> &)>(std::conj));
}
void FFT::TransformRadix2(vector<complex<float> > &vec) {
	size_t n = vec.size();
  int levels = 0;  // Compute levels = floor(log2(n))
	for(size_t temp = n; temp > 1U; temp >>= 1) levels++;
	if(static_cast<size_t>(1U) << levels != n) throw std::domain_error("Length is not a power of 2");
	vector<complex<float> > expTable(n / 2);
	for(size_t i = 0; i < n / 2; i++) expTable[i] = std::exp(complex<float>(0, -2 * M_PI * i / n));
	for(size_t i = 0; i < n; i++) {
		size_t j = ReverseBits(i, levels);
		if(j > i) std::swap(vec[i], vec[j]);
	}
	for(size_t size = 2; size <= n; size *= 2) {
		size_t halfsize = size / 2;
		size_t tablestep = n / size;
		for(size_t i = 0; i < n; i += size) {
			for(size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep) {
				complex<float> temp = vec[j + halfsize] * expTable[k];
				vec[j + halfsize] = vec[j] - temp;
				vec[j] += temp;
			}
		}
		if (size == n) break;
	}
}
void FFT::TransformBluestein(vector<complex<float> > &vec) {
	size_t n = vec.size();
	size_t m = 1;
	while(m / 2 <= n) {
		if(m > SIZE_MAX / 2) throw std::length_error("Vector too large");
		m *= 2;
	}
	vector<complex<float> > expTable(n);
	for(size_t i = 0; i < n; i++) {
		unsigned long long temp = static_cast<unsigned long long>(i) * i;
		temp %= static_cast<unsigned long long>(n) * 2;
		float angle = M_PI * temp / n;
		expTable[i] = std::exp(complex<float>(0, -angle));
	}
	vector<complex<float> > av(m);
	for(size_t i = 0; i < n; i++) av[i] = vec[i] * expTable[i];
	vector<complex<float> > bv(m);
	bv[0] = expTable[0];
	for(size_t i = 1; i < n; i++)	bv[i] = bv[m - i] = std::conj(expTable[i]);
	vector<complex<float> > cv(m);
	Convolve(av, bv, cv);
	for(size_t i = 0; i < n; i++) vec[i] = cv[i] * expTable[i];
}
void FFT::Convolve(const vector<complex<float> > &xvec, const vector<complex<float> > &yvec,	vector<complex<float> > &outvec) {
	size_t n = xvec.size();
	if(n != yvec.size() || n != outvec.size()) throw std::domain_error("Mismatched lengths");
	vector<complex<float> > xv = xvec;
	vector<complex<float> > yv = yvec;
	Transform(xv);
	Transform(yv);
	for(size_t i = 0; i < n; i++)	xv[i] *= yv[i];
	InverseTransform(xv);
	for(size_t i = 0; i < n; i++)  outvec[i] = xv[i] / static_cast<float>(n);
}
size_t FFT::ReverseBits(size_t x, int n) {
	size_t result = 0;
	for(int i = 0; i < n; i++, x >>= 1) result = (result << 1) | (x & 1U);
	return result;
}