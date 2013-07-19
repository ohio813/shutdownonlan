#include "stdafx.h"

#include "globals.h"
#include "listeningSocket.h"
#include "log.h"
#include "readDataAndCheckForMagic.h"

DWORD listenForData(BOOL bWaitForData) {
  // Initialize WSA
  WORD wWSAVersion = MAKEWORD(2, 2);
  WSADATA oWSAData;
  int iErrorCode = WSAStartup(wWSAVersion, &oWSAData);
  if (iErrorCode) {
    logError(_T("WSAStartup failed: error code %d (0x%X)."), iErrorCode, iErrorCode);
    return iErrorCode;
  }
  if (oWSAData.wVersion != wWSAVersion) {
    logError(_T("Winsock version 2.2 not available."));
    return WSAVERNOTSUPPORTED;
  }

  // When running as a service, wait for data on two ports AND the service stop event (3 objects)
  // When not running as a service, wait for data on two ports only (2 objects);
  size_t iPortCount = sizeof(gaiPorts) / sizeof(gaiPorts[0]);
  SOCKET* poSockets = new SOCKET[iPortCount];
  size_t iWaitForHandlesCount = iPortCount + (gbThisIsAServiceProcess ? 1 : 0);
  HANDLE* phWaitForHandles = new HANDLE[iWaitForHandlesCount];

  for (size_t iPortIndex = 0; iPortIndex < iPortCount; iPortIndex++) {
    DWORD dwErrorCode = listeningSocket(gaiPorts[iPortIndex], *(poSockets + iPortIndex), *(phWaitForHandles + iPortIndex));
    if (dwErrorCode != ERROR_SUCCESS) {
      while (iPortIndex-- > 0) closesocket(*(poSockets + iPortIndex));
      delete []phWaitForHandles;
      delete []poSockets;
      WSACleanup();
      return dwErrorCode;
    }
  }
  if (gbThisIsAServiceProcess) {
    *(phWaitForHandles + iPortCount) = ghServiceStopEvent;
  }
  while (bWaitForData) {
    DWORD dwWaitObject = WaitForMultipleObjects((DWORD)iWaitForHandlesCount, phWaitForHandles, FALSE, INFINITE);
    int iHandleIndex = dwWaitObject - WAIT_OBJECT_0;
    if (iHandleIndex < 0 || iHandleIndex >= iWaitForHandlesCount) {
      logError(_T("WaitForMultipleObjects return unhandled value %d (0x%X)."), dwWaitObject, dwWaitObject);
    }
    if (iHandleIndex == iPortCount) {
      // ghServiceStopEvent set.
      bWaitForData = FALSE;
    } else {
      readDataAndCheckForMagic(*(poSockets + iHandleIndex));
    }
  }
  delete []phWaitForHandles;
  DWORD dwErrorCode = ERROR_SUCCESS;
  for (size_t iPortIndex = 0; iPortIndex < iPortCount; iPortIndex++) {
    if (closesocket(*(poSockets + iPortIndex)) != 0) {
      DWORD dwWSAErrorCode = WSAGetLastError();
      logError(_T("Unable to close listening socket for port %d: error code %d (0x%X)."), gaiPorts[iPortIndex], dwWSAErrorCode, dwWSAErrorCode);
      if (dwErrorCode == ERROR_SUCCESS) dwErrorCode = dwWSAErrorCode;
    }
  }
  delete []poSockets;
  if (WSACleanup()) {
    DWORD dwWSAErrorCode = WSAGetLastError();
    logError(_T("WSACleanup failed: error code %d (0x%X)."), dwWSAErrorCode, dwWSAErrorCode);
    if (dwErrorCode == ERROR_SUCCESS) dwErrorCode = dwWSAErrorCode;
  }
  return dwErrorCode;
}

DWORD listenForDataToWakeUpFirewall() {
  return listenForData(FALSE);
}
DWORD listenAndWaitForData() {
  return listenForData(TRUE);
}
