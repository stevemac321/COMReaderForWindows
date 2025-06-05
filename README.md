# COM Port Reader

A simple Windows GUI application to read serial data from a COM port — similar in spirit to PuTTY or Tera Term, but much lighter. This app is ideal for debugging embedded systems, microcontrollers, or any serial-based device that outputs ASCII data.

---

## 🧰 Features

- ✅ Reads from COM4 at 115200 baud (defaults for now)
- ✅ Displays raw ASCII and hex-formatted output (e.g. `41 42 43`)
- ✅ Properly handles `\r\n` newlines like PuTTY
- ✅ Scrollable, read-only display window
- ✅ Minimal build — just Win32 C++, no frameworks
- ✅ Menu bar with placeholders for future settings (TODOs welcome!)

---

## 🔧 How to Build

This project is plain Win32 C++. You can build it in Visual Studio.

### 📦 Requirements

- Windows
- Visual Studio (Community edtition is fine).

### 🧱 Build Instructions


1. Open the `.sln` 
2. Build & run.

License: GPL version 2.
