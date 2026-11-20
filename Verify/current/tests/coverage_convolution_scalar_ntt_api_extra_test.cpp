// SPDX-License-Identifier: CC0-1.0
// 任意法畳み込みに同梱するNTTの標準C++経路を独立参照と照合する。
#ifndef SIRKIT_DISABLE_SIMD
#define SIRKIT_DISABLE_SIMD
#endif
#define NTT_HEADER "../../../Math/Convolution.cpp"
#include "ntt_montgomery_reference_test.cpp"
