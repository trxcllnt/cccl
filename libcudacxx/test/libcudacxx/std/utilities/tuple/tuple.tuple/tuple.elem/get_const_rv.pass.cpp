//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <cuda/std/tuple>

// template <class... Types> class tuple;

// template <size_t I, class... Types>
//   const typename tuple_element<I, tuple<Types...> >::type&&
//   get(const tuple<Types...>&& t);

#include <cuda/std/tuple>
#include <cuda/std/utility>
// cuda::std::string not supported
// #include <cuda/std/string>
#include <cuda/std/cassert>
#include <cuda/std/type_traits>

#include "test_macros.h"

int main(int, char**)
{
  {
    using T = cuda::std::tuple<int>;
    const T t(3);
    static_assert(cuda::std::is_same<const int&&, decltype(cuda::std::get<0>(cuda::std::move(t)))>::value, "");
    static_assert(noexcept(cuda::std::get<0>(cuda::std::move(t))), "");
    const int&& i = cuda::std::get<0>(cuda::std::move(t));
    assert(i == 3);
  }

  // cuda::std::string not supported
  /*
  {
  using T = cuda::std::tuple<cuda::std::string, int>;
  const T t("high", 5);
  static_assert(cuda::std::is_same<const cuda::std::string&&, decltype(cuda::std::get<0>(cuda::std::move(t)))>::value,
  ""); static_assert(noexcept(cuda::std::get<0>(cuda::std::move(t))), ""); static_assert(cuda::std::is_same<const int&&,
  decltype(cuda::std::get<1>(cuda::std::move(t)))>::value, "");
  static_assert(noexcept(cuda::std::get<1>(cuda::std::move(t))), "");
  const cuda::std::string&& s = cuda::std::get<0>(cuda::std::move(t));
  const int&& i = cuda::std::get<1>(cuda::std::move(t));
  assert(s == "high");
  assert(i == 5);
  }
  */

  {
    int x       = 42;
    int const y = 43;
    cuda::std::tuple<int&, int const&> const p(x, y);
    static_assert(cuda::std::is_same<int&, decltype(cuda::std::get<0>(cuda::std::move(p)))>::value, "");
    static_assert(noexcept(cuda::std::get<0>(cuda::std::move(p))), "");
    static_assert(cuda::std::is_same<int const&, decltype(cuda::std::get<1>(cuda::std::move(p)))>::value, "");
    static_assert(noexcept(cuda::std::get<1>(cuda::std::move(p))), "");
  }

  {
    int x       = 42;
    int const y = 43;
    cuda::std::tuple<int&&, int const&&> const p(cuda::std::move(x), cuda::std::move(y));
    static_assert(cuda::std::is_same<int&&, decltype(cuda::std::get<0>(cuda::std::move(p)))>::value, "");
    static_assert(noexcept(cuda::std::get<0>(cuda::std::move(p))), "");
    static_assert(cuda::std::is_same<int const&&, decltype(cuda::std::get<1>(cuda::std::move(p)))>::value, "");
    static_assert(noexcept(cuda::std::get<1>(cuda::std::move(p))), "");
  }

  {
    using T = cuda::std::tuple<double, int>;
    constexpr const T t(2.718, 5);
    static_assert(cuda::std::get<0>(cuda::std::move(t)) == 2.718, "");
    static_assert(cuda::std::get<1>(cuda::std::move(t)) == 5, "");
  }

  return 0;
}
