#include "stdafx.h"

#include "getShutdownPrivilege.h"
#include "globals.h"
#include "listenForData.h"
#include "log.h"
#include "serviceMain.h"
#include "serviceManager.h"

int _tmain(int iArgumentCount, TCHAR *asArguments[]) {
  // The code may be executed as a console application or as a service. The code will try to start service control dispatchers
  // If this succeeds, it is running as a service. If not, it is running as a console application.
  SERVICE_TABLE_ENTRY oServiceTable[] = {
    {gsServiceName, serviceMain},
    {NULL, NULL}
  };
  gbThisIsAServiceProcess = TRUE;
  if (StartServiceCtrlDispatcher(oServiceTable) != 0) {
    // The process was started as a service and starting control dispatchers succeeded.
    return 0;
  }
  DWORD dwErrorCode = GetLastError();
  if (dwErrorCode != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) {
    // The process was started as a service but starting control dispatchers failed.
    logError(_T("Service control dispatcher cannot be started: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  // The process was started as a console application.
  gbThisIsAServiceProcess = FALSE;
  BOOL bInstall = FALSE, bStart = FALSE, bStop = FALSE, bUninstall = FALSE, bRun = FALSE, bShowUsage = FALSE, bBadArgument = FALSE;
  for (int iArgumentIndex = 1; !bBadArgument && iArgumentIndex < iArgumentCount; iArgumentIndex++) {
    if (lstrcmpi(asArguments[iArgumentIndex], _T("--install")) == 0) {
      bInstall = TRUE;
    } else if (lstrcmpi(asArguments[iArgumentIndex], _T("--start")) == 0) {
      bStart = TRUE;
    } else if (lstrcmpi(asArguments[iArgumentIndex], _T("--stop")) == 0) {
      bStop = TRUE;
    } else if (lstrcmpi(asArguments[iArgumentIndex], _T("--uninstall")) == 0) {
      bUninstall = TRUE;
    } else if (lstrcmpi(asArguments[iArgumentIndex], _T("--run")) == 0) {
      bRun = TRUE;
    } else if (lstrcmp(asArguments[iArgumentIndex], _T("-?")) == 0 || lstrcmpi(asArguments[iArgumentIndex], _T("-h")) == 0 ||
               lstrcmp(asArguments[iArgumentIndex], _T("/?")) == 0 || lstrcmpi(asArguments[iArgumentIndex], _T("/h")) == 0 ||
               lstrcmp(asArguments[iArgumentIndex], _T("--help")) == 0) {
      bShowUsage = TRUE;
    } else {
      logError(_T("Unknown argument %s"), asArguments[iArgumentIndex]);
      bShowUsage = TRUE;
      bBadArgument = TRUE;
    }
  }
  BOOL bWorkWithService = (bInstall || bStart || bStop || bUninstall);
  bShowUsage |= (!bWorkWithService && !bRun);
  if (bShowUsage) {
    _tprintf(
      _T("Usage:\r\n")
      _T("  ShutdownOnLAN [options]\r\n")
      _T("Options:\r\n")
      _T("  --install    Install as a service.\r\n")
      _T("  --start      Start the service.\r\n")
      _T("  --stop       Stop the service.\r\n")
      _T("  --uninstall  Uninstall the service.\r\n")
      _T("  --run        Run as a console application.\r\n")
      _T("  -?           Show this message\r\n")
    );
    return bBadArgument ? -1 : 0;
  }
  if (bWorkWithService) {
    // We will need access to the service manager for any of these:
    dwErrorCode = openServiceManager();
    if (dwErrorCode != ERROR_SUCCESS) {
      return dwErrorCode;
    }
    if (bInstall) {
      // When installing the service, request to listen on UDP port 7 and 9 to trigger the Windows firewall. It
      // will ask the user if this application is allowed to do this. This needs to be done now, because if the
      // first time the application does this is when it is running as a service, the user will not be asked and
      // the firewall will deny the request. If the user says no to the request, the service will not work, but
      // there is no check for this at the moment.
      dwErrorCode = listenForDataToWakeUpFirewall();
      if (dwErrorCode != ERROR_SUCCESS) {
        return dwErrorCode;
      }
      dwErrorCode = installService(gsServiceName, gsServiceDescription);
      if (dwErrorCode == ERROR_SUCCESS) {
        logInformation(_T("The service was successfully installed."));
      } else if (dwErrorCode == ERROR_SERVICE_EXISTS) {
        dwErrorCode = ERROR_SUCCESS;
      } else {
        return dwErrorCode;
      }
    }
    if (bStart) {
      dwErrorCode = startService(gsServiceName);
      if (dwErrorCode == ERROR_SUCCESS) {
        logInformation(_T("Starting service..."));
        for (int iWaitTime = 10000; iWaitTime > 0 && isServiceStarting(); iWaitTime -= 100) {
          Sleep(100);
        }
        if (!isServiceStarted()) {
          DWORD dwServiceErrorCode;
          dwErrorCode = getServiceErrorCode(dwServiceErrorCode);
          if (dwErrorCode) {
            return dwErrorCode;
          }
          logError(_T("The service failed to start: error code %d (0x%X)."), dwServiceErrorCode, dwServiceErrorCode);
          return dwServiceErrorCode;
        }
        logInformation(_T("The service was successfully started."));
      } else if (dwErrorCode == ERROR_SERVICE_ALREADY_RUNNING) {
        dwErrorCode = ERROR_SUCCESS;
      } else {
        return dwErrorCode;
      }
    }
    if (bStop) {
      dwErrorCode = stopService(gsServiceName);
      if (dwErrorCode == ERROR_SUCCESS) {
        logInformation(_T("Stopping service..."));
        for (int iWaitTime = 10000; iWaitTime > 0 && isServiceStopping(); iWaitTime -= 100) {
          Sleep(100);
        }
        if (!isServiceStopped()) {
          logError(_T("The service failed to stop."));
          return -1;
        }
        DWORD dwServiceErrorCode;
        dwErrorCode = getServiceErrorCode(dwServiceErrorCode);
        if (dwErrorCode) {
          return dwErrorCode;
        }
        if (dwServiceErrorCode == ERROR_SUCCESS) {
          logError(_T("The service was successfully stopped."));
        } else {
          logError(_T("The service stopped with error code %d (0x%X)."), dwServiceErrorCode, dwServiceErrorCode);
        }
      } else if (dwErrorCode == ERROR_SERVICE_NOT_ACTIVE) {
        dwErrorCode = ERROR_SUCCESS;
      } else {
        return dwErrorCode;
      }
    }
    if (bUninstall) {
      dwErrorCode = uninstallService(gsServiceName);
      if (dwErrorCode == ERROR_SUCCESS) {
        logInformation(_T("The service was successfully uninstalled."));
      } else if (dwErrorCode == ERROR_SERVICE_DOES_NOT_EXIST) {
        dwErrorCode = ERROR_SUCCESS;
      } else {
        return dwErrorCode;
      }
    }
  }
  if (bRun) {
    dwErrorCode = getShutdownPrivilege();
    if (dwErrorCode == ERROR_SUCCESS) {
      dwErrorCode = listenAndWaitForData();
    }
  }
  return dwErrorCode;
}

