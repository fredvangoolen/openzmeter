/* ================================== zMeter ===========================================================================
 * File:     FFT.c
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fftw3.h>
#include "Main.h"
/* FFT de muestras de entrada ==========================================================================================
 * Realiza FFT en paralelo usando FFTW o la funcion interna (actualmente FFTW, aunque la interna funciona) */

// Tipos privados ======================================================================================================
// =====================================================================================================================