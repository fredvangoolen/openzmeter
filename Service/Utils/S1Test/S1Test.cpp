// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
// Unit tests of the S1 frame decoder and stream parser. No hardware, database or openZmeter build needed: make test
// ---------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <random>
#include "DeviceS1Frame.h"
#include "DeviceS1Rate.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
typedef vector<uint8_t> Bytes;
static int Checks = 0, Failures = 0;
#define CHECK(c) do { Checks++; if(!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); Failures++; } } while(0)
#define NEAR(a, b, tol) CHECK(fabs((a) - (b)) < (tol))
// Example telegram of the specification (e-MUCS H ed. 1.3, section 4.2.5)
static const uint8_t SPEC[45] = {
  0x7E, 0x80, 0x2B, 0xFF, 0x03, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0A, 0x34, 0xC3, 0xBB, 0x0F, 0x0B, 0x15, 0x00, 0x05, 0xFA,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xC5, 0xB2, 0x7E };
static Bytes Seal(Bytes f) {
  uint16_t CRC = DeviceS1Frame::CRC16(&f[1], 41);
  f[42] = CRC & 0xFF;
  f[43] = CRC >> 8;
  return f;
}
static Bytes Frame(const uint8_t seq) {
  Bytes F(SPEC, SPEC + 45);
  F[23] = seq;
  return Seal(F);
}
static void Append(Bytes &to, const Bytes &from) { to.insert(to.end(), from.begin(), from.end()); }
// Feeds data in pieces of 'chunk' bytes and returns the sequence numbers of the decoded frames
static vector<int> Parse(DeviceS1Parser &parser, const Bytes &data, const size_t chunk) {
  vector<int> Seqs;
  for(size_t n = 0; n < data.size(); n += chunk) {
    size_t Len = (data.size() - n < chunk) ? data.size() - n : chunk;
    parser.Push(&data[n], Len, [&](const DeviceS1Frame::Frame_t &f) { Seqs.push_back(f.Sequence); });
  }
  return Seqs;
}
static vector<int> Range(const int count) {
  vector<int> R;
  for(int n = 0; n < count; n++) R.push_back(n & 0xFF);
  return R;
}
// ---------------------------------------------------------------------------------------------------------------------
static void TestSpecExample() {
  DeviceS1Frame::Frame_t F;
  CHECK(DeviceS1Frame::CRC16(&SPEC[1], 41) == 0xB2C5);
  CHECK(DeviceS1Frame::Decode(SPEC, F));
  CHECK(!F.PolyPhase && F.PerPeriod && !F.FourWires && F.Valid && !F.Neutral);
  CHECK(F.Version == 0);
  CHECK(F.Sampling == 52);
  CHECK(F.Sequence == 0x0F);
  CHECK(F.MeterID[0] == 0);
  NEAR(F.Frequency, 50.107, 1e-4);
  NEAR(F.Voltage[0], 70.925, 1e-3);
  NEAR(F.Current[0], 1.530, 1e-5);
}
static void TestPolyPhase() {
  Bytes G(SPEC, SPEC + 45);
  memcpy(&G[5], "1ABC0000000001", 14);
  G[19] = 0x1F;                                        //Poly phase, per period, 4 wires, valid, neutral
  G[24] = 0xF4; G[25] = 0x48;                          //-3000 * 25 mV
  G[26] = 0xFF; G[27] = 0xFF; G[28] = 0xFF;            //-1 mA
  G[29] = 0x7F; G[30] = 0xFF;                          //Max positive voltage
  G[39] = 0x80; G[40] = 0x00; G[41] = 0x00;            //Max negative current (neutral)
  G = Seal(G);
  DeviceS1Frame::Frame_t F;
  CHECK(DeviceS1Frame::Decode(&G[0], F));
  CHECK(strcmp(F.MeterID, "1ABC0000000001") == 0);
  CHECK(F.PolyPhase && F.PerPeriod && F.FourWires && F.Valid && F.Neutral);
  NEAR(F.Voltage[0], -75.0, 1e-3);
  NEAR(F.Current[0], -0.001, 1e-6);
  NEAR(F.Voltage[1], 819.175, 1e-2);
  NEAR(F.Current[3], -8388.608, 1e-2);
}
static void TestRejected() {
  DeviceS1Frame::Frame_t F;
  for(int n = 0; n < 45 * 8; n++) {                    //Any single bit error must be detected
    Bytes G(SPEC, SPEC + 45);
    G[n / 8] ^= 1 << (n % 8);
    CHECK(!DeviceS1Frame::Decode(&G[0], F));
  }
}
static void TestParserStreams() {
  Bytes Separate, Shared;                              //Each frame with both flags / closing flag is the next opening flag
  for(int n = 0; n < 300; n++) {
    Bytes F = Frame(n);
    Append(Separate, F);
    if(n > 0) F.erase(F.begin());
    Append(Shared, F);
  }
  for(size_t Chunk : { 1, 7, 44, 45, 46, 512, 4096, 100000 }) {
    DeviceS1Parser A, B;
    CHECK(Parse(A, Separate, Chunk) == Range(300));
    CHECK(Parse(B, Shared, Chunk) == Range(300));
    CHECK((A.CRCErrors() == 0) && (A.Discarded() == 0) && (A.Frames() == 300));
    CHECK((B.CRCErrors() == 0) && (B.Discarded() == 0) && (B.Frames() == 300));
  }
}
static void TestParserErrors() {
  Bytes Data(100, 0x55);                               //Garbage before the first frame
  Append(Data, Frame(0));
  Bytes Bad = Frame(1);
  Bad[30] ^= 0x10;
  Append(Data, Bad);                                   //Corrupted frame
  Append(Data, Frame(2));
  Bytes Short = Frame(3);
  Short.erase(Short.begin() + 20);
  Append(Data, Short);                                 //Frame with a missing byte
  Append(Data, Frame(4));
  Append(Data, Frame(5));
  DeviceS1Parser P;
  CHECK(Parse(P, Data, 64) == (vector<int>{ 0, 2, 4, 5 }));
  CHECK(P.CRCErrors() >= 2);
  CHECK(P.Discarded() >= 100);
  // Flags and a fake header inside the data (transparency is disabled) must not break synchronization
  Bytes Tricky(SPEC, SPEC + 45);
  const uint8_t Fake[] = { 0x7E, 0x80, 0x2B, 0xFF, 0x03, 0x7E, 0x7E };
  memcpy(&Tricky[24], Fake, sizeof(Fake));
  Tricky[23] = 9;
  Tricky = Seal(Tricky);
  Bytes Data2;
  for(int n = 0; n < 5; n++) Append(Data2, Tricky);
  DeviceS1Parser Q;
  CHECK(Parse(Q, Data2, 33) == vector<int>(5, 9));
  // Reset drops a partial frame
  DeviceS1Parser R;
  Bytes F = Frame(7);
  CHECK(Parse(R, Bytes(F.begin(), F.begin() + 20), 20).empty());
  R.Reset();
  CHECK(Parse(R, F, 45) == vector<int>(1, 7));
}
// Feeds 'seconds' of a stream of 'rate' samples/s, arriving in 3 ms pieces with up to 'jitter' seconds delay
static double Feed(DeviceS1Rate &r, double start, double rate, double seconds, double jitter) {
  static mt19937 Gen(1);
  uniform_real_distribution<double> Delay(0.0, 1.0);
  uint64_t Sent = 0;
  double   Last = 0.0;
  for(double T = 0.003; T < seconds; T += 0.003) {
    uint64_t Total = T * rate;
    double   Time  = start + T + Delay(Gen) * jitter;
    if(Time < Last) Time = Last;
    Last = Time;
    r.Push(Time, Total - Sent);
    Sent = Total;
  }
  return start + seconds;
}
static void TestRate() {
  DeviceS1Rate R;
  R.Config(3900.0, 100.0, 600);
  CHECK(!R.Valid() && (R.Rate() == 3900.0));           //Announced rate until measured
  double T = Feed(R, 1000.0, 3906.25, 10.0, 0.005);
  CHECK(!R.Valid());
  T = Feed(R, T, 3906.25, 10.0, 0.005);
  CHECK(R.Valid());
  NEAR(R.Rate(), 3906.25, 2.0);                        //25 mHz at 50 Hz after 20 s
  T = Feed(R, T, 3906.25, 60.0, 0.005);
  NEAR(R.Rate(), 3906.25, 0.3);                        //4 mHz
  T = Feed(R, T, 3906.25, 900.0, 0.005);
  NEAR(R.Rate(), 3906.25, 0.02);                       //0.3 mHz
  // A pause (port closed, samples lost) must not affect the result
  double Before = R.Rate();
  T = Feed(R, T + 30.0, 3906.25, 5.0, 0.005);
  CHECK(R.Rate() == Before);
  T = Feed(R, T, 3906.25, 60.0, 0.005);
  NEAR(R.Rate(), 3906.25, 0.3);
  // Buffered data arriving at once after a pause
  T += 1.0;
  R.Push(T, 1500);
  T = Feed(R, T, 3906.25, 60.0, 0.005);
  NEAR(R.Rate(), 3906.25, 0.3);
  // Results that do not fit the announced rate are rejected
  DeviceS1Rate S;
  S.Config(3900.0, 100.0, 600);
  Feed(S, 0.0, 2000.0, 60.0, 0.0);
  CHECK(!S.Valid() && (S.Rate() == 3900.0));
  // Per period sampling: short window follows the grid frequency
  DeviceS1Rate P;
  P.Config(2600.0, 390.0, DeviceS1Rate::MIN_POINTS);
  T = Feed(P, 0.0, 52 * 50.05, 120.0, 0.001);
  NEAR(P.Rate(), 52 * 50.05, 0.5);
  Feed(P, T, 52 * 49.95, 30.0, 0.001);
  NEAR(P.Rate(), 52 * 49.95, 0.5);
}
int main() {
  TestRate();
  TestSpecExample();
  TestPolyPhase();
  TestRejected();
  TestParserStreams();
  TestParserErrors();
  printf("%d checks, %d failures\n", Checks, Failures);
  return (Failures == 0) ? 0 : 1;
}
// ---------------------------------------------------------------------------------------------------------------------
