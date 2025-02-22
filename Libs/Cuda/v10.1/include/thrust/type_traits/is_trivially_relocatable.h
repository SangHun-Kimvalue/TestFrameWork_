///////////////////////////////////////////////////////////////////////////////
//  Copyright (c)      2018 NVIDIA Corporation
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///////////////////////////////////////////////////////////////////////////////

/*! \file is_trivially_relocatable.h
 *  \brief <a href="https://wg21.link/P1144R0">P1144R0</a>'s
 *         \c is_trivially_relocatable, an extensible type trait indicating
 *         whether a type can be bitwise copied (e.g. via \c memcpy).
 */

#pragma once

#include <thrust/detail/config.h>
#include <thrust/detail/static_assert.h>
#include <thrust/detail/type_traits.h>
#include <thrust/type_traits/is_contiguous_iterator.h>

#if THRUST_CPP_DIALECT >= 2011
  #include <type_traits>
#endif

THRUST_BEGIN_NS

namespace detail
{

template <typename T>
struct is_trivially_relocatable_impl;

} // namespace detail

/// Unary metafunction returns \c true_type if \c T is \a TriviallyRelocatable, 
/// e.g. can be bitwise copied (with a facility like \c memcpy), and
/// \c false_type otherwise.
template <typename T>
#if THRUST_CPP_DIALECT >= 2011
using is_trivially_relocatable =
#else
struct is_trivially_relocatable :
#endif
  detail::is_trivially_relocatable_impl<T>
#if THRUST_CPP_DIALECT < 2011
{}
#endif
;

#if THRUST_CPP_DIALECT >= 2014
/// <code>constexpr bool</code> that is \c true if \c T is
/// \a TriviallyRelocatable e.g. can be copied bitwise (with a facility like
/// \c memcpy), and \c false otherwise.
template <typename T>
constexpr bool is_trivially_relocatable_v = is_trivially_relocatable<T>::value;
#endif

/// Unary metafunction returns \c true_type if \c From is \a TriviallyRelocatable
/// to \c To, e.g. can be bitwise copied (with a facility like \c memcpy), and
/// \c false_type otherwise.
template <typename From, typename To>
#if THRUST_CPP_DIALECT >= 2011
using is_trivially_relocatable_to =
#else
struct is_trivially_relocatable_to :
#endif
  integral_constant<
    bool
  , detail::is_same<From, To>::value && is_trivially_relocatable<To>::value
  >
#if THRUST_CPP_DIALECT < 2011
{}
#endif
;

#if THRUST_CPP_DIALECT >= 2014
/// <code>constexpr bool</code> that is \c true if \c From is 
/// \a TriviallyRelocatable to \c To, e.g. can be copied bitwise (with a
/// facility like \c memcpy), and \c false otherwise.
template <typename From, typename To>
constexpr bool is_trivially_relocatable_to_v
  = is_trivially_relocatable_to<From, To>::value;
#endif

/// Unary metafunction that returns \c true_type if the element type of
/// \c FromIterator is \a TriviallyRelocatable to the element type of
/// \c ToIterator, and \c false_type otherwise.
template <typename FromIterator, typename ToIterator>
#if THRUST_CPP_DIALECT >= 2011
using is_indirectly_trivially_relocatable_to =
#else
struct is_indirectly_trivially_relocatable_to :
#endif
  integral_constant<
    bool
  ,    is_contiguous_iterator<FromIterator>::value
    && is_contiguous_iterator<ToIterator>::value
    && is_trivially_relocatable_to<
         typename thrust::iterator_traits<FromIterator>::value_type,
         typename thrust::iterator_traits<ToIterator>::value_type
       >::value
  >
#if THRUST_CPP_DIALECT < 2011
{}
#endif
;

#if THRUST_CPP_DIALECT >= 2014
/// <code>constexpr bool</code> that is \c true if the element type of
/// \c FromIterator is \a TriviallyRelocatable to the element type of
/// \c ToIterator, and \c false otherwise.
template <typename FromIterator, typename ToIterator>
constexpr bool is_trivial_relocatable_sequence_copy_v
  = is_indirectly_trivially_relocatable_to<FromIterator, ToIterator>::value;
#endif

/// Customization point that can be customized to indicate that a type \c T is
/// \a TriviallyRelocatable, e.g. can be copied bitwise (with a facility like
/// \c memcpy).
template <typename T>
struct proclaim_trivially_relocatable : false_type {};

/// Declares that the type \c T is \a TriviallyRelocatable by specializing
/// `thrust::proclaim_trivially_relocatable`.
#define THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(T)                              \
  THRUST_BEGIN_NS                                                             \
  template <>                                                                 \
  struct proclaim_trivially_relocatable<T> : ::thrust::true_type {};          \
  THRUST_END_NS                                                               \
  /**/

///////////////////////////////////////////////////////////////////////////////

namespace detail
{

// https://wg21.link/P1144R0#wording-inheritance
template <typename T>
struct is_trivially_relocatable_impl
  : integral_constant<
      bool
      #if    THRUST_CPP_DIALECT >= 2011                                       \
          && (  (THRUST_HOST_COMPILER != THRUST_HOST_COMPILER_GCC)            \
             || (THRUST_GCC_VERSION >= 50000))
    ,    std::is_trivially_copyable<T>::value
      #else
    ,    has_trivial_assign<T>::value
      #endif
      || proclaim_trivially_relocatable<T>::value
    >
{};

template <typename T, std::size_t N>
struct is_trivially_relocatable_impl<T[N]> : is_trivially_relocatable_impl<T> {};

} // namespace detail
 
THRUST_END_NS

#if THRUST_DEVICE_SYSTEM == THRUST_DEVICE_SYSTEM_CUDA

#include <thrust/system/cuda/detail/guarded_cuda_runtime_api.h>

THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(char1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(char2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(char3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(char4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uchar1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uchar2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uchar3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uchar4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(short1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(short2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(short3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(short4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ushort1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ushort2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ushort3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ushort4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(int1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(int2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(int3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(int4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uint1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uint2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uint3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(uint4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(long1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(long2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(long3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(long4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulong1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulong2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulong3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulong4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(longlong1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(longlong2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(longlong3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(longlong4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulonglong1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulonglong2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulonglong3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(ulonglong4)

struct __half;
struct __half2;

THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(__half)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(__half2)

THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(float1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(float2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(float3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(float4)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(double1)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(double2)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(double3)
THRUST_PROCLAIM_TRIVIALLY_RELOCATABLE(double4)
#endif

