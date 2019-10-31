#pragma once
#include <curl/curl.h>
//----------------------------------------------------------------------------------------------------------------------
class HTTPClient final {
  private :
    typedef struct {
      char  *Buffer;
      size_t Size;
      size_t MaxSize;
    } CurlWriteBuff_t;
  private :
    CURLSH         *pCurl;                    //Shared object for curl lib
    curl_slist     *pCurlHeaders;             //Common headers
    pthread_mutex_t pCurlLocks;               //Curl access control
  private :
        static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static void   CurlLockCallback(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr);
    static void   CurlUnlockCallback(CURL *handle, curl_lock_data data, void *userptr);

  public:
    HTTPClient();
    virtual ~HTTPClient();
private:

};