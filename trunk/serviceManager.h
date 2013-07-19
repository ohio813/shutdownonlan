#include "stdafx.h"

extern SC_HANDLE ghSCManager;
DWORD openServiceManager();
DWORD closeServiceManager();

DWORD installService(LPTSTR sServiceName, LPTSTR sServiceDescription);
DWORD startService(LPTSTR sServiceName);
DWORD stopService(LPTSTR sServiceName);
DWORD uninstallService(LPTSTR sServiceName);

DWORD setServiceStatusStarting();
DWORD setServiceStatusStarted();
DWORD setServiceStatusStopping(DWORD dwErrorCode);
DWORD setServiceStatusStopped(DWORD dwErrorCode);

BOOL isServiceStarting();
BOOL isServiceStarted();
BOOL isServiceStopping();
BOOL isServiceStopped();

DWORD getServiceErrorCode(DWORD& dwErrorCode);
