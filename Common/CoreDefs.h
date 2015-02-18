#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#endif // _WIN32

#if !defined(TESTS) && !defined(__BRAIN)

#   if defined(ARDUINO) && ARDUINO >= 100
#   include <alloca.h>
#       include "Arduino.h"
#   else
#       include "WProgram.h"
#   endif
#elif defined(TESTS)
#   include <gmock/gmock.h>
#endif

#ifdef TESTS
#   define HIGH    1
#   define LOW     0
#endif //TESTS

//////////////////////////////////////////////////////////////////////////

// ARRAY_SIZE
#ifdef _MSC_VER
#   define ARRAY_SIZE _countof
#else
#   define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))
#endif // _MSC_VER


// ASSERT
#ifdef TESTS
#   define ASSERT assert
#else
#   define ASSERT 
#endif // TESTS


// UNUSED
#ifndef UNUSED
#   if defined(__GNUC__)
#       define UNUSED(Name) Name __attribute__((unused))
#   elif defined(_MSC_VER)
#       define UNUSED(Name) Name
#   endif
#endif


// STATIC_ASSERT
#ifndef STATIC_ASSERT
#   if __cplusplus >= 201103L 
#       define STATIC_ASSERT(e)  static_assert(e, #e)
#       define STATIC_MSGASSERT(e, message)  static_assert(e, message)
#   else
#       define _STATIC_ASSERT_INNER_TOKENPASTE(x, y) x ## y
#       define _STATIC_ASSERT_TOKENPASTE(x, y) _STATIC_ASSERT_INNER_TOKENPASTE(x, y)
#       define STATIC_ASSERT(e) typedef char UNUSED(_STATIC_ASSERT_TOKENPASTE(_CStaticAssertionFailed_, __LINE__)[(e)?1:-1])
#       define STATIC_MSGASSERT(e, message)  STATIC_ASSERT(e)
#   endif
#endif

