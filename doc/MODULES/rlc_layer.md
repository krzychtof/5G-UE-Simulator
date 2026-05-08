# RLC Layer - Detailed Explanation

## 🎯 What Does the RLC Layer Do?

**RLC (Radio Link Control)** is the layer responsible for:
- **Packet queuing** - Holds packets in a FIFO queue
- **Segmentation** - Creates PDUs (Protocol Data Units) from PDCP data
- **Priority management** - Handles 3 priority levels
- **Thread-safe operations** - Safe multi-threaded access

**Analogy**: RLC is a **warehouse with a queue** - it accepts packages from PDCP, assigns them numbers, lines them up in a queue, and hands them to MAC on demand.

---

## 📋 Class Structure

```cpp
class RLCLayer : public ILayer {
private:
    RlcMode mode_;                    // RLC mode (UM/AM/TM)
    std::queue<PDU> txQueue_;         // Transmit queue (FIFO)
    std::atomic<uint32_t> pduCounter_; // PDU counter (thread-safe)
    mutable std::mutex mutex_;        // Mutex for thread-safety
    std::condition_variable cv_;      // Thread synchronization
    Statistics stats_;                // Statistics
    
    static constexpr size_t MAX_QUEUE_SIZE = 1000; // Max 1000 PDUs
};
```

---

## 🔧 Functions - Step by Step

### 1️⃣ Constructor

```cpp
RLCLayer::RLCLayer(RlcMode mode) 
    : mode_(mode), pduCounter_(0) {}
```

**What it does:**
- Sets the RLC mode (Unacknowledged Mode - UM)
- Initializes the PDU counter to 0

**When called:**
```cpp
// In ue_simulator.cpp:
rlcLayer_ = std::make_shared<RLCLayer>(RlcMode::UM);
```

---

### 2️⃣ segmentSDU() - Creating a PDU

```cpp
void RLCLayer::segmentSDU(const std::vector<uint8_t>& sdu, Priority priority) {
    PDU pdu;
    pdu.id = pduCounter_++;           // Assign unique ID
    pdu.data = sdu;                   // Copy data from PDCP
    pdu.priority = priority;          // Set priority (1-3)
    pdu.timestamp = std::chrono::steady_clock::now(); // Timestamp
    
    sendPDU(pdu);                     // Add to queue
}
```

**What it does:**
1. Creates a new PDU (Protocol Data Unit)
2. Assigns it a unique ID (0, 1, 2, 3...)
3. Copies data from PDCP (already with 0xAA...0xFF header)
4. Sets priority (LOW=1, MEDIUM=2, HIGH=3)
5. Records timestamp (for latency measurement)
6. Sends to queue

**Example:**
```
Input:  sdu = [0xAA, 'H', 'e', 'l', 'l', 'o', 0xFF], priority = HIGH
Output: PDU { id=5, data=[0xAA,'H','e','l','l','o',0xFF], priority=3, timestamp=100ms }
```

**When called:**
```cpp
// In ue_simulator.cpp - generateTraffic():
rlcLayer_->segmentSDU(pdu, priority);
```

---

### 3️⃣ sendPDU() - Adding to the Queue

```cpp
bool RLCLayer::sendPDU(const PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);  // 🔒 Lock mutex
    
    // Check if queue is full
    if (txQueue_.size() >= MAX_QUEUE_SIZE) {
        stats_.droppedPackets++;               // Count dropped packets
        return false;                          // ❌ Queue full
    }
    
    txQueue_.push(pdu);                        // ✅ Add to queue
    stats_.queueSize = txQueue_.size();        // Update statistics
    cv_.notify_one();                          // 📢 Notify waiting threads
    
    // Print log
    std::cout << "[RLC] Segmented PDU ID=" << pdu.id 
              << " size=" << pdu.data.size() << "B"
              << " priority=" << static_cast<int>(pdu.priority) << "\n";
    
    return true;
}
```

**What it does step by step:**

1. **Locks mutex** - Only one thread can modify the queue at a time
2. **Checks queue size** - Max 1000 PDUs
3. **If full** → Drops packet, increments `droppedPackets` counter
4. **If space available** → Appends PDU to end of queue (FIFO)
5. **Updates statistics** - Queue size
6. **Notifies threads** - If any thread is waiting for data
7. **Prints log** - `[RLC] Segmented PDU ID=5 size=873B priority=3`

