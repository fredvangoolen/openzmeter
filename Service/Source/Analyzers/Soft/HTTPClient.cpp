#include "HTTPClient.h"
#include "pthread.h"
HTTPClient::HTTPClient() {
  pCurlLocks   = PTHREAD_MUTEX_INITIALIZER;
  pCurl = curl_share_init();
  curl_share_setopt(pCurl, CURLSHOPT_LOCKFUNC, CurlLockCallback);
  curl_share_setopt(pCurl, CURLSHOPT_UNLOCKFUNC, CurlUnlockCallback);
  curl_share_setopt(pCurl, CURLSHOPT_USERDATA, &pCurlLocks);
  curl_share_setopt(pCurl, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Accept: application/json");
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Connection: keep-alive");
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Content-Type: application/json");
}
HTTPClient::~HTTPClient() {
  //Wait for queries to terminate
  if(pCurl != NULL) {
    curl_share_cleanup(pCurl);
    pCurl = NULL;
  }
  if(pCurlHeaders != NULL) {
    curl_slist_free_all(pCurlHeaders);
    pCurlHeaders = NULL;
  }
}
void HTTPClient::CurlLockCallback(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr) {
  pthread_mutex_lock((pthread_mutex_t*)userptr); 
}
void HTTPClient::CurlUnlockCallback(CURL *handle, curl_lock_data data, void *userptr) {
  pthread_mutex_unlock((pthread_mutex_t*)userptr); 
}
size_t HTTPClient::CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  CurlWriteBuff_t *Buffer = (CurlWriteBuff_t*)userp;
  if((Buffer->Size + size * nmemb + 1) > Buffer->MaxSize) {
    Buffer->Buffer = (char*)realloc(Buffer->Buffer, Buffer->Size + size * nmemb + 1);
    if(Buffer->Buffer == NULL) {
      return 0;
    } else {
      Buffer->MaxSize = Buffer->Size + size * nmemb + 1;
    }
  }
  memcpy(&Buffer->Buffer[Buffer->Size], contents, size * nmemb);
  Buffer->Size += size * nmemb;
  Buffer->Buffer[Buffer->Size] = 0;
  return size * nmemb;
}
bool HTTPClient::SendPost() {
    CurlWriteBuff_t CurlBuff;
      long            HTTPCode;

  CURL           *Curl        = curl_easy_init();
  curl_easy_setopt(Curl, CURLOPT_SHARE, This->pCurl);
  curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, This->pCurlHeaders);
  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(Curl, CURLOPT_POST, 1L);
  curl_easy_setopt(Curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&CurlBuff);
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);

  
}