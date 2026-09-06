# C Memory Allocator + Garbage Collector

A from-scratch memory allocator and (eventually) garbage collector in C,
built to understand how allocators and GCs actually work under the hood —
no `malloc`/`free`, no external libraries.

Following along with tsoding's implementation as the primary reference.

## How it works

Memory is served from a single fixed-size static buffer (`heap[HEAP_CAP]`).
Two sorted arrays of `{start, size}` chunks track the heap's state:

- `freed_chunks` — currently free regions, starts pre-seeded with one
  chunk covering the entire heap
- `alloced_chunks` — currently allocated regions

Both are instances of the same `Chunk_List` type, kept sorted by start
address so that lookups (`chunk_list_find`) can binary search instead of
scanning linearly.

**Allocation (`heap_alloc`)** is first-fit: walk `freed_chunks`, take the
first chunk big enough for the request, remove it from `freed_chunks`,
and insert an entry of the requested size into `alloced_chunks`. If the
chunk was larger than needed, the leftover tail is reinserted into
`freed_chunks` as its own chunk (chunk splitting).

**Freeing (`heap_free`)** looks up the pointer in `alloced_chunks` via
binary search, removes it from there, and inserts it back into
`freed_chunks` — making that region available for future allocations.

This replaces the earlier bump allocator (`heap_size` boundary that only
ever grew) — memory can now actually be reused after being freed.

## Current state

| Function | Status |
|---|---|
| `heap_alloc` | Implemented — first-fit search over freed list, with splitting |
| `heap_free` | Implemented — returns chunk to freed list |
| `chunk_list_insert` / `chunk_list_remove` / `chunk_list_find` | Implemented — sorted insert, removal, binary search |
| `chunk_list_dump` | Implemented — debug print of a chunk list |
| `heap_collect` | Stub — not implemented (`UNIMPLEMENTED` macro, aborts if called) |

## Design notes / known gaps

- **No coalescing.** Adjacent freed chunks are never merged back into one
  larger chunk, so repeated alloc/free of varying sizes will fragment
  `freed_chunks` over time. This is the natural next thing to add.
- `heap_alloc(0)` still returns `NULL` rather than a chunk, mirroring
  the C standard's allowance for `malloc(0)`.
- Freeing an already-freed or invalid pointer trips `assert(index >= 0)`
  in `chunk_list_find` — there's no protection against double-free yet
  beyond that assertion.
- `HEAP_CAP` is set to 640,000 bytes (640 KB) for now — small on purpose,
  to make bugs and capacity limits easy to hit and observe.
- The eventual GC (`heap_collect`) will scan the stack in pointer-sized
  windows for addresses that fall inside `heap`, treating matches as
  live roots — a conservative GC, no explicit root set required.

## Build & run
``` bash
gcc -o allocator main.c
./allocator
```

## Roadmap

- [ ] Coalesce adjacent freed chunks on `heap_free`
- [ ] Implement `heap_collect` (mark via conservative stack scan + sweep)
- [ ] Guard against double-free more gracefully
