# 🥧 Pi Cruncher for Raspberry Pi

A highly optimized, multi-threaded C++ benchmarking tool designed to calculate millions of digits of Pi and push the Raspberry Pi (and other Linux ARM boards) to its absolute limits.

This tool uses the **Chudnovsky algorithm** alongside **Binary Splitting** and the **GMP (GNU Multiple Precision)** library to achieve extreme speeds. It also features automatic, real-time verification against Google's Pi Delivery API.

## ✨ Features
* **Extreme Performance:** Bypasses interpreted languages to run raw, highly optimized C++ machine code.
* **Dynamic Multi-threading:** Automatically detects your CPU cores and spawns threads using `std::async` to maximize CPU utilization during the binary splitting phase.
* **Hardware Detection:** Automatically logs your OS, architecture, and available RAM.
* **Auto-Verification:** Connects directly to Google's Cloud API via `libcurl` to verify the mathematical integrity of the final 50 digits of your calculation.
* **Standardized Output:** Generates a professional `y-cruncher`-style validation log.

## 🚀 Benchmarks
*Tested on a Raspberry Pi 5 (8GB RAM, Broadcom BCM2712, 4 Cores)*
* **100,000 Digits:** ~0.038 seconds
* **1,000,000 Digits:** ~0.532 seconds
* **3,141,592 Digits:** ~2.18 seconds
* **100,000,000 Digits:** ~168.87 seconds (The "Yorkshire Tea" benchmark)

## 🛠️ Installation & Prerequisites

You will need the GNU C++ compiler, the GMP math library, and libcurl installed.

```bash
sudo apt update
sudo apt install g++ libgmp-dev libgmpxx4ldbl libcurl4-openssl-dev -y
```

⚙️ Compiling
Compile the code with the -O3 flag for maximum speed optimization:

```bash
g++ -O3 pi_cruncher.cpp -o pi_cruncher_cpp -lgmpxx -lgmp -lcurl -pthread
```
🏃‍♂️ Usage
Run the compiled executable and enter the number of digits you wish to calculate:

```bash
./pi_cruncher_cpp
```

Note: Calculating over 100 million digits requires significant RAM. Watch your system's memory usage with htop to avoid Out-Of-Memory (OOM) crashes!
