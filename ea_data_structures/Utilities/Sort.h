#pragma once

// libc++ ships every algorithm in its own header, and std::sort's plus
// std::stable_sort's are cheaper to parse than the full <algorithm> - the
// single most expensive standard header. Other standard libraries only ship
// the monolithic header.
#if __has_include(<__algorithm/sort.h>) \
    && __has_include(<__algorithm/stable_sort.h>)
#include <__algorithm/sort.h>
#include <__algorithm/stable_sort.h>
#else
#include <algorithm>
#endif
