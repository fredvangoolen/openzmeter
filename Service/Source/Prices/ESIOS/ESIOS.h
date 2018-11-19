/* ================================== zMeter ===========================================================================
 * File:     ESIOS.cpp
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include <json-c/json.h>
#include "PricesInterface.h"
class ESIOS : public PricesInterface {
  private :
    typedef struct {
      char  *Buffer;
      size_t Size;
      size_t MaxSize;
    } CurlBuff_t;
  private :
    static bool    pClassRegistered;
  private :
    const char *pRate;
    CURL       *pCurl;
    CurlBuff_t  pCurlBuff;
  private :
    static size_t Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp);
    static PricesInterface *CreateInstance(const char *name, json_object *config);
    static void XMLError(void *ctx, const char *msg,...);
  private :
    void CheckPrices(uint64_t from);
  private :
    ESIOS(const char *id);
  public :
    ~ESIOS();
};
