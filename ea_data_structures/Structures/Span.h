#pragma once

#include "SizeType.h"
#include "../Utilities/VectorUtilities.h"
#include <cassert>
#include <cstddef>
#include <iterator>
#include <limits>
#include <span>
#include <type_traits>

namespace EA
{
template <typename T>
class Span;

namespace Detail
{
template <typename T>
struct IsSpan : std::false_type
{
};

template <typename T>
struct IsSpan<Span<T>> : std::true_type
{
};

template <typename T>
struct IsStdSpan : std::false_type
{
};

template <typename T, std::size_t N>
struct IsStdSpan<std::span<T, N>> : std::true_type
{
};

//The element type a contiguous container's data() points at, keeping the
//cv-qualification (so a const Vector<int> gives 'const int')
template <typename Container>
using ElementOf =
    std::remove_reference_t<decltype(*std::declval<Container&>().data())>;

template <typename Container>
concept HasDataAndSize = requires(Container& container) {
    container.data();
    container.size();
};

template <typename Container>
concept HasGetSize = requires(const Container& container) {
    { container.getSize() } -> std::convertible_to<std::size_t>;
};

//The (*)[] conversion is what allows T* -> const T* while rejecting
//Derived* -> Base*, which would silently break the element stride
template <typename Container, typename T>
concept ViewableAs = std::is_convertible_v<ElementOf<Container> (*)[], T (*)[]>;

//The whole element count of a container. The EA containers report an int
//from size() and the full size_t from getSize(); the STL ones are size_t
//already, so size() is the full count there
template <typename Container>
constexpr std::size_t fullSizeOf(const Container& container)
{
    if constexpr (HasGetSize<Container>)
        return (std::size_t) container.getSize();
    else
        return (std::size_t) container.size();
}

constexpr bool fitsInt(std::size_t size)
{
    return size <= (std::size_t) std::numeric_limits<int>::max();
}
} // namespace Detail

//Any contiguous container (one with data() and size()) whose elements can be
//viewed as T. Covers EA::Vector, Array, StaticVector, SmallVector,
//std::vector, std::array, std::string, ... with no per-type code. std::span
//has its own constructor, since a view may be built from a temporary one
template <typename Container, typename T>
concept SpanCompatible =
    !Detail::IsSpan<std::remove_cvref_t<Container>>::value
    && !Detail::IsStdSpan<std::remove_cvref_t<Container>>::value
    && !std::is_array_v<std::remove_cvref_t<Container>>
    && Detail::HasDataAndSize<std::remove_reference_t<Container>>
    && Detail::ViewableAs<std::remove_reference_t<Container>, T>;

//A non-owning view over a contiguous sequence of T: a std::span wrapped the
//way Vector wraps std::vector, with the int/SizeType indexing and the
//Vectors:: helper set used by the rest of this library.
//
//size() is an int, like every other container here, and asserts that the
//count fits one. getSize() is the full size_t count, for a view over more
//than an int describes (a memory-mapped file, say) and for handing to STL
//calls; getSpan() is the std::span itself.
//
//Constness is shallow, exactly like std::span: every accessor is a const member
//returning T&, and a read-only view is spelled Span<const T>. Taking a Span<T>
//by const reference therefore still lets you write through it.
//
//Helpers that would need to resize (add, transform, filter, ...) are absent by
//design, and the mutating ones are unavailable when T is const.
template <typename T>
class Span
{
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = int;
    using Iterator = T*;
    using Const_Iterator = const T*;
    using ContainerType = std::span<T>;

    //Passed as subspan's count to mean "everything up to the end"
    static constexpr int toEnd = -1;

    Span() = default;

    constexpr Span(T* dataToUse, SizeType sizeToUse) noexcept
        : container(dataToUse, sizeToUse.get<std::size_t>())
    {
    }

    constexpr Span(T* first, T* last) noexcept
        : container(first, last)
    {
    }

    template <std::size_t N>
    constexpr Span(T (&array)[N]) noexcept
        : container(array, N)
    {
    }

