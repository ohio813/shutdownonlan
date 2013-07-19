#include "stdafx.h"

#include "globals.h"
#include "log.h"

SC_HANDLE ghSCManager = NULL;

DWORD openServiceManager() {
  ghSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (ghSCManager == NULL) {
    DWORD dwErrorCode = GetLastError();
    if (dwErrorCode == ERROR_ACCESS_DENIED) {
      logError(_T("You have insufficient access rights to open the service manager."));
      logError(_T("Please try again with administrative privileges."));
    } else {
      logError(_T("Unable to open service manager: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    }
    return dwErrorCode;
  }
  return ERROR_SUCCESS;
}

DWORD closeServiceManager() {
  if (CloseServiceHandle(ghSCManager) == 0) {
    DWORD dwErrorCode = GetLastError();
    logError(_T("Unable to close service manager: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  return ERROR_SUCCESS;
}

DWORD openService(LPTSTR sServiceName, DWORD dwDesiredAccess, SC_HANDLE& hService) {
  hService = OpenService(ghSCManager, sServiceName, dwDesiredAccess);
  if (hService == NULL) {
    DWORD dwErrorCode = GetLastError();
    if (dwErrorCode == ERROR_SERVICE_DOES_NOT_EXIST) {
      logError(_T("The service \"%s\" is not installed."), sServiceName);
    } else {
      logError(_T("Unable to open service \"%s\": error code %d (0x%X)."), sServiceName, dwErrorCode, dwErrorCode);
    }
    return dwErrorCode;
  }
  return ERROR_SUCCESS;
}

DWORD closeService(LPTSTR sServiceName, SC_HANDLE& hService) {
  if (!CloseServiceHandle(hService)) {
    DWORD dwErrorCode = GetLastError();
    logError(_T("Unable to close service \"%s\": error code %d (0x%X)."), sServiceName, dwErrorCode, dwErrorCode);
  }
  hService = NULL;
  return ERROR_SUCCESS;
}

DWORD installService(LPTSTR sServiceName, LPTSTR sServiceDescription) {
  TCHAR sProcessEXEPath[MAX_PATH];
  if (!GetModuleFileName(NULL, sProcessEXEPath, MAX_PATH)){
    DWORD dwErrorCode = GetLastError();
    logError(_T("Unable to determine executable file name for this process: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  SC_HANDLE hService = CreateService(ghSCManager, sServiceName, sServiceDescription, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
      SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, sProcessEXEPath, NULL, NULL, NULL, NULL, NULL);
  if (hService == NULL) {
    DWORD dwErrorCode = GetLastError();
    if (dwErrorCode == ERROR_SERVICE_EXISTS) {
      logError(_T("The service \"%s\" is already installed."), sServiceName);
    } else {
      logError(_T("Unable to create service \"%s\": error code %d (0x%X)."), sServiceName, dwErrorCode, dwErrorCode);
    }
    return dwErrorCode;
  }
  closeService(sServiceName, hService);
  return ERROR_SUCCESS;
}

DWORD startService(LPTSTR sServiceName) {
  SC_HANDLE hService;
  DWORD dwErrorCode = openService(sServiceName, SERVICE_START, hService);
  if (hService == NULL) {
    return dwErrorCode;
  }
  if (!StartService(hService, 0 , NULL)) {
    dwErrorCode = GetLastError();
    if (dwErrorCode == ERROR_SERVICE_ALREADY_RUNNING) {
      logError(_T("The service \"%s\" is already running."), sServiceName);
    } else {
      logError(_T("Unable to start service \"%s\": error code %d (0x%X)."), sServiceName, dwErrorCode, dwErrorCode);
    }
    closeService(sServiceName, hService);
    return dwErrorCode;
  }
  dwErrorCode = closeService(sServiceName, hService);
  return dwErrorCode;
}

DWORD stopService(LPTSTR sServiceName) {
  SC_HANDLE hService;
  DWORD dwErrorCode = openService(sServiceName, SERVICE_STOP, hService);
  if (hService == NULL) {
    return dwErrorCode;
  }
  SERVICE_STATUS oServiceStatus;
  if (!ControlService(hService, SERVICE_CONTROL_STOP, &oServiceStatus)) {
    dwErrorCode = GetLastError();
    if (dwErrorCode == ERROR_SERVICE_NOT_ACTIVE) {
      logError(_T("The service \"%s\" is not running."), sServiceName);
    } else {
      logError(_T("Unable to stop service \"%s\": error code %d (0x%X)."), sServiceName, dwErrorCode, dwErrorCode);
    }
    closeService(sServiceName, hService);
    return dwErrorCode;
  }
  dwErrorCode = closeService(sServiceName, hService);
  return dwErrorCode;
}
DWORD uninstallService(LPTSTR sServiceName) {
  SC_HANDLE hService;
  DWORD dwErrorCode = openService(sServiceName, DELETE, hService);
  if (hService == NULL) {
    return dwErrorCode;
  }
  if (!DeleteService(hService)) {
    dwErrorCode = GetLastError();
    logError(_T("Unable to uninstall service \"%s\": error code %d (0x%X)."), sServiceName, dwErrorCode, dwErrorCode);
  }
  dwErrorCode = closeService(sServiceName, hService);
  return dwErrorCode;
}

DWORD getServiceStatus(SERVICE_STATUS& oServiceStatus) {
  SC_HANDLE hService;
  DWORD dwErrorCode = openService(gsServiceName, SERVICE_QUERY_STATUS, hService);
  if (dwErrorCode != ERROR_SUCCESS) {
    return dwErrorCode;
  }
  if (!QueryServiceStatus(hService, &oServiceStatus)) {
    dwErrorCode = GetLastError();
    logError(_T("QueryServiceStatus failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  return ERROR_SUCCESS;
}

DWORD getServiceState(DWORD& dwState) {
  if (gbThisIsAServiceProcess) {
    dwState = gdwServiceState;
  } else {
    SERVICE_STATUS oServiceStatus = {0};
    DWORD dwErrorCode = getServiceStatus(oServiceStatus);
    if (dwErrorCode != ERROR_SUCCESS) {
      return dwErrorCode;
    }
    dwState = oServiceStatus.dwCurrentState;
  }
  return ERROR_SUCCESS;
}
BOOL isServiceStarting() {
  DWORD dwState, dwErrorCode = getServiceState(dwState);
  return dwErrorCode == ERROR_SUCCESS && dwState == SERVICE_START_PENDING;
}
BOOL isServiceStarted() {
  DWORD dwState, dwErrorCode = getServiceState(dwState);
  return dwErrorCode == ERROR_SUCCESS && dwState == SERVICE_RUNNING;
}
BOOL isServiceStopping() {
  DWORD dwState, dwErrorCode = getServiceState(dwState);
  return dwErrorCode == ERROR_SUCCESS && dwState == SERVICE_STOP_PENDING;
}
BOOL isServiceStopped() {
  DWORD dwState, dwErrorCode = getServiceState(dwState);
  return dwErrorCode == ERROR_SUCCESS && dwState == SERVICE_STOPPED;
}

DWORD gdwCheckPoint = 0;

DWORD setServiceStatus(DWORD dwCurrentState, DWORD dwControlsAccepted, DWORD dwServiceErrorCode) {
  gdwServiceState = dwCurrentState;
  gdwServiceErrorCode = dwServiceErrorCode;
  SERVICE_STATUS oServiceStatus = {0};
  oServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  oServiceStatus.dwCurrentState = dwCurrentState;
  oServiceStatus.dwControlsAccepted = dwControlsAccepted;
  oServiceStatus.dwWin32ExitCode = dwServiceErrorCode;
  oServiceStatus.dwServiceSpecificExitCode = 0;
  oServiceStatus.dwCheckPoint = gdwCheckPoint++;
  oServiceStatus.dwWaitHint = 1000;

  if (!SetServiceStatus(ghServiceStatusHandle, &oServiceStatus)) {
    DWORD dwErrorCode = GetLastError();
    logError(_T("SetServiceStatus failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  return ERROR_SUCCESS;
}

DWORD setServiceStatusStarting() {
  logInformation(_T("Service status = starting"));
  return setServiceStatus(SERVICE_START_PENDING, 0, ERROR_SUCCESS);
}
DWORD setServiceStatusStarted() {
  logInformation(_T("Service status = started"));
  return setServiceStatus(SERVICE_RUNNING, SERVICE_ACCEPT_STOP, ERROR_SUCCESS);
}
DWORD setServiceStatusStopping(DWORD dwErrorCode) {
  logInformation(_T("Service status = stopping"));
  return setServiceStatus(SERVICE_STOP_PENDING, 0, dwErrorCode);
}
DWORD setServiceStatusStopped(DWORD dwErrorCode) {
  logInformation(_T("Service status = stopped"));
  return setServiceStatus(SERVICE_STOPPED, 0, dwErrorCode);
}

DWORD getServiceErrorCode(DWORD& dwServiceErrorCode) {
  if (gbThisIsAServiceProcess) {
    dwServiceErrorCode = gdwServiceErrorCode;
  } else {
    SERVICE_STATUS oServiceStatus = {0};
    DWORD dwErrorCode = getServiceStatus(oServiceStatus);
    if (dwErrorCode != ERROR_SUCCESS) {
      return dwErrorCode;
    }
    dwServiceErrorCode = oServiceStatus.dwWin32ExitCode;
  }
  return ERROR_SUCCESS;
}
