# UE Simulator - How Does It Work?


## 🎯 What Does the UE Simulator Do?

The UE Simulator is a **conductor of the orchestra**. Like a director - it creates all the layers, starts the threads, and coordinates everything!

```
┌─────────────────────────────────────────────────────────────┐
│               UE SIMULATOR = CONDUCTOR                      │
└─────────────────────────────────────────────────────────────┘

UESimulator creates and connects:
┌──────────┐    ┌──────────┐    ┌──────────┐
│   PDCP   │───►│   RLC    │───►│   MAC    │
│  Layer   │    │  Layer   │    │  Layer   │
└──────────┘    └──────────┘    └──────────┘
                                      │
                                      │ notifyObserver()
                                      ▼
                               ┌──────────────┐
                               │ UESimulator  │ ← Observer
                               │ (this)       │
                               └──────────────┘

UESimulator also manages:
├─ TX Thread     (sends packets every 50ms)
└─ Stats Thread  (prints statistics every 3s)
```

---

## 📦 What's Inside?

### Constructor - Creating the UE Simulator

```cpp
UESimulator::UESimulator() : running_(false) {
    pdcpLayer_ = std::make_shared<PDCPLayer>();
    rlcLayer_  = std::make_shared<RLCLayer>(RlcMode::UM);
    macLayer_  = std::make_shared<MACLayer>(rlcLayer_);
    macLayer_->registerObserver(
        std::shared_ptr<IStatisticsObserver>(this, [](IStatisticsObserver*){}));
}
```

**What does it do?**

```
STEP 1: Creating UE Simulator
│
├─ Sets: running_ = false
│   (simulator not yet started)
│
├─ Creates: PDCPLayer
│   (header compression, ciphering)
│
├─ Creates: RLCLayer (UM mode)
│   (segmentation, queue management)
│
├─ Creates: MACLayer (with RLC reference)
│   (scheduling, transmission)
│
└─ Registers itself as observer in MAC
    (to receive statistics updates)

Result:
┌──────────────────────────────────┐
│ UE Simulator ready!              │
│ ├─ PDCP Layer created ✅         │
│ ├─ RLC Layer created ✅          │
│ ├─ MAC Layer created ✅          │
│ └─ Observer registered ✅        │
└──────────────────────────────────┘
```

---

## 🚀 Main Function: start()

This is the **ENTRY POINT** of the simulator! Called once by main().

```cpp
void UESimulator::start() {
    if (running_.load()) return;

    printBanner();
    running_ = true;

    txThread_    = std::thread(&UESimulator::transmissionLoop, this);
    statsThread_ = std::thread(&UESimulator::statisticsLoop, this);
}
```

### Step by step:

```
┌─────────────────────────────────────────────────────────────┐
│                    start() - FLOW                           │
└─────────────────────────────────────────────────────────────┘

STEP 1: Check if already running
│
│   if (running_.load()) return;
│
├─ YES, already running ───────────┐
│                                   │
│                                   ▼
│                           ┌───────────────┐
│                           │ END           │
│                           │ (do nothing)  │
│                           └───────────────┘
│
└─ NO, not running yet ────────────┐
                                   │
                                   ▼
                           ┌───────────────┐
                           │ STEPS 2-4     │
                           │ (start up)    │
                           └───────────────┘

STEP 2: Print banner
│
│   printBanner();
│   
│   ╔════════════════════════════════════════╗
│   ║   UE SIMULATOR 5G - L2 STACK          ║
│   ║   3GPP TS 38.321/322/323 Compliant    ║
│   ╚════════════════════════════════════════╝
│
▼

STEP 3: Set running flag
│
│   running_ = true;
│   
│   Before: running_ = false
│   After:  running_ = true  ✅
│
▼

STEP 4: Spawn worker threads
│
│   txThread_    = std::thread(&UESimulator::transmissionLoop, this);
│   statsThread_ = std::thread(&UESimulator::statisticsLoop, this);
│   
│   ┌─────────────────────────────────┐
│   │ TX Thread started ✅            │
│   │ Stats Thread started ✅         │
│   └─────────────────────────────────┘
│
▼

END ✅
```

---

## 🛑 stop() - Stopping the Simulator

