#include "stdafx.h"

extern LPTSTR                gsServiceName;
extern LPTSTR                gsServiceDescription;
extern LPTSTR                gsShutdownReason;
extern int                   gaiPorts[2];

extern BOOL                  gbThisIsAServiceProcess;
extern DWORD                 gdwServiceState;
extern DWORD                 gdwServiceErrorCode;
extern SERVICE_STATUS_HANDLE ghServiceStatusHandle;
extern HANDLE                ghServiceStopEvent;
