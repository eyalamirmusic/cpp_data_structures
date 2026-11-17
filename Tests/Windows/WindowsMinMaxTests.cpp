//Regression test: every public header must compile after a raw <windows.h>,
//whose min/max function-like macros break unparenthesized calls such as
//std::numeric_limits<int>::max(). Consumers shouldn't need NOMINMAX for this
//library, so make sure it isn't defined, include <windows.h> before anything
//else, then include every public header.
#ifdef NOMINMAX
#undef NOMINMAX
#endif

#include <windows.h>

#ifndef max
#error "<windows.h> was expected to define the max macro"
#endif

#include <ea_data_structures/ea_data_structures.h>
#include <ea_data_structures/Allocators/DefaultAllocators.h>
#include <ea_data_structures/Allocators/MultiPoolAllocator.h>
#include <ea_data_structures/Allocators/PMR.h>
#include <ea_data_structures/Allocators/SmallVectorAllocator.h>
#include <ea_data_structures/Allocators/StaticVectorAllocator.h>
#include <ea_data_structures/Flags/AtomicHelpers.h>
#include <ea_data_structures/Flags/Bool.h>
#include <ea_data_structures/Flags/CopyableAtomic.h>
#include <ea_data_structures/Flags/Locks.h>
#include <ea_data_structures/Flags/RecursiveSpinLock.h>
#include <ea_data_structures/Flags/SpinHint.h>
#include <ea_data_structures/Pointers/Any.h>
#include <ea_data_structures/Pointers/AnyRef.h>
#include <ea_data_structures/Pointers/Broadcaster.h>
#include <ea_data_structures/Pointers/Broadcasting.h>
#include <ea_data_structures/Pointers/CallbackFunc.h>
#include <ea_data_structures/Pointers/Cloneable.h>
#include <ea_data_structures/Pointers/DynamicFunc.h>
#include <ea_data_structures/Pointers/OwningPointer.h>
#include <ea_data_structures/Pointers/Ref.h>
#include <ea_data_structures/Pointers/RefOrOwn.h>
#include <ea_data_structures/Structures/Array.h>
#include <ea_data_structures/Structures/BufferView.h>
#include <ea_data_structures/Structures/CircularBuffer.h>
#include <ea_data_structures/Structures/CopyOnWrite.h>
#include <ea_data_structures/Structures/Fifo.h>
#include <ea_data_structures/Structures/Filtered.h>
#include <ea_data_structures/Structures/FixedDynamicArray.h>
#include <ea_data_structures/Structures/MapVector.h>
#include <ea_data_structures/Structures/MultiVector.h>
#include <ea_data_structures/Structures/OwnedVector.h>
#include <ea_data_structures/Structures/PlanarView.h>
#include <ea_data_structures/Structures/SharedGUIData.h>
#include <ea_data_structures/Structures/SizeType.h>
#include <ea_data_structures/Structures/SmallVector.h>
#include <ea_data_structures/Structures/Span.h>
#include <ea_data_structures/Structures/SpecialVectors.h>
#include <ea_data_structures/Structures/StaticVector.h>
#include <ea_data_structures/Structures/Vector.h>
#include <ea_data_structures/Utilities/GenericUtilities.h>
#include <ea_data_structures/Utilities/MapUtilities.h>
#include <ea_data_structures/Utilities/Sort.h>
#include <ea_data_structures/Utilities/StaticObjects.h>
#include <ea_data_structures/Utilities/TupleUtilities.h>
#include <ea_data_structures/Utilities/VectorUtilities.h>
#include <ea_data_structures/ValueWrapper/Constructed.h>
#include <ea_data_structures/ValueWrapper/RawStorage.h>
#include <ea_data_structures/ValueWrapper/Value.h>
#include <ea_data_structures/ValueWrapper/Variant.h>
#include <ea_data_structures/ValueWrapper/VariantVisit.h>

#include <NanoTest/NanoTest.h>
#include <climits>

using namespace nano;

auto windowsMinMax = test("Windows.headers_compile_without_NOMINMAX") = []
{
    check(EA::ValueWrapper<int>::getMax() == INT_MAX);
    check(EA::ValueWrapper<int>::getMin() == INT_MIN);

    auto vec = EA::StaticVector<int, 4>();
    vec.resize(10);
    check(vec.size() == 4);
    check(vec.getLastValidElementIndex() == 3);
};
