# C Memory Allocator + Garbage Collector

A from-scratch memory allocator and garbage collector in C, built to
understand how allocators and GCs actually work under the hood — no
`malloc`/`free`.

Started as a naive bump allocator before moving to free-list tracking,
then a word-based heap with a conservative mark-and-sweep collector.

Following tsoding's "Writing My Own Malloc in C" and "Writing a Garbage
Collector in C" as the primary reference.

## References

- [Memory Allocator](https://www.youtube.com/watch?v=sZ8GJ1TiMdk&pp=ygUWbWVtb3J5IGFsbG9jYXRpb24gaW4gYw%3D%3D)
- [Garbage Collector](https://www.youtube.com/watch?v=2JgEKEd3tw8&t=3140s)
- [Github Repo for Tsoding's memory manager](https://github.com/tsoding/memalloc)
- [Jim File](https://github.com/tsoding/jim)

## Files

- `heap.h` / `heap.c` — the allocator and GC: `heap_alloc`, `heap_free`,
  `heap_collect`, and the `Chunk_List` bookkeeping underneath them
- `main.c` — usage/demo code: builds a small binary tree via `heap_alloc`
  and exercises `heap_collect` to show reachable vs. unreachable chunks
  being kept or swept
- `notes.txt` — running dev notes (struct alignment, free-list mechanics,
  sample output) kept separate to keep `main.c` readable

## How it works

### Allocation

Memory is served from a single fixed-size static buffer, `heap[HEAP_CAP_WORDS]`,
declared as `uintptr_t[]` rather than `char[]` — this keeps every allocation
naturally pointer-aligned, avoiding the padding trap where a byte-addressed
heap can hand back a misaligned address for a struct containing a pointer
(see `notes.txt` for the worked example: `{char x; void *ptr;}` needs 16
bytes, not 9, due to alignment).

Two sorted arrays of `{start, size}` chunks (in **words**, not bytes) track
the heap's state:

- `freed_chunks` — currently free regions, starts pre-seeded with one
  chunk covering the entire heap
- `alloced_chunks` — currently allocated regions

Both are instances of `Chunk_List`, kept sorted by start address, enabling
binary search (`chunk_list_find`) and coalescing of adjacent free chunks.

**`heap_alloc(size_bytes)`**:
1. Rounds `size_bytes` up to `size_words`.
2. Coalesces all of `freed_chunks` via `chunk_list_merge`.
3. First-fit search over the defragmented `freed_chunks`.
4. Splits the chunk if it's larger than needed, returning the leftover
   tail to `freed_chunks`.

**`heap_free(ptr)`** looks up `ptr` in `alloced_chunks` via binary search,
removes it, and inserts it back into `freed_chunks`.

### Garbage collection

**`heap_collect()`** is a conservative mark-and-sweep collector:

1. **Mark** — `mark_region` scans a range of memory (starting from the
   stack, between the current frame address and `stack_base`) one word
   at a time. Each word is treated as a *candidate* pointer: if its value
   falls inside any chunk in `alloced_chunks`, that chunk is marked
   reachable, and `mark_region` recurses into the chunk itself — so
   objects reachable only through another heap object (like tree
   children) are still found.
2. **Sweep** — any chunk in `alloced_chunks` that was never marked
   reachable is freed via `heap_free`.

This is "conservative" because it doesn't know which stack words are
real pointers vs. plain integers that happen to look like valid
addresses — it just checks whether the value *could* be a pointer into
the heap and treats it as one if so.

## Current state: complete

| Function | Status |
|---|---|
| `heap_alloc` | Implemented — word-based, first-fit + coalescing + splitting |
| `heap_free` | Implemented |
| `heap_collect` | Implemented — conservative mark-and-sweep via stack scanning |
| `chunk_list_merge` / `insert` / `remove` / `find` / `dump` | Implemented |

## Design notes / known gaps

- `chunk_start_compar` uses explicit `<`/`>` comparison (fixed from an
  earlier version that subtracted pointers and narrowed to `int`, which
  was UB for large address differences).
- Coalescing happens lazily in `heap_alloc`, not immediately in `heap_free`.
- `heap_alloc(0)` returns `NULL`, mirroring `malloc(0)`'s allowed behavior.
- Freeing an invalid/already-freed pointer trips an `assert` rather than
  failing gracefully — fine for a learning project, not production-grade.
- `HEAP_CAP_BYTES` is 640,000 (640 KB) — small on purpose, to make bugs
  and capacity limits easy to hit and observe.

## Limitations

- GC roots are limited to the stack and the heap itself — no static/global
  memory is scanned as a root source.
- No support for packed structs; all pointer-containing data must stay
  naturally aligned for the word-based heap and conservative scan to work.
- No tricks that obscure pointers (e.g. XOR linked lists) — the collector
  relies on being able to recognize a heap address directly in memory.

## Build & run

```bash
gcc -Wall -Wextra -Werror -std=c11 -pedantic -ggdb -o heap main.c heap.c
./heap
```

## Roadmap

- [x] Bump allocator (superseded)
- [x] Free-list allocator with first-fit + splitting
- [x] Coalescing of adjacent freed chunks
- [x] Word-based heap (alignment-safe)
- [x] Conservative mark-and-sweep GC via stack scanning
