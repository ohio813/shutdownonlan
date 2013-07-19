#include "stdafx.h"
#include "Iphlpapi.h"
#include "getMACAddresses.h"
#include "log.h"

// Thanks: http://www.codeguru.com/cpp/i-n/network/networkinformation/article.php/c5451/Three-ways-to-get-your-MAC-address.htm
DWORD getMACAddresses(std::list<PBYTE>& lpbMACAddresses) {
  DWORD dwBufferSize = 0;
  PBYTE pBuffer = 0;
  DWORD dwErrorCode = GetAdaptersInfo((PIP_ADAPTER_INFO)pBuffer, &dwBufferSize);
  if (dwErrorCode == ERROR_BUFFER_OVERFLOW) {
    pBuffer = new (std::nothrow) BYTE[dwBufferSize];
    if (!pBuffer) {
      logError(_T("Cannot allocate %d bytes of memory for adapter information."), dwBufferSize);
      return ERROR_OUTOFMEMORY;
    }
    dwErrorCode = GetAdaptersInfo((PIP_ADAPTER_INFO)pBuffer, &dwBufferSize);
  }
  if (dwErrorCode != ERROR_SUCCESS) {
    logError(_T("GetAdaptersInfo failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
  } else {
    PIP_ADAPTER_INFO pAdapterInfo = (PIP_ADAPTER_INFO)pBuffer;
    while (pAdapterInfo) {
      if (pAdapterInfo->AddressLength != 6) {
      } else {
        PBYTE pAddress = new (std::nothrow) BYTE[6];
        if (!pAddress) {
          delete[] pBuffer;
          logError(_T("Cannot allocate %d bytes of memory for MAC address."), 6);
          return ERROR_OUTOFMEMORY;
        }
        memcpy(pAddress, pAdapterInfo->Address, 6);
        lpbMACAddresses.push_back(pAddress);
        pAdapterInfo = pAdapterInfo->Next;
      }
    }
  }
  if (pBuffer) {
    delete[] pBuffer;
  }
  return dwErrorCode;
}