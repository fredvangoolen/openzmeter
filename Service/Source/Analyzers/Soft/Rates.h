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
    void  GetRates(HTTPRequest &req);
    void  GetRate(HTTPRequest &req);
    void  SetRate(HTTPRequest &req);
    void  DelRate(HTTPRequest &req);
    void  AddRate(HTTPRequest &req);
    void  GetPrices(HTTPRequest &req);
};