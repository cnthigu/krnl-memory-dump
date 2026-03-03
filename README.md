# Windows Kernel Memory Dump

*read process memory from kernel*

## How it works

You send a process ID and module name to the driver. It dumps the module's memory and returns it. The app saves to `processdumpcnt.bin` and fixes the PE headers.

## What it does

- Dumps a selected module from a running process
- Uses `DeviceIoControl` to communicate with the driver
- Fixes PE headers for reverse engineering tools

## Demo

![Memory Dump Demo](./demo.png)

## Loading the driver

- **Option 1** — Test Mode (`sc create` / `sc start`)
- **Option 2** — [KDMapper](https://github.com/TheCruZ/kdmapper)

---

MAKE SURE TO ENABLE TEST MODE TO TEST THIS PROJECT. IF YOU WISH TO USE IT OUTSIDE TEST MODE, USE YOUR CUSTOM DRIVER LOADER OR SIGN THE DRIVER.

NOTE: THIS IS FOR EDUCATIONAL PURPOSES ONLY.

---
