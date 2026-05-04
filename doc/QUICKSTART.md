# Quick Start Guide

Get the UE Simulator 5G/6G running in 5 minutes!

---

## ⚡ Fast Track

### Windows (MinGW)

```bash
# 1. Clone/navigate to project
cd 5_symulator_5g

# 2. Build
build.bat

# Done! The simulator runs automatically.
```

### Windows (Manual)

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
ue_simulator.exe
```

### Linux/macOS

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
./ue_simulator
```

---

## 📦 Prerequisites

| Tool | Minimum Version | Check Command |
|------|----------------|---------------|
| CMake | 3.10+ | `cmake --version` |
| GCC | 7+ | `g++ --version` |
| Clang | 5+ | `clang++ --version` |
| MSVC | 2017+ | Check Visual Studio |

---

## 🎯 Expected Output

```
╭────────────────────────────────────────╮
│   UE SIMULATOR 5G/6G - L2 STACK       │
│   3GPP TS 38.321/322/323 Compliant    │
╰────────────────────────────────────────╯

[SYSTEM] Generating traffic...

[RLC] Segmented PDU ID=0 size=873B priority=1
[MAC] Transmitted PDU ID=0 latency=25ms
[RLC] Segmented PDU ID=1 size=785B priority=3
[MAC] Transmitted PDU ID=1 latency=78ms
...

=== MAC STATISTICS ===
Transmitted packets: 20
Total bytes: 16662
RLC queue size: 0
======================

[SYSTEM] Simulation completed successfully!
```

---

## 🔧 Troubleshooting

### Issue: CMake not found
```bash
# Windows: Install from https://cmake.org/download/
# Linux: sudo apt install cmake
# macOS: brew install cmake
```

### Issue: Compiler not found
```bash
# Windows: Install MinGW or Visual Studio
# Linux: sudo apt install build-essential
# macOS: xcode-select --install
```

### Issue: Build fails with errors
```bash
# Clean and rebuild
cd build
cmake --build . --clean-first
```

---

## 📖 Next Steps

1. **Understand the Basics**:
   - [HOW_IT_WORKS.md](HOW_IT_WORKS.md) - Simple explanation for everyone

2. **Read Documentation**:
   - [README.md](README.md) - Full documentation
   - [EXECUTION_RESULTS.md](EXECUTION_RESULTS.md) - Real execution logs and flow diagrams
   - [EXAMPLES.md](EXAMPLES.md) - Usage examples
   - [ARCHITECTURE.md](ARCHITECTURE.md) - Design details
   - [3GPP_COMPLIANCE.md](3GPP_COMPLIANCE.md) - 3GPP compliance

2. **Explore Code**:
   - `include/` - Header files
   - `src/` - Implementation files
   - `src/main.cpp` - Entry point

3. **Customize**:
   - Modify packet count in `main.cpp`
   - Adjust timing in layer implementations
   - Add custom observers

---

## 🚀 Quick Customization

### Change Packet Count

Edit `src/main.cpp`:
```cpp
simulator.generateTraffic(50);  // Change from 20 to 50
```

### Change Transmission Delay

Edit `include/mac_layer.hpp`:
```cpp
static constexpr int TRANSMISSION_DELAY_MS = 5;  // Change from 10 to 5
```

### Change Statistics Interval

Edit `src/ue_simulator.cpp` in `statisticsLoop()`:
```cpp
std::this_thread::sleep_for(std::chrono::seconds(1));  // Change from 3 to 1
```

---

## 📊 Project Structure

```
5_symulator_5g/
├── include/          # Headers (6 files)
├── src/              # Implementation (5 files)
├── build/            # Build output
├── README.md         # Main documentation
├── EXECUTION_RESULTS.md # Real execution logs
├── ARCHITECTURE.md   # Design details
├── EXAMPLES.md       # Usage examples
├── 3GPP_COMPLIANCE.md # 3GPP compliance
└── CMakeLists.txt    # Build configuration
```

---

## ✅ Verification

After building, verify:

1. **Executable exists**: `build/ue_simulator.exe` (Windows) or `build/ue_simulator` (Linux/macOS)
2. **No warnings**: Build output shows `[100%] Built target ue_simulator`
3. **Runs successfully**: Output shows statistics and "Simulation completed successfully!"

---

## 🎓 Learning Path

1. **Beginner**: Run default simulation, read README.md
2. **Intermediate**: Modify parameters, read EXAMPLES.md
3. **Advanced**: Extend layers, read ARCHITECTURE.md
4. **Expert**: Add 3GPP features, read 3GPP_COMPLIANCE.md

---

## 💡 Tips

- Use `build.bat` on Windows for easiest build
- Check `EXAMPLES.md` for code snippets
- Read `ARCHITECTURE.md` for design patterns
- See `3GPP_COMPLIANCE.md` for specification details

---

## 🆘 Getting Help

1. Check error messages carefully
2. Read relevant documentation section
3. Verify prerequisites are installed
4. Try clean rebuild: `cmake --build . --clean-first`

---

## 🎉 Success!

If you see "Simulation completed successfully!", you're ready to:
- Explore the code
- Modify parameters
- Add new features
- Learn 3GPP protocols

---

**Time to first run**: < 5 minutes  
**Difficulty**: Easy  
**Prerequisites**: CMake + C++ compiler  
**Platform**: Windows, Linux, macOS
