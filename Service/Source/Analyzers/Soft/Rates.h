/* ================================== zMeter ===========================================================================
 * File:     Rates.h
 * Author:   Eduardo Viciana (2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "AnalyzerSoft.h"
class AnalyzerSoft::Rates final {
  private :
    AnalyzerSoft *pParent;
  public:
    Rates(AnalyzerSoft *parent);
    ~Rates();
    void  GetRates(const json &params, Response &response);
    void  GetRate(const json &params, Response &response);
    void  SetRate(const json &params, Response &response);
    void  DelRate(const json &params, Response &response);
    void  AddRate(const json &params, Response &response);
    void  GetPrices(const json &params, Response &response);
};