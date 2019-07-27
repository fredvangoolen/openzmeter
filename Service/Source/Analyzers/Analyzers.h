/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.h
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <complex>
#include <nlohmann/json.hpp>
#include "Tools.h"
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Analyzer : protected RWLocks {
  protected :
    static vector<float>  FrequencyRanges;   //Rangos de frecuencias para estadisticas
    static vector<float>  VoltageRanges;     //Rangos de tensiones para estadisticas
    static vector<float>  THDRanges;         //Rangos de distorsion harmonica para estadisticas
    static vector<float>  HarmonicsAllowed;  //Rangos permitidos para cada harmonico
  public :
    enum AnalyzerMode { PHASE1, PHASE3, PHASE3N };
    enum AnalyzerRights { READ = 0x01, WRITE = 0x02, CONFIG = 0x04 };
  protected :
    uint64_t        pLastSave;                //Timestamp of last save or communication
    string          pSerial;                  //Serial number
    string          pName;                    //Analyzer description
  protected :
    bool CheckRights(HTTPRequest &req, const uint neededAccess) const;
  protected :
    virtual void GetAnalyzer(HTTPRequest &req) = 0;
    virtual void SetAnalyzer(HTTPRequest &req) = 0;
    virtual void GetSeries(HTTPRequest &req) = 0;
    virtual void GetSeriesNow(HTTPRequest &req) = 0;
    virtual void GetSeriesRange(HTTPRequest &req) = 0;
    virtual void GetEvents(HTTPRequest &req) = 0;
    virtual void GetEvent(HTTPRequest &req) = 0;
    virtual void SetEvent(HTTPRequest &req) = 0;
    virtual void DelEvent(HTTPRequest &req) = 0;
    virtual void GetRates(HTTPRequest &req) = 0;
    virtual void GetRate(HTTPRequest &req) = 0;
    virtual void SetRate(HTTPRequest &req) = 0;
    virtual void DelRate(HTTPRequest &req) = 0;
    virtual void AddRate(HTTPRequest &req) = 0;
    virtual void GetPrices(HTTPRequest &req) = 0;
  public :
    Analyzer(const string &id);
    virtual ~Analyzer();
    string Serial() const;
    json   GetJSONShortDescriptor() const;
    void   GetAnalyzerCallback(HTTPRequest &req);
    void   SetAnalyzerCallback(HTTPRequest &req);
    void   GetSeriesCallback(HTTPRequest &req);
    void   GetSeriesNowCallback(HTTPRequest &req);
    void   GetSeriesRangeCallback(HTTPRequest &req);
    void   GetEventsCallback(HTTPRequest &req);
    void   GetEventCallback(HTTPRequest &req);
    void   SetEventCallback(HTTPRequest &req);
    void   DelEventCallback(HTTPRequest &req);
    void   GetRatesCallback(HTTPRequest &req);
    void   GetRateCallback(HTTPRequest &req);
    void   SetRateCallback(HTTPRequest &req);
    void   DelRateCallback(HTTPRequest &req);
    void   AddRateCallback(HTTPRequest &req);
    void   GetPricesCallback(HTTPRequest &req);
};