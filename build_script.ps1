Write-Output $Env:PATH
$Env:PATH += ";C:\MinGW\bin\"
# msbuild "C:\projects\addmusickff\src\AddmusicK.sln" /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
# if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }
ls C:\mingw-w64
mingw32-make
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }