# PDCP Layer - How Does It Work?

## 🎯 What Does the PDCP Layer Do?

**PDCP = Packet Data Convergence Protocol**

Think of PDCP as a **PACKER** in a factory:
- Receives raw data (like a gift without wrapping)
- Adds a **header** (like a "FRAGILE" sticker on a package)
- Adds a **trailer** (like tape sealing the box)
- Done! The package is wrapped and ready to ship

```
┌─────────────────────────────────────────────────────────┐
│                    PDCP = PACKER                        │
│                                                         │
│  Receives:  [Hello World!]                              │
│             ↓                                           │
│  Packs:     [0xAA][Hello World!][0xFF]                  │
│             ↑                      ↑                    │
│           Header                Trailer                 │
└─────────────────────────────────────────────────────────┘
```

---

## 📦 PDCPLayer Class Structure

```cpp
class PDCPLayer {
private:
    uint32_t sequenceNumber_;    // Packet sequence number
    Statistics stats_;           // Statistics (how many sent)
    mutable std::mutex mutex_;   // Lock (thread safety)
    
    const uint8_t PDCP_HEADER = 0xAA;    // Header = 0xAA
    const uint8_t PDCP_TRAILER = 0xFF;   // Trailer = 0xFF
};
```

### What Does It Mean?

```
┌─────────────────────────────────────────────────────────┐
│  sequenceNumber_ = 0                                    │
│  ↓                                                      │
│  Like numbering packages: #1, #2, #3...                │
│  (Not used in this prototype, but ready for use)       │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  stats_ = { packets: 0, bytes: 0 }                     │
│  ↓                                                      │
│  Counter: how many packets sent, how many bytes        │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  mutex_ = 🔒                                            │
│  ↓                                                      │
│  Lock - only one thread can enter at a time            │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│  PDCP_HEADER = 0xAA (170 in decimal)                   │
│  PDCP_TRAILER = 0xFF (255 in decimal)                  │
│  ↓                                                      │
│  Constant values - always the same                     │
└─────────────────────────────────────────────────────────┘
```

---

## 🔧 Function 1: Constructor

```cpp
PDCPLayer::PDCPLayer() : sequenceNumber_(0) {}
```

### What Does It Do?

```
STEP 1: Creating a new PDCP Layer
│
├─ sequenceNumber_ = 0     ← Start from zero
├─ stats_ = {0, 0}         ← Automatically zeroed
└─ mutex_ = 🔒             ← Lock ready

┌─────────────────────────────────────────────────────────┐
│  PDCP Layer READY!                                      │
│  Waiting for data to pack...                            │
└─────────────────────────────────────────────────────────┘
```

---

## 📤 Function 2: sendPDU() - Sending a Packet

```cpp
bool PDCPLayer::sendPDU(const PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);  // 🔒 Lock the door
    stats_.transmittedPackets++;               // Counter +1
    stats_.transmittedBytes += pdu.data.size(); // Add bytes
    return true;                               // Success!
}
```

### Step by Step:

```
┌─────────────────────────────────────────────────────────┐
│  STEP 1: Someone calls sendPDU(pdu)                    │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 2: 🔒 Lock the door                              │
│  Only I can enter now!                                 │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 3: Increment packet counter                      │
│  stats_.transmittedPackets++                           │
│                                                         │
│  Before: 5 packets                                     │
│  After:  6 packets  ✓                                  │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 4: Add bytes                                     │
│  stats_.transmittedBytes += pdu.data.size()            │
│                                                         │
│  Before:  1000 bytes                                   │
│  Packet:   500 bytes                                   │
│  After:  1500 bytes  ✓                                 │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 5: Return true (success!)                        │
│  🔓 Unlock automatically (end of lock scope)           │
└─────────────────────────────────────────────────────────┘
```

### Example:

```
Before:
stats_ = { packets: 10, bytes: 5000 }

sendPDU(pdu of size 250 bytes)
│
├─ 🔒 Lock
├─ packets: 10 → 11
├─ bytes: 5000 → 5250
├─ return true
└─ 🔓 Unlock

After:
stats_ = { packets: 11, bytes: 5250 }
```

---

## 📥 Function 3: receivePDU() - Receiving a Packet

```cpp
bool PDCPLayer::receivePDU(PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);  // 🔒 Lock
    (void)pdu;  // Not used (prototype)
    return false;  // Always false (not implemented)
}
```

### What Does It Do?

```
┌─────────────────────────────────────────────────────────┐
│  ⚠️  THIS FUNCTION IS NOT USED IN THE PROTOTYPE        │
│                                                         │
│  receivePDU() - receiving packets                      │
│  ↓                                                      │
│  Full version: would receive packets from the network  │
│  Prototype: always returns false                       │
│                                                         │
│  (void)pdu; ← Tells the compiler:                      │
│              "I know I'm not using pdu, don't warn"    │
└─────────────────────────────────────────────────────────┘
```

---

## 📊 Function 4: getStatistics() - Get Statistics

```cpp
Statistics PDCPLayer::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);  // 🔒 Lock
    return stats_;                             // Return a copy
}
```

