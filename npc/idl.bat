setlocal
cd /d %~dp0

npidl.exe --out-inc-dir include\npc --out-src-dir src idl\db.npidl
npidl.exe --out-inc-dir include\npc --out-src-dir src idl\server.npidl

pause