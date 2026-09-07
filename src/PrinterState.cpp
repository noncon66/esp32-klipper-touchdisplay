#include "PrinterState.h"

void PrinterState::invalidateLiveData() {
  stale = true;
  extruder.actualValid = false;
  extruder.targetValid = false;
  bed.actualValid = false;
  bed.targetValid = false;
  filenameValid = false;
  progressValid = false;
}

const char *linkStateText(LinkState state) {
  switch (state) {
    case LinkState::NotConfigured: return "nicht konfiguriert";
    case LinkState::Disconnected: return "getrennt";
    case LinkState::Connecting: return "verbindet";
    case LinkState::Connected: return "verbunden";
  }
  return "unbekannt";
}

const char *klipperStateText(KlipperState state) {
  switch (state) {
    case KlipperState::Unknown: return "unbekannt";
    case KlipperState::Startup: return "startet";
    case KlipperState::Ready: return "bereit";
    case KlipperState::Shutdown: return "shutdown";
    case KlipperState::Error: return "fehler";
  }
  return "unbekannt";
}

const char *printStateText(PrintState state) {
  switch (state) {
    case PrintState::Unknown: return "unbekannt";
    case PrintState::Standby: return "bereit";
    case PrintState::Printing: return "druckt";
    case PrintState::Paused: return "pausiert";
    case PrintState::Complete: return "fertig";
    case PrintState::Cancelled: return "abgebrochen";
    case PrintState::Error: return "fehler";
  }
  return "unbekannt";
}
