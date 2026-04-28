#pragma once

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

namespace EA
{
//Hint to the CPU that the calling thread is in a spin-wait loop.
//Emits PAUSE on x86 / YIELD on AArch64; no-op on unknown targets.
//C++ has no portable spelling for this as of C++23, so the platform
//dispatch is encapsulated here and nowhere else in the library.
inline void spinHint() noexcept
{
#if defined(__x86_64__) || defined(_M_X64) \
    || defined(__i386__) || defined(_M_IX86)
    #if defined(_MSC_VER)
    _mm_pause();
    #else
    __builtin_ia32_pause();
    #endif
#elif defined(__aarch64__) || defined(_M_ARM64) \
    || defined(__arm__) || defined(_M_ARM)
    #if defined(_MSC_VER)
    __yield();
    #else
    __asm__ __volatile__("yield" ::: "memory");
    #endif
#endif
}
} // namespace EA
