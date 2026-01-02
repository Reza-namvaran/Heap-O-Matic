# Heap-O-Matic 🧠⚙️

**A Secure, Multithreaded Heap Allocator with Conservative Garbage Collection**

Heap-O-Matic is an educational yet security-oriented **custom heap allocator** written in C, featuring:

* A **manual heap implementation** on top of `mmap`
* **Conservative mark-and-sweep garbage collection**
* **Pool allocators** for small objects
* **Quarantine and heap spray mitigation**
* **Chunk splitting & coalescing**
* **Thread safety**
* **Extensive unit and stress testing**

This project is designed to explore **low-level memory management**, **allocator internals**, and **security-aware runtime design**.

---

## ✨ Features

### 🧩 Heap Allocator Core

* Explicit heap initialization via `mmap`
* Chunk metadata with integrity cookies
* Alignment-aware allocation
* Chunk splitting and bidirectional coalescing
* Accurate free-space accounting

### ♻️ Garbage Collection (GC)

* **Stop-the-world conservative GC**
* Stack scanning (POSIX/Linux safe)
* Global variable scanning (ELF symbols)
* Interior pointer detection
* Recursive payload scanning
* Allocation-triggered GC fallback

### 🧵 Multithreading

* Thread-safe allocation and free
* Global heap lock
* GC lock to stop mutators during collection
* Stress-tested under concurrent workloads

### 🛡️ Security Features

* Heap poisoning (use-after-free detection)
* Quarantine system (delayed reuse)
* Heap spray mitigation (random padding)
* Metadata validation & magic cookies
* Pointer validation on free

### ⚡ Performance Optimizations

* Pool allocator fast-path for small objects
* Inline assembly for atomic GC marking (`x86_64`)
* Minimal metadata footprint
* Avoids unnecessary syscalls after init

---

## 🗂️ Project Structure

```
.
├── include/                # Public headers
│   ├── allocator.h
│   ├── hgc.h               # Garbage collector
│   ├── pool.h              # Pool allocator
│   ├── quarantine.h
│   ├── validate.h
│   └── ...
├── src/
│   ├── core/               # Heap allocator core
│   │   ├── alloc.c
│   │   ├── heap_init.c
│   │   ├── hfree.c
│   │   ├── split.c
│   │   └── coalescing.c
│   ├── gc/
│   │   └── gc.c            # Mark & sweep GC
│   ├── pools/
│   │   └── pool.c
│   ├── security/
│   │   ├── quarantine.c
│   │   └── validate.c
│   └── utils/
│       └── debug.c
├── tests/
│   ├── unit/               # Unit tests
│   │   ├── test_gc.c
│   │   ├── test_halloc.c
│   │   ├── test_hfree.c
│   │   └── ...
│   └── stress/
│       └── stress_mt.c     # Multithreaded stress test
├── Makefile
└── README.md
```

---

## 🔧 Build Instructions

### Requirements

* Linux (POSIX)
* `gcc` or `clang`
* `pthread`
* `make`

### Build everything

```bash
make
```

### Build allocator only

```bash
make halloc
```

### Build GC tests

```bash
make gc
```

---

## 🧪 Running Tests

### Unit tests

```bash
make test
```

### Garbage Collector tests

```bash
./gc.out
```

### Stress tests (multithreaded)

```bash
./stress_mt.out
```

---

## 🧠 Garbage Collector Design

Heap-O-Matic uses a **conservative mark-and-sweep GC**:

### Root Set

* Stack memory (per thread)
* Global `.data` and `.bss` segments (via linker symbols)

### Mark Phase

* Pointer validation against heap bounds
* Interior pointer detection
* Recursive payload scanning
* Atomic mark bit (x86 inline assembly)

### Sweep Phase

* Unmarked chunks reclaimed
* Optional quarantine
* Coalescing of adjacent free chunks
* Accurate `avail` recalculation

### Safety Guarantees

* Stop-the-world GC (no concurrent mutation)
* No relocation (non-moving GC)
* Safe with unmanaged C pointers

---

## 🔐 Security Considerations

This project intentionally includes **defensive allocator techniques**:

* Heap spray randomization
* Quarantine to mitigate UAF exploitation
* Payload poisoning
* Metadata validation
* Strict pointer checks

⚠️ This allocator is **not intended for production use**, but models real techniques used in hardened runtimes.

---

## 📚 Educational Goals

Heap-O-Matic is ideal for:

* Systems programming courses
* Operating systems labs
* Compiler/runtime experimentation
* Security research
* Interview preparation (allocators, GC, concurrency)

It demonstrates **real allocator bugs and fixes**, including:

* Free space accounting errors
* GC false positives
* Concurrency races
* Metadata corruption scenarios

---

## 🚀 Future Work

Possible extensions:

* Concurrent / incremental GC
* Write barriers
* Generational GC
* Per-thread arenas
* ASAN / Valgrind integration
* Performance benchmarking
