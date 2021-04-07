# Cortex-M Atomics

Polyfill implementation of atomics for the `armv6m` architecture. It uses critical sections for CAS operations, while just normal ldr and str instructions for aligned atomic read/writes, which don't need the ldrex or strex instructions.

Using this library with multi-core systems will **not** ensure operations are seen as atomic from the other core. Special synchronization mechanisms need to be used, which largely depend on the multi-core architecture

This library does not provide any headers, since builds on top of the standard `atomic` and `stdatomic.h` headers by implementing compiler intrinsics for `Clang` and `GCC`. The only requirement is to link against it.

Can be used with both arm-none-eabi-gcc and clang-10 or newer.
