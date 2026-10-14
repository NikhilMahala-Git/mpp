#include "../generic_utility.hpp"
// TODO: I don't know what is going on here. The code is AI generated.
//       Learn and implement what they would call shared memory or things like that.

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

/*
 * dlmalloc exports dlmalloc(), dlfree(), and dlrealloc() when USE_DL_PREFIX
 * is enabled. Keep those names separate from any platform-level malloc API.
 */
#include "./dlmalloc.hpp"

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

PLATFORM_NAMESPACE_BEGIN

void *malloc(size_t byte_size) {
    return ::dlmalloc(byte_size);
}

void *realloc(void *ptr, size_t byte_size) {
    return ::dlrealloc(ptr, byte_size);
}

void free(void *ptr) {
    ::dlfree(ptr);
}

PLATFORM_NAMESPACE_END
