/*
 * dlamalloc.c - dlmalloc 2.8.6 adapter for bare Clang wasm32.
 *
 * Vendor Doug Lea's upstream source as dlmalloc_upstream.c next to this file.
 * Compile this translation unit ONLY for wasm32 with Clang.
 *
 * This adapter assumes wasm-ld, a single-threaded wasm instance, and that no
 * other component independently manages/grows the allocator's heap. The
 * linker-provided __heap_base keeps the allocator from wasting the initial
 * memory or overlapping the static data/stack reservation.
 */

#if !defined(__clang__) || !defined(__wasm32__)
# error "dlamalloc.c is only for Clang's wasm32 target"
#endif

#include <stddef.h>
#include <stdint.h>

#define WASM_PAGE_SIZE ((size_t)65536u)

/* dlmalloc's MORECORE-style interface. */
static void *wasm_morecore(ptrdiff_t increment);

/* Configure dlmalloc before including its implementation. */
#define USE_DL_PREFIX             1
#define HAVE_MORECORE             1
#define MORECORE                  wasm_morecore
#define MORECORE_CONTIGUOUS       1
#define MORECORE_CANNOT_TRIM      1
#define UNSIGNED_MORECORE         0
#define HAVE_MMAP                 0
#define HAVE_MREMAP               0
#define NO_MALLINFO               1
#define NO_MALLOC_STATS           1
#define USE_LOCKS                 0
#define MALLOC_FAILURE_ACTION
#define ABORT                     __builtin_trap()

/* No host OS headers/syscalls in a bare wasm build. */
#define LACKS_UNISTD_H            1
#define LACKS_FCNTL_H              1
#define LACKS_SYS_PARAM_H          1
#define LACKS_SYS_TYPES_H          1
#define LACKS_SYS_MMAN_H            1
#define LACKS_STRING_H              1
#define LACKS_STDLIB_H              1
#define LACKS_STRINGS_H             1
#define LACKS_TIME_H                1
#define LACKS_SCHED_H               1
#define LACKS_ERRNO_H               1

#define ENOMEM (12) // : Cannot allocate memory (Out of memory)
#define EINVAL (22) // : Invalid argumen

/* malloc must satisfy the target's maximum fundamental alignment. */
#define MALLOC_ALIGNMENT (__alignof__(max_align_t))

/* Keep dlmalloc's segment granularity aligned with wasm memory pages. */
#define DEFAULT_GRANULARITY WASM_PAGE_SIZE

void* memset_dlmalloc(void* s, int c, size_t n) {
    // MPP_MACRO_ASSERT(s != nullptr, "memset: target buffer cannot be null");

    auto* p = static_cast<unsigned char*>(s);
    unsigned char byte_value = static_cast<unsigned char>(c);

    // Optimization: Broadcast the single byte across a full word width block
    // e.g., if c = 0xAA, word_value becomes 0xAAAAAAA...
    size_t word_value = 0;
    for (size_t i = 0; i < sizeof(size_t); ++i) {
        word_value = (word_value << 8) | byte_value;
    }

    // Fill by word chunks
    while (n >= sizeof(size_t)) {
        *reinterpret_cast<size_t*>(p) = word_value;
        p += sizeof(size_t);
        n -= sizeof(size_t);
    }

    // Fill remaining trailing bytes
    while (n > 0) {
        *p++ = byte_value;
        n--;
    }

    return s;
}

void* memcpy_dlmalloc(void* dest, const void* src, size_t n) {
    // MPP_MACRO_ASSERT(dest != nullptr, "memcpy: dest cannot be null");
    // MPP_MACRO_ASSERT(src != nullptr, "memcpy: src cannot be null");

    auto* d = static_cast<unsigned char*>(dest);
    const auto* s = static_cast<const unsigned char*>(src);

    // Optimization: Copy by CPU architecture word size (4 or 8 bytes at a time)
    while (n >= sizeof(size_t)) {
        *reinterpret_cast<size_t*>(d) = *reinterpret_cast<const size_t*>(s);
        d += sizeof(size_t);
        s += sizeof(size_t);
        n -= sizeof(size_t);
    }

    // Clean up remaining trailing bytes
    while (n > 0) {
        *d++ = *s++;
        n--;
    }

    return dest;
}

/*
 * dlmalloc exports dlmalloc(), dlfree(), and dlrealloc() when USE_DL_PREFIX
 * is enabled. Keep those names separate from any platform-level malloc API.
 */
#include "./dlmalloc_upstream.cpp"

/* wasm-ld defines __heap_base as the first address available to the heap. */
#ifndef DLMALLOC_HEAP_BASE_ADDRESS
extern unsigned char __heap_base;
#define DLMALLOC_HEAP_BASE_ADDRESS ((uintptr_t)&__heap_base)
#endif

/* MORECORE state is a byte offset into wasm linear memory. */
static size_t wasm_heap_break;
static int wasm_heap_break_initialized;

static int wasm_get_memory_bytes(size_t *out_bytes)
{
    size_t pages = (size_t)__builtin_wasm_memory_size(0);

    /* Avoid overflow, including the unrepresentable 4-GiB end on wasm32. */
    if (pages > (SIZE_MAX / WASM_PAGE_SIZE))
        return 0;

    *out_bytes = pages * WASM_PAGE_SIZE;
    return 1;
}

static void *wasm_morecore(ptrdiff_t increment)
{
    size_t memory_bytes;
    size_t old_break;
    size_t new_break;

    if (!wasm_heap_break_initialized) {
        size_t memory_bytes;
        uintptr_t heap_base = (uintptr_t)DLMALLOC_HEAP_BASE_ADDRESS;

        if (!wasm_get_memory_bytes(&memory_bytes) || heap_base > memory_bytes)
            return (void *)(intptr_t)-1;

        wasm_heap_break = (size_t)heap_base;
        wasm_heap_break_initialized = 1;
    }

    old_break = wasm_heap_break;

    /* dlmalloc is configured not to trim; reject negative requests defensively. */
    if (increment < 0)
        return (void *)(intptr_t)-1;

    /* A zero increment is the usual sbrk query for the current break. */
    if (increment == 0)
        return (void *)(uintptr_t)old_break;

    if ((size_t)increment > SIZE_MAX - old_break)
        return (void *)(intptr_t)-1;

    new_break = old_break + (size_t)increment;

    if (!wasm_get_memory_bytes(&memory_bytes))
        return (void *)(intptr_t)-1;

    if (new_break > memory_bytes) {
        size_t needed = new_break - memory_bytes;
        size_t pages_to_grow = needed / WASM_PAGE_SIZE;
        size_t previous_pages;

        if ((needed % WASM_PAGE_SIZE) != 0)
            ++pages_to_grow;

        if (pages_to_grow == 0)
            pages_to_grow = 1;

        previous_pages = (size_t)__builtin_wasm_memory_grow(0, pages_to_grow);
        if (previous_pages == SIZE_MAX)
            return (void *)(intptr_t)-1;
    }

    wasm_heap_break = new_break;
    return (void *)(uintptr_t)old_break;
}
