/******************************************************************************
 * Copyright (c) 2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the NVIDIA CORPORATION nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NVIDIA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#include <cub/device/device_segmented_reduce.cuh>

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/iterator/constant_iterator.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/discard_iterator.h>

#include <cstdint>

#include "catch2_test_device_reduce.cuh"

// Has to go after all cub headers. Otherwise, this test won't catch unused
// variables in cub kernels.
#include "c2h/custom_type.cuh"
#include "catch2/catch.hpp"
#include "catch2_test_cdp_helper.h"
#include "catch2_test_helper.h"

DECLARE_CDP_WRAPPER(cub::DeviceSegmentedReduce::Reduce, device_segmented_reduce);
DECLARE_CDP_WRAPPER(cub::DeviceSegmentedReduce::Sum, device_segmented_sum);

// %PARAM% TEST_CDP cdp 0:1

// List of types to test
using custom_t           = std::size_t; //c2h::custom_type_t<c2h::accumulateable_t, c2h::equal_comparable_t>;
using iterator_type_list = c2h::type_list<type_pair<custom_t>, type_pair<std::size_t>>;

// multiply by constant
template <typename T>
struct cmult {
    __host__ __device__
    constexpr cmult(T mult) : mult(mult) {}
    constexpr cmult(cmult<T> const&) = default;
    constexpr cmult(cmult<T>&&) = default;
    constexpr cmult<T>& operator=(cmult<T> const&) = default;
    constexpr cmult<T>& operator=(cmult<T>&&) = default;
    __host__ __device__
    constexpr T operator()(T const& value) const {
        return value*mult;
    }
    private:
    T mult;
};

template <typename T1,typename T2>
struct assert_same_type {
	static_assert(std::is_same<T1,T2>::value,"T1 must match T2");
};

CUB_TEST("Device segmented reduce works with fancy input iterators and 64-bit offsets",
         "[reduce][device]",
         iterator_type_list)
{
  using params   = params_t<TestType>;
  using item_t   = typename params::item_t;
  using output_t = typename params::output_t;
  using offset_t = std::ptrdiff_t;

  constexpr offset_t min_items_rand = static_cast<offset_t>(0);
  constexpr offset_t max_items_rand = static_cast<offset_t>(1) << 7;
  constexpr offset_t offset_base_value = static_cast<offset_t>(1) << 32;
  constexpr int max_segments = 4;

  // Number of items
  const offset_t num_items_rand = GENERATE_COPY(take(2, random(min_items_rand, max_items_rand)),
                                           values({
                                             min_items_rand,
                                             max_items_rand,
                                           }));
  INFO("Test num_items_rand: " << num_items_rand);

  // Range of segment sizes to generate
  const std::tuple<offset_t, offset_t> seg_size_range =
    GENERATE_COPY(table<offset_t, offset_t>({{1, 1}, {1, max_segments}, {max_segments, max_segments}}));
  INFO("Test seg_size_range: [" << std::get<0>(seg_size_range) << ", "
                                << std::get<1>(seg_size_range) << "]");

  // Generate randomized offsets that will be added to 2^32
  thrust::device_vector<offset_t> segment_offsets =
    c2h::gen_uniform_offsets<offset_t>(CUB_SEED(1),
                                       num_items_rand,
                                       std::get<0>(seg_size_range),
                                       std::get<1>(seg_size_range));
  const int num_segments = static_cast<int>(segment_offsets.size() - 1);
  // Add 2^32 to offsets
  thrust::transform(segment_offsets.begin(),
                    segment_offsets.end(),
                    thrust::make_transform_iterator(
                      thrust::make_counting_iterator(static_cast<offset_t>(0)),
                      cmult<offset_t>(offset_base_value)
                    ),
                    segment_offsets.begin(),
                    thrust::plus<offset_t>{});
  auto d_offsets_it = thrust::raw_pointer_cast(segment_offsets.data());

  // Prepare input data
  item_t default_constant{};
  init_default_constant(default_constant);
  auto in_it = thrust::make_constant_iterator(default_constant);

  using op_t   = cub::Sum;
  using init_t = output_t;

  // Binary reduction operator
  auto reduction_op = op_t{};

  // Prepare verification data
  using accum_t = cub::detail::accumulator_t<op_t, init_t, item_t>;
  thrust::host_vector<output_t> expected_result(num_segments);
  thrust::host_vector<offset_t> segment_offsets_host = segment_offsets;
  offset_t init_constant_host {};
  init_default_constant(init_constant_host);
  //assert_same_type<decltype(init_constant_host),std::ptrdiff_t>{};
  for(int n = 0;n < num_segments;n++) {
    std::size_t valn = default_constant*(segment_offsets_host[n+1]-segment_offsets_host[n]);
    expected_result[n] = valn;
  }

  // Run test
  thrust::device_vector<output_t> out_result(num_segments);
  auto d_out_it = thrust::raw_pointer_cast(out_result.data());
  device_segmented_reduce(in_it,
                          d_out_it,
                          num_segments,
                          d_offsets_it,
                          d_offsets_it + 1,
                          reduction_op,
                          init_t{});

  // Verify result
  REQUIRE(expected_result == out_result);
}
