#pragma once

namespace EA
{

//A templated base class to implement the clone pattern in a polymorphic object

template <typename Base>
class CloneableInterface
{
public:
    virtual ~CloneableInterface() = default;
    virtual Base* clone() const
    {
        assert(false);
        //You must implement this function to copy this class in a polymorphic way

        return nullptr;
    }
};

//A templated implementation of the clone method you can derive from

template <typename Base, typename Derived>
class Cloneable : public Base
{
public:
    Base* clone() const override
    {
        auto& converted = static_cast<const Derived&>(*this);
        return new Derived(converted);
    }
};
} // namespace EA