**Example:**
```
Before: txQueue_ = [PDU#3, PDU#4]
Call:   sendPDU(PDU#5)
After:  txQueue_ = [PDU#3, PDU#4, PDU#5]
                    ▲ front()      ▲ back()
```

---

### 4️⃣ receivePDU() - Dequeuing

```cpp
bool RLCLayer::receivePDU(PDU& pdu) {
    std::lock_guard<std::mutex> lock(mutex_);  // 🔒 Lock mutex
    
    // Check if queue is empty
    if (txQueue_.empty()) {
        return false;                          // ❌ No data
    }
    
    pdu = txQueue_.front();                    // Get first element
    txQueue_.pop();                            // Remove from queue
    stats_.queueSize = txQueue_.size();        // Update statistics
    
    return true;                               // ✅ Success
}
```

**What it does:**
1. **Locks mutex** - Thread-safe
2. **Checks if queue is empty** - Returns `false` if so
3. **Gets first PDU** - FIFO (First In, First Out)
4. **Removes from queue** - `pop()`
5. **Updates statistics** - New queue size
6. **Returns true** - Success

**Example:**
```
Before: txQueue_ = [PDU#3, PDU#4, PDU#5]
                    ▲ front()
Call:   receivePDU(pdu)
After:  txQueue_ = [PDU#4, PDU#5]
        pdu = PDU#3  (returned by reference)
```

**When called:**
```cpp
// In mac_layer.cpp - scheduleTransmission():
if (rlcLayer_->receivePDU(pdu)) {
    // Transmit PDU
}
```

---

### 5️⃣ getStatistics() - Retrieving Statistics

```cpp
Statistics RLCLayer::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}
```

**What it does:**
- Returns a copy of statistics (thread-safe)

**RLC Statistics:**
```cpp
struct Statistics {
    size_t queueSize;        // Current queue size
    size_t droppedPackets;   // Number of dropped packets
};
```

---

### 6️⃣ getQueueSize() - Queue Size

```cpp
size_t RLCLayer::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return txQueue_.size();
}
```

**What it does:**
- Returns current queue size (thread-safe)

---

### 7️⃣ isQueueEmpty() - Is Queue Empty?

```cpp
bool RLCLayer::isQueueEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return txQueue_.empty();
}
```

**What it does:**
- Checks whether the queue is empty (thread-safe)

---

## 🔄 Data Flow

```
┌─────────────────────────────────────────────────────────────┐
│                  RLC LAYER DATA FLOW                        │
└─────────────────────────────────────────────────────────────┘

1. PDCP sends data:
   ┌──────┐  segmentSDU(sdu, priority)  ┌──────┐
   │ PDCP ├────────────────────────────►│ RLC  │
   └──────┘                             └──────┘
   
   RLC: "Creating PDU with ID=5, adding to queue"

2. RLC creates PDU:
   PDU {
       id: 5
       data: [0xAA, 'H', 'e', 'l', 'l', 'o', 0xFF]
       priority: HIGH (3)
       timestamp: 100ms
   }

3. RLC adds to queue:
   txQueue_: [PDU#3] [PDU#4] [PDU#5] ← New
             ▲ front()        ▲ back()

4. MAC dequeues:
   ┌──────┐  receivePDU(pdu)  ┌──────┐
   │ MAC  │◄──────────────────┤ RLC  │
   └──────┘                   └──────┘
   
   MAC: "Give me the next PDU"
   RLC: "Here is PDU#3" (removes from queue)

5. After dequeue:
   txQueue_: [PDU#4] [PDU#5]
             ▲ front()
```

---

## 🧵 Thread Safety

### Why Mutex?

RLC is used by **3 threads simultaneously**:

```
MAIN THREAD:
│
└─ generateTraffic()
   └─ rlcLayer_->segmentSDU()  ← Adds to queue

TX THREAD:
│
└─ scheduleTransmission()
   └─ rlcLayer_->receivePDU()  ← Removes from queue

STATS THREAD:
│
└─ printStatistics()
   └─ rlcLayer_->getQueueSize() ← Reads size
```

**Without mutex:**
```
❌ RACE CONDITION:
Thread 1: txQueue_.push(pdu)     ← Adding
Thread 2: txQueue_.pop()         ← Removing
Thread 3: txQueue_.size()        ← Reading
         ↓
      CRASH! 💥
```

**With mutex:**
```
✅ THREAD-SAFE:
Thread 1: lock → push → unlock
Thread 2:        lock → pop → unlock
Thread 3:               lock → size → unlock
         ↓
      Safe! ✓
```

---

