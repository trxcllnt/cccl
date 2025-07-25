//===----------------------------------------------------------------------===//
//
// Part of CUDA Experimental in CUDA C++ Core Libraries,
// under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES.
//
//===----------------------------------------------------------------------===//

#ifndef CUDAX_TEST_CONTAINER_VECTOR_HELPER_H
#define CUDAX_TEST_CONTAINER_VECTOR_HELPER_H

#include <thrust/equal.h>

#include <cuda/functional>
#include <cuda/std/__algorithm_>
#include <cuda/std/iterator>
#include <cuda/std/type_traits>

#include <cuda/experimental/execution.cuh>
#include <cuda/experimental/memory_resource.cuh>

#include "test_resources.h"

namespace cudax = cuda::experimental;

// Default data to compare against
__device__ constexpr int device_data[] = {1, 42, 1337, 0, 12, -1};
constexpr int host_data[]              = {1, 42, 1337, 0, 12, -1};

template <class Buffer>
bool equal_range(const Buffer& buf)
{
  if constexpr (Buffer::__is_host_only)
  {
    buf.stream().sync();
    return cuda::std::equal(buf.begin(), buf.end(), cuda::std::begin(host_data), cuda::std::end(host_data));
  }
  else
  {
    cuda::experimental::__ensure_current_device guard{cuda::device_ref{0}};
    return buf.size() == cuda::std::size(device_data)
        && thrust::equal(
             thrust::cuda::par.on(buf.stream().get()), buf.begin(), buf.end(), cuda::get_device_address(device_data[0]));
  }
}

template <bool HostOnly, class T>
bool compare_value(const T& value, const T& expected)
{
  if constexpr (HostOnly)
  {
    return value == expected;
  }
  else
  {
    cuda::experimental::__ensure_current_device guard{cuda::device_ref{0}};
    // copy the value to host
    T host_value;
    _CCCL_TRY_CUDA_API(
      ::cudaMemcpy,
      "failed to copy value",
      cuda::std::addressof(host_value),
      cuda::std::addressof(value),
      sizeof(T),
      ::cudaMemcpyDefault);
    return host_value == expected;
  }
}

template <bool HostOnly, class T>
void assign_value(T& value, const T& input)
{
  if constexpr (HostOnly)
  {
    value = input;
  }
  else
  {
    cuda::experimental::__ensure_current_device guard{cuda::device_ref{0}};
    // copy the input to device
    _CCCL_TRY_CUDA_API(
      ::cudaMemcpy,
      "failed to copy value",
      cuda::std::addressof(value),
      cuda::std::addressof(input),
      sizeof(T),
      ::cudaMemcpyDefault);
  }
}

// Helper to compare a range with all equal values
struct equal_to_value
{
  int value_;

  template <class T>
  __host__ __device__ bool operator()(const T lhs, const T) const noexcept
  {
    return lhs == static_cast<T>(value_);
  }
};

template <class Buffer>
bool equal_size_value(const Buffer& buf, const size_t size, const int value)
{
  if constexpr (Buffer::__is_host_only)
  {
    buf.stream().sync();
    return buf.size() == size
        && cuda::std::equal(buf.begin(), buf.end(), cuda::std::begin(host_data), equal_to_value{value});
  }
  else
  {
    cuda::experimental::__ensure_current_device guard{cuda::device_ref{0}};
    return buf.size() == size
        && thrust::equal(thrust::cuda::par.on(buf.stream().get()),
                         buf.begin(),
                         buf.end(),
                         cuda::std::begin(device_data),
                         equal_to_value{value});
  }
}

// Helper function to compare two ranges
template <class Range1, class Range2>
bool equal_range(const Range1& range1, const Range2& range2)
{
  if constexpr (Range1::__is_host_only)
  {
    range1.stream().sync();
    return cuda::std::equal(range1.begin(), range1.end(), range2.begin(), range2.end());
  }
  else
  {
    cuda::experimental::__ensure_current_device guard{cuda::device_ref{0}};
    return range1.size() == range2.size()
        && thrust::equal(thrust::cuda::par.on(range1.stream().get()), range1.begin(), range1.end(), range2.begin());
  }
}

struct dev0_device_memory_resource : cudax::device_memory_resource
{
  dev0_device_memory_resource()
      : cudax::device_memory_resource{cuda::device_ref{0}}
  {}

  using default_queries = cudax::properties_list<cuda::mr::device_accessible>;
};

// helper class as we need to pass the properties in a tuple to the catch tests
template <class>
struct extract_properties;

template <class... Properties>
struct extract_properties<cuda::std::tuple<Properties...>>
{
  using env          = cudax::env_t<other_property, Properties...>;
  using async_buffer = cudax::async_buffer<int, Properties...>;
  using resource     = cuda::std::conditional_t<cuda::mr::__is_host_accessible<Properties...>,
#if _CCCL_CUDACC_AT_LEAST(12, 6)
                                            cudax::pinned_memory_resource,
#else
                                            void,
#endif
                                            dev0_device_memory_resource>;
  using iterator       = cudax::heterogeneous_iterator<int, Properties...>;
  using const_iterator = cudax::heterogeneous_iterator<const int, Properties...>;

  using matching_vector   = cudax::async_buffer<int, other_property, Properties...>;
  using matching_resource = memory_resource_wrapper<other_property, Properties...>;
};

#endif // CUDAX_TEST_CONTAINER_VECTOR_HELPER_H
