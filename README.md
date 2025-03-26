# LLVM Optimization Passes

[![LLVM Version](https://img.shields.io/badge/LLVM-19.1.7-orange.svg)](https://llvm.org/)
![License](https://img.shields.io/badge/License-MIT-blue.svg)

Repository containing Compilers Course assignments at **Unimore** (2024/2025) - Implementation of LLVM IR optimization passes as plugins.

## First Assignment 

### 1. Algebraic Identity
- `x + 0` → `x`
- `1 × 𝑥` → `x`
- `x - 0` → `x`
- `x / 1` → `x`

### 2. Strength Reduction
- `15 × 𝑥 = 𝑥 × 15` → `(𝑥 ≪ 4) – x`
- `y = x / 8` → `y = x >> 3`

### 3. Multi-Instruction Optimization
- `𝑎 = 𝑏 + 1, 𝑐 = 𝑎 − 1` → `𝑎 = 𝑏 + 1, 𝑐 = b`

## Usage

### Build 
```bash

```
### Test
```bash

```
## Contributors
- Aurora Lin
- Eleonora Muzzi
