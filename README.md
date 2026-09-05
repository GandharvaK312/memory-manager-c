# C Memory Allocator + Garbage Collector

A from-scratch memory allocator and (eventually) garbage collector in C,
built to understand how allocators and GCs actually work under the hood —
no `malloc`/`free`, no external libraries.

Following along with tsoding's implementation as the primary reference.

## How it works

Memory is served from a single fixed-size static buffer (`heap[HEAP_CAP]`)
using a **bump allocator**: `heap_size` is a boundary that only ever moves
forward, and each allocation just returns `heap + heap_size` before
advancing it.

[ allocated .......... | free .................... ]
 0                heap_size                    HEAP_CAPACITY

Every allocation's `{start, size}` is recorded in a separate metadata
array (`heap_alloced`) so the allocator can later figure out which chunks
are live — this bookkeeping is what a future garbage collector will scan.

## Current state

| Function | Status |
|---|---|
| `heap_alloc` | Implemented — bump allocation, size-0 returns `NULL`, records metadata |
| `heap_dump_alloced_chunks` | Implemented — debug print of all allocated chunks |
| `heap_free` | Stub — not implemented yet |
| `heap_collect` | Stub — not implemented yet |

`heap_freed` / `heap_freed_size` exist as metadata scaffolding for the
upcoming `heap_free` implementation, mirroring `heap_alloced`.

## Design notes

- `heap_alloc(0)` returns `NULL` rather than a zero-size chunk, mirroring
  the C standard's allowance for `malloc(0)`.
- No individual chunk can be freed yet — `heap_size` only grows. This is
  intentional: the plan is to reclaim memory via `heap_collect`
  (a conservative GC that scans the stack in pointer-sized windows for
  addresses that fall inside the heap) rather than an explicit `free()`
  that shifts memory around.
- `HEAP_CAP` is set to 640,000 bytes (640 KB) for now — small on purpose,
  to make bugs and capacity limits easy to hit and observe.

## Build & run
``` bash
gcc -o allocator main.c
./allocator
```

## Roadmap

- [ ] Implement `heap_free`
- [ ] Implement `heap_collect` (mark via conservative stack scan + sweep)
- [ ] Consider compaction / reuse of freed chunks