## 📊 Usage Example

### Scenario: Sending 3 Packets

```cpp
// 1. Create RLC
RLCLayer rlc(RlcMode::UM);

// 2. PDCP sends 3 packets
std::vector<uint8_t> data1 = {0xAA, 'A', 'B', 'C', 0xFF};
std::vector<uint8_t> data2 = {0xAA, 'D', 'E', 'F', 0xFF};
std::vector<uint8_t> data3 = {0xAA, 'G', 'H', 'I', 0xFF};

rlc.segmentSDU(data1, Priority::HIGH);    // PDU ID=0
rlc.segmentSDU(data2, Priority::MEDIUM);  // PDU ID=1
rlc.segmentSDU(data3, Priority::LOW);     // PDU ID=2

// Queue state:
// txQueue_ = [PDU#0, PDU#1, PDU#2]
// queueSize = 3

// 3. MAC dequeues packets
PDU pdu;
rlc.receivePDU(pdu);  // Returns PDU#0
// txQueue_ = [PDU#1, PDU#2], queueSize = 2

rlc.receivePDU(pdu);  // Returns PDU#1
// txQueue_ = [PDU#2], queueSize = 1

rlc.receivePDU(pdu);  // Returns PDU#2
// txQueue_ = [], queueSize = 0

rlc.receivePDU(pdu);  // Returns false (empty queue)
```

---

## 🎯 Key Concepts

### 1. FIFO (First In, First Out)
```
Enqueue:  [1] → [1,2] → [1,2,3]
Dequeue:  [1,2,3] → [2,3] → [3] → []
           ▲ Always the first
```

### 2. Atomic Counter
```cpp
std::atomic<uint32_t> pduCounter_;
```
- **Thread-safe** without a mutex
- Auto-incremented: `pduCounter_++`
- Guarantees unique ID for every PDU

### 3. Condition Variable
```cpp
cv_.notify_one();
```
- Notifies threads waiting for data
- Not actively used in this project (no `wait()`)
- Prepared for future extensions

### 4. Lock Guard
```cpp
std::lock_guard<std::mutex> lock(mutex_);
```
- **RAII** - Automatic mutex release
- Locks on construction, unlocks on destruction
- Safe even in the presence of exceptions

---

## 📈 Statistics

### What Does RLC Track?

```cpp
struct Statistics {
    size_t queueSize;        // Current queue size
    size_t droppedPackets;   // Dropped packets (queue full)
};
```

### When Are Packets Dropped?

```cpp
if (txQueue_.size() >= MAX_QUEUE_SIZE) {  // >= 1000
    stats_.droppedPackets++;
    return false;  // ❌ Dropped
}
```

**Example:**
```
Queue: [PDU#0, PDU#1, ..., PDU#999]  ← 1000 elements
Attempt to add PDU#1000:
→ Dropped! droppedPackets++
```

---

## 🔍 Differences: RLC vs MAC

| Aspect | RLC Layer | MAC Layer |
|--------|-----------|-----------|
| **Role** | Warehouse (queue) | Sender (transmission) |
| **Main function** | PDU queuing | PDU sending |
| **Data structure** | `std::queue<PDU>` | No queue |
| **Timing** | Immediate | Every 50ms |
| **Latency** | Not measured | Measured (timestamp) |
| **Statistics** | Queue size | Packets, bytes, latency |

---

## 💡 Summary

### RLC Layer in 5 Points:

1. **Receives data from PDCP** → Creates PDU with ID, priority, timestamp
2. **Queues PDUs** → FIFO queue (max 1000)
3. **Hands PDUs to MAC** → On demand (receivePDU)
4. **Thread-safe** → Mutex protects the queue
5. **Tracks statistics** → Queue size, dropped packets

### Analogy:

```
RLC = Warehouse with a conveyor belt

PDCP delivers packages → RLC assigns numbers and lines them up
                         ↓
                     [FIFO Queue]
                         ↓
MAC picks up packages ← RLC hands them out in order
```

### Key Code:

```cpp
// Add to queue (PDCP → RLC)
void segmentSDU(sdu, priority) {
    PDU pdu = { id++, sdu, priority, now() };
    txQueue_.push(pdu);
}

// Dequeue (RLC → MAC)
bool receivePDU(PDU& pdu) {
    if (txQueue_.empty()) return false;
    pdu = txQueue_.front();
    txQueue_.pop();
    return true;
}
```

**RLC is simple but essential - without it, MAC would have nothing to transmit!** 🎯
