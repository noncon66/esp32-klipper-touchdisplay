#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>
class FakeWire {
 public:
  uint8_t device = 0x5D, address = 0;
  uint16_t reg = 0;
  bool busError = false, shortRead = false;
  size_t cursor = 0;
  unsigned acks = 0;
  std::vector<uint8_t> tx, rx;
  std::map<uint16_t,std::vector<uint8_t>> memory;
  void begin(int,int) {}
  void setClock(int) {}
  void setTimeOut(int) {}
  void beginTransmission(uint8_t a) { address=a; tx.clear(); }
  void write(uint8_t b) { tx.push_back(b); }
  int endTransmission() {
    if (busError || address != device) return 2;
    if (tx.size() >= 2) reg = (uint16_t(tx[0])<<8)|tx[1];
    if (tx.size() == 3 && reg == 0x814E) { ++acks; memory[reg]={tx[2]}; }
    return 0;
  }
  size_t requestFrom(uint8_t,size_t n) {
    rx=memory[reg]; if (rx.size()>n) rx.resize(n);
    if (shortRead && !rx.empty()) rx.pop_back();
    cursor=0; return rx.size();
  }
  bool available() { return cursor < rx.size(); }
  int read() { return available() ? rx[cursor++] : -1; }
};
extern FakeWire Wire;
