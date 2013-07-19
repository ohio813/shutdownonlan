#include "stdafx.h"

#include "getMACAddresses.h"
#include "globals.h"
#include "log.h"

DWORD readDataAndCheckForMagic(SOCKET oSocket) {
  BYTE abBuffer[4096];
  int iBytesReceived = recv(oSocket, (char*)abBuffer, sizeof(abBuffer), 0);
  if (iBytesReceived == SOCKET_ERROR) {
    DWORD dwErrorCode = WSAGetLastError();
    // WSAEWOULDBLOCK appears to happen quite frequently
    if (dwErrorCode != WSAEWOULDBLOCK) {
      logError(_T("Socket receive failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    }
    return dwErrorCode;
  } else {
    int iFFCount = 0;
    // look for 6 x 0xFF
    for (int iFFScanIndex = 0; iFFScanIndex - iFFCount <= iBytesReceived - 6 * 17; iFFScanIndex++) {
      if (abBuffer[iFFScanIndex] == 0xFF) {
        iFFCount++;
      } else {
        iFFCount = 0;
      }
      if (iFFCount >= 6) {
        // Found 6 x 0xFF, look up MAC addresses of all network connections
        std::list<PBYTE> lpbMACAddresses;
        DWORD dwErrorCode = getMACAddresses(lpbMACAddresses);
        if (dwErrorCode != ERROR_SUCCESS) {
          return dwErrorCode;
        }
        while (!lpbMACAddresses.empty()) {
          PBYTE pbMACAddress = lpbMACAddresses.front();
          lpbMACAddresses.pop_front();
          // Look for 16 x MAC address following the 6 x 0xFF
          size_t iMACScanIndex = iFFScanIndex + 1;
          BOOL bMismatchFound = FALSE;
          for (size_t iCopy = 0; !bMismatchFound && iCopy < 16; iCopy++) {
            for (size_t iByte = 0; !bMismatchFound && iByte < 6; iByte++) {
              bMismatchFound = abBuffer[iMACScanIndex++] != pbMACAddress[iByte];
            }
          }
          delete[] pbMACAddress;
          if (!bMismatchFound) {
            logInformation(_T("Shutdown-on-LAN request received."));
            // Found 6 x 0xFF followed by 16 x MAC address: shut down.
            if (!InitiateSystemShutdownEx(
              NULL, // This machine
              gsShutdownReason,
              0, // Shut down immediately
              TRUE, // Force-close applications (do not save data)
              FALSE, // Do not reboot
              gdwShutdownReason
            )) {
              DWORD dwErrorCode = GetLastError();
              logError(_T("InitiateSystemShutdownEx failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
              return dwErrorCode;
            }
          }
        }
      }
    } // magic data not found
    return 0;
  }
}