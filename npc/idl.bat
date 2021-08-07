@echo off
setlocal
cd /d %~dp0

npidl.exe --gen-ts 0 --out-inc-dir include\npc --out-src-dir src idl\db.npidl
npidl.exe --gen-ts 0 --out-inc-dir include\npc --out-src-dir src idl\server.npidl

pause