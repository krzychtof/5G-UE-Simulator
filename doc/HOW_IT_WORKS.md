# How It Works - Simple Explanation

A straightforward, no-nonsense explanation of what this simulator does and how it works.

---

## 🎯 What Is This Simulator?

**In Simple Terms:**
This is a software that simulates how a mobile phone (User Equipment - UE) sends data in 5G networks.

**Why Do We Need It?**
- Testing 5G software **without real phones** (saves money)
- Testing **without real network** (faster development)
- Educational purposes - understanding L2 protocol stack
- Developers can test new features before deploying to real networks

**What Does It Simulate?**
The **Layer 2 (L2) protocol stack** - the part that:
- Packages your data (like wrapping a gift)
- Queues it for transmission (like waiting in line)
- Sends it over the air (like mailing the package)

---

## 🏗️ The Big Picture - What's Inside?

```
┌─────────────────────────────────────────────────────────────┐
│                    YOUR MOBILE PHONE                        │
│                                                             │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  Apps (YouTube, WhatsApp, etc.)                       │ │
│  └───────────────────────┬───────────────────────────────┘ │
│                          │ Your data (video, messages)     │
│                          ▼                                  │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  LAYER 2 - DATA PACKAGING & SENDING                   │ │
│  │  ◄── THIS IS WHAT WE SIMULATE ──►                     │ │
│  │                                                        │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ PDCP: Adds security header                       │ │ │
│  │  │ "This data is from WhatsApp"                     │ │ │
│  │  └────────────────┬─────────────────────────────────┘ │ │
│  │                   ▼                                    │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ RLC: Splits into smaller pieces                  │ │ │
│  │  │ "Too big? Let me break it down"                  │ │ │
│  │  └────────────────┬─────────────────────────────────┘ │ │
│  │                   ▼                                    │ │
│  │  ┌──────────────────────────────────────────────────┐ │ │
│  │  │ MAC: Decides when to send                        │ │ │
│  │  │ "Your turn! Send now!"                           │ │ │
│  │  └────────────────┬─────────────────────────────────┘ │ │
│  └───────────────────┼────────────────────────────────────┘ │
│                      ▼                                      │
│  ┌───────────────────────────────────────────────────────┐ │
│  │  Radio (PHY Layer) - Sends over the air              │ │
│  └───────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                      │
                      ▼ Radio waves
              ┌───────────────┐
              │  5G Base      │
              │  Station      │
              └───────────────┘
```

---

## 🚀 How Does It Work? - Step by Step

### Step 1: You Start The Simulator

```
You run: ./ue_simulator.exe

What happens:
┌─────────────────────────────────────┐
│ 1. Create 3 workers (layers):      │
│    - PDCP (packaging guy)           │
│    - RLC (splitting guy)            │
│    - MAC (sending guy)              │
│                                     │
│ 2. Start 2 background threads:     │
│    - TX Thread: Sends data         │
│    - Stats Thread: Shows progress  │
└─────────────────────────────────────┘
```

---

### Step 2: Generate Test Data

```
Simulator creates 20 fake "messages" (like sending 20 WhatsApp messages)

Each message:
- Random size: 64 to 1500 bytes
- Random priority: Low, Medium, or High
- Example: "Message #5, 1200 bytes, High priority"
```

---

### Step 3: Process Each Message

**Here's what happens to ONE message:**

```
TIME: 0ms
┌─────────────────────────────────────────────────────────────┐
│ MAIN THREAD: "I have a message to send!"                   │
│ Message: 1000 bytes of data                                 │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
TIME: 1ms
┌─────────────────────────────────────────────────────────────┐
│ PDCP LAYER: "Let me add a header"                          │
│                                                             │
│ Before: [1000 bytes of data]                               │
│ After:  [0xAA][1000 bytes of data][0xFF]                   │
│         ▲                           ▲                       │
│       Header                     Trailer                    │
│                                                             │
│ Result: 1002 bytes                                          │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
TIME: 2ms
┌─────────────────────────────────────────────────────────────┐
│ RLC LAYER: "Let me prepare this for sending"               │
│                                                             │
│ Creates a PDU (Protocol Data Unit):                        │
│ - ID: 5                                                     │
│ - Data: 1002 bytes                                          │
│ - Priority: High                                            │
│ - Timestamp: 2ms (for measuring latency later)             │
│                                                             │
│ Puts it in QUEUE (waiting line)                            │
│ Queue: [PDU#3][PDU#4][PDU#5] ← New one added               │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
TIME: 2-50ms
┌─────────────────────────────────────────────────────────────┐
│ WAITING IN QUEUE...                                         │
│ (Like waiting in line at the post office)                  │
│                                                             │
│ Queue: [PDU#3][PDU#4][PDU#5]                               │
│         ▲                                                   │
│         Next to send                                        │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
TIME: 50ms
┌─────────────────────────────────────────────────────────────┐
│ TX THREAD: "Time to send!" (runs every 50ms)               │
│                                                             │
│ MAC LAYER: "Let me grab the next one from queue"           │
│ Takes PDU#3 from queue                                      │
│ Queue: [PDU#4][PDU#5] ← PDU#3 removed                      │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
TIME: 50-60ms
┌─────────────────────────────────────────────────────────────┐
│ MAC LAYER: "Sending over the air..."                       │
│                                                             │
│ Simulates radio transmission (10ms delay)                  │
│ [████████████████████████] 100%                            │
│                                                             │
│ Calculates latency: 60ms - 2ms = 58ms                      │
│ Updates statistics: packets++, bytes += 1002               │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
TIME: 60ms
┌─────────────────────────────────────────────────────────────┐
│ DONE! Message sent successfully!                           │
│                                                             │
│ Console output:                                             │
│ [RLC] Segmented PDU ID=5 size=1002B priority=3             │
│ [MAC] Transmitted PDU ID=5 latency=58ms                    │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔄 What's Running in Parallel?

The simulator has **3 threads** (workers) running at the same time:

```
┌─────────────────────────────────────────────────────────────┐
│                    THREAD TIMELINE                          │
└─────────────────────────────────────────────────────────────┘

