#pragma once

namespace EA
{
inline void toggleBool(bool& value) noexcept
{
    value = !value;
}
} // namespace EA