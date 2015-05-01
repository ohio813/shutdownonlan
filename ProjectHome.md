Shutdown-on-LAN (SOL) is complementary to [Wake-on-LAN (WOL)](https://en.wikipedia.org/wiki/Wake-on-LAN), in that it allows a computer to be shutdown by a network message. Combined with WOL, this allows a user to switch a computer on and off over a network. Shutdown-on-LAN can be installed as a Windows service to run each time the computer starts. The service will listen for a WOL message sent to the computer. When such a message is received, the computer is shut down.

# Download #
Shutdown-on-LAN is available in two version: a 32- and 64-bit build. If unsure, download the 32-bit build, as it will also work on 64-bit versions of Windows. (The 64-bit build will not work on 32-bit versions of Windows).
  * [Download ShutdownOnLAN.exe](https://shutdownonlan.googlecode.com/svn/trunk/build/Win32_Release/ShutdownOnLAN.exe) 32-bit build (x86).
  * [Download ShutdownOnLAN.exe](https://shutdownonlan.googlecode.com/svn/trunk/build/x64_Release/ShutdownOnLAN.exe) 64-bit build (x64).

# Installation #
Shutdown-on-LAN does not come with an installer because it is easier to install manually and/or automate installation on multiple machines through scripting. The following steps describe how to install Shutdown-on-LAN on a computer:
  1. Download a copy of [Shutdown-on-LAN.exe](https://shutdownonlan.googlecode.com/svn/trunk/build/Win32_Release/ShutdownOnLAN.exe), or build one from [source](https://code.google.com/p/shutdownonlan/source/checkout).
  1. Copy Shutdown-on-LAN.exe (and optionally Shutdown-on-LAN.pdb) to a local folder on the computer (for instance `"%ProgramFiles%\Shutdown-on-LAN\"`).
  1. Start a CMD prompt with administrative privileges and change directory to the local folder where Shutdown-on-LAN.exe was copied in step #2.
  1. Run the command: `"Shutdown-on-LAN.exe --install --start"` to install Shutdown-on-LAN as a service and start it.
  1. Allow Shutdown-on-LAN access to the network when a Windows firewall shows a popup asking about this.

# Uninstalling #
Uninstalling is very similar to installing, but in reverse order. The following steps describe how to uninstall Shutdown-on-LAN on a computer:
  1. Start a CMD prompt with administrative privileges and change directory to the local folder where Shutdown-on-LAN.exe was copied.
  1. Run the command: `"Shutdown-on-LAN.exe --stop --uninstall"` to stop the Shutdown-on-LAN service and uninstall it.
  1. Delete ShutdownOnLAN.exe and optionally the folder in which it was copied.

# Command line arguments #
`--install`
> Install Shutdown-on-LAN as a Windows Service.
`--start`
> Start the Shutdown-on-LAN Windows Service. It must be installed first for this to work.
`--stop`
> Stop the Shutdown-on-LAN Windows Service if it is installed and started.
`--uninstall`
> Uninstall the Shutdown-on-LAN Windows Service. It must be stopped first for this to work.
`--run`
> Run Shutdown-on-LAN in the console, rather than as a service. Useful for debugging, as all errors and informational messages are output to the console.
Multiple arguments can be combined, as in `"Shutdown-on-LAN.exe --install --start --stop --uninstall"`, which will install the service, start it, stop it and uninstall it in again. Arguments are processed in the order as they are listed above and not in the order on which they appear on the command line. In other words `"Shutdown-on-LAN.exe --start --install"` will install the service first and then start it.

# Notes #
**There is no time-out before the computer is shut down and all open applications are closed. It is assumed there is no user logged on to the computer to answer any questions from application about whether to save unsaved data. For most server applications, this should not be a problem, but for some client applications this may lead to data loss.** The name of the Shutdown-on-LAN service is [ShutdownOnLAN](https://code.google.com/p/shutdownonlan/source/browse/trunk/globals.cpp#3).
**Shutdown-on-LAN listens for WOL packets on UDP ports [7 and 9](https://code.google.com/p/shutdownonlan/source/browse/trunk/globals.cpp#7).** The shutdown [reason](http://msdn.microsoft.com/en-us/library/windows/desktop/aa376885(v=vs.85).aspx) given is "[Other (Planned)](https://code.google.com/p/shutdownonlan/source/browse/trunk/globals.cpp#6)" and the accompanying Event log message "[Shutdown-on-LAN requested.](https://code.google.com/p/shutdownonlan/source/browse/trunk/globals.cpp#5)"

# Troubleshooting #
The Shutdown-on-LAN service will output error and informational messages to the Windows [Event log](http://windows.microsoft.com/en-ph/windows-vista/open-event-viewer). If for some reason it is not working, the information there may help.
To test Shutdown-on-LAN, it may be easier to run it from the command-line, as error and informational messages will be output to the console instead of the event log. Make sure to stop the service before doing this, or there will be two copies of Shutdown-on-LAN responding to each network packet, which may interfere with testing.

## Bug reports and feature requests ##
Please use the issue tracker to [report a bug](https://code.google.com/p/shutdownonlan/issues/entry?template=Defect%20report%20from%20user) or [request a feature](https://code.google.com/p/shutdownonlan/issues/entry?template=Feature%20request).