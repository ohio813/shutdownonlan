#include "stdafx.h"
#include "listenForData.h"

DWORD WINAPI serviceWorkerThread(LPVOID lpParam) {
  return listenAndWaitForData();
}