```cpp
void UESimulator::stop() {
    if (!running_.load()) return;

    running_ = false;

    if (txThread_.joinable())    txThread_.join();
    if (statsThread_.joinable()) statsThread_.join();

    std::cout << "\n[SYSTEM] Simulator stopped\n";

    auto stats = macLayer_->getStatistics();
    std::cout << "\n=== FINAL STATISTICS ===\n";
    std::cout << "Transmitted packets: " << stats.transmittedPackets << "\n";
    std::cout << "Total bytes: "         << stats.transmittedBytes   << "\n";
    std::cout << "Queue size: "          << stats.queueSize          << "\n";
    std::cout << "========================\n";
}
```

**How does it work?**

```
┌─────────────────────────────────────────────────────────────┐
│                    stop() - FLOW                            │
└─────────────────────────────────────────────────────────────┘

STEP 1: Check if running
│
│   if (!running_.load()) return;
│
├─ NOT running → return early
└─ IS running  → continue

STEP 2: Signal threads to stop
│
│   running_ = false;
│   
│   TX Thread sees:    running_ == false → exits loop
│   Stats Thread sees: running_ == false → exits loop
│
▼

STEP 3: Wait for threads to finish
│
│   txThread_.join();
│   statsThread_.join();
│   
│   ┌─────────────────────────────────┐
│   │ Waiting for TX Thread...    ✅  │
│   │ Waiting for Stats Thread... ✅  │
│   └─────────────────────────────────┘
│
▼

STEP 4: Print final statistics
│
│   [SYSTEM] Simulator stopped
│   
│   === FINAL STATISTICS ===
│   Transmitted packets: 20
│   Total bytes: 16662
│   Queue size: 0
│   ========================
│
▼

END ✅
```

---

## 📡 generateTraffic() - Generating Test Data

```cpp
void UESimulator::generateTraffic(size_t numPackets) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sizeDist(64, 1500);
    std::uniform_int_distribution<> prioDist(1, 3);

    std::cout << "[SYSTEM] Generating traffic...\n\n";

    for (size_t i = 0; i < numPackets; ++i) {
        size_t   size     = sizeDist(gen);
        Priority priority = static_cast<Priority>(prioDist(gen));

        std::vector<uint8_t> data(size);
        for (auto& byte : data) byte = static_cast<uint8_t>(gen() % 256);

        auto pdcpPdu = pdcpLayer_->addHeader(data);
        rlcLayer_->segmentSDU(pdcpPdu, priority);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
```

### Step by step:

```
┌─────────────────────────────────────────────────────────────┐
│              generateTraffic() - FLOW                       │
└─────────────────────────────────────────────────────────────┘

STEP 1: Set up random generator
│
│   std::mt19937 gen(rd());
│   sizeDist → [64, 1500] bytes
│   prioDist → [1, 3] priority
│
▼

STEP 2: Loop numPackets times
│
│   for (size_t i = 0; i < numPackets; ++i) {
│
▼

STEP 3: Draw random size and priority
│
│   size     = 873   (example)
│   priority = HIGH  (example)
│
▼

STEP 4: Generate random payload
│
│   data = [0x3F, 0xA1, 0x7C, ...]  (873 random bytes)
│
▼

STEP 5: PDCP processing
│
│   pdcpPdu = pdcpLayer_->addHeader(data);
│   
│   Before: [873 bytes]
│   After:  [0xAA][873 bytes][0xFF]  = 875 bytes
│
▼

STEP 6: RLC segmentation
│
│   rlcLayer_->segmentSDU(pdcpPdu, priority);
│   
│   RLC Queue: [...][PDU#new] ← Added
│   Console: [RLC] Segmented PDU ID=5 size=875B priority=3
│
▼

STEP 7: Wait 200ms before next packet
│
│   sleep(200ms)
│   
│   (simulates real network traffic pace)
│
▼

Repeat for next packet...

END ✅
```

**Example with 3 packets:**

```
Packet 1: size=873, priority=HIGH
├─ PDCP: [0xAA][873B][0xFF] = 875B
└─ RLC:  PDU#0 queued

Packet 2: size=785, priority=LOW
├─ PDCP: [0xAA][785B][0xFF] = 787B
└─ RLC:  PDU#1 queued

Packet 3: size=505, priority=MEDIUM
├─ PDCP: [0xAA][505B][0xFF] = 507B
└─ RLC:  PDU#2 queued
```

