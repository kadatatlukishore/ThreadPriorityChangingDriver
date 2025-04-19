### ThreadPriorityChangingDriver

# 🧵 Windows Thread Priority Control via Driver

This project provides a Windows kernel-mode driver and a user-mode console application that lets you set the priority of any thread by specifying its thread ID—independently of its owning process's priority class.

This tool is useful for developers and system tuners who want more granular control over thread scheduling beyond what standard APIs offer.

---

## 🚀 Features

- Set priority of individual threads using Thread ID (TID)
- Ignores process priority class
- Clean separation of kernel and user-mode logic
- Simple command-line interface

---

## 📁 Project Structure

```
/ThreadPriorityChangingDriver
├── ThreadPriorityIncrement/                     # Kernel-mode driver (PriorityControlDriver.sys)
├── ThreadPriorityTestUserModeExe/               # User-mode console app (PrioritySetterApp.exe)
├── README.md              # This file
└── LICENSE
```

---

## 🛠️ Requirements

- Windows 10 or later (x64)
- Visual Studio with Windows Driver Kit (WDK) installed
- Administrator privileges
- Code-signing certificate (if testing with Secure Boot enabled)

---

## ⚙️ Build Instructions

### 🔧 Driver

1. Open the driver project (`.vcxproj`) in Visual Studio.
2. Set the configuration to **Release** or **Debug**.
3. Build the driver to produce `ThreadPriorityIncrement.sys`.

### 💻 User-mode Application

1. Open the user app project in Visual Studio.
2. Build the solution as a **Win32 Console Application**.
3. This will produce `ThreadPriorityTestUserModeExe.exe`.

---

## 🧪 Installation & Usage

> ⚠️ **You must have administrator privileges to install and run the driver.**

### 🧩 Step 1: Load the Driver

Using `sc.exe` from an Administrator Command Prompt:

```bash
sc create ThreadPriorityIncrement type= kernel binPath= "C:\Path\To\ThreadPriorityIncrement.sys"
sc start ThreadPriorityIncrement
```

Or unload it later:

```bash
sc stop ThreadPriorityIncrement
sc delete ThreadPriorityIncrement
```

---

### 🕹️ Step 2: Set Thread Priority

Run the app from an Administrator Command Prompt:

```bash
ThreadPriorityTestUserModeExe.exe <ThreadID> <Priority>
```

**Example:**

```bash
ThreadPriorityTestUserModeExe.exe 6320 3
```

Read more about the ThreadPriorties here - https://scorpiosoftware.net/2023/07/14/thread-priorities-in-windows/

---

## 🧠 How It Works

1. The user-mode app opens a handle to the driver using `CreateFile`.
2. It sends an IOCTL request containing:
   - Target thread ID (TID)
   - Desired priority level
3. The driver:
   - Looks up the thread using `ObReferenceObjectByHandle(data->hThread, THREAD_SET_INFORMATION,
				*PsThreadType, UserMode, (PVOID*)&Thread, nullptr);`
   - Validates the input
   - Calls `KeSetPriorityThread` to apply the new priority

---

## 🔐 Security Considerations

- Only administrators can install/load the driver.
- You **must** ensure only trusted users have access.
- Improper use can lead to system instability or thread starvation.

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).

---

## ⚠️ Disclaimer

This is a low-level system tool that directly modifies thread priorities in the Windows kernel. Use it responsibly. The authors are not liable for any system instability, crashes, or data loss caused by misuse.


---
