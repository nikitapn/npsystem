# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

npsystem is a Distributed Control System (DCS) for 8-bit AVR microcontrollers networked over a multi-master RS-485 bus (protocol loosely based on P-NET). Control logic (FBD blocks) is compiled to AVR code and can be uploaded to running controllers on the fly. All desktop/server components talk to each other through **nprpc**, an in-house RPC framework vendored in `nprpc/` (own README there).

## Build (Linux)

```bash
./configure.sh            # configure Debug build into build/linux (Unix Makefiles)
./build.sh                # cmake --build build/linux -j$(nproc)
./launch.py               # start npnameserver + npdbserver + npserver together
./launch.py --no-server   # omit npserver
```

- `configure.sh` passes extra args through to CMake. Useful options:
  - `-DOPT_BUILD_ONLY_RPC=ON` — build only nplib + nprpc (skips the whole DCS)
  - `-DOPT_NPRPC_SKIP_TESTS=ON` — skip GoogleTest fetch and test targets
- Binaries land in `build/linux/bin`.
- C++23; on Linux `compile_commands.json` is exported.
- The GUI (`npsystem/`) and AVR firmware build (`avr.cmake`) are **Windows-only** (WTL/Scintilla GUI; firmware needs the AVR-GCC toolchain and perl — see README for the Windows dependency list, configure with `configure.bat`).

## Tests

Tests are GoogleTest binaries in `build/linux/bin`, run directly:

```bash
build/linux/bin/nprpc_test                        # nprpc integration tests
build/linux/bin/npidl_test                        # IDL compiler tests
build/linux/bin/test_nplib
build/linux/bin/test_shared_memory_endpoint
build/linux/bin/nprpc_test --gtest_filter='Suite.Case'   # single test
```

Some (e.g. `nprpc_test`) expect `npnameserver` running. `tests/` at the repo root (test_compiler for npcompiler) is not wired into the root build.

## Architecture

Data flows: **GUI/web ⇄ nprpc ⇄ npserver ⇄ RS-485 bus ⇄ AVR controllers (real or simulated)**, with all persistent state in the object database.

- **nprpc/** — CORBA-like RPC framework: `npidl/` is the IDL compiler (generates C++ and TypeScript from `.npidl` files), `npnameserver/` is the object-discovery service, `nprpc_js/` the TypeScript client runtime. Transports: TCP, WebSocket/WSS, HTTP, shared memory. `nprpc/cmake/npidl.cmake` provides `npidl_generate_idl_files()` used by consumers.
- **npc/** — the system's service contracts: `idl/*.npidl` (db, server, npwebserver, constants) compiled into the `npc` static library. Change an interface here and both C++ and TS stubs regenerate at build time.
- **npdbserver/** + **npdbclient/** + **npdbstatic/** — object database on leveldb. Client side (`npdb/` headers) provides serialization, memento/undo, observable nodes, and typed node references.
- **npsys/** — the shared domain model (networks, AVR controllers, control units, FBD blocks, variables, memory types) persisted via npdb and used by every component that touches system state.
- **npserver/** — the runtime master of the field bus. `protocol*.{h,cpp}` implement the multi-master protocol (frame layout documented in `avr_firmware/include/avr_firmware/net.h`); `bridge_udp` forwards frames to the physical bus via UDP, `bridge_avr_virtual`/`avr_virtual` run firmware on simulated controllers in-process.
- **sim/** — a full AVR instruction-set emulator (ATmega8/16 including UART and peripherals) that executes real firmware images, letting logic be tested on virtual controllers.
- **avr_firmware/** — firmware for ATmega8 (`m8/`), ATmega16 (`m16/`) and the virtual variant (`m16v/`), built by perl scripts/makefiles; `make_info` + `gen-map*.pl` generate the memory-address maps the desktop side uses to address controller variables.
- **avr_info/** — static metadata about supported MCUs.
- **npsystem/** (directory) — the Windows WTL GUI configurator: FBD algorithm editor, controller configuration, on-the-fly upload (flat file list, ~60 cpp files).
- **npwebserver/** — web-facing server plus a Svelte frontend in `web/`.
- **npcompiler/** — LLVM-based compiler for control-logic languages (WIP, currently not part of the build).
- **nplib/** — small shared utility library.

The wire protocol between npserver and controllers is byte-level (read/write bytes/bits/pages with per-signal quality status), CRC-16-checked, at 76800 baud — see the frame diagrams in `avr_firmware/include/avr_firmware/net.h`.

## Notes

- A newer standalone evolution of nprpc lives in a separate repo (`nikitapn/nprpc`); the copy here is part of this repo's history and is what this build uses.
- `CMAKE_GENERATED_FILES_GUIDE.md` documents the repo's conventions for CMake custom commands around generated (IDL) files — follow it when touching codegen rules.
