# $ErrorActionPreference = "Stop"
msbuild "C:\projects\addmusickff\src\AddmusicK.sln" /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }
# ls C:\MinGW\bin\
# mingw32-make
$ if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }