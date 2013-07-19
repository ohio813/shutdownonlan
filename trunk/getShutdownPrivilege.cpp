#include "stdafx.h"

#include "log.h"

DWORD getShutdownPrivilege() {
  // Adjust privileges so this process can shutdown the system.
  HANDLE hToken; 
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    DWORD dwErrorCode = GetLastError();
    logError(_T("OpenProcessToken failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  // Create token to enable SE_SHUTDOWN_NAME
  TOKEN_PRIVILEGES oTokenPrivileges; 
  oTokenPrivileges.PrivilegeCount = 1;
  oTokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
  if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &oTokenPrivileges.Privileges[0].Luid)) {
    DWORD dwErrorCode = GetLastError();
    logError(_T("LookupPrivilegeValue failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  // Apply token
  if (!AdjustTokenPrivileges(hToken, FALSE, &oTokenPrivileges, 0, (PTOKEN_PRIVILEGES)NULL, 0)) {
    DWORD dwErrorCode = GetLastError();
    logError(_T("AdjustTokenPrivileges failed: error code %d (0x%X)."), dwErrorCode, dwErrorCode);
    return dwErrorCode;
  }
  return ERROR_SUCCESS;
}
