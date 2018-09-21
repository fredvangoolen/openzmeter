/* ================================== zMeter ===========================================================================
 * File:     RingBuffer.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <stdint.h>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
template <class T>
class RingBuffer {
  private:
    vector<T> pBuffer;
    uint32_t  pReadPos;
    uint32_t  pWritePos;
    uint32_t  pSize;
    
//    const size_t max_size_;
//    bool full_ = 0;
  public:
    RingBuffer() {
      Reset(10);
    }
    RingBuffer(uint32_t size) {
      Reset(size);
    }
    void Reset() {
      pReadPos = 0;
      pWritePos = 0;
      pSize = 0;
    }
    void Reset(uint32_t capacity) {
      pBuffer.reserve(capacity);
      Reset();
    }
    bool Empty() const {
      return (pSize == 0);
    }
    bool Full() const {
      return (pSize == pBuffer.capacity());
    }
    uint32_t Capacity() const {
      return pBuffer.capacity();
    }
    uint32_t Size() const {
      return pSize;
    }
    void Push(T item) {
      if(Full()) return;
      pBuffer[pWritePos] = item;
      pWritePos = (pWritePos + 1) % pBuffer.capacity();
      pSize++;
    }
    T Pop() {
      if(Empty()) return T();
      T Val = pBuffer[pReadPos];
      pReadPos = (pReadPos + 1) % pBuffer.capacity();
      pSize--;
      return Val;
    }
};