MAIN THREAD (You):
│
├─ 0ms:    Start simulator
├─ 100ms:  Generate message #1 → RLC queue
├─ 300ms:  Generate message #2 → RLC queue
├─ 500ms:  Generate message #3 → RLC queue
├─ ...
├─ 4000ms: Generate message #20 → RLC queue
├─ 6000ms: Stop simulator
│
═══════════════════════════════════════════════════════════════

TX THREAD (Automatic sender):
│
├─ 0ms:    Start
├─ 50ms:   Check queue → Send message #1
├─ 100ms:  Check queue → Send message #2
├─ 150ms:  Check queue → Send message #3
├─ 200ms:  Check queue → Send message #4
├─ ...
├─ 6000ms: Stop
│
═══════════════════════════════════════════════════════════════

STATS THREAD (Progress reporter):
│
├─ 0ms:    Start
├─ 3000ms: Print statistics (15 packets sent)
├─ 6000ms: Print statistics (20 packets sent)
├─ 6000ms: Stop
│
═══════════════════════════════════════════════════════════════
```

---

## 📊 Real Example - What You See

### Console Output Explained

```
[RLC] Segmented PDU ID=0 size=269B priority=2
      ▲              ▲       ▲         ▲
      │              │       │         └─ Priority level (1=low, 3=high)
      │              │       └─────────── Size in bytes
      │              └─────────────────── Unique ID number
      └────────────────────────────────── Which layer is talking

[MAC] Transmitted PDU ID=0 latency=18ms
      ▲                  ▲           ▲
      │                  │           └─ How long it took (timestamp to now)
      │                  └───────────── Same ID as above
      └──────────────────────────────── MAC layer sent it
```

### Statistics Explained

```
=== MAC STATISTICS ===
Transmitted packets: 20      ← Sent 20 messages successfully
Total bytes: 18806           ← Total data sent (18.8 KB)
RLC queue size: 0            ← No messages waiting (all sent)
======================
```

---

## 🎯 Why These Numbers?

### Latency (15-79ms)

**What is latency?**
Time from "I want to send" to "Message sent"

**Why does it vary?**

```
Fast message (15ms):
┌──────────────────────────────────────┐
│ Queue: [Empty]                       │
│ Your message arrives                 │
│ Immediately sent (no waiting)        │
│ Time: 10ms transmission + 5ms overhead = 15ms
└──────────────────────────────────────┘

Slow message (79ms):
┌──────────────────────────────────────┐
│ Queue: [Msg1][Msg2][Msg3][Msg4]      │
│ Your message arrives                 │
│ Waits for 4 messages ahead           │
│ Time: 60ms waiting + 10ms transmission + 9ms overhead = 79ms
└──────────────────────────────────────┘
```

### Packet Sizes (70-1488 bytes)

**Why random?**
Real apps send different sizes:
- Small: Text message (70 bytes)
- Medium: Photo thumbnail (800 bytes)
- Large: Video chunk (1488 bytes)

We simulate this randomness to test realistic scenarios.

---

## 🧵 Thread Safety - Why It Matters

**The Problem:**
Multiple threads accessing the same queue = CHAOS!

```
WITHOUT PROTECTION (BAD):
┌─────────────────────────────────────────────────────────────┐
│ TX Thread: "Let me take message #5 from queue"             │
│ Main Thread: "Let me add message #6 to queue"              │
│                                                             │
│ BOTH ACCESS QUEUE AT SAME TIME → CRASH! 💥                 │
└─────────────────────────────────────────────────────────────┘

