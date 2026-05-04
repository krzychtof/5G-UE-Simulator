# Class Diagram - UE Simulator 5G/6G

## Overview
This document describes the class relationships and design patterns used in the simulator.

---

## Class Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                      <<interface>>                          │
│                         ILayer                              │
├─────────────────────────────────────────────────────────────┤
│ + sendPDU(pdu: PDU): bool                                   │
│ + receivePDU(pdu: PDU&): bool                               │
│ + getStatistics(): Statistics                               │
└────────────────────────┬────────────────────────────────────┘
                         │
                         │ implements
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
    ┌─────────┐    ┌─────────┐    ┌─────────┐
    │  PDCP   │    │   RLC   │    │   MAC   │
    │  Layer  │    │  Layer  │    │  Layer  │
    └─────────┘    └─────────┘    └─────────┘
```

---

## Detailed Class Diagrams

### 1. ILayer Interface (Dependency Inversion Principle)

```
┌───────────────────────────────────────────┐
│         <<interface>> ILayer              │
├───────────────────────────────────────────┤
│ + sendPDU(pdu: PDU): bool                 │
│ + receivePDU(pdu: PDU&): bool             │
│ + getStatistics(): Statistics             │
└───────────────────────────────────────────┘
```

**Purpose**: Abstract interface for all protocol layers
**Pattern**: Interface Segregation Principle (ISP)

---

### 2. PDCPLayer (3GPP TS 38.323)

```
┌───────────────────────────────────────────┐
│            PDCPLayer : ILayer             │
├───────────────────────────────────────────┤
│ - mutex_: std::mutex                      │
│ - stats_: Statistics                      │
│ - sequenceNumber_: uint32_t               │
├───────────────────────────────────────────┤
│ + PDCPLayer()                             │
│ + sendPDU(pdu: PDU): bool                 │
│ + receivePDU(pdu: PDU&): bool             │
│ + getStatistics(): Statistics             │
│ + addHeader(sdu: vector<uint8_t>): vector │
│ + removeHeader(pdu: vector<uint8_t>): vec │
└───────────────────────────────────────────┘
```

**Responsibilities**:
- Header compression
- Ciphering (framework)
- Integrity protection (framework)

---

### 3. RLCLayer (3GPP TS 38.322)

```
┌───────────────────────────────────────────┐
│             RLCLayer : ILayer             │
├───────────────────────────────────────────┤
│ - mutex_: std::mutex                      │
│ - cv_: std::condition_variable            │
│ - txQueue_: std::queue<PDU>               │
│ - stats_: Statistics                      │
│ - mode_: RlcMode                          │
│ - pduCounter_: uint32_t                   │
├───────────────────────────────────────────┤
│ + RLCLayer(mode: RlcMode)                 │
│ + sendPDU(pdu: PDU): bool                 │
│ + receivePDU(pdu: PDU&): bool             │
│ + getStatistics(): Statistics             │
│ + segmentSDU(sdu: vector, priority): void │
│ + getQueueSize(): size_t                  │
│ + isQueueEmpty(): bool                    │
└───────────────────────────────────────────┘
```

**Responsibilities**:
- Segmentation & reassembly
- ARQ (framework)
- Duplicate detection
- Queue management

---

### 4. MACLayer (3GPP TS 38.321)

```
┌───────────────────────────────────────────┐
│             MACLayer : ILayer             │
├───────────────────────────────────────────┤
│ - rlcLayer_: shared_ptr<RLCLayer>         │
│ - transmittedPackets_: atomic<uint64_t>   │
│ - transmittedBytes_: atomic<uint64_t>     │
│ - observer_: shared_ptr<IStatObserver>    │
├───────────────────────────────────────────┤
│ + MACLayer(rlc: shared_ptr<RLCLayer>)     │
│ + sendPDU(pdu: PDU): bool                 │
│ + receivePDU(pdu: PDU&): bool             │
│ + getStatistics(): Statistics             │
│ + scheduleTransmission(): void            │
│ + registerObserver(obs): void             │
│ - notifyObserver(): void                  │
└───────────────────────────────────────────┘
```

**Responsibilities**:
- Scheduling
- Multiplexing/demultiplexing
- HARQ management (framework)
- Statistics collection

---

### 5. IStatisticsObserver (Observer Pattern)

```
┌───────────────────────────────────────────┐
│    <<interface>> IStatisticsObserver      │
├───────────────────────────────────────────┤
│ + onStatisticsUpdate(stats: Statistics)   │
└───────────────────────────────────────────┘
```

**Purpose**: Notify interested parties of statistics updates
**Pattern**: Observer Pattern

---

### 6. UESimulator (Facade Pattern)

```
┌───────────────────────────────────────────┐
│  UESimulator : IStatisticsObserver        │
├───────────────────────────────────────────┤
│ - pdcpLayer_: shared_ptr<PDCPLayer>       │
│ - rlcLayer_: shared_ptr<RLCLayer>         │
│ - macLayer_: shared_ptr<MACLayer>         │
│ - running_: atomic<bool>                  │
│ - txThread_: std::thread                  │
│ - statsThread_: std::thread               │
├───────────────────────────────────────────┤
│ + UESimulator()                           │
│ + ~UESimulator()                          │
│ + start(): void                           │
│ + stop(): void                            │
│ + generateTraffic(numPackets): void       │
│ + onStatisticsUpdate(stats): void         │
│ - transmissionLoop(): void                │
│ - statisticsLoop(): void                  │
│ - printBanner(): void                     │
└───────────────────────────────────────────┘
```

**Responsibilities**:
- Orchestrate protocol stack
- Manage threads
- Generate test traffic
- Display statistics

---

## Design Patterns Applied

### 1. Facade Pattern
- **Class**: UESimulator
- **Purpose**: Simplify complex L2 stack interaction
- **Benefit**: Single entry point for users

### 2. Observer Pattern
- **Interface**: IStatisticsObserver
- **Subject**: MACLayer
- **Observer**: UESimulator
- **Benefit**: Decoupled statistics monitoring

### 3. Dependency Injection
- **Example**: MACLayer receives RLCLayer via constructor
- **Benefit**: Loose coupling, testability

### 4. Strategy Pattern (Implicit)
- **Context**: RLCLayer
- **Strategy**: RlcMode (TM, UM, AM)
- **Benefit**: Different RLC behaviors

---

## SOLID Principles Mapping

### Single Responsibility Principle (SRP)
- **PDCPLayer**: Header management only
- **RLCLayer**: Segmentation and queuing only
- **MACLayer**: Scheduling only

### Open/Closed Principle (OCP)
- All layers implement ILayer interface
- New implementations can be added without modifying existing code

### Liskov Substitution Principle (LSP)
- Any ILayer implementation can replace another
- Polymorphic behavior guaranteed

### Interface Segregation Principle (ISP)
- ILayer: Minimal protocol layer interface
- IStatisticsObserver: Separate statistics interface

### Dependency Inversion Principle (DIP)
- High-level (UESimulator) depends on abstractions (ILayer)
- Low-level (PDCPLayer, RLCLayer, MACLayer) implement abstractions

---

## Object Relationships

```
UESimulator
    │
    ├──> PDCPLayer (composition)
    │
    ├──> RLCLayer (composition)
    │       │
    │       └──> std::queue<PDU>
    │
    └──> MACLayer (composition)
            │
            └──> RLCLayer (dependency injection)
