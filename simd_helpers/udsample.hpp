#ifndef _SIMD_HELPERS_UDSAMPLE_HPP
#define _SIMD_HELPERS_UDSAMPLE_HPP

#if (__cplusplus < 201103) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
#error "This source file needs to be compiled with C++11 support (g++ -std=c++11)"
#endif

#include "simd_t.hpp"
#include "simd_ntuple.hpp"

namespace simd_helpers {
#if 0
}  // pacify emacs c-mode
#endif


// -------------------------------------------------------------------------------------------------
//
// template<typename T, unsigned int S, unsigned int N>
// inline simd_t<T,S> downsample(const simd_ntuple<T,S,N> &v)
//
// Note: the downsampling kernel does not divide the result by N !!


inline __m128 _kernel128_downsample2(__m128 a, __m128 b)
{
    __m128 u = _mm_shuffle_ps(a, b, 0x88);   // [a0 a2 b0 b2],  0x88 = (2020)_4
    __m128 v = _mm_shuffle_ps(a, b, 0xdd);   // [a1 a3 b1 b3],  0xdd = (3131)_4
    return u + v;

}

inline __m128 _kernel128_downsample4(__m128 a, __m128 b, __m128 c, __m128 d)
{
    // I think this is fastest.
    __m128 u = _kernel128_downsample2(a, b);
    __m128 v = _kernel128_downsample2(c, d);
    return _kernel128_downsample2(u, v);
}

inline __m256 _kernel256_downsample2(__m256 a, __m256 b)
{
    // Similar to _kernel128_downsample2(), but with an extra 64-bit swap
    // which in theory helps the subsequent permute/blend block pipeline.
    // (Not sure if this actually helps!)

    __m256 u = _mm256_shuffle_ps(b, a, 0x88);   // [b0 b2 a0 a2],  0x88 = (2020)_4
    __m256 v = _mm256_shuffle_ps(b, a, 0xdd);   // [b1 b3 a1 a3],  0xdd = (3131)_4
    __m256 w = u + v;

    // Now we have a vector [ w2 w0 w3 w1 ], where w_i is 64 bits
    // and we want to rearrange to [ w0 w1 w2 w3 ].

    __m256 x = _mm256_permute_ps(w, 0x4e);          // [ w0 w2 w1 w3 ],  0x4e = (1032)_4
    __m256 y = _mm256_permute2f128_ps(w, w, 0x01);  // [ w3 w1 w2 w0 ]

    return _mm256_blend_ps(x, y, 0x3c);   // (00111100)_2
}


inline simd_t<float,4> downsample(const simd_ntuple<float,4,2> &t)
{
    return _kernel128_downsample2(t.extract<0>().x, t.extract<1>().x);
}

inline simd_t<float,4> downsample(const simd_ntuple<float,4,4> &t)
{
    return _kernel128_downsample4(t.extract<0>().x, t.extract<1>().x, t.extract<2>().x, t.extract<3>().x);
}

inline simd_t<float,8> downsample(const simd_ntuple<float,8,2> &t)
{
    return _kernel256_downsample2(t.extract<0>().x, t.extract<1>().x);
}


}  // namespace simd_helpers

#endif  // _SIMD_HELPERS_UDSAMPLE_HPP