---

## 🔄 transmissionLoop() - TX Thread

```cpp
void UESimulator::transmissionLoop() {
    while (running_.load()) {
        macLayer_->scheduleTransmission();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
```

**How does it work?**

```
┌─────────────────────────────────────────────────────────────┐
│             transmissionLoop() - FLOW                       │
└─────────────────────────────────────────────────────────────┘

Runs in TX Thread (separate from main thread):

LOOP (every 50ms while running_ == true):
│
├─ STEP 1: Call MAC scheduling
│   macLayer_->scheduleTransmission();
│   
│   MAC takes PDU from RLC queue and sends it
│   Console: [MAC] Transmitted PDU ID=5 latency=77ms
│
├─ STEP 2: Wait 50ms (TTI - Transmission Time Interval)
│   sleep(50ms)
│
└─ STEP 3: Check running_ flag
   ├─ true  → repeat loop
   └─ false → exit loop, thread ends

Timeline:
│
├─ 0ms:   scheduleTransmission() → PDU#0 sent
├─ 50ms:  scheduleTransmission() → PDU#1 sent
├─ 100ms: scheduleTransmission() → PDU#2 sent
├─ 150ms: scheduleTransmission() → PDU#3 sent
└─ ...
```

---

## 📊 statisticsLoop() - Stats Thread

```cpp
void UESimulator::statisticsLoop() {
    while (running_.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(3));

        auto stats = macLayer_->getStatistics();
        std::cout << "\n=== MAC STATISTICS ===\n";
        std::cout << "Transmitted packets: " << stats.transmittedPackets << "\n";
        std::cout << "Total bytes: "         << stats.transmittedBytes   << "\n";
        std::cout << "RLC queue size: "      << stats.queueSize          << "\n";
        std::cout << "======================\n\n";
    }
}
```

**How does it work?**

```
┌─────────────────────────────────────────────────────────────┐
│             statisticsLoop() - FLOW                         │
└─────────────────────────────────────────────────────────────┘

Runs in Stats Thread (separate from TX and main threads):

LOOP (every 3s while running_ == true):
│
├─ STEP 1: Wait 3 seconds
│   sleep(3s)
│
├─ STEP 2: Get statistics from MAC
│   auto stats = macLayer_->getStatistics();
│
├─ STEP 3: Print statistics
│   
│   === MAC STATISTICS ===
│   Transmitted packets: 15
│   Total bytes: 13774
│   RLC queue size: 0
│   ======================
│
└─ STEP 4: Check running_ flag
   ├─ true  → repeat loop
   └─ false → exit loop, thread ends

Timeline:
│
├─ 3000ms:  Print statistics (15 packets sent so far)
├─ 6000ms:  Print statistics (20 packets sent so far)
└─ 9000ms:  Print statistics (20 packets, queue empty)
```

---

## 🔔 onStatisticsUpdate() - Observer Callback

```cpp
void UESimulator::onStatisticsUpdate(const Statistics& stats) {
    (void)stats; // Can be extended for real-time monitoring
}
```

**How does it work?**

```
┌─────────────────────────────────────────────────────────────┐
│           onStatisticsUpdate() - FLOW                       │
└─────────────────────────────────────────────────────────────┘

STEP 1: MAC Layer sends statistics after each transmission
│
│   MAC Layer ──────────────► UESimulator
│   "I just sent a packet, here are stats"
│
▼

STEP 2: UESimulator receives callback
│
│   onStatisticsUpdate(stats) called
│   stats.transmittedPackets = 6
│   stats.transmittedBytes   = 5002
│   stats.queueSize          = 2
│
▼

STEP 3: Currently does nothing (extensible)
│
│   (void)stats;
│   
│   ┌─────────────────────────────────┐
│   │ Future extensions:              │
│   │ ├─ Log to file                  │
│   │ ├─ Send to dashboard            │
│   │ └─ Trigger alerts               │
│   └─────────────────────────────────┘
│
▼

END ✅
```

---

## 🎬 Full Flow - Step by Step

### Scenario: Complete Simulation Run

