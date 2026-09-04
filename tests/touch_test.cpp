#include <cassert>
#include <iostream>
#include "../include/SafeGT911.h"
uint32_t fakeNow = 0;
FakeSerial Serial;
FakeWire Wire;
void fixture(uint8_t address = 0x5D) {
  Wire=FakeWire{}; Wire.device=address; fakeNow=10;
  Wire.memory[0x8140]={'9','1','1',0};
}
void report(uint8_t count, uint16_t x=0, uint16_t y=0) {
  Wire.memory[0x814E]={uint8_t(0x80|count)};
  Wire.memory[0x814F]={0,uint8_t(x),uint8_t(x>>8),uint8_t(y),uint8_t(y>>8),1,0,0};
}
int main() {
  fixture(); SafeGT911 t; assert(t.begin() && t.address==0x5D);
  report(1,100,200); t.poll(); assert(t.pressed && t.x==100 && t.y==200 && Wire.acks==1);
  fakeNow+=50; t.poll(); assert(t.pressed); // no new report, keep contact briefly
  fakeNow+=151; t.poll(); assert(!t.pressed);
  report(1,479,479); t.poll(); assert(t.pressed && t.x==479 && t.y==479);
  report(0); t.poll(); assert(!t.pressed);
  report(1,480,2); t.poll(); assert(!t.pressed && t.errors==1);
  report(6); t.poll(); assert(!t.online && !t.pressed && t.errors==2);
  assert(t.probe()); report(1,20,30); Wire.shortRead=true; t.poll();
  assert(!t.online && !t.pressed && t.errors==3);
  Wire.shortRead=false; assert(t.probe()); Wire.busError=true; t.poll();
  assert(!t.online && t.errors==4);
  fixture(0x14); SafeGT911 other; assert(other.begin() && other.address==0x14);
  Wire.memory[0x8140]={'1','2','3',0}; assert(!other.probe());
  std::cout << "PASS: detection, alternate address, product ID, coordinates, edges, release, stale state, count limit, short reads, bus errors, reconnect\n";
}
