
  enum SyncState   { NOT_LOGGED, LOGGED, CONNECTED, TERMINATE };

  private : //Readed in thread, changed in configure
    pthread_t                  pSyncThread;              //Export thread
    bool                       pSyncTerminate;           //Sync terminate signal
    string                     pSyncServer;              //Export server address
    string                     pSyncUser;                //Export server user
    string                     pSyncPass;                //Export server password


        typedef struct {
      pthread_mutex_t         *Mutex;                    //Export list locks
      pthread_cond_t          *Cond;                     //Wakeup parent after last thread ends
      uint32_t                *Count;                    //Count of running sync threads
      string                   Server;                   //
      string                   Cookie;                   //
      string                   Method;                   //
      json                     Params;                   //
      uint64_t                 Sequence;                 //
      AnalyzerSoft            *Analyzer;                 //
    } Sync_t;
    static void *Sync_Thread(void *params);
    static void *Sync_RespThread(void *params);
