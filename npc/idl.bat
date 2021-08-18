@echo off
setlocal
cd /d %~dp0

npidl.exe --gen-ts 0 --out-ts-dir \\wsl$\Debian\home\png\projects\npweb\src --out-inc-dir include\npc --out-src-dir src idl\db.npidl
npidl.exe --gen-ts 1 --out-ts-dir \\wsl$\Debian\home\png\projects\npweb\src --out-inc-dir include\npc --out-src-dir src idl\server.npidl

pause