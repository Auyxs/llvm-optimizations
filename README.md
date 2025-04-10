# LLVM Optimization Passes

[![LLVM Version](https://img.shields.io/badge/LLVM-19.1.7-orange.svg)](https://llvm.org/)
![License](https://img.shields.io/badge/License-MIT-blue.svg)

Repository containing Compilers Course assignments at **Unimore (2024/2025)** - Implementation of **LLVM IR** optimization passes as plugins.

## 📂 First Assignment 

1. **Algebraic Identity**
    - `x + 0` → `x`
    - `1 × 𝑥` → `x`

2. **Strength Reduction**
    - `15 × 𝑥 = 𝑥 × 15` → `(𝑥 ≪ 4) – x`
    - `y = x / 8` → `y = x >> 3`

3. **Multi-Instruction Optimization**
    - `𝑎 = 𝑏 + 1, 𝑐 = 𝑎 − 1` → `𝑎 = 𝑏 + 1, 𝑐 = b`

### Usage

#### 🔧 Build
To build the LLVM plugin, follow these steps:
```bash
mkdir -p build
cd build
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR /path/to/assignment-01
make
```
The plugin (`libLocalOpts.so`) will be generated in the build directory.
> Note : Ensure that $LLVM_DIR is correctly set to your LLVM installation path. 
> (e.g., /usr/lib/llvm-19/bin on Linux)
#### ▶️ Run Optimization Pass
Once the plugin is built, you can run the optimization script to test the implemented passes:
1. Place your `.cpp` test files in the `test/cpp` directory.
2. Before running the script, set the `OPT_PLUGIN_PATH` environment variable:
```bash
export OPT_PLUGIN_PATH=/path/to/libLocalOpts.so
```
3. Make the script executable and run it:
```bash
chmod +x run_opt.sh
./run_opt.sh
```
## 📂 Second Assignment - Dataflow Analysis
> Note: no code implementation required
1. **Very Busy Expressions**
2. **Dominator Analysis**
3. **Constant Propagation**

## Contributors
- Aurora Lin
- Eleonora Muzzi
