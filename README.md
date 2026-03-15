# libppcemu

libppcemu is a hackable CPU emulation library meant for emulating various PowerPC processors.  It does not implement a complete system, of any kind, and the application is expected to provide everything outside of the CPU itself.  This makes it uniquely suitable as part of a solution for emulating nonstandard systems, where full-system emulators like QEMU would have more trouble.

## Supported CPUs

libppcemu supports emulating the following PowerPC CPUs:
- 750CXe
- 750CL
- Gekko
- Broadway
- Espresso
