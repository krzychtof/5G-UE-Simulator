# MAC Layer - How Does It Work?


## 🎯 What Does the MAC Layer Do?

The MAC Layer is a **packet sender**. Like a postman - it takes letters from the queue and sends them!

```
┌─────────────────────────────────────────────────────────────┐
│                    MAC LAYER = POSTMAN                      │
└─────────────────────────────────────────────────────────────┘

RLC Queue (letter queue):
┌─────────┬─────────┬─────────┬─────────┐
│ Letter1 │ Letter2 │ Letter3 │ Letter4 │
└─────────┴─────────┴─────────┴─────────┘
    ▲
    │
    │ MAC takes the first letter
    │
┌───┴────┐
│  MAC   │ ← Postman
│ Layer  │
└───┬────┘
    │
    │ Sends the letter
    ▼
📡 5G Network
```

---

## 📦 What's Inside?

### Constructor - Creating the MAC Layer

```cpp
MACLayer::MACLayer(std::shared_ptr<RLCLayer> rlcLayer)
    : rlcLayer_(rlcLayer), transmittedPackets_(0), transmittedBytes_(0) {}
```

**What does it do?**

```
STEP 1: Creating MAC Layer
│
├─ Receives: Pointer to RLC Layer
│   (to be able to take packets from the RLC queue)
│
├─ Sets: transmittedPackets_ = 0
│   (counter of sent packets)
│
└─ Sets: transmittedBytes_ = 0
    (counter of sent bytes)

Result:
┌──────────────────────────────────┐
│ MAC Layer ready to work!         │
│ ├─ Connected to RLC ✅           │
│ ├─ Counters zeroed ✅            │
│ └─ Waiting for packets ✅        │
└──────────────────────────────────┘
```

---

## 🚀 Main Function: scheduleTransmission()

This is the **HEART** of the MAC Layer! Called every 50ms by the TX Thread.

```cpp
void MACLayer::scheduleTransmission() {
    PDU pdu;
    
    if (receivePDU(pdu)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TRANSMISSION_DELAY_MS));
        
        transmittedPackets_++;
        transmittedBytes_ += pdu.data.size();
        
        auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - pdu.timestamp).count();
        
        std::cout << "[MAC] Transmitted PDU ID=" << pdu.id 
                  << " latency=" << latency << "ms\n";
        
        notifyObserver();
    }
}
```

### Step by step:

```
┌─────────────────────────────────────────────────────────────┐
│              scheduleTransmission() - FLOW                  │
└─────────────────────────────────────────────────────────────┘

STEP 1: Create empty PDU
│
│   PDU pdu;  ← Empty "envelope" for the packet
│
▼

STEP 2: Try to get a packet from RLC
│
│   if (receivePDU(pdu)) {
│       ▲
│       │
│       └─ Ask RLC: "Do you have a packet for me?"
│
├─ YES, there is a packet! ────────┐
│                                   │
│                                   ▼
│                           ┌───────────────┐
│                           │ STEPS 3-7     │
│                           │ (sending)     │
│                           └───────────────┘
│
└─ NO, no packets ─────────────────┐
                                   │
                                   ▼
                           ┌───────────────┐
                           │ END           │
                           │ (do nothing)  │
                           └───────────────┘
```

### If there IS a packet:

```
┌─────────────────────────────────────────────────────────────┐
│                  SENDING A PACKET                           │
└─────────────────────────────────────────────────────────────┘

STEP 3: Simulate transmission (wait 10ms)
│
│   std::this_thread::sleep_for(10ms);
│   
│   ┌─────────────────────────────────┐
│   │ We pretend to send over radio   │
│   │ (in real 5G this would go       │
│   │ through an antenna)             │
│   │                                 │
│   │ [████████████████] 10ms         │
│   └─────────────────────────────────┘
│
▼

STEP 4: Increment packet counter
│
│   transmittedPackets_++;
│   
│   Before: transmittedPackets_ = 5
│   After:  transmittedPackets_ = 6  ✅
│
▼

STEP 5: Add bytes to counter
│
│   transmittedBytes_ += pdu.data.size();
│   
│   Example:
│   Before: transmittedBytes_ = 1000
│   Packet: 250 bytes
│   After:  transmittedBytes_ = 1250  ✅
│
▼

STEP 6: Calculate latency
│
│   auto latency = now - pdu.timestamp;
│   
│   ┌─────────────────────────────────┐
│   │ Packet created: 100ms           │
│   │ Now:            177ms           │
│   │ Latency:        77ms            │
│   └─────────────────────────────────┘
│
▼

STEP 7: Print info
│
│   std::cout << "[MAC] Transmitted PDU ID=5 latency=77ms\n";
│   
│   Console:
│   [MAC] Transmitted PDU ID=5 latency=77ms
│
▼

STEP 8: Notify observer
│
│   notifyObserver();
│   
│   (Sends statistics to UESimulator)
│
▼

END ✅
```

