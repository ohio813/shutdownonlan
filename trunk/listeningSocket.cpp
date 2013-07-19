#include "stdafx.h"

#include "log.h"

DWORD listeningSocket(UINT iPort, SOCKET &oSocket, HANDLE &hSocketEvents) {
  oSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (oSocket == INVALID_SOCKET) {
    DWORD dwErrorCode = WSAGetLastError();
    logError(_T("Cannot create socket: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  BOOL bTrue = TRUE;
  if (setsockopt(oSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bTrue, sizeof(bTrue))) {
    DWORD dwErrorCode = WSAGetLastError();
    logError(_T("Cannot set SO_REUSEADDR socket option: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  if (setsockopt(oSocket, SOL_SOCKET, SO_BROADCAST, (char*)&bTrue, sizeof(bTrue))) {
    DWORD dwErrorCode = WSAGetLastError();
    logError(_T("Cannot set SO_BROADCAST socket option: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  struct sockaddr_in oAddress;
  oAddress.sin_family = AF_INET;
  oAddress.sin_addr.s_addr = INADDR_ANY;
  oAddress.sin_port = htons(iPort);
  if (bind(oSocket, (sockaddr*)&oAddress, sizeof(oAddress))) {
    DWORD dwErrorCode = WSAGetLastError();
    logError(_T("Cannot bind socket to UDP port %d: error code %d (0x%X)."), iPort, dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  hSocketEvents = WSACreateEvent();
  if (hSocketEvents == WSA_INVALID_EVENT) {
    DWORD dwErrorCode = WSAGetLastError();
    logError(_T("Cannot create socket event: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  if (WSAEventSelect(oSocket, hSocketEvents, FD_READ) != 0) {
    DWORD dwErrorCode = WSAGetLastError();
    logError(_T("Cannot select FD_READ socket event: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  return ERROR_SUCCESS;
}