### Step by Step:

```
┌─────────────────────────────────────────────────────────┐
│  STEP 1: Someone calls getStatistics()                 │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 2: 🔒 Lock the door                              │
│  (So nobody changes stats_ while reading)              │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 3: Copy stats_                                   │
│                                                         │
│  stats_ = { packets: 20, bytes: 10000 }                │
│  ↓                                                      │
│  Copy  = { packets: 20, bytes: 10000 }                 │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 4: Return the copy                               │
│  🔓 Unlock automatically                               │
└─────────────────────────────────────────────────────────┘
```

---

## 📦 Function 5: addHeader() - THE MAIN FUNCTION!

```cpp
std::vector<uint8_t> PDCPLayer::addHeader(const std::vector<uint8_t>& sdu) {
    std::vector<uint8_t> pdu;
    pdu.reserve(sdu.size() + 2);  // Reserve space
    
    pdu.push_back(PDCP_HEADER);   // Add 0xAA at the beginning
    pdu.insert(pdu.end(), sdu.begin(), sdu.end());  // Add data
    pdu.push_back(PDCP_TRAILER);  // Add 0xFF at the end
    
    return pdu;
}
```

### This Is the MAGIC of PDCP! Step by Step:

```
┌─────────────────────────────────────────────────────────┐
│  STEP 1: Receive raw data (SDU)                        │
│                                                         │
│  sdu = [72, 101, 108, 108, 111]  ← "Hello" in ASCII   │
│         H    e    l    l    o                          │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 2: Create an empty PDU vector                    │
│                                                         │
│  pdu = []  ← Empty box                                 │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 3: Reserve space (optimization)                  │
│                                                         │
│  pdu.reserve(5 + 2) = 7 bytes                          │
│  ↓                                                      │
│  [_][_][_][_][_][_][_]  ← 7 empty slots               │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 4: Add HEADER (0xAA)                             │
│                                                         │
│  pdu.push_back(0xAA)                                   │
│  ↓                                                      │
│  [0xAA][_][_][_][_][_][_]                              │
│    ↑                                                    │
│  Header!                                               │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 5: Insert DATA (sdu)                             │
│                                                         │
│  pdu.insert(pdu.end(), sdu.begin(), sdu.end())         │
│  ↓                                                      │
│  [0xAA][72][101][108][108][111][_]                     │
│    ↑     H   e    l    l    o                          │
│  Header + Data                                         │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 6: Add TRAILER (0xFF)                            │
│                                                         │
│  pdu.push_back(0xFF)                                   │
│  ↓                                                      │
│  [0xAA][72][101][108][108][111][0xFF]                  │
│    ↑     H   e    l    l    o     ↑                    │
│  Header                        Trailer                 │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 7: Return the packed PDU                         │
│                                                         │
│  return pdu;                                           │
│  ↓                                                      │
│  DONE! Package packed! 📦                              │
└─────────────────────────────────────────────────────────┘
```

### Visualization:

```
BEFORE (SDU - raw data):
┌───┬───┬───┬───┬───┐
│ H │ e │ l │ l │ o │
└───┴───┴───┴───┴───┘
  72  101 108 108 111

        ↓ addHeader() ↓

AFTER (PDU - packed data):
┌────┬───┬───┬───┬───┬───┬────┐
│0xAA│ H │ e │ l │ l │ o │0xFF│
└────┴───┴───┴───┴───┴───┴────┘
 170  72  101 108 108 111 255
  ↑                         ↑
Header                   Trailer
```

---

## 📭 Function 6: removeHeader() - Unpacking

```cpp
std::vector<uint8_t> PDCPLayer::removeHeader(const std::vector<uint8_t>& pdu) {
    if (pdu.size() < 2) {
        return {};  // Not enough data - return empty
    }
    return std::vector<uint8_t>(pdu.begin() + 1, pdu.end() - 1);
}
```

### Step by Step:

```
┌─────────────────────────────────────────────────────────┐
│  STEP 1: Receive packed PDU                            │
│                                                         │
│  pdu = [0xAA][72][101][108][108][111][0xFF]            │
│          ↑     H   e    l    l    o     ↑              │
│        Header                        Trailer           │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 2: Check size                                    │
│                                                         │
│  if (pdu.size() < 2)  ← At least 2 bytes?             │
│  ↓                                                      │
│  7 < 2? NO ✓ (OK, continue)                            │
│                                                         │
│  If < 2:                                               │
│  return {}; ← Return empty (error)                     │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 3: Cut off header and trailer                    │
│                                                         │
│  pdu.begin() + 1  ← Start from 2nd element            │
│  pdu.end() - 1    ← Stop before last element          │
│                                                         │
│  [0xAA][72][101][108][108][111][0xFF]                  │
│         ↑                       ↑                       │
│       Start                   Stop                     │
│        here                   here                     │
└─────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────┐
│  STEP 4: Return raw data (SDU)                         │
│                                                         │
│  return [72][101][108][108][111]                       │
│          H   e    l    l    o                          │
│                                                         │
│  DONE! Unpacked! 📭                                    │
└─────────────────────────────────────────────────────────┘
```