---

## 🔄 receivePDU() - Getting a Packet

```cpp
bool MACLayer::receivePDU(PDU& pdu) {
    return rlcLayer_->receivePDU(pdu);
}
```

**How does it work?**

```
┌─────────────────────────────────────────────────────────────┐
│                  receivePDU() - FLOW                        │
└─────────────────────────────────────────────────────────────┘

MAC Layer:
│
│ "Hey RLC, give me a packet!"
│
├─ Calls: rlcLayer_->receivePDU(pdu)
│
▼

RLC Layer:
│
├─ Checks the queue:
│  
│  Queue: [PDU#3][PDU#4][PDU#5]
│          ▲
│          │
│          └─ Takes the first one (PDU#3)
│
├─ If queue is NOT EMPTY:
│  └─ Returns: true + fills pdu
│
└─ If queue is EMPTY:
   └─ Returns: false

▼

Back to MAC:
│
├─ true  → "I have a packet! Sending!"
└─ false → "No packets, waiting..."
```

**Example:**

```
Situation 1: Queue has packets
┌───────────────────────────────────┐
│ RLC Queue: [PDU#5][PDU#6][PDU#7]  │
└───────────────────────────────────┘
         │
         │ receivePDU(pdu)
         ▼
┌───────────────────────────────────┐
│ MAC gets: PDU#5                   │
│ Returns: true ✅                  │
└───────────────────────────────────┘


Situation 2: Queue empty
┌───────────────────────────────────┐
│ RLC Queue: [EMPTY]                │
└───────────────────────────────────┘
         │
         │ receivePDU(pdu)
         ▼
┌───────────────────────────────────┐
│ MAC gets: nothing                 │
│ Returns: false ❌                 │
└───────────────────────────────────┘
```

---

## 📊 getStatistics() - Getting Statistics

```cpp
Statistics MACLayer::getStatistics() const {
    Statistics stats;
    stats.transmittedPackets = transmittedPackets_.load();
    stats.transmittedBytes = transmittedBytes_.load();
    stats.queueSize = rlcLayer_->getQueueSize();
    return stats;
}
```

**How does it work?**

```
┌─────────────────────────────────────────────────────────────┐
│              getStatistics() - FLOW                         │
└─────────────────────────────────────────────────────────────┘

STEP 1: Create empty Statistics object
│
│   Statistics stats;
│
▼

STEP 2: Get number of sent packets
│
│   stats.transmittedPackets = transmittedPackets_.load();
│   
│   ┌─────────────────────────────────┐
│   │ Atomic counter (thread-safe!)   │
│   │ Value: 20 packets               │
│   └─────────────────────────────────┘
│
▼

STEP 3: Get number of sent bytes
│
│   stats.transmittedBytes = transmittedBytes_.load();
│   
│   ┌─────────────────────────────────┐
│   │ Atomic counter (thread-safe!)   │
│   │ Value: 18806 bytes              │
│   └─────────────────────────────────┘
│
▼

STEP 4: Ask RLC for queue size
│
│   stats.queueSize = rlcLayer_->getQueueSize();
│   
│   ┌─────────────────────────────────┐
│   │ RLC Queue: [PDU#7][PDU#8]       │
│   │ Size: 2 packets                 │
│   └─────────────────────────────────┘
│
▼

STEP 5: Return statistics
│
│   return stats;
│   
│   ┌─────────────────────────────────┐
│   │ Statistics:                     │
│   │ ├─ Packets: 20                  │
│   │ ├─ Bytes: 18806                 │
│   │ └─ Queue: 2                     │
│   └─────────────────────────────────┘
│
▼

END ✅
```

---

## 🔔 notifyObserver() - Notifying the Observer

