//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// type_traits

// remove_pointer

#include <cuda/std/type_traits>

#include "test_macros.h"

template <class T, class U>
__host__ __device__ void test_remove_pointer()
{
  static_assert(cuda::std::is_same_v<U, typename cuda::std::remove_pointer<T>::type>);
  static_assert(cuda::std::is_same_v<U, cuda::std::remove_pointer_t<T>>);
}

int main(int, char**)
{
  test_remove_pointer<void, void>();
  test_remove_pointer<int, int>();
  test_remove_pointer<int[3], int[3]>();
  test_remove_pointer<int*, int>();
  test_remove_pointer<const int*, const int>();
  test_remove_pointer<int**, int*>();
  test_remove_pointer<int** const, int*>();
  test_remove_pointer<int* const*, int* const>();
  test_remove_pointer<const int**, const int*>();

  test_remove_pointer<int&, int&>();
  test_remove_pointer<const int&, const int&>();
  test_remove_pointer<int (&)[3], int (&)[3]>();
  test_remove_pointer<int (*)[3], int[3]>();
  test_remove_pointer<int*&, int*&>();
  test_remove_pointer<const int*&, const int*&>();

  return 0;
}