```

---

## Thread Architecture

```
┌─────────────────────────────────────────┐
│           Main Thread                   │
│  - UESimulator::generateTraffic()       │
│  - Creates PDUs and sends to RLC        │
└────────────┬────────────────────────────┘
             │
             │ spawns
             │
    ┌────────┴────────┐
    │                 │
    ▼                 ▼
┌─────────┐      ┌─────────┐
│TX Thread│      │Stats    │
│         │      │Thread   │
│50ms loop│      │3s loop  │
│         │      │         │
│MAC::    │      │MAC::    │
│schedule │      │print    │
│Trans()  │      │Stats()  │
└─────────┘      └─────────┘
```

---

## Data Flow

```
User Data (SDU)
    │
    ▼
┌─────────────┐
│ PDCP Layer  │ addHeader()
│ [0xAA][DATA][0xFF]
└──────┬──────┘
       │ PDCP PDU
       ▼
┌─────────────┐
│  RLC Layer  │ segmentSDU()
│ Create PDU with ID, priority, timestamp
│ Add to txQueue_
└──────┬──────┘
       │ RLC PDU
       ▼
┌─────────────┐
│  MAC Layer  │ scheduleTransmission()
│ Dequeue from RLC
│ Simulate transmission (10ms)
│ Measure latency
│ Update statistics
└─────────────┘
```

---

## Memory Management

- **Smart Pointers**: std::shared_ptr for layer ownership
- **RAII**: Automatic resource cleanup
- **Move Semantics**: Efficient data transfer
- **No Raw Pointers**: Memory safety guaranteed

---

## Thread Safety

- **Mutex**: Protects shared data (RLC queue, statistics)
- **Atomic**: Lock-free counters (MAC statistics)
- **Condition Variable**: Efficient thread synchronization
- **Lock Guards**: RAII-based locking

---

## Extension Points

### Adding New RLC Mode
```cpp
class RLCLayerAM : public RLCLayer {
    // Implement Acknowledged Mode
    void handleRetransmission();
    void processStatusPDU();
};
```

### Adding New Observer
```cpp
class LoggingObserver : public IStatisticsObserver {
    void onStatisticsUpdate(const Statistics& stats) override {
        // Log to file
    }
};
```

---

## Performance Considerations

- **Lock-free operations**: Atomic counters for hot paths
- **Minimal locking**: Short critical sections
- **Efficient queuing**: O(1) enqueue/dequeue
- **Memory reservation**: Pre-allocate vectors
- **Move semantics**: Avoid unnecessary copies

---

## Testing Strategy

### Unit Tests (Future)
- Test each layer independently
- Mock dependencies using interfaces
- Verify thread safety with thread sanitizer

### Integration Tests (Future)
- Test full protocol stack
- Verify end-to-end latency
- Stress test with high load

---

## Compliance Matrix

| 3GPP Spec | Layer | Feature | Status |
|-----------|-------|---------|--------|
| TS 38.323 | PDCP  | Header compression | ✓ Framework |
| TS 38.323 | PDCP  | Ciphering | ✓ Framework |
| TS 38.322 | RLC   | Segmentation | ✓ Implemented |
| TS 38.322 | RLC   | UM mode | ✓ Implemented |
| TS 38.322 | RLC   | AM mode | ○ Future |
| TS 38.321 | MAC   | Scheduling | ✓ Implemented |
| TS 38.321 | MAC   | Priority handling | ✓ Implemented |
| TS 38.321 | MAC   | HARQ | ○ Future |

Legend: ✓ = Implemented, ○ = Planned

---

## Conclusion

This architecture provides:
- **Clean separation of concerns** (SOLID)
- **Extensibility** (Design patterns)
- **3GPP compliance** (Standard protocols)
- **Thread safety** (Proper synchronization)
- **Performance** (Lock-free where possible)
- **Maintainability** (Clear structure)
