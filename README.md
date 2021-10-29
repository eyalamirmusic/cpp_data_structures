ea_data_structures:

A collection of useful C++ data structures and helper functions I find myself using quite alot.

Some of these are wrappers around STL containers, some are alternatives to them with some useful functionality, and some are just tools for my needs.

Some examples of what's in here:

``Vector<T>/Array<T, Size>``

Wrappers around the STL ones with int semantics instead of size_t, and many helper functions like `vec.sort`, `vec.containes(element)`, `vec.eraseIf(pred)`, `vec.addIfNotThere(element)`, etc.

``MapVector<Key, Value>``
An std::map-like container that's based on a vector, allowing a way faster iteration with the cost of slower random access/inserting.

``GUIToRealTime<T>``
A FIFO-like structure allowing lock-free data sharing between the GUI and audio

``OwningPointer<T>``
An alternative to std::unique_ptr with slightly different semantics and more features.

And many others...

More documentations, examples, unit tests, coming soon...

To use this library, just include ``ea_data_structures.h`` in your code.
If like me you're using the JUCE framework, you can also use it as a JUCE-style module in CMake/Projucer

Hope you find it useful!