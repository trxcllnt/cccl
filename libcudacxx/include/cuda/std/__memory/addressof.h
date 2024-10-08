// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___MEMORY_ADDRESSOF_H
#define _LIBCUDACXX___MEMORY_ADDRESSOF_H

#include <cuda/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// addressof
// NVCXX has the builtin defined but did not mark it as supported
#if defined(_LIBCUDACXX_ADDRESSOF)

template <class _Tp>
_LIBCUDACXX_HIDE_FROM_ABI _CCCL_CONSTEXPR_CXX14 _LIBCUDACXX_NO_CFI _Tp* addressof(_Tp& __x) noexcept
{
  return __builtin_addressof(__x);
}

#else

template <class _Tp>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_NO_CFI _Tp* addressof(_Tp& __x) noexcept
{
  return reinterpret_cast<_Tp*>(const_cast<char*>(&reinterpret_cast<const volatile char&>(__x)));
}

#endif // defined(_LIBCUDACXX_ADDRESSOF)

#if defined(_LIBCUDACXX_HAS_OBJC_ARC) && !defined(_LIBCUDACXX_PREDEFINED_OBJC_ARC_ADDRESSOF)
// Objective-C++ Automatic Reference Counting uses qualified pointers
// that require special addressof() signatures. When
// _LIBCUDACXX_PREDEFINED_OBJC_ARC_ADDRESSOF is defined, the compiler
// itself is providing these definitions. Otherwise, we provide them.
template <class _Tp>
_LIBCUDACXX_HIDE_FROM_ABI __strong _Tp* addressof(__strong _Tp& __x) noexcept
{
  return &__x;
}

#  ifdef _LIBCUDACXX_HAS_OBJC_ARC_WEAK
template <class _Tp>
_LIBCUDACXX_HIDE_FROM_ABI __weak _Tp* addressof(__weak _Tp& __x) noexcept
{
  return &__x;
}
#  endif

template <class _Tp>
_LIBCUDACXX_HIDE_FROM_ABI __autoreleasing _Tp* addressof(__autoreleasing _Tp& __x) noexcept
{
  return &__x;
}

template <class _Tp>
_LIBCUDACXX_HIDE_FROM_ABI __unsafe_unretained _Tp* addressof(__unsafe_unretained _Tp& __x) noexcept
{
  return &__x;
}
#endif

template <class _Tp>
_Tp* addressof(const _Tp&&) noexcept = delete;

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___MEMORY_ADDRESSOF_H
