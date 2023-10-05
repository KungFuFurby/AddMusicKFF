$Env:PATH += ";C:\cygwin\bin\"
msbuild "C:\projects\addmusickff\src\AddmusicK.sln" /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }
make
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }