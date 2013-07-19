#include "stdafx.h"
#include "globals.h" // gsServiceName

#define EVENT_ID_ERROR        ((DWORD)0xC0020003L)
#define EVENT_ID_INFORMATION  ((DWORD)0x40020001L)

VOID log(WORD wType, DWORD dwEventId, LPTSTR sFormat, va_list axArguments) {
  size_t iLength = _tcslen(sFormat);
  LPTSTR sMessage;
  while (1) {
    sMessage = (LPTSTR) new TCHAR[iLength + 1];
    if (_vsntprintf_s(sMessage, (iLength + 1), iLength, sFormat, axArguments) >= 0) {
      break;
    }
    delete[] sMessage;
    iLength *= 2;
  }
  if (gbThisIsAServiceProcess) {
    HANDLE hEventSource = RegisterEventSource(NULL, gsServiceName);
    LPCTSTR asMessageAndHeader[2] = {gsServiceName, sMessage};

    if (hEventSource) {
      ReportEvent(hEventSource, wType, 0, dwEventId, NULL, 2, 0, asMessageAndHeader, NULL);
      DeregisterEventSource(hEventSource);
    }
  } else {
    _tprintf(sMessage);
    _tprintf(_T("\r\n"));
  }
  delete[] sMessage;
}
VOID logError(LPTSTR sFormat, ...) {
  va_list axArguments;
  va_start(axArguments, sFormat);
  return log(EVENTLOG_ERROR_TYPE, EVENT_ID_ERROR, sFormat, axArguments);
}
VOID logInformation(LPTSTR sFormat, ...) {
  va_list axArguments;
  va_start(axArguments, sFormat);
  return log(EVENTLOG_INFORMATION_TYPE, EVENT_ID_INFORMATION, sFormat, axArguments);
}
