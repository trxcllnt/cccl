//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <cuda/std/tuple>

// template <class... Types> class tuple;

// template <class... UTypes> tuple(tuple<UTypes...>&& u);

#include <cuda/std/__memory_>
#include <cuda/std/cassert>
#include <cuda/std/tuple>
#include <cuda/std/type_traits>

#include "test_macros.h"

struct Explicit
{
  int value;
  __host__ __device__ explicit Explicit(int x)
      : value(x)
  {}
};

struct Implicit
{
  int value;
  __host__ __device__ Implicit(int x)
      : value(x)
  {}
};

struct B
{
  int id_;

  __host__ __device__ explicit B(int i)
      : id_(i)
  {}

  __host__ __device__ virtual ~B() {}
};

struct D : B
{
  __host__ __device__ explicit D(int i)
      : B(i)
  {}
};

struct BonkersBananas
{
  template <class T>
  __host__ __device__ operator T() &&;
  template <class T, class = void>
  __host__ __device__ explicit operator T() && = delete;
};

__host__ __device__ void test_bonkers_bananas_conversion()
{
  using ReturnType = cuda::std::tuple<int, int>;
  static_assert(cuda::std::is_convertible<BonkersBananas, ReturnType>(), "");
  // TODO: possibly a compiler bug that allows NVCC to think that it can construct a tuple from this type
  //  static_assert(!cuda::std::is_constructible<ReturnType, BonkersBananas>(), "");
}

int main(int, char**)
{
  {
    using T0 = cuda::std::tuple<long>;
    using T1 = cuda::std::tuple<long long>;
    T0 t0(2);
    T1 t1 = cuda::std::move(t0);
    assert(cuda::std::get<0>(t1) == 2);
  }
  {
    using T0 = cuda::std::tuple<long, char>;
    using T1 = cuda::std::tuple<long long, int>;
    T0 t0(2, 'a');
    T1 t1 = cuda::std::move(t0);
    assert(cuda::std::get<0>(t1) == 2);
    assert(cuda::std::get<1>(t1) == int('a'));
  }
  {
    using T0 = cuda::std::tuple<long, char, D>;
    using T1 = cuda::std::tuple<long long, int, B>;
    T0 t0(2, 'a', D(3));
    T1 t1 = cuda::std::move(t0);
    assert(cuda::std::get<0>(t1) == 2);
    assert(cuda::std::get<1>(t1) == int('a'));
    assert(cuda::std::get<2>(t1).id_ == 3);
  }
  {
    D d(3);
    using T0 = cuda::std::tuple<long, char, D&>;
    using T1 = cuda::std::tuple<long long, int, B&>;
    T0 t0(2, 'a', d);
    T1 t1 = cuda::std::move(t0);
    d.id_ = 2;
    assert(cuda::std::get<0>(t1) == 2);
    assert(cuda::std::get<1>(t1) == int('a'));
    assert(cuda::std::get<2>(t1).id_ == 2);
  }

  {
    using T0 = cuda::std::tuple<long, char, cuda::std::unique_ptr<D>>;
    using T1 = cuda::std::tuple<long long, int, cuda::std::unique_ptr<B>>;
    T0 t0(2, 'a', cuda::std::unique_ptr<D>(new D(3)));
    T1 t1 = cuda::std::move(t0);
    assert(cuda::std::get<0>(t1) == 2);
    assert(cuda::std::get<1>(t1) == int('a'));
    assert(cuda::std::get<2>(t1)->id_ == 3);
  }

  {
    cuda::std::tuple<int> t1(42);
    cuda::std::tuple<Explicit> t2(cuda::std::move(t1));
    assert(cuda::std::get<0>(t2).value == 42);
  }
  {
    cuda::std::tuple<int> t1(42);
    cuda::std::tuple<Implicit> t2 = cuda::std::move(t1);
    assert(cuda::std::get<0>(t2).value == 42);
  }

  return 0;
}
