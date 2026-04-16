# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

`ea_data_structures` is a **header-only C++17 library** of data structures, smart pointers, allocators, and utilities. Everything lives under the `EA::` namespace. The library is also shaped to work as a **JUCE module** (see the `BEGIN_JUCE_MODULE_DECLARATION` block in `ea_data_structures/ea_data_structures.h`), which is why every feature is included through that single umbrella header.

## Build

The CMake target is an `INTERFACE` library — there is nothing to compile on its own. Consumers link against `ea_data_structures` and include `ea_data_structures/ea_data_structures.h`:

```cmake
add_subdirectory(path/to/cpp_data_structures)
target_link_libraries(my_target PRIVATE ea_data_structures)
```

There is no test suite in-tree yet (README notes "unit tests, coming soon"), and no lint/build command beyond whatever consumers run. `_clang-format` and `.clang-tidy` at the repo root define the expected style and checks; run `clang-format` / `clang-tidy` against changed files before committing.

## Code layout

Everything user-facing is re-exported from `ea_data_structures/ea_data_structures.h`. When adding a new public header, add its `#include` there — otherwise JUCE-module consumers won't see it. The subdirectories group related concerns:

- `Structures/` — containers (`Vector`, `Array`, `MapVector`, `SmallVector`, `StaticVector`, `FixedDynamicArray`, `OwnedVector`, `MultiVector`, `CircularBuffer`, `Fifo`, `BufferView`, `CopyOnWrite`, `Filtered`, `SharedGUIData`). `SizeType.h` is the shared size alias.
- `Pointers/` — ownership and type-erasure primitives (`OwningPointer`, `Ref`, `RefOrOwn`, `Any`, `AnyRef`, `Cloneable`, `CallbackFunc`, `DynamicFunc`).
- `Allocators/` — allocator machinery backing `SmallVector` / `StaticVector`, plus a PMR wrapper and `MultiPoolAllocator`.
- `Flags/` — concurrency and atomic helpers (`Locks`, `RecursiveSpinLock`, `CopyableAtomic`, `AtomicHelpers`, `Bool`).
- `ValueWrapper/` — `Value` and `Constructed` (in-place-optional-like object with no heap allocation).
- `Utilities/` — free-function helpers (`VectorUtilities`, `MapUtilities`, `TupleUtilities`, `GenericUtilities`, `StaticObjects`).

## Conventions that matter for edits

- **Always run clang-format on every changed file.**
- **Use modern, RAII style.**
- **`int` size semantics, not `size_t`.** The `Vector` / `Array` wrappers deliberately expose `int`-based sizes and indices — do not "fix" this to `size_t`. Helper methods (`contains`, `eraseIf`, `addIfNotThere`, `sort`, …) are the whole point of these wrappers over raw STL.
- **Real-time safety.** `SharedGUIData` / `GUIToRealTime`, `Fifo`, `CircularBuffer`, and `Flags/` types are designed for audio/real-time threads — preserve lock-free / allocation-free behavior when editing them.
- **`Atomic<T>`** (in `Flags/`) static-asserts `T` is always lock-free; keep that guarantee intact.
- **Formatting:** Allman braces, 4-space indent, 85-column limit, left-aligned pointers, C++17, `SortIncludes: false` (header order is intentional — notably in `ea_data_structures.h`).
