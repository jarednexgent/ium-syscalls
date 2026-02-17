# IUM Syscalls

A minimal import library shim for loading `iumdll.dll` into a process, providing an alternative source of `syscall` instructions outside `ntdll.dll`.

This project provides:

- A module-definition file (`iumbase.def`) for generating an import library
- A PowerShell script to generate `iumbase.lib`
- A small C helper (`IumSyscalls.c` / `IumSyscalls.h`) for forcing inclusion into the Import Address Table (IAT)
- A Visual Studio example project demonstrating usage with a Hell's Hall implementation

## Overview

As part of the [Isolated User Mode (IUM)](https://learn.microsoft.com/en-us/windows/win32/procthread/isolated-user-mode--ium--processes) subsystem, modern Windows includes `iumdll.dll`, a core system library that contains numerous `syscall; ret` gadgets suitable for indirect syscall implementations.

![iumdll syscall instructions](images/iumdll-syscalls.png)

By generating and linking against a custom import library for `iumbase.dll`, the process import table forces `iumbase.dll` to load, which in turn proxy-loads `iumdll.dll`. Once present in the process address space, `iumdll.dll` can be parsed to retrieve valid syscall instructions for dynamic resolution outside `ntdll.dll`.

## Repository Structure

The repository is organized as follows:

- `/include`: Public headers.
  - `IumSyscalls.h` – Declares the helper function used to force IAT inclusion.
- `/src`: Core source files.
  - `IumSyscalls.c` – Minimal helper implementation.
  - `iumbase.def` – Module-definition file used to generate the import library.
- `/scripts`: Build utilities.
  - `generate_lib.ps1` – Generates `iumbase.lib` using MSVC `lib.exe`.
- `/examples`: Example Visual Studio project.
  - `/IumHellsHall` – Demonstrates integration with a Hell's Hall–style syscall resolver.
- `/images`: Documentation assets (screenshots and diagrams).
- `/lib`: Generated output directory (created dynamically, not tracked by Git).
- `README.md`: Project documentation.
- `LICENSE`: License information.
- `NOTICE`: Third-party attribution.

The `lib/` directory is created dynamically and is not committed to the repository.

## Generating the Import Library

Open **Developer PowerShell for Visual Studio** and run `.\scripts\generate_lib.ps1` :

```
PS C:\ium-syscalls> .\scripts\generate_lib.ps1

Created directory: C:\ium-syscalls\lib
Generating import library...
Architecture: X64
Microsoft (R) Library Manager Version 14.44.35217.0
Copyright (C) Microsoft Corporation.  All rights reserved.

Creating library C:\ium-syscalls\lib\iumbase.lib and object C:\ium-syscalls\lib\iumbase.exp

Successfully generated:
C:\ium-syscalls\lib\iumbase.lib
```

This generates `lib\iumbase.lib`. 


## Usage

To integrate into your own Visual Studio project:

1. Add `include\` to:
   - C/C++ → Additional Include Directories

2. Add `lib\` to:
   - Linker → General → Additional Library Directories

3. Add `iumbase.lib` to:
   - Linker → Input → Additional Dependencies

4. Add the implementation file:
   - Right-click your project
   - Add → Existing Item…
   - Select `src\IumSyscalls.c`

5. Include `IumSyscalls.h` and call `AddIumdllToIAT()` at the beginning of `main`.

```
#include "IumSyscalls.h"

int main(void)
{
    AddIumdllToIAT();

    // Your syscall resolution logic here

    return 0;
}
```

This ensures `iumdll.dll` is loaded before export parsing and syscall resolution.

## Example

The example solution is located at `examples/IumHellsHall/IumHellsHall.sln`.

**Build instructions:**
1. Generate the import library  
2. Open the solution  
3. Build (x64 configuration)

## Acknowledgements

- [MaldevAcademy](https://maldevacademy.com) for [HellsHall](https://github.com/Maldev-Academy/HellHall) and [MaldevAcademyLdr.1](https://github.com/Maldev-Academy/MaldevAcademyLdr.1)  
- [Am0nSec](https://amonsec.net/) and RtlMateusz for [HellsGate](https://github.com/am0nsec/HellsGate)
- [Sektor7](https://institute.sektor7.net/) for [Halosgate](https://blog.sektor7.net/#!res/2021/halosgate.md)  
- [trickster0](https://trickster0.github.io/) for [TartarusGate](https://github.com/trickster0/TartarusGate) 

Portions of the example implementation are adapted from the above projects in accordance with their respective licenses.

## Requirements

- Windows 10 / 11 (x64)
- Visual Studio 2022 (MSVC toolchain)
- Developer PowerShell (for generating the import library)

## Disclaimer

Intended for research and educational purposes.