    //A std::span is a view already, so binding a temporary one is safe
    template <typename U, std::size_t N>
        requires(std::is_convertible_v<U (*)[], T (*)[]>)
    constexpr Span(std::span<U, N> other) noexcept
        : container(other.data(), other.size())
    {
    }

    template <typename Container>
        requires(SpanCompatible<Container, T>
                 && std::is_lvalue_reference_v<Container>)
    constexpr Span(Container&& other) noexcept
        : container(other.data(), Detail::fullSizeOf(other))
    {
    }

    //A view must never be bound to a temporary container
    template <typename Container>
        requires(SpanCompatible<Container, T>
                 && !std::is_lvalue_reference_v<Container>)
    Span(Container&&) = delete;

    //Allows Span<T> -> Span<const T>, but not the other way around
    template <typename U>
        requires(!std::same_as<U, T> && std::is_convertible_v<U (*)[], T (*)[]>)
    constexpr Span(const Span<U>& other) noexcept
        : container(other.data(), other.getSize())
    {
    }

    constexpr int size() const noexcept
    {
        assert(Detail::fitsInt(container.size()));
        return (int) container.size();
    }

    constexpr std::size_t getSize() const noexcept { return container.size(); }

    constexpr int sizeInBytes() const noexcept
    {
        assert(Detail::fitsInt(container.size_bytes()));
        return (int) container.size_bytes();
    }

    constexpr std::size_t getSizeInBytes() const noexcept
    {
        return container.size_bytes();
    }

    constexpr bool empty() const noexcept { return container.empty(); }

    constexpr T* data() const noexcept { return container.data(); }

    constexpr ContainerType getSpan() const noexcept { return container; }

    constexpr T& operator[](SizeType index) const noexcept
    {
        return container[index.get<std::size_t>()];
    }

    constexpr T& get(SizeType index) const noexcept { return (*this)[index]; }

    constexpr T& front() const noexcept { return container.front(); }
    constexpr T& back() const noexcept { return container.back(); }

    constexpr T* begin() const noexcept { return container.data(); }
    constexpr T* end() const noexcept { return container.data() + container.size(); }

    constexpr const T* cbegin() const noexcept { return begin(); }
    constexpr const T* cend() const noexcept { return end(); }

    constexpr auto rbegin() const noexcept
    {
        return std::reverse_iterator<T*>(end());
    }

    constexpr auto rend() const noexcept
    {
        return std::reverse_iterator<T*>(begin());
    }

    constexpr Span first(SizeType count) const noexcept
    {
        return Span(container.first(count.get<std::size_t>()));
    }

    constexpr Span last(SizeType count) const noexcept
    {
        return Span(container.last(count.get<std::size_t>()));
    }

    //A count of toEnd takes everything from offset onwards: as a size_t it is
    //std::dynamic_extent, which is what std::span's subspan means by it too
    constexpr Span subspan(SizeType offset, SizeType count = toEnd) const noexcept
    {
        return Span(
            container.subspan(offset.get<std::size_t>(), count.get<std::size_t>()));
    }

    //The half-open range [first, last) as its own view
    constexpr Span between(SizeType first, SizeType last) const noexcept
    {
        auto start = first.get<std::size_t>();
        return Span(container.subspan(start, last.get<std::size_t>() - start));
    }

    //Element-wise, like Vector and Array. Note that std::span has no
    //comparison at all, so this is a deliberate addition
    template <typename U>
    bool operator==(const Span<U>& other) const
    {
        if (getSize() != other.getSize())
            return false;

        for (std::size_t index = 0; index < getSize(); ++index)
        {
            if (!(container[index] == other[index]))
                return false;
        }

        return true;
    }

    int getLastElementIndex() const noexcept { return size() - 1; }

    template <typename A>
    bool contains(const A& element) const
    {
        return Vectors::contains(*this, element);
    }

    template <typename A>
    int getIndexOf(const A& element) const
    {
        return Vectors::getIndexOf(*this, element);
    }

    template <typename Predicate>
    int getIndexIf(Predicate&& predicate) const
    {
        return Vectors::getIndexIf(*this, std::forward<Predicate>(predicate));
    }

