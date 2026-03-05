# IUM Syscalls

Proof-of-concept import library shim that loads `iumdll.dll` into a process and sources syscall stubs outside `ntdll.dll`.

The repository provides:

- `iumbase.def` – module-definition file used to generate an import library  
- `generate_lib.ps1` – PowerShell script that builds `iumbase.lib`  
- `IumSyscalls.c / IumSyscalls.h` – minimal helper used to force IAT inclusion  
- Example Visual Studio project demonstrating usage with Hell's Hall

## Overview

Modern Windows includes `iumdll.dll` as part of the [Isolated User Mode (IUM)](https://learn.microsoft.com/en-us/windows/win32/procthread/isolated-user-mode--ium--processes) architecture used by "Trustlet" processes. This system library contains numerous `syscall; ret` instruction sequences suitable for indirect syscall invocation.

![iumdll syscall instructions](assets/iumdll-syscalls.png)

By linking against a custom import library, the process import table forces `iumbase.dll` to load, which proxy-loads `iumdll.dll`. Once mapped into the process, it can be parsed to locate usable syscall instructions.

## Repository Layout

```
.
├── assets/
├── include/
│   └── IumSyscalls.h
├── src/
│   ├── IumSyscalls.c
│   └── iumbase.def
├── scripts/
│   └── generate_lib.ps1
├── examples/
│   └── IumHellsHall/
│       └── IumHellsHall.sln
├── lib/
├── LICENSE
├── NOTICE
└── README.md
```

Key directories:

- `include/` – public headers  
- `src/` – core implementation  
- `scripts/` – build utilities  
- `examples/` – demonstration project  

> **Note:** `iumbase.lib` is generated in the `lib/` directory during the build process and is not committed to the repository.

## Generating the Import Library

Open **Developer PowerShell for Visual Studio** and run: `.\scripts\generate_lib.ps1` 

```
PS C:\ium-syscalls> .\scripts\generate_lib.ps1

Created directory: C:\ium-syscalls\lib
Generating import library...
Architectur: X64
Microsoft (R) Library Manager Version 14.44.35217.0
Copyright (C) Microsoft Corporation. All rights reserved.

Creating library C:\ium-syscalls\lib\iumbase.lib and object C:\ium-syscalls\lib\iumbase.exp

Successfully generated:
C:\ium-syscalls\lib\iumbase.lib
```

## Usage

To integrate with a Visual Studio project:

1. Add `include\` to **C/C++ → Additional Include Directories**  
2. Add `lib\` to **Linker → Additional Library Directories**  
3. Add `iumbase.lib` to **Linker → Additional Dependencies**  
4. Add `src\IumSyscalls.c` to the project  
5. Include `IumSyscalls.h` in your source file 
6. Call `AddIumdllToIAT()` before resolving syscalls.

```
#include "IumSyscalls.h"

int main(void)
{
    AddIumdllToIAT();

    // syscall resolution logic

    return 0;
}
```

## Example

An example implementation is provided in: `examples/IumHellsHall/IumHellsHall.sln`

**Build steps:**

1. Generate `iumbase.lib`  
2. Open the solution  
3. Build (x64 configuration)

The project demonstrates resolving syscall stubs from `iumdll.dll` and executing a simple payload (Metasploit x64 calc).

```
PS C:\ium-syscalls> & ".\examples\IumHellsHall\x64\Release\IumHellsHall.exe"

[+] IUMDLL syscall : 0x00007FFDA6201198
[+] IUMDLL syscall : 0x00007FFDA6201168
[+] IUMDLL syscall : 0x00007FFDA6201158
[+] IUMDLL syscall : 0x00007FFDA6201148
[+] IUMDLL syscall : 0x00007FFDA6201178
[+] pTmpBuffer     : 0x00000288AE600000

[*] Press any key to decrypt...
```


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
