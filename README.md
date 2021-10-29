ea_data_structures:

A collection of useful C++ data structures and helper functions I find myself using quite alot.

Some of these are wrappers around STL containers, some are alternatives to them with some useful functionality, and some are just tools for my needs.

Some examples of what's in here:

``Vector<T>``
``Array<T, Size>``

Wrappers around the STL ones with int semantics instead of size_t, and many helper functions like 
```cpp
EA::Vector<int> vec;

vec.sort;

if (vec.contains(element))
  doSomething();

vec.eraseIf(pred);
vec.addIfNotThere(element);

```
And many others.

``MapVector<Key, Value>``
An std::map-like container that's based on a vector, allowing a way faster iteration with the cost of slower random access/inserting.

``GUIToRealTime<T>``
A FIFO-like structure allowing lock-free data sharing between the GUI and audio/real time threads

``OwningPointer<T>``
An alternative to std::unique_ptr with slightly different semantics and more features.

``Constructed<T>``
Kinda similar to std::optional, allows for an object to be created on the stack, and created/destroyed dynamically
without the need for heap allocation

``Atomic<T>``
A wrapper around std::atomic, but with a compile time static checking that makes sure T is always lock free.

``CopyableAtomic<T>``
Allows copying a std::atomic

``CircularBuffer<T>``
Encapsulates the basic Circular Buffer sometimes needed for delay effects, etc

``SmallVector<T, Size>``
A vector with a custom allocator, allows a preallocated stack size to be used before any heap access happens.

And many others...

More documentations, examples, unit tests, coming soon...

To use this library, just include ``ea_data_structures.h`` in your code.
If like me you're using the JUCE framework, you can also use it as a JUCE-style module in CMake/Projucer

Hope you find it useful!
