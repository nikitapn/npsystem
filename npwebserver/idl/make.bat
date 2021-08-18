@echo off
setlocal
cd /d %~dp0

npidl.exe ^
	--out-ts-dir \\wsl$\Debian\home\png\projects\npweb\src ^
	--out-inc-dir ..\src ^
	--out-src-dir ..\src ^
	npwebserver.npidl

pause