```cpp
void MACLayer::notifyObserver() {
    if (observer_) {
        observer_->onStatisticsUpdate(getStatistics());
    }
}
```

**How does it work?**

```
┌─────────────────────────────────────────────────────────────┐
│             notifyObserver() - FLOW                         │
└─────────────────────────────────────────────────────────────┘

STEP 1: Check if there is an observer
│
│   if (observer_) {
│       ▲
│       │
│       └─ Is anyone observing us?
│
├─ YES, there is an observer ──────┐
│                                   │
│                                   ▼
│                           ┌───────────────┐
│                           │ STEP 2        │
│                           │ (notify)      │
│                           └───────────────┘
│
└─ NO, no observer ────────────────┐
                                   │
                                   ▼
                           ┌───────────────┐
                           │ END           │
                           │ (do nothing)  │
                           └───────────────┘
```

### If there IS an observer:

```
┌─────────────────────────────────────────────────────────────┐
│                  NOTIFYING                                  │
└─────────────────────────────────────────────────────────────┘

STEP 2: Get statistics
│
│   getStatistics()
│   
│   ┌─────────────────────────────────┐
│   │ Packets: 20                     │
│   │ Bytes: 18806                    │
│   │ Queue: 0                        │
│   └─────────────────────────────────┘
│
▼

STEP 3: Send to observer
│
│   observer_->onStatisticsUpdate(stats);
│   
│   MAC Layer ──────────────► UESimulator
│   "Hey, I have new statistics!"
│
▼

UESimulator:
│
└─ Receives statistics
   (can display, save, etc.)

END ✅
```

---

## 🎬 Full Flow - Step by Step

### Scenario: Sending 1 Packet

```
┌─────────────────────────────────────────────────────────────┐
│           FULL PACKET SENDING FLOW                          │
└─────────────────────────────────────────────────────────────┘

TIME: 0ms
│
│ TX Thread wakes up (every 50ms)
│
▼

TIME: 0ms - scheduleTransmission() called
│
├─ STEP 1: Create empty PDU
│   PDU pdu;
│
├─ STEP 2: Try to get from RLC
│   receivePDU(pdu)
│   │
│   ├─ MAC asks RLC: "Do you have a packet?"
│   │
│   ├─ RLC checks queue:
│   │  Queue: [PDU#5][PDU#6][PDU#7]
│   │          ▲
│   │          └─ Takes PDU#5
│   │
│   └─ RLC returns: true + PDU#5
│
├─ STEP 3: Simulate transmission
│   sleep(10ms)
│   
│   [████████████████] 10ms
│
▼

TIME: 10ms - Transmission complete
│
├─ STEP 4: Increment counters
│   transmittedPackets_++  (was 4, now 5)
│   transmittedBytes_ += 1002  (was 4000, now 5002)
│
├─ STEP 5: Calculate latency
│   Packet created: 0ms
│   Now: 10ms
│   Latency: 10ms
│
├─ STEP 6: Print
│   [MAC] Transmitted PDU ID=5 latency=10ms
│
└─ STEP 7: Notify observer
    notifyObserver()
    │
    └─ UESimulator receives statistics:
       Packets: 5
       Bytes: 5002
       Queue: 2

▼

TIME: 10ms - END
│
└─ TX Thread sleeps until next cycle (50ms)
```

---

## 🧵 Thread Interaction

```
┌─────────────────────────────────────────────────────────────┐
│              THREADS AND MAC LAYER                          │
└─────────────────────────────────────────────────────────────┘

TX THREAD (every 50ms):
│
├─ 0ms:   scheduleTransmission() → Sent PDU#1
├─ 50ms:  scheduleTransmission() → Sent PDU#2
├─ 100ms: scheduleTransmission() → Sent PDU#3
├─ 150ms: scheduleTransmission() → Sent PDU#4
└─ 200ms: scheduleTransmission() → Sent PDU#5

═══════════════════════════════════════════════════════════════

STATS THREAD (every 3s):
│
├─ 3000ms: getStatistics() → Print statistics
├─ 6000ms: getStatistics() → Print statistics
└─ 9000ms: getStatistics() → Print statistics

═══════════════════════════════════════════════════════════════

MAIN THREAD:
│
├─ Creates MAC Layer
├─ Registers observer
└─ Waits for completion
```

---

