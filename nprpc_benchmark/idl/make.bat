@echo off
setlocal
cd /d %~dp0

npidl.exe ^
	--gen-ts 1 ^
	--out-ts-dir \\wsl$\Arch\home\nikita\projects\nprpc_benchmark\src\rpc ^
	--out-inc-dir ..\ ^
	--out-src-dir ..\ ^
	benchmark.npidl

pause