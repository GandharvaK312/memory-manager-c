# C Memory Allocator + Garbage Collector

A from-scratch memory allocator and garbage collector in C, built to
understand how allocators and GCs actually work under the hood — no
`malloc`/`free`.

Started as a naive bump allocator before moving to free-list tracking

Following tsoding's "Writing My Own Malloc in C" and "Writing a Garbage
Collector in C" as the primary reference.

## How it works

Memory is served from a single fixed-size static buffer (`heap[HEAP_CAP]`).
Two sorted arrays of `{start, size}` chunks track the heap's state:

- `freed_chunks` — currently free regions, starts pre-seeded with one
  chunk covering the entire heap
- `alloced_chunks` — currently allocated regions

Both are instances of the same `Chunk_List` type, kept sorted by start
address, which enables binary search (`chunk_list_find`) and, critically,
makes **coalescing** possible: adjacent free chunks can only be detected
and merged if the list is in address order.

**Allocation (`heap_alloc`)**:
1. Coalesce all of `freed_chunks` via `chunk_list_merge` — any two chunks
   where one's end address equals the next one's start are merged into a
   single larger chunk, undoing fragmentation from prior frees.
2. First-fit search over the (now defragmented) `freed_chunks`.
3. Remove the chosen chunk, insert a same-sized entry into `alloced_chunks`.
4. If the chunk was larger than requested, reinsert the leftover tail
   into `freed_chunks` (splitting).

**Freeing (`heap_free`)** looks up the pointer in `alloced_chunks` via
binary search, removes it, and inserts it back into `freed_chunks` —
coalescing is deferred until the next `heap_alloc` call rather than
happening immediately on every free.

## Current state — malloc/free: complete

| Function | Status |
|---|---|
| `heap_alloc` | Implemented — first-fit + coalescing + splitting |
| `heap_free` | Implemented — returns chunk to freed list |
| `chunk_list_merge` | Implemented — coalesces adjacent freed chunks |
| `chunk_list_insert` / `chunk_list_remove` / `chunk_list_find` | Implemented — sorted insert, removal, binary search |
| `chunk_list_dump` | Implemented — debug print of a chunk list |
| `heap_collect` (GC) | **Not started** — next milestone (`UNIMPLEMENTED`, aborts if called) |

## Design notes / known gaps

- **Coalescing happens on `heap_alloc`, not on `heap_free`.** Frees are
  cheap (just move the chunk to `freed_chunks`); the merge cost is paid
  lazily the next time memory is requested. This means `freed_chunks`
  can transiently hold fragmented chunks between a `heap_free` call and
  the next `heap_alloc` call — fine here since nothing reads `freed_chunks`
  in between, but worth remembering if that assumption ever changes.
- `chunk_start_compar` does raw pointer subtraction narrowed to `int`,
  which is UB if the address difference doesn't fit in an `int`. Hasn't
  caused a visible failure on this heap size, but should be replaced with
  explicit `<`/`>` comparisons before depending on it further.
- `heap_alloc(0)` returns `NULL`, mirroring the C standard's allowance
  for `malloc(0)`.
- Freeing an invalid/already-freed pointer trips `assert` in
  `chunk_list_find`/`heap_free` rather than failing gracefully — acceptable
  for a learning project, not for production use.
- `HEAP_CAP` is 640,000 bytes (640 KB) — small on purpose, to make bugs
  and capacity limits easy to hit and observe.

## Build & run
```bash
gcc -Wall -Wextra -std=c11 -pedantic -o heap main.c
./heap
```

## Roadmap

- [x] Bump allocator (superseded)
- [x] Free-list allocator with first-fit + splitting
- [x] Coalescing of adjacent freed chunks
- [ ] Implement `heap_collect` — conservative GC via stack scanning
      (rolling pointer-sized window over the stack, checking whether
      each candidate address falls inside `heap[]` and is currently
      tracked in `alloced_chunks`)
- [ ] Consider scanning static/global data as additional GC roots
