
/**
 * MIT License
 *
 * Copyright (c) 2021 Francisco Javier Alvarez Garcia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <atomic>
#include <cstdint>
#include <type_traits>

// Type traits that check if an action returns void
template <class Action, class... Args>
using returns_void = std::is_void<std::result_of_t<Action(Args...)>>;

template <class Action, class... Args>
inline constexpr bool returns_void_v = returns_void<Action, Args...>::value;

/*
 * @brief Gets the state of the processors interrupt mask. This is 1 if
 * interrupts are masked. 0 otherwise.
 */
inline auto get_interrupt_mask() -> bool {
  std::uint32_t primask;
  asm volatile("mrs %0, primask" : "=r"(primask) :);
  return primask != 0;
}

/**
 * @brief Runs some code within a critical section. Ensures that the interrupt
 * state is restored if it needed to disable interrupts.
 */
template <class Action, std::enable_if_t<std::is_invocable_v<Action> &&
                                             !returns_void_v<Action>,
                                         bool> = false>
inline auto critical_section(Action action) {
  const auto previously_enabled = get_interrupt_mask() == 0;
  // Disable interrupts only if they were actually enabled. Otherwise there is
  // no harm done, as they are already disabled
  if (previously_enabled) {
    asm volatile("cpsid i");
  }

  // We execute the action in the critical section and capture the return value
  const auto retval = action();

  // We reenable interrupts if we disabled them, otherwise someone else must
  // already be relying on them being disabled, so it is not safe to reenable
  // them at this point. no harm done, as they are already disabled
  if (previously_enabled) {
    asm volatile("cpsie i");
  }
  return retval;
}

/**
 * @brief Runs some code within a critical section. Ensures that the interrupt
 * state is restored if it needed to disable interrupts.
 *
 * This verision of
 */
template <class Action, std::enable_if_t<std::is_invocable_v<Action> &&
                                             returns_void_v<Action>,
                                         bool> = false>
inline auto critical_section(Action action) {
  const auto previously_enabled = get_interrupt_mask() == 0;
  // Disable interrupts only if they were actually enabled. Otherwise there is
  // no harm done, as they are already disabled
  if (previously_enabled) {
    asm volatile("cpsid i");
  }

  // We execute the action in the critical section
  action();

  // We reenable interrupts if we disabled them, otherwise someone else must
  // already be relying on them being disabled, so it is not safe to reenable
  // them at this point. no harm done, as they are already disabled
  if (previously_enabled) {
    asm volatile("cpsie i");
  }
}

inline void memory_barrier() { asm volatile("dmb"); }

template <class T>
inline void atomic_store(volatile void* ptr, T value, std::memory_order order) {
  if (order != std::memory_order_relaxed) {
    memory_barrier();
  }
  *reinterpret_cast<volatile T*>(ptr) = value;
  switch (order) {
    case std::memory_order_seq_cst:
    case std::memory_order_acq_rel:
    case std::memory_order_acquire:
      memory_barrier();
      break;
    default:
      break;
  }
}

extern "C" void __atomic_store_8(volatile void* ptr, uint64_t value,
                                 int order) {
  critical_section([&]() {
    atomic_store(ptr, value, static_cast<std::memory_order>(order));
  });
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
      memory_barrier();
      break;
    default:
      break;
  }
  T value = *reinterpret_cast<const volatile T*>(ptr);
  if (order != std::memory_order_relaxed) {
    memory_barrier();
  }
  return value;
}

extern "C" uint64_t __atomic_load_8(const volatile void* ptr, int order) {
  const auto value = critical_section([&]() {
    return atomic_load<uint64_t>(ptr, static_cast<std::memory_order>(order));
  });
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

template <class T>
T atomic_exchange(volatile void* ptr, T value, std::memory_order order) {
  return critical_section([&]() {
    if (order != std::memory_order_relaxed) {
      memory_barrier();
    }
    auto& atomic = *reinterpret_cast<volatile T*>(ptr);
    const auto prev_val = atomic;
    atomic = value;
    if (order != std::memory_order_relaxed) {
      memory_barrier();
    }
    return prev_val;
  });
}

extern "C" uint64_t __atomic_exchange_8(volatile void* ptr, uint64_t value,
                                        int order) {
  return atomic_exchange(ptr, value, static_cast<std::memory_order>(order));
}

extern "C" unsigned int __atomic_exchange_4(volatile void* ptr,
                                            unsigned int value, int order) {
  return atomic_exchange(ptr, value, static_cast<std::memory_order>(order));
}

extern "C" uint16_t __atomic_exchange_2(volatile void* ptr, uint16_t value,
                                        int order) {
  return atomic_exchange(ptr, value, static_cast<std::memory_order>(order));
}

extern "C" uint8_t __atomic_exchange_1(volatile void* ptr, uint8_t value,
                                       int order) {
  return atomic_exchange(ptr, value, static_cast<std::memory_order>(order));
}
