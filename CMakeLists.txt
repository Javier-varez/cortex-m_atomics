cmake_minimum_required(VERSION 3.16)

project(Cortex-M_Atomics)

add_library(cortex-m_atomics STATIC
  src/atomic.cpp)

target_compile_options(cortex-m_atomics
  PRIVATE
    -Wall
    -Wextra)

target_compile_features(cortex-m_atomics
  PRIVATE
    cxx_std_20)

target_compile_definitions(cortex-m_atomics
  PRIVATE
    -D__LIBATOMIC_SUPPORTS_I1
    -D__LIBATOMIC_SUPPORTS_I2
    -D__LIBATOMIC_SUPPORTS_I4)
target_include_directories(cortex-m_atomics
  PUBLIC
    inc)
target_compile_options(cortex-m_atomics
  PRIVATE
    -Os)
