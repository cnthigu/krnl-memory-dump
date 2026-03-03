# Windows Kernel Memory Dump

Read process memory from kernel mode by dumping loaded modules

## How does it work?

This project demonstrates cross-process memory reading by exposing IOCTLs from a kernel driver. A user-mode application sends the target process ID and module name (e.g. `notepad.exe`, `ntdll.dll`), and the driver returns the module's memory or its size.

The user-mode app saves the dump to `processdumpcnt.bin` and automatically corrects the PE headers.

## Demo

![Memory Dump Demo](./demo.png)

## Loading the driver

- **Option 1** — Test Mode
- **Option 2** — [KDMapper](https://github.com/TheCruZ/kdmapper)

---

MAKE SURE TO ENABLE TEST MODE TO TEST THIS PROJECT. IF YOU WISH TO USE IT OUTSIDE TEST MODE, USE YOUR CUSTOM DRIVER LOADER OR SIGN THE DRIVER.

NOTE: THIS IS FOR EDUCATIONAL PURPOSES ONLY.

---
