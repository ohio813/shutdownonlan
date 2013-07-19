#include "stdafx.h"

#include "getShutDownPrivilege.h"
#include "globals.h"
#include "log.h"
#include "serviceCtrlHandler.h"
#include "serviceManager.h"
#include "serviceWorkerThread.h"

VOID WINAPI serviceMain(DWORD dwArgumentCount, LPTSTR *asArguments) {
  DWORD dwErrorCode;
  logInformation(_T("Start serviceMain."));

  ghServiceStatusHandle = RegisterServiceCtrlHandler(gsServiceName, serviceCtrlHandler);

  if (ghServiceStatusHandle == NULL) {
    dwErrorCode = GetLastError();
    logError(_T("Cannot register service control handler: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return;
  }
  dwErrorCode = setServiceStatusStarting();
  if (dwErrorCode != ERROR_SUCCESS) {
    setServiceStatusStopped(dwErrorCode);
    return;
  }

  dwErrorCode = getShutdownPrivilege();
  if (dwErrorCode != ERROR_SUCCESS) {
    setServiceStatusStopped(dwErrorCode);
    return;
  }

  ghServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (ghServiceStopEvent == NULL) {
    dwErrorCode = GetLastError();
    logError(_T("Cannot create service stop event: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    setServiceStatusStopped(dwErrorCode);
    return;
  }
  
  // Run code in a separate thread, so any exceptions there will not impact the functioning of this code
  HANDLE hThread = CreateThread(NULL, 0, serviceWorkerThread, NULL, 0, NULL);
  if (hThread == NULL) {
    dwErrorCode = GetLastError();
    logError(_T("Cannot create service thread: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    setServiceStatusStopped(dwErrorCode);
    return;
  }

  dwErrorCode = setServiceStatusStarted();
  if (dwErrorCode != ERROR_SUCCESS) {
    TerminateThread(hThread, ERROR_SUCCESS);
    setServiceStatusStopped(dwErrorCode);
    return;
  }

  if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0) {
    dwErrorCode = GetLastError();
    TerminateThread(hThread, ERROR_SUCCESS);
    logError(_T("Cannot wait for service thread: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    setServiceStatusStopped(dwErrorCode);
    return;
  }
  
  if (!CloseHandle(ghServiceStopEvent)) {
    dwErrorCode = GetLastError();
    logError(_T("Cannot close service stop event: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    setServiceStatusStopped(dwErrorCode);
    return;
  }

  if (!GetExitCodeThread(hThread, &dwErrorCode)) {
    dwErrorCode = GetLastError();
    logError(_T("Cannot retreive service thread exit code: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    setServiceStatusStopped(dwErrorCode);
    return;
  }

  setServiceStatusStopped(dwErrorCode);
  logInformation(_T("serviceMain stopped."));
  return;
}