```
┌─────────────────────────────────────────────────────────────┐
│              FULL SIMULATION FLOW                           │
└─────────────────────────────────────────────────────────────┘

TIME: 0ms - main() calls simulator.start()
│
├─ printBanner() → prints header
├─ running_ = true
├─ TX Thread spawned
└─ Stats Thread spawned

TIME: 0ms - main() calls simulator.generateTraffic(20)
│
├─ Packet #0: 873B, priority=HIGH
│  ├─ PDCP: adds header → 875B
│  └─ RLC:  PDU#0 queued
│  sleep(200ms)
│
├─ Packet #1: 785B, priority=LOW
│  ├─ PDCP: adds header → 787B
│  └─ RLC:  PDU#1 queued
│  sleep(200ms)
│
└─ ... (20 packets total)

PARALLEL - TX Thread (every 50ms):
│
├─ 50ms:  MAC sends PDU#0 → [MAC] Transmitted PDU ID=0 latency=25ms
├─ 100ms: MAC sends PDU#1 → [MAC] Transmitted PDU ID=1 latency=78ms
├─ 150ms: MAC sends PDU#2 → [MAC] Transmitted PDU ID=2 latency=79ms
└─ ...

PARALLEL - Stats Thread (every 3s):
│
├─ 3000ms:
│  === MAC STATISTICS ===
│  Transmitted packets: 15
│  Total bytes: 13774
│  RLC queue size: 0
│  ======================
│
└─ 6000ms:
   === MAC STATISTICS ===
   Transmitted packets: 20
   Total bytes: 16662
   RLC queue size: 0
   ======================

TIME: ~4000ms - generateTraffic() returns
│
└─ main() calls simulator.stop()
   ├─ running_ = false
   ├─ Waits for TX Thread
   ├─ Waits for Stats Thread
   └─ Prints final statistics:
      === FINAL STATISTICS ===
      Transmitted packets: 20
      Total bytes: 16662
      Queue size: 0
      ========================
```

---

## 🧵 Thread Interaction

```
┌─────────────────────────────────────────────────────────────┐
│              THREADS AND UE SIMULATOR                       │
└─────────────────────────────────────────────────────────────┘

MAIN THREAD:
│
├─ Creates UESimulator (constructor)
├─ Calls start()       → spawns TX + Stats threads
├─ Calls generateTraffic(20) → feeds PDCP → RLC
└─ Calls stop()        → joins TX + Stats threads

═══════════════════════════════════════════════════════════════

TX THREAD (every 50ms):
│
├─ 50ms:  MAC scheduleTransmission()
├─ 100ms: MAC scheduleTransmission()
└─ ...

═══════════════════════════════════════════════════════════════

STATS THREAD (every 3s):
│
├─ 3000ms: Print MAC statistics
├─ 6000ms: Print MAC statistics
└─ ...

═══════════════════════════════════════════════════════════════

Shared state (thread-safe):
├─ RLC queue   → protected by mutex
├─ MAC counters → std::atomic
└─ running_    → std::atomic<bool>
```

---

## 🔢 Atomic running_ Flag - Why?

```
┌─────────────────────────────────────────────────────────────┐
│              ATOMIC FLAG - SAFETY                           │
└─────────────────────────────────────────────────────────────┘

Problem without atomic:
│
├─ Main Thread:   running_ = false  (writes)
│                 ▲
│                 │ AT THE SAME TIME!
│                 ▼
└─ TX Thread:     while (running_)  (reads)

RACE CONDITION! ❌


Solution with atomic:
│
├─ Main Thread:   running_ = false         (atomic write)
│                 ▲
│                 │ SAFE!
│                 ▼
└─ TX Thread:     while (running_.load())  (atomic read)

SAFE! ✅

┌─────────────────────────────────────┐
│ std::atomic<bool>                   │
│ ├─ Guarantees visibility            │
│ ├─ No race conditions               │
│ └─ Thread-safe without mutex        │
└─────────────────────────────────────┘
```

---

## 📝 Function Summary

