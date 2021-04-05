
#include <atomic>

template <class T>
inline void atomic_store(volatile void* ptr, T value, std::memory_order order) {
  if (order != std::memory_order_relaxed) {
    asm volatile("dmb");
  }
  *reinterpret_cast<volatile T*>(ptr) = value;
  switch (order) {
    case std::memory_order_seq_cst:
    case std::memory_order_acq_rel:
    case std::memory_order_acquire:
      asm volatile("dmb");
      break;
    default:
      break;
  }
}

extern "C" void __atomic_store_8(volatile void* ptr, uint64_t value,
                                 int order) {
  asm volatile("cpsid i");
  atomic_store(ptr, value, static_cast<std::memory_order>(order));
  asm volatile("cpsie i");
}

extern "C" void __atomic_store_4(volatile void* ptr, unsigned int value,
                                 int order) {
  atomic_store(ptr, value, static_cast<std::memory_order>(order));
}

extern "C" void __atomic_store_2(volatile void* ptr, uint16_t value,
                                 int order) {
  atomic_store(ptr, value, static_cast<std::memory_order>(order));
}

extern "C" void __atomic_store_1(volatile void* ptr, uint8_t value, int order) {
  atomic_store(ptr, value, static_cast<std::memory_order>(order));
}

template <class T>
inline T atomic_load(const volatile void* ptr, std::memory_order order) {
  switch (order) {
    case std::memory_order_seq_cst:
    case std::memory_order_acq_rel:
    case std::memory_order_release:
      asm volatile("dmb");
      break;
    default:
      break;
  }
  T value = *reinterpret_cast<const volatile T*>(ptr);
  if (order != std::memory_order_relaxed) {
    asm volatile("dmb");
  }
  return value;
}

extern "C" uint64_t __atomic_load_8(const volatile void* ptr, int order) {
  asm volatile("cpsid i");
  const auto value =
      atomic_load<uint64_t>(ptr, static_cast<std::memory_order>(order));
  asm volatile("cpsie i");
  return value;
}

extern "C" unsigned int __atomic_load_4(const volatile void* ptr, int order) {
  return atomic_load<uint32_t>(ptr, static_cast<std::memory_order>(order));
}

extern "C" uint16_t __atomic_load_2(const volatile void* ptr, int order) {
  return atomic_load<uint16_t>(ptr, static_cast<std::memory_order>(order));
}

extern "C" uint8_t __atomic_load_1(const volatile void* ptr, int order) {
  return atomic_load<uint8_t>(ptr, static_cast<std::memory_order>(order));
}