WITH PROTECTION (GOOD):
┌─────────────────────────────────────────────────────────────┐
│ TX Thread: "Lock the queue, let me take message #5"        │
│ Main Thread: "Waiting... (queue is locked)"                │
│ TX Thread: "Done! Unlock the queue"                        │
│ Main Thread: "Now I can add message #6"                    │
│                                                             │
│ SAFE! ✅                                                    │
└─────────────────────────────────────────────────────────────┘
```

**How we protect:**
- **Mutex** (lock): Only one thread can access queue at a time
- **Atomic counters**: Statistics updated safely without locks

---

## 📈 What Gets Measured?

```
┌─────────────────────────────────────────────────────────────┐
│                    MEASUREMENTS                             │
└─────────────────────────────────────────────────────────────┘

1. LATENCY (How fast?)
   ├─ When message created: Timestamp = T0
   ├─ When message sent: Timestamp = T1
   └─ Latency = T1 - T0
   
   Example: Created at 100ms, sent at 177ms → Latency = 77ms

2. THROUGHPUT (How much?)
   ├─ Count packets: 20 packets
   ├─ Count bytes: 18,806 bytes
   └─ Time: 8 seconds
   
   Result: ~2,350 bytes/second

3. QUEUE DEPTH (How busy?)
   ├─ Messages waiting: 0-5 at any time
   ├─ Max capacity: 1000 messages
   └─ Overflow: 0 (never full)

4. SUCCESS RATE (How reliable?)
   ├─ Sent: 20 packets
   ├─ Dropped: 0 packets
   └─ Success: 100%
```

---

## 🎓 3GPP Standards - What Are They?

**3GPP = 3rd Generation Partnership Project**
- Organization that writes rules for mobile networks
- Like building codes for houses, but for 5G networks

**Our simulator follows these rules:**

```
┌─────────────────────────────────────────────────────────────┐
│ 3GPP TS 38.323 - PDCP Layer Rules                          │
│ "How to add headers and security"                          │
│ ✅ We follow this                                           │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ 3GPP TS 38.322 - RLC Layer Rules                           │
│ "How to split data and queue it"                           │
│ ✅ We follow this                                           │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│ 3GPP TS 38.321 - MAC Layer Rules                           │
│ "How to schedule and send data"                            │
│ ✅ We follow this                                           │
└─────────────────────────────────────────────────────────────┘
```

**Why follow standards?**
- Ensures compatibility with real 5G equipment
- Industry-standard implementation
- Other companies can understand the code

---

## 🔧 Design Patterns - Simplified

### 1. Facade Pattern (Front Door)

```
WITHOUT FACADE (Complicated):
You: "I need to start PDCP, then RLC, then MAC, then threads..."
     (Too many steps!)

WITH FACADE (Simple):
You: "simulator.start()"
     (One command does everything!)
```

### 2. Observer Pattern (News Subscription)

```
MAC Layer: "I just sent a packet!"
           ↓ (notifies)
Simulator: "Thanks for the update! I'll log it."

Like subscribing to news - you get notified automatically.
```

### 3. Dependency Injection (Plug & Play)

```
MAC Layer needs RLC Layer:

BAD: MAC creates its own RLC (hard-coded, can't change)
GOOD: Give MAC an RLC when creating it (flexible, testable)

Like USB - plug in any compatible device.
```

---

## 🎯 Summary - The Whole Picture

```
┌─────────────────────────────────────────────────────────────┐
│                    WHAT HAPPENS                             │
└─────────────────────────────────────────────────────────────┘

1. START
   └─ Create 3 layers (PDCP, RLC, MAC)
   └─ Start 2 threads (TX, Stats)

2. GENERATE DATA
   └─ Create 20 fake messages
   └─ Each message: random size, random priority

3. PROCESS EACH MESSAGE
   └─ PDCP: Add header
   └─ RLC: Create PDU, add to queue
   └─ MAC: Take from queue, send
   └─ Measure: latency, throughput

4. REPORT
   └─ Every 3 seconds: print statistics
   └─ At end: print final results

5. STOP
   └─ Stop threads
   └─ Clean up
   └─ Done!

RESULT:
✅ 20 messages sent successfully
✅ 18,806 bytes transmitted
✅ Average latency: 63.5ms
✅ Zero errors
```

---

## 💡 Key Takeaways

1. **What it does**: Simulates how a phone sends data in 5G
2. **Why it exists**: Test 5G software without real phones/networks, educational purposes
3. **How it works**: 3 layers process data, 2 threads send it
4. **What you see**: Console logs showing each step
5. **What it measures**: Speed, reliability, efficiency
6. **Why it matters**: Understanding L2 protocol stack in 5G/6G networks

---

## 🚀 In One Sentence

**This simulator pretends to be a 5G phone, showing exactly how data gets packaged, queued, and sent - perfect for understanding L2 protocol stack behavior in 5G networks.**

---

**Document Version**: 1.0  
**Audience**: Anyone (no technical background needed)  
**Reading Time**: 10 minutes  
**Complexity**: ⭐☆☆☆☆ (Very Simple)
