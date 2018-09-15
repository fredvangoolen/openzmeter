/* ================================== zMeter ===========================================================================
 * File:     ESIOS.cpp
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include <json-c/json.h>
#include "PricesInterface.h"
class ESIOS : public PricesInterface {
  private : 
    static void *Update_Thread(void *args);
  public :
    ESIOS(AnalyzerInterface *analyzer, json_object *config);
    ~ESIOS();
};
