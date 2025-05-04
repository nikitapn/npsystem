# npsystem

A DCS system built around 8-bit AVR micro controller.

## Key Features
- Multi master protocol loosely based on P-NET
- Control units can be uploaded on the fly
- Modify and see propagation of the signal on the running algorithms
- Signal quality bit propagation
- Each micro contoller is an independet unit which can send requests to other devices on the network
- Logic can be tested on virtual controlles
- Controllers supports integration with custom modules via I2C and memory addresses mapping

## Dependencies on Windows
- cygwin https://www.cygwin.com
- make https://gnuwin32.sourceforge.net/packages/make.htm
- perl https://strawberryperl.com
- boost https://www.boost.org/
- leveldb https://github.com/google/leveldb
- scintilla https://www.scintilla.org/SciTEDownload.html
- wtl https://wtl.sourceforge.io/
- AVR toolchain https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers
- avrdude https://www.nongnu.org/avrdude

## Pictures

<p align="center">
  <b>How it looks</b>
  <img src="https://user-images.githubusercontent.com/86890989/187889033-cefc38d6-a4a2-458e-8b0a-cc21c07e97b0.png">
  <b>Structural Diagram</b>
  <img src="https://user-images.githubusercontent.com/86890989/194228189-b999a2b3-aa35-41a7-9b53-975eb641f8f7.png">
</p>

## TODO
- [ ] A proper compiler based on LLVM, maybe? With a support for ST language and quality bit propagation.
- [ ] Probably need to remove bit values. It's hard to deal with.
- [ ] Sort dependencies and load all algorithm at once.
- [ ] Add more tests.
- [ ] Add another more powerfull MC, maybe SBP based on Linux (probably a driver will be required for the protocol).
- [ ] Add more FBD blocks (structured text block, etc...).
- [ ] Add SFC language support

## References
1. [MAX3082 Transcievers [PDF]](https://www.analog.com/media/jp/technical-documentation/data-sheets/1522.pdf)
2. [Detection of RS-485 signal loss [PDF]](https://www.ti.com/lit/an/slyt257/slyt257.pdf?ts=1746389303529&ref_url=https%253A%252F%252Fwww.google.com%252F)
3. [P-NET [PDF]](https://www.p-net.org/old/download/mctext.pdf)