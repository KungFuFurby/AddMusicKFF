Write-Output $Env:PATH
$Env:PATH += ";C:\msys64\usr\bin\"
ls C:\msys64\usr\bin\
# msbuild "C:\projects\addmusickff\src\AddmusicK.sln" /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
# if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }
# ls C:\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin
make
if ($LastExitCode -ne 0) { $host.SetShouldExit($LastExitCode)  }