    template <typename Predicate>
    int countIf(Predicate&& predicate) const
    {
        return Vectors::countIf(*this, std::forward<Predicate>(predicate));
    }

    template <typename A>
    T* find(const A& element) const
    {
        return Vectors::find(*this, element);
    }

    template <typename Predicate>
    T* findIf(Predicate&& predicate) const
    {
        return Vectors::findIf(*this, std::forward<Predicate>(predicate));
    }

    template <typename Value, typename KeyOf = Vectors::Identity>
    int lowerBoundIndex(const Value& value, KeyOf keyOf = {}) const
    {
        return Vectors::lowerBoundIndex(*this, value, keyOf);
    }

    template <typename Value, typename KeyOf = Vectors::Identity>
    int upperBoundIndex(const Value& value, KeyOf keyOf = {}) const
    {
        return Vectors::upperBoundIndex(*this, value, keyOf);
    }

    template <typename FloatType>
    FloatType getIndexAsRelative(int index) const
    {
        return Vectors::getIndexAsRelative<FloatType>(*this, index);
    }

    template <typename FloatType>
    int getRelativeIndex(FloatType proportion) const
    {
        return Vectors::getRelativeIndex(*this, proportion);
    }

    template <typename FloatType>
    T& getRelative(FloatType proportion) const
    {
        return Vectors::getRelative(*this, proportion);
    }

    template <typename FloatType, typename A>
    FloatType getRelativeIndexOf(const A& item) const
    {
        return Vectors::getRelativeIndexOf<FloatType>(*this, item);
    }

    void fill(const T& value) const
        requires(!std::is_const_v<T>)
    {
        Vectors::fill(*this, value);
    }

    void fill(const T& value, int numItems) const
        requires(!std::is_const_v<T>)
    {
        Vectors::fill(*this, value, numItems);
    }

    //Copies elements from the other container, stopping at whichever of the
    //two runs out first. The view can't resize, so nothing is ever appended
    template <typename Source>
    void copyFrom(const Source& other) const
        requires(!std::is_const_v<T>)
    {
        auto numToCopy = Detail::fullSizeOf(other);

        if (numToCopy > getSize())
            numToCopy = getSize();

        for (std::size_t index = 0; index < numToCopy; ++index)
            container[index] = other[index];
    }

    template <typename Source>
    void fillFrom(const Source& other) const
        requires(!std::is_const_v<T>)
    {
        copyFrom(other);
    }

    template <typename Source>
    void mixFrom(const Source& other) const
        requires(!std::is_const_v<T>)
    {
        Vectors::mixFrom(*this, other);
    }

    const Span& sort(bool forward = true) const
        requires(!std::is_const_v<T>)
    {
        Vectors::sort(*this, forward);
        return *this;
    }

    template <typename Predicate>
    const Span& sort(const Predicate& pred, bool forward = true) const
        requires(!std::is_const_v<T>)
    {
        Vectors::sort(*this, pred, forward);
        return *this;
    }

    const Span& stableSort(bool forward = true) const
        requires(!std::is_const_v<T>)
    {
        Vectors::stableSort(*this, forward);
        return *this;
    }

    template <typename Predicate>
    const Span& stableSort(const Predicate& pred, bool forward = true) const
        requires(!std::is_const_v<T>)
    {
        Vectors::stableSort(*this, pred, forward);
        return *this;
    }

    const Span& reverse() const
        requires(!std::is_const_v<T>)
    {
        Vectors::reverse(*this);
        return *this;
    }

private:
    ContainerType container;
};

template <typename T>
Span(T*, SizeType) -> Span<T>;

template <typename T>
Span(T*, T*) -> Span<T>;

template <typename T, std::size_t N>
Span(T (&)[N]) -> Span<T>;

template <typename T, std::size_t N>
Span(std::span<T, N>) -> Span<T>;

template <typename Container>
Span(Container&&) -> Span<Detail::ElementOf<std::remove_reference_t<Container>>>;

} // namespace EA
