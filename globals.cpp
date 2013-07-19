#include "stdafx.h"

LPTSTR                gsServiceName = _T("ShutdownOnLAN");
LPTSTR                gsServiceDescription = _T("Shutdown-on-LAN network listener");
LPTSTR                gsShutdownReason = _T("Shutdown-on-LAN requested.");
int                   gaiPorts[2] = {7, 9};

BOOL                  gbThisIsAServiceProcess = FALSE;
DWORD                 gdwServiceState = SERVICE_STOPPED;
DWORD                 gdwServiceErrorCode = ERROR_SUCCESS;
SERVICE_STATUS_HANDLE ghServiceStatusHandle = NULL;
HANDLE                ghServiceStopEvent = INVALID_HANDLE_VALUE;
