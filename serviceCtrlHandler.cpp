#include "stdafx.h"

#include "globals.h"
#include "log.h"
#include "serviceManager.h"

VOID WINAPI serviceCtrlHandler(DWORD dwCtrlCode) {
  DWORD dwErrorCode;
  switch (dwCtrlCode) {
    case SERVICE_CONTROL_STOP: {
      if (!isServiceStarted()) {
        logError(_T("Cannot stop service: it is not started."));
      } else {
        setServiceStatusStopping(0);
        if (!SetEvent(ghServiceStopEvent)) {
          dwErrorCode = GetLastError();
          logError(_T("Cannot set service stop event: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
        }
      }
      break;
    }
    default: {
      logInformation(_T("Unhandled service control code %d (0x%X)."), dwCtrlCode, dwCtrlCode);
      break;
    }
  }
}