## 🔢 Atomic Counters - Why?

```
┌─────────────────────────────────────────────────────────────┐
│              ATOMIC COUNTERS - SAFETY                       │
└─────────────────────────────────────────────────────────────┘

Problem without atomic:
│
├─ TX Thread:     transmittedPackets_++  (reads: 5, writes: 6)
│                 ▲
│                 │ AT THE SAME TIME!
│                 ▼
└─ Stats Thread:  x = transmittedPackets_  (reads: 5 or 6?)

RACE CONDITION! ❌


Solution with atomic:
│
├─ TX Thread:     transmittedPackets_++  (atomic operation)
│                 ▲
│                 │ SAFE!
│                 ▼
└─ Stats Thread:  x = transmittedPackets_.load()  (atomic read)

SAFE! ✅

┌─────────────────────────────────────┐
│ std::atomic<uint64_t>               │
│ ├─ Guarantees atomicity             │
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

1. MACLayer() - Constructor
   │
   └─ Creates MAC Layer, zeroes counters

2. scheduleTransmission() - MAIN FUNCTION
   │
   ├─ Gets packet from RLC
   ├─ Simulates transmission (10ms)
   ├─ Updates counters
   ├─ Calculates latency
   ├─ Prints info
   └─ Notifies observer

3. receivePDU() - Getting a packet
   │
   └─ Forwards the call to RLC Layer

4. getStatistics() - Statistics
   │
   ├─ Gets transmittedPackets_
   ├─ Gets transmittedBytes_
   ├─ Gets queueSize from RLC
   └─ Returns Statistics

5. registerObserver() - Register observer
   │
   └─ Saves pointer to observer

6. notifyObserver() - Notifying
   │
   ├─ Checks if observer exists
   └─ Sends statistics to observer

7. sendPDU() - Unused
   │
   └─ Returns false (not implemented)
```

---

## 🎯 Key Concepts

### 1. Atomic Counters (Thread-safe)

```
transmittedPackets_  ← std::atomic<uint64_t>
transmittedBytes_    ← std::atomic<uint64_t>

Why atomic?
├─ Multiple threads read/write
├─ No mutex needed (faster!)
└─ Guaranteed atomicity
```

### 2. Observer Pattern

```
MAC Layer ────────► Observer (UESimulator)
"I have new statistics!"

Benefits:
├─ Loose coupling
├─ Easy to add observers
└─ Separation of concerns
```

### 3. Dependency Injection

```
MACLayer(std::shared_ptr<RLCLayer> rlcLayer)
         ▲
         │
         └─ RLC Layer injected via constructor

Benefits:
├─ Easy testing
├─ Swappable implementations
└─ Loose coupling
```

---

## 🚦 Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                  DATA FLOW                                  │
└─────────────────────────────────────────────────────────────┘

RLC Layer:
┌───────────────────────────────────┐
│ Queue: [PDU#1][PDU#2][PDU#3]      │
└───────────────┬───────────────────┘
                │
                │ receivePDU()
                ▼
MAC Layer:
┌───────────────────────────────────┐
│ scheduleTransmission()            │
│ ├─ Gets PDU#1                     │
│ ├─ Simulates transmission (10ms)  │
│ ├─ Updates counters               │
│ └─ Notifies observer              │
└───────────────┬───────────────────┘
                │
                │ notifyObserver()
                ▼
UESimulator:
┌───────────────────────────────────┐
│ onStatisticsUpdate()              │
│ └─ Receives statistics            │
└───────────────────────────────────┘
```

---

## 💡 Most Important!

```
┌─────────────────────────────────────────────────────────────┐
│                  WHAT YOU MUST REMEMBER                     │
└─────────────────────────────────────────────────────────────┘

1. MAC Layer = Postman
   └─ Takes packets from RLC and sends them

2. scheduleTransmission() = Main function
   └─ Called every 50ms by TX Thread

3. Atomic counters = Thread-safe
   └─ Safe for multiple threads

4. Observer pattern = Notification
   └─ MAC informs UESimulator about statistics

5. Simulation = sleep(10ms)
   └─ We pretend to transmit over radio
```

---

**Document created for**: Beginners  
**Difficulty level**: ⭐☆☆☆☆ (Very easy)  
**Reading time**: 10 minutes  
**Goal**: Understand mac_layer.cpp without reading the code!