### Visualization:

```
BEFORE (PDU - packed):
┌────┬───┬───┬───┬───┬───┬────┐
│0xAA│ H │ e │ l │ l │ o │0xFF│
└────┴───┴───┴───┴───┴───┴────┘
  ↑                         ↑
Remove                    Remove

        ↓ removeHeader() ↓

AFTER (SDU - raw data):
┌───┬───┬───┬───┬───┐
│ H │ e │ l │ l │ o │
└───┴───┴───┴───┴───┘
```

---

## 🎬 Full Data Flow

```
┌─────────────────────────────────────────────────────────┐
│  SCENARIO: Sending the message "Hi"                    │
└─────────────────────────────────────────────────────────┘

STEP 1: Application sends data
│
│  data = [72, 105]  ← "Hi" in ASCII
│         H   i
│
▼

STEP 2: PDCP.addHeader(data)
│
│  [72, 105]
│   ↓
│  [0xAA, 72, 105, 0xFF]  ← Header and trailer added
│
▼

STEP 3: Pass to RLC
│
│  RLC receives: [0xAA, 72, 105, 0xFF]
│  RLC creates PDU and adds to queue
│
▼

STEP 4: MAC sends
│
│  MAC takes PDU from RLC queue
│  MAC simulates transmission (10ms)
│
▼

STEP 5: Receiver gets it
│
│  Received: [0xAA, 72, 105, 0xFF]
│
▼

STEP 6: PDCP.removeHeader(pdu)
│
│  [0xAA, 72, 105, 0xFF]
│   ↓
│  [72, 105]  ← Header and trailer removed
│   H   i
│
▼

STEP 7: Application receives
│
│  data = "Hi"  ✓ Success!
│
```

---

## 🔒 Thread Safety

```
┌─────────────────────────────────────────────────────────┐
│  PROBLEM: Two threads try to access at the same time   │
└─────────────────────────────────────────────────────────┘

Thread A: sendPDU()             Thread B: getStatistics()
│                               │
├─ 🔒 lock(mutex_)              ├─ ⏳ Waiting...
├─ stats_.packets++            │
├─ stats_.bytes += 100         │
├─ return true                 │
└─ 🔓 unlock                    │
                               │
                               ├─ 🔒 lock(mutex_)  ← Can go now!
                               ├─ return stats_
                               └─ 🔓 unlock

┌─────────────────────────────────────────────────────────┐
│  SOLUTION: std::lock_guard                             │
│  ↓                                                      │
│  Automatically locks and unlocks                       │
│  Safe for multiple threads! ✓                          │
└─────────────────────────────────────────────────────────┘
```

---

## 📊 Usage Example

```cpp
// Create PDCP Layer
PDCPLayer pdcp;

// Raw data
std::vector<uint8_t> data = {72, 101, 108, 108, 111};  // "Hello"

// Pack
std::vector<uint8_t> packed = pdcp.addHeader(data);
// packed = [0xAA, 72, 101, 108, 108, 111, 0xFF]

// Unpack
std::vector<uint8_t> unpacked = pdcp.removeHeader(packed);
// unpacked = [72, 101, 108, 108, 111]  // "Hello"

// Statistics
Statistics stats = pdcp.getStatistics();
// stats = { packets: 0, bytes: 0 }  (because sendPDU was not called)
```

---

## 🎯 Summary

### What Does the PDCP Layer Do?

```
┌─────────────────────────────────────────────────────────┐
│  1. addHeader()     → Packs data (adds 0xAA and 0xFF)  │
│  2. removeHeader()  → Unpacks data                     │
│  3. sendPDU()       → Counts statistics                │
│  4. getStatistics() → Returns statistics               │
└─────────────────────────────────────────────────────────┘
```

### Why Is This Important?

```
┌─────────────────────────────────────────────────────────┐
│  PDCP is the FIRST layer in the L2 stack               │
│  ↓                                                      │
│  Prepares data for transmission                        │
│  Adds headers compliant with 3GPP TS 38.323            │
│  Enables packet identification                         │
└─────────────────────────────────────────────────────────┘
```

### Analogy:

```
PDCP = Packer in a factory
│
├─ Receives raw product (data)
├─ Packs it in a box (adds header and trailer)
├─ Counts how many packed (statistics)
└─ Passes to warehouse (RLC)
```

---

## 🚀 Key Points

1. **addHeader()** - The most important function! Adds 0xAA at the start and 0xFF at the end
2. **removeHeader()** - The inverse of addHeader(), removes header and trailer
3. **sendPDU()** - Counts statistics (how many packets, how many bytes)
4. **mutex_** - Lock protecting against thread conflicts
5. **reserve()** - Optimization - reserves space upfront

---

**🎓 PDCP Layer in a nutshell:**
Wraps data in an "envelope" (0xAA...data...0xFF) and counts statistics!

**📦 Simple recipe:**
1. Take data
2. Add 0xAA at the beginning
3. Add 0xFF at the end
4. Done!
