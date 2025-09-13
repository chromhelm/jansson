// Copyright 2020-2025 Junekey Jeon

#ifndef __DRAGONBOX__
#define __DRAGONBOX__
//
// The contents of this file may be used under the terms of
// the Apache License v2.0 with LLVM Exceptions.
//
//    (See accompanying file LICENSE-Apache or copy at
//     https://llvm.org/foundation/relicensing/LICENSE.txt)
//
// Alternatively, the contents of this file may be used under the terms of
// the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE-Boost or copy at
//     https://www.boost.org/LICENSE_1_0.txt)
//
// Unless required by applicable law or agreed to in writing, this software
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.

// We want to achieve the followings regarding the macro leakage of this header file:
//   - Normally, no macro other than the header guard JKJ_HEADER_DRAGONBOX is leaked.
//   - For internal library use, macros defined below are leaked if explicitly requested.
//   - This header file does not include any other header file in this library.
//   - The order of inclusions and repeated inclusions of header files in this library must not matter.
// To achieve all of these at once, we do some preprocessing hack:
//   - Macros are populated if JKJ_HEADER_DRAGONBOX is not defined (which means the header file is being
//   included for the first time), or JKJ_DRAGONBOX_LEAK_MACROS is defined and
//   JKJ_DRAGONBOX_MACROS_DEFINED is not defined at the same time.
//   - JKJ_DRAGONBOX_MACROS_DEFINED is defined at the end of this header file if (and only if)
//   JKJ_DRAGONBOX_LEAK_MACROS is defined.
//   - The actual content (excluding the macros) of this header file is guarded by JKJ_HEADER_DRAGONBOX
//   as usual.
//   - JKJ_DRAGONBOX_MACROS_DEFINED and all macros defined below except for JKJ_HEADER_DRAGONBOX are
//   #undef-ed at the end of this header if JKJ_DRAGONBOX_LEAK_MACROS is not defined.

#include <stdint.h>

typedef struct {
    uint64_t significand;
    int exponent;
} decimal_fp;

decimal_fp dragonbox_to_decimal(double x);

#endif /* __DRAGONBOX__ */
