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
#pragma once
#include <vector>
#include <complex>
#include <stdint.h>
#include <pthread.h>
#include "Tools.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class AnalyzerSoft::FFT final {
  public :
    vector<complex<float> > Output;                //Datos de salida
  private :
    pthread_t               pThread;               //Hilo de ejecucion
    WorkerLocks             pWorkerLocks;          //Threadworker sync
    vector<float>          *pInput;                //Muestras de entrada
    bool                    pTerminate;            //Marca para finalizar proceso
  private :
    static pthread_mutex_t  pPlannerMutex;
  private :
    static void *FFT_Thread(void *args);
  public :
    static float CalcAngle(const complex<float> &a, const complex<float> &b);
  public :
    FFT(const string &name);
    void  Start(vector<float> &in);
    float BinPower(const uint bin);
    void  Wait();
    ~FFT();
};
