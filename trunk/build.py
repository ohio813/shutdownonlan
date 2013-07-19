import os, subprocess, threading;

gsWDExpressPath = r"C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\WDExpress.exe";
assert os.path.isfile(gsWDExpressPath), "Cannot find WDExpress.exe";
giErrorCount = 0;

def build(sFolderPath, sFileName, sPlatform, sConfig):
  global giErrorCount;
  oOutputLock.acquire();
  print "Building %s (%s, %s)..." % (sFileName, sPlatform, sConfig);
  oOutputLock.release();
  sFilePath = os.path.join(sFolderPath, sFileName);
  iTryCount = 1;
  while iTryCount <= 2:
    asCommandLine = [gsWDExpressPath, sFilePath, "/build"];
    if sConfig:
      asCommandLine.append(sPlatform and "%s|%s" % (sConfig, sPlatform) or sConfig);
    oProcess = subprocess.Popen(asCommandLine, executable = gsWDExpressPath);
    iReturnCode = oProcess.wait();
    if iReturnCode == 1:
      iTryCount += 1;
    else:
      break;
  oOutputLock.acquire();
  if iReturnCode != 0:
    print "Build %s (%s, %s) failed! Error code: %d" % (sFileName, sPlatform, sConfig, iReturnCode);
    giErrorCount += 1;
  else:
    print "Build %s (%s, %s) success!" % (sFileName, sPlatform, sConfig);
  oOutputLock.release();

if __name__ == "__main__":
  import sys;
  oOutputLock = threading.Lock();
  aoThreads = [];
  sFolderPath = os.path.dirname(__file__);
  for sFileName in os.listdir(sFolderPath):
    if sFileName[-4:].lower() == ".sln" and os.path.isfile(sFileName):
      for sConfig in ["Debug", "Release"]:
        for sPlatform in ["Win32", "x64"]:
          oThread = threading.Thread(target = build, args = (sFolderPath, sFileName, sPlatform, sConfig));
          oThread.start();
          aoThreads.append(oThread);
  for oThread in aoThreads:
    oThread.join();
  if giErrorCount > 0:
    raw_input("Press ENTER to exit...");
