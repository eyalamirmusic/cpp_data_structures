#pragma once

/*Bool: A bool-like struct for situations where a container like std::vector<bool>
doesn't behave like you'd like. You can then use std::vector<Bool> instead

Also unlike the regular bool, this default initializes.
*/
namespace EA
{
struct Bool
{
    Bool() = default;
    Bool(bool valueToUse)
        : value(valueToUse)
    {
    }

    Bool& operator=(const bool& other)
    {
        value = other;
        return *this;
    }

    operator bool() const { return value; }
    bool operator==(const Bool other) const { return value == other.value; }
    bool operator==(const bool other) const { return value == other; }
    bool operator!=(const Bool other) const { return value != other.value; }
    bool operator!=(const bool other) const { return value != other; }

    bool value = false;
};
} // namespace EA