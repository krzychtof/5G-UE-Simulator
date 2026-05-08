# 🧵 Multi-Threading in 5G UE Simulator

## 📋 Table of Contents
1. [Thread Overview](#thread-overview)
2. [Why Multi-Threading?](#why-multi-threading)
3. [ASCII Flow Diagram](#ascii-flow-diagram)
4. [Detailed Thread Description](#detailed-thread-description)
5. [Synchronization and Thread Safety](#synchronization-and-thread-safety)
6. [Thread Lifecycle](#thread-lifecycle)

---

## 🎯 Thread Overview

The simulator uses **3 threads**:

```cpp
std::atomic<bool> running_;      // Control flag (thread-safe)
std::thread txThread_;           // Transmission thread (TX)
std::thread statsThread_;        // Statistics thread
// + Main thread
```

| Thread | Frequency | Task |
|--------|-----------|------|
| **Main Thread** | One-time | Generates packets, creates data |
| **TX Thread** | Every 50ms | Sends packets from RLC queue |
| **Stats Thread** | Every 3s | Displays statistics |

---

## 🤔 Why Multi-Threading?

### 1. **Simulates Real 5G System**
In a real 5G phone, different processes run in parallel:
- **Applications** generate data (e.g., YouTube, WhatsApp)
- **Modem** transmits data over radio
- **System** monitors performance

### 2. **Independence of Operations**
```
Data Generation ≠ Data Transmission ≠ Monitoring
```
Each operation has its own rhythm:
- Data can be generated **quickly** (burst)
- Transmission works **regularly** (every 50ms)
- Statistics are **infrequent** (every 3s)

### 3. **Avoiding Blocking**
Without multi-threading:
```
Generate packet → Wait 50ms → Send → Wait 3s → Show stats → Repeat
❌ SLOW! Everything waits for everything
```

With multi-threading:
```
Main:  Generate → Generate → Generate → DONE
TX:    Send → Wait 50ms → Send → Wait 50ms → ...
Stats: Wait 3s → Show → Wait 3s → Show → ...
✅ FAST! Everything runs in parallel
```

---

## 🎨 ASCII Flow Diagram

### Complete Simulator Lifecycle

```
╔═══════════════════════════════════════════════════════════════════════╗
║                    MULTI-THREADING IN 5G SIMULATOR                    ║
╚═══════════════════════════════════════════════════════════════════════╝

TIME →
═══════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────┐
│ MAIN THREAD                                                          │
└─────────────────────────────────────────────────────────────────────┘

T=0ms    │ main() starts
         │
         ├─► UESimulator simulator;
         │   ├─ Creates PDCP layer
         │   ├─ Creates RLC layer
         │   └─ Creates MAC layer
         │
T=10ms   ├─► simulator.start()
         │   │
         │   ├─► running_ = true  ← Atomic flag
         │   │
         │   ├─► txThread_ = std::thread(&UESimulator::transmissionLoop)
         │   │   │
         │   │   └──────────────────────────────────────┐
         │   │                                           │
         │   └─► statsThread_ = std::thread(&UESimulator::statisticsLoop)
         │       │                                       │
         │       └────────────────────────┐             │
         │                                │             │
T=20ms   ├─► simulator.generateTraffic(20)             │             │
         │   │                            │             │             │
         │   ├─ For i=0 to 19:           │             │             │
         │   │  ├─ Generate data          │             │             │
         │   │  ├─ pdcp.addHeader()      │             │             │
         │   │  └─ rlc.segmentSDU()      │             │             │
         │   │     └─► Add to queue      │             │             │
         │   │         (thread-safe)     │             │             │
         │   │                            │             │             │
T=500ms  │   └─ Generation DONE          │             │             │
         │                                │             │             │
T=10s    ├─► simulator.stop()            │             │             │
         │   │                            │             │             │
         │   ├─► running_ = false  ← STOP signal       │             │
         │   │                            │             │             │
         │   ├─► txThread_.join()        │             │             │
         │   │   └─ Wait for completion──┼─────────────┘             │
         │   │                            │                           │
         │   └─► statsThread_.join()     │                           │
         │       └─ Wait for completion──┼───────────────────────────┘
         │                                │
T=10.1s  └─► Program ends                │


┌─────────────────────────────────────────────────────────────────────┐
│ TX THREAD (Transmission Thread) - transmissionLoop()                │
└─────────────────────────────────────────────────────────────────────┘

T=10ms   │ Thread starts
         │
         ├─► while (running_) {  ← Checks atomic flag
         │
T=60ms   │   ├─► macLayer_->scheduleTransmission()
         │   │   │
         │   │   ├─► Get PDU from RLC (thread-safe)
         │   │   ├─► Simulate transmission (10ms)
         │   │   ├─► Calculate latency
         │   │   └─► Update statistics (atomic)
         │   │
         │   └─► std::this_thread::sleep_for(50ms)  ← Wait
         │
T=110ms  │   ├─► macLayer_->scheduleTransmission()
         │   └─► sleep(50ms)
         │
T=160ms  │   ├─► macLayer_->scheduleTransmission()
         │   └─► sleep(50ms)
         │
         │   ... (repeats every 50ms)
         │
T=10s    │   running_ == false  ← STOP
         │   └─► Exit loop
         │
T=10.05s └─► Thread ends


┌─────────────────────────────────────────────────────────────────────┐
│ STATS THREAD (Statistics Thread) - statisticsLoop()                 │
└─────────────────────────────────────────────────────────────────────┘

T=10ms   │ Thread starts
         │
         ├─► while (running_) {  ← Checks atomic flag
         │
T=3s     │   ├─► std::this_thread::sleep_for(3s)  ← Wait
         │   │
         │   └─► auto stats = macLayer_->getStatistics()
         │       │
         │       ├─► Read counters (atomic)
         │       └─► Display on console:
         │           "=== MAC STATISTICS ==="
         │           "Transmitted packets: X"
         │           "Total bytes: Y"
         │           "RLC queue size: Z"
         │
T=6s     │   ├─► sleep(3s)
         │   └─► Display statistics
         │
T=9s     │   ├─► sleep(3s)
         │   └─► Display statistics
         │
T=10s    │   running_ == false  ← STOP
         │   └─► Exit loop
         │
T=10.01s └─► Thread ends


═══════════════════════════════════════════════════════════════════════
INTER-THREAD INTERACTIONS
═══════════════════════════════════════════════════════════════════════

┌──────────────┐         ┌──────────────┐         ┌──────────────┐
│ MAIN THREAD  │         │  TX THREAD   │         │ STATS THREAD │
└──────┬───────┘         └──────┬───────┘         └──────┬───────┘
       │                        │                        │
       │ running_ = true        │                        │
       ├───────────────────────►│                        │
       ├────────────────────────┼───────────────────────►│
       │                        │                        │
       │ rlc.segmentSDU()       │                        │
       ├─► [RLC QUEUE]          │                        │
       │   (mutex lock)         │                        │
       │                        │                        │
       │                        │ mac.scheduleTransmission()
       │                        ├─► [RLC QUEUE]          │
       │                        │   (mutex lock)         │
       │                        │   Get PDU              │
       │                        │                        │
       │                        │ stats++ (atomic)       │
       │                        │                        │
       │                        │                        │ getStatistics()
       │                        │                        ├─► Read (atomic)
       │                        │                        │   Display
       │                        │                        │
       │ running_ = false       │                        │
       ├───────────────────────►│                        │
       ├────────────────────────┼───────────────────────►│
       │                        │                        │
       │ join()                 │ Ends                   │
       ├───────────────────────►│                        │
       │                        │                        │
       │ join()                 │                        │ Ends
       ├────────────────────────┼───────────────────────►│
       │                        │                        │
       ▼                        ▼                        ▼
     END                      END                      END
```

---

## 📝 Detailed Thread Description

### 1️⃣ **MAIN THREAD**

**Role:** Data producer

```cpp
int main() {
    UESimulator simulator;
    
    simulator.start();           // Starts TX and Stats threads
    simulator.generateTraffic(); // Generates 20 packets
    
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    simulator.stop();            // Stops all threads
}
```

**What it does:**
1. Creates protocol layers (PDCP, RLC, MAC)
2. Starts 2 worker threads
3. Generates test packets
4. Waits 10 seconds
5. Stops threads and exits

**Why separate thread?**
- Can generate data **quickly** without waiting for transmission
- Simulates user applications (YouTube, WhatsApp)

---

### 2️⃣ **TX THREAD** (Transmission Thread)

**Role:** Data consumer, modem simulation

```cpp
void UESimulator::transmissionLoop() {
    while (running_) {
        macLayer_->scheduleTransmission();  // Send packet
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
```

**What it does:**
1. Wakes up every 50ms
2. Calls `MAC::scheduleTransmission()`
3. MAC retrieves PDU from RLC queue
4. Simulates transmission (10ms delay)
5. Updates statistics

**Why separate thread?**
- Simulates **regular rhythm** of radio transmission
- In real 5G, modem sends data in specific time slots
- Doesn't block generation of new data

**Frequency:** 50ms = 20 transmissions/second

---

### 3️⃣ **STATS THREAD** (Statistics Thread)

**Role:** Performance monitor

```cpp
void UESimulator::statisticsLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        auto stats = macLayer_->getStatistics();
        std::cout << "=== MAC STATISTICS ===" << std::endl;
        std::cout << "Transmitted packets: " << stats.transmittedPackets << std::endl;
        // ...
    }
}
```

**What it does:**
1. Wakes up every 3 seconds
2. Retrieves statistics from MAC
3. Displays on console

**Why separate thread?**
- Doesn't interfere with transmission
- Can display data **in real-time**
- Simulates system monitoring

**Frequency:** 3s = ~0.33 times/second

---

## 🔒 Synchronization and Thread Safety

### 1. **std::atomic<bool> running_**

```cpp
std::atomic<bool> running_;  // Thread-safe flag
```

**Why atomic?**
- Read by **3 threads** simultaneously
- Without `atomic`, there could be race conditions
- Guarantees atomic operations (read/write)

**Usage:**
```cpp
// Main thread
running_ = true;   // Start
running_ = false;  // Stop

// TX thread
while (running_) { ... }  // Checks

// Stats thread
while (running_) { ... }  // Checks
```

---

### 2. **Mutex in RLC Queue**

```cpp
// In RLC Layer
std::mutex queueMutex_;
std::queue<PDU> txQueue_;

void segmentSDU(...) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    txQueue_.push(pdu);  // Thread-safe
}

bool receivePDU(PDU& pdu) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (!txQueue_.empty()) {
        pdu = txQueue_.front();
        txQueue_.pop();  // Thread-safe
        return true;
    }
    return false;
}
```

**Why mutex?**
- **Main thread** adds to queue (`push`)
- **TX thread** retrieves from queue (`pop`)
- Without mutex: **race condition** → crash or data loss

---

### 3. **Atomic Counters in MAC**

```cpp
// In MAC Layer
std::atomic<uint64_t> transmittedPackets_{0};
std::atomic<uint64_t> totalBytes_{0};

void scheduleTransmission() {
    // ...
    transmittedPackets_++;  // Thread-safe increment
    totalBytes_ += pdu.data.size();  // Thread-safe
}

Statistics getStatistics() const {
    return {
        transmittedPackets_.load(),  // Thread-safe read
        totalBytes_.load()
    };
}
```

**Why atomic?**
- **TX thread** increments counters
- **Stats thread** reads counters
- Without `atomic`: could get incorrect values

---

## ⏱️ Thread Lifecycle

### Timeline Diagram

```
TIME (seconds)
│
0s   ├─ main() START
     │  └─ Creates UESimulator
     │
0.01s├─ simulator.start()
     │  ├─ running_ = true
     │  ├─ txThread_ started       ──┐
     │  └─ statsThread_ started    ──┼──┐
     │                                │  │
0.02s├─ generateTraffic()            │  │
     │  └─ Generates 20 packets      │  │
     │     (adds to RLC queue)       │  │
     │                                │  │
0.5s ├─ Generation complete          │  │
     │                                │  │
     │  ┌────────────────────────────┘  │
     │  │ TX THREAD (every 50ms)        │
0.05s│  ├─ Transmission #1              │
0.1s │  ├─ Transmission #2              │
0.15s│  ├─ Transmission #3              │
     │  │ ... (every 50ms)              │
     │  │                                │
     │  │  ┌─────────────────────────────┘
     │  │  │ STATS THREAD (every 3s)
3s   │  │  ├─ Display statistics #1
6s   │  │  ├─ Display statistics #2
9s   │  │  ├─ Display statistics #3
     │  │  │
10s  ├─ simulator.stop()
     │  ├─ running_ = false
     │  │  │
     │  ├─ txThread_.join()
     │  │  └─ TX ends (exits loop)
     │  │
     │  └─ statsThread_.join()
     │     └─ Stats ends (exits loop)
     │
10.1s└─ main() END
```

---

## 🎯 Summary

### Why 3 Threads?

| Thread | Frequency | Reason |
|--------|-----------|--------|
| **Main** | One-time | Fast data generation (burst) |
| **TX** | Every 50ms | Regular transmission (like real modem) |
| **Stats** | Every 3s | Monitoring without blocking |

### Key Mechanisms

1. **std::atomic<bool> running_** → Thread control
2. **std::mutex** in RLC queue → Safe add/retrieve
3. **std::atomic** counters → Safe statistics
4. **std::thread::join()** → Wait for completion

### Benefits

✅ **Parallelism** - Everything runs simultaneously  
✅ **Realism** - Simulates real 5G system  
✅ **Performance** - No blocking operations  
✅ **Safety** - Thread-safe with mutex and atomic  

### Thread Communication Pattern

```
┌─────────────────────────────────────────────────────────────┐
│                  SHARED RESOURCES                           │
└─────────────────────────────────────────────────────────────┘

1. ATOMIC FLAG (running_)
   ├─ Main Thread:  WRITE (true/false)
   ├─ TX Thread:    READ (check if running)
   └─ Stats Thread: READ (check if running)

2. RLC QUEUE (txQueue_)
   ├─ Main Thread:  WRITE (push PDUs) [mutex protected]
   └─ TX Thread:    READ (pop PDUs)   [mutex protected]

3. MAC STATISTICS (counters)
   ├─ TX Thread:    WRITE (increment) [atomic]
   └─ Stats Thread: READ (display)    [atomic]
```

### Thread Safety Guarantees

| Resource | Protection | Accessed By |
|----------|------------|-------------|
| `running_` | `std::atomic<bool>` | All threads |
| `txQueue_` | `std::mutex` | Main + TX |
| `transmittedPackets_` | `std::atomic<uint64_t>` | TX + Stats |
| `totalBytes_` | `std::atomic<uint64_t>` | TX + Stats |

---

## 🔍 Code Examples

### Starting Threads

```cpp
void UESimulator::start() {
    std::cout << "[SYSTEM] Starting simulator..." << std::endl;
    
    running_ = true;  // Atomic write
    
    // Spawn TX thread
    txThread_ = std::thread(&UESimulator::transmissionLoop, this);
    
    // Spawn Stats thread
    statsThread_ = std::thread(&UESimulator::statisticsLoop, this);
    
    std::cout << "[SYSTEM] Threads started" << std::endl;
}
```

### Stopping Threads

```cpp
void UESimulator::stop() {
    std::cout << "[SYSTEM] Stopping simulator..." << std::endl;
    
    running_ = false;  // Atomic write - signals threads to stop
    
    // Wait for TX thread to finish
    if (txThread_.joinable()) {
        txThread_.join();
    }
    
    // Wait for Stats thread to finish
    if (statsThread_.joinable()) {
        statsThread_.join();
    }
    
    std::cout << "[SYSTEM] Simulator stopped" << std::endl;
}
```

### Thread-Safe Queue Operations

```cpp
// Main thread - Producer
void UESimulator::generateTraffic(size_t numPackets) {
    for (size_t i = 0; i < numPackets; ++i) {
        auto data = generateRandomData();
        auto pdu = pdcpLayer_->addHeader(data);
        
        // Thread-safe: mutex inside segmentSDU
        rlcLayer_->segmentSDU(pdu, priority);
    }
}

// TX thread - Consumer
void UESimulator::transmissionLoop() {
    while (running_) {  // Atomic read
        macLayer_->scheduleTransmission();
        // Inside scheduleTransmission():
        // - Calls rlc.receivePDU() [mutex protected]
        // - Increments counters [atomic]
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
```

---

## 🚀 Performance Considerations

### Why These Specific Intervals?

| Interval | Reason |
|----------|--------|
| **50ms TX** | Simulates 5G slot duration (~1ms) with processing overhead |
| **3s Stats** | Human-readable update rate, doesn't spam console |
| **10ms Transmission** | Realistic radio transmission delay |

### CPU Usage

```
Main Thread:  ████░░░░░░░░░░░░░░░░  (20% - burst at start)
TX Thread:    ████████████████████  (100% - continuous)
Stats Thread: █░░░░░░░░░░░░░░░░░░░  (5% - periodic)
```

### Memory Access Pattern

```
┌─────────────────────────────────────────────────────────────┐
│                    MEMORY LAYOUT                            │
└─────────────────────────────────────────────────────────────┘

STACK (per thread):
├─ Main Thread:  Local variables, function calls
├─ TX Thread:    Loop variables, PDU copies
└─ Stats Thread: Statistics struct copy

HEAP (shared):
├─ PDCP Layer:   Shared pointer (thread-safe ref count)
├─ RLC Layer:    Shared pointer + mutex-protected queue
└─ MAC Layer:    Shared pointer + atomic counters

ATOMIC VARIABLES (lock-free):
├─ running_:             1 byte (bool)
├─ transmittedPackets_:  8 bytes (uint64_t)
└─ totalBytes_:          8 bytes (uint64_t)
```

---

## 🎓 Learning Points

### Key Takeaways

1. **Atomic Variables** - Use for simple flags and counters
2. **Mutex** - Use for complex data structures (queues, maps)
3. **Thread Lifecycle** - Always join threads before destruction
4. **Separation of Concerns** - Each thread has one responsibility
5. **Real-World Simulation** - Multi-threading mimics actual 5G systems

### Common Pitfalls Avoided

❌ **Deadlock** - Avoided by using single mutex per resource  
❌ **Race Condition** - Prevented with atomic and mutex  
❌ **Memory Leak** - Smart pointers handle cleanup  
❌ **Dangling Threads** - Always join before exit  

### Best Practices Applied

✅ **RAII** - Threads managed by class lifecycle  
✅ **Lock Guards** - Automatic mutex unlock  
✅ **Atomic Operations** - Lock-free where possible  
✅ **Clear Ownership** - Each thread owns its loop  

---

**In short:** Multi-threading allows the simulator to behave like a real 5G phone, where different processes (data generation, transmission, monitoring) run independently and in parallel! 🚀
