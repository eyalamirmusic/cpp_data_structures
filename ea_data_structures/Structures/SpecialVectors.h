#pragma once

#include "../Allocators/SmallVectorAllocator.h"
#include "../Allocators/StaticVectorAllocator.h"
#include "Vector.h"

namespace EA
{
template <typename T, size_t N>
using SmallVector = Vector<T, Allocators::SmallVector::Allocator<T, N>>;

template <typename T, size_t N>
using StaticVector = Vector<T, Allocators::StaticVector::Allocator<T, N>>;
}
