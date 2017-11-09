#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <sndfile.h>

int Terminate = 0;
uint8_t Buffer[128];
int     BufferLen = 0;
SNDFILE* sound_file;
typedef struct __attribute__((packed)) {
  uint16_t Voltage;
  uint16_t Current;
} Sample_t;

void SignalHandler(int signal) {
  if((signal == SIGINT) || (signal == SIGTERM)) Terminate = 1;
}
int main(int argc, char* argv[]) {
  struct pollfd FD;
  FD.events = POLLIN;
  FD.fd = open("/dev/ttyACM0", O_RDWR | O_NONBLOCK | O_CLOEXEC);
  if(FD.fd == -1) {
    perror("open failed");
    exit(-1);
  }
  SF_INFO info;
  info.samplerate = 10000;
  info.channels = 2;
  info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
  if(!sf_format_check(&info)){
    printf("Invalid soundfile format\n");
    exit(1);
  }
  if(! (sound_file = sf_open("./out.wav", SFM_WRITE, &info)) ){
    printf("Could not open soundfile\n");
    exit(1);
  }

  signal(SIGINT, SignalHandler);
  signal(SIGTERM, SignalHandler);
  struct sched_param Prio;
  Prio.sched_priority = 40;
//  if(sched_setscheduler(0, SCHED_FIFO, &Prio) == -1) {
//    perror("sched_setscheduler failed");
//    exit(-1);
//  }
//  if(mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
//    perror("mlockall failed");
//    exit(-2);
//  }
  struct timespec Time;
  clock_gettime(CLOCK_REALTIME, &Time);
  Time.tv_sec++;
  int Sync = 0;
  int Print = 0;
  while(!Terminate) {
    FD.revents = 0;
    int Result = poll(&FD, 1, 100);
    if(Result == -1) {
      printf("ERROR en %s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      continue;
    } else {
      int Count = read(FD.fd, &Buffer[BufferLen], sizeof(Buffer) - BufferLen);
      if((Count == -1) && (errno != EAGAIN)) {
        printf("ERROR en %s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      } else if(Count > 0) {
        BufferLen = BufferLen + Count;
        uint8_t *Ptr = Buffer;
        while(BufferLen >= 4) {
          if((Ptr[0] == 0xFF) && (Ptr[1] == 0xFF) && (Ptr[2] == 0xFF) && (Ptr[3] == 0xFF)) {
            Sync = 1;
            Ptr += 4;
            BufferLen -= 4;
          } else if(!Sync) {
            Ptr++;
            BufferLen--;
          } else if(Sync) {
            Sample_t *Val = (Sample_t*)Ptr;
            float Samples[2];
            Samples[0] = ((Val->Current & 0x0FFF) - 2048.0) * 12.5;
            Samples[1] = ((Val->Voltage & 0x0FFF) - 877.54) * (311.0 / 620.60);
            if((++Print % 1000) == 0) printf("%6.1f V - %6.1f mA\n", Samples[1], Samples[0]);            
            Samples[0] = Samples[0] / 5000.0;
            Samples[1] = Samples[1] / 500.0;
            if(sf_write_float(sound_file, Samples, 2) != 2) {
              perror("sf_write_float");
              exit(-2);
            }
            BufferLen -= 4;
            Ptr += 4;
          }
        }
        memmove(Buffer, Ptr, BufferLen);
      }  
    }
  }
  sf_close(sound_file);
}

