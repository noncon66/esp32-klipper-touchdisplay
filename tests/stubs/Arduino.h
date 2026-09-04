#pragma once
#include <cstdint>
#include <cstddef>
extern uint32_t fakeNow;
inline uint32_t millis() { return fakeNow; }
struct FakeSerial { template<class... Args> void printf(const char *, Args...) {} };
extern FakeSerial Serial;