```
┌─────────────────────────────────────────────────────────────┐
│                  ALL FUNCTIONS                              │
└─────────────────────────────────────────────────────────────┘

1. UESimulator() - Constructor
   │
   └─ Creates all 3 layers, registers as observer

2. ~UESimulator() - Destructor
   │
   └─ Calls stop() to clean up threads

3. start() - Start simulator
   │
   ├─ Prints banner
   ├─ Sets running_ = true
   └─ Spawns TX Thread and Stats Thread

4. stop() - Stop simulator
   │
   ├─ Sets running_ = false
   ├─ Joins TX Thread and Stats Thread
   └─ Prints final statistics

5. generateTraffic() - Generate test data
   │
   ├─ Creates random packets (64-1500 bytes)
   ├─ Sends through PDCP → RLC pipeline
   └─ 200ms delay between packets

6. transmissionLoop() - TX Thread body
   │
   └─ Calls MAC scheduleTransmission() every 50ms

7. statisticsLoop() - Stats Thread body
   │
   └─ Prints MAC statistics every 3s

8. onStatisticsUpdate() - Observer callback
   │
   └─ Receives stats from MAC (extensible)

9. printBanner() - Print startup header
   │
   └─ Displays simulator name and 3GPP compliance
```

---

## 🎯 Key Concepts

### 1. Facade Pattern

```
UESimulator = Single entry point
│
├─ Hides complexity of 3 protocol layers
├─ Hides thread management
└─ Simple API: start() / stop() / generateTraffic()

Without Facade:
main() must know about PDCP, RLC, MAC, threads...

With Facade:
main() only knows about UESimulator ✅
```

### 2. Observer Pattern

```
UESimulator implements IStatisticsObserver
│
└─ Registered in MAC Layer constructor

MAC Layer ────────► UESimulator
"I sent a packet!"   onStatisticsUpdate()

Benefits:
├─ Loose coupling
├─ MAC doesn't know about UESimulator details
└─ Easy to add more observers
```

### 3. Dependency Injection

```
UESimulator creates layers and injects dependencies:
│
├─ pdcpLayer_ = new PDCPLayer()
├─ rlcLayer_  = new RLCLayer(UM)
└─ macLayer_  = new MACLayer(rlcLayer_)  ← RLC injected!

Benefits:
├─ Easy testing (swap with mocks)
├─ Loose coupling between layers
└─ Clear ownership via shared_ptr
```

---

## 🚦 Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                  DATA FLOW                                  │
└─────────────────────────────────────────────────────────────┘

generateTraffic() [Main Thread]:
┌───────────────────────────────────┐
│ Raw data: [random bytes]          │
└───────────────┬───────────────────┘
                │ addHeader()
                ▼
┌───────────────────────────────────┐
│ PDCP PDU: [0xAA][data][0xFF]      │
└───────────────┬───────────────────┘
                │ segmentSDU()
                ▼
┌───────────────────────────────────┐
│ RLC Queue: [PDU#0][PDU#1]...      │
└───────────────────────────────────┘

transmissionLoop() [TX Thread]:
┌───────────────────────────────────┐
│ MAC scheduleTransmission()        │
│ ├─ Dequeues PDU from RLC          │
│ ├─ Simulates 10ms transmission    │
│ └─ Notifies UESimulator           │
└───────────────┬───────────────────┘
                │ onStatisticsUpdate()
                ▼
┌───────────────────────────────────┐
│ UESimulator (Observer)            │
│ └─ Receives statistics callback   │
└───────────────────────────────────┘
```

---

## 💡 Most Important!

```
┌─────────────────────────────────────────────────────────────┐
│                  WHAT YOU MUST REMEMBER                     │
└─────────────────────────────────────────────────────────────┘

1. UESimulator = Conductor
   └─ Creates layers, manages threads, coordinates everything

2. start() = Entry point
   └─ Spawns TX Thread and Stats Thread

3. generateTraffic() = Data source
   └─ Feeds random packets through PDCP → RLC pipeline

4. transmissionLoop() = TX Thread
   └─ Calls MAC every 50ms to send packets

5. statisticsLoop() = Stats Thread
   └─ Prints statistics every 3 seconds

6. Facade Pattern = Simple API
   └─ Hides all complexity behind start/stop/generateTraffic

7. running_ = atomic<bool>
   └─ Thread-safe flag to stop all threads cleanly
```

---

**Document created for**: Beginners  
**Difficulty level**: ⭐☆☆☆☆ (Very easy)  
**Reading time**: 10 minutes  
**Goal**: Understand ue_simulator.cpp without reading the code!
