// Misc.h
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <climits> //Needed for CHAR_BIT.
#include <limits>
#include <type_traits>

//------------------------------------------------------------------------------------------------------
//----------------------------------- Error/Warning/Verbosity macros -----------------------------------
//------------------------------------------------------------------------------------------------------

//------- Executable name variants.
#ifndef EXPLICATOR_BASIC_ERRFUNC_
#define EXPLICATOR_BASIC_ERRFUNC_
#define ERR(x)                                                                                  \
    {                                                                                           \
        std::cerr << "--(E) " << argv[0] << ": " << x << ". Terminating program." << std::endl; \
        std::cerr.flush();                                                                      \
        std::exit(-1);                                                                          \
    }
#endif

#ifndef EXPLICATOR_BASIC_WARNFUNC_
#define EXPLICATOR_BASIC_WARNFUNC_
#define WARN(x)                                                            \
    {                                                                      \
        std::cout << "--(W) " << argv[0] << ": " << x << "." << std::endl; \
        std::cout.flush();                                                 \
    }
#endif

#ifndef EXPLICATOR_BASIC_INFOFUNC_
#define EXPLICATOR_BASIC_INFOFUNC_
#define INFO(x)                                                            \
    {                                                                      \
        std::cout << "--(I) " << argv[0] << ": " << x << "." << std::endl; \
        std::cout.flush();                                                 \
    }
#endif

//------- Function name variants.
#ifdef __GNUC__ // If using gcc..
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __func__
#endif
#endif                      //__GNUC__
#ifndef __PRETTY_FUNCTION__ //(this is a fallback!)
#define __PRETTY_FUNCTION__ '(function name not available)'
#endif

#ifndef FUNCEXPLICATOR_BASIC_ERRFUNC_
#define FUNCEXPLICATOR_BASIC_ERRFUNC_
#define FUNCEXPLICATORERR(x)                                                       \
    {                                                                    \
        std::cerr << "--(E) In function: " << __PRETTY_FUNCTION__;       \
        std::cerr << ": " << x << ". Terminating program." << std::endl; \
        std::cerr.flush();                                               \
        std::exit(-1);                                                   \
    }
#endif

#ifndef FUNCEXPLICATOR_BASIC_WARNFUNC_
#define FUNCEXPLICATOR_BASIC_WARNFUNC_
#define FUNCEXPLICATORWARN(x)                                                \
    {                                                              \
        std::cout << "--(W) In function: " << __PRETTY_FUNCTION__; \
        std::cout << ": " << x << "." << std::endl;                \
        std::cout.flush();                                         \
    }
#endif

#ifndef FUNCEXPLICATOR_BASIC_INFOFUNC_
#define FUNCEXPLICATOR_BASIC_INFOFUNC_
#define FUNCEXPLICATORINFO(x)                                                \
    {                                                              \
        std::cout << "--(I) In function: " << __PRETTY_FUNCTION__; \
        std::cout << ": " << x << "." << std::endl;                \
        std::cout.flush();                                         \
    }
#endif

//------------------------------------------------------------------------------------------------------
//------------------------------------- Convenience (math) macros --------------------------------------
//------------------------------------------------------------------------------------------------------

#define EXPLICATORABS(X) ((X) < 0 ? -(X) : (X))
#define EXPLICATORMAX(A, B) ((A) > (B) ? (A) : (B))
#define EXPLICATORMIN(A, B) ((A) < (B) ? (A) : (B))

#ifndef isininc
// Inclusive_in_range()      isininc( 0, 10, 100) == true
//                          isininc( 0, 100, 10) == false
//                          isininc( 0, 10, 10)  == true
//                          isininc( 10, 10, 10) == true
#define isininc(A, x, B) (((x) >= (A)) && ((x) <= (B)))
#endif

// Checks if a variable, bitwise AND'ed with a bitmask, equals the bitmask. Ensure the ordering is observed
// because the operation is non-commutative.
//
// For example: let A = 0110011
//        and BITMASK = 0010010
//   then A & BITMASK = 0010010 == BITMASK.
//
// For example: let A = 0100011
//        and BITMASK = 0010010
//   then A & BITMASK = 0000010 != BITMASK.
//
// NOTE: This operation is mostly useful for checking for embedded 'flags' within a variable. These flags
// are set by bitwise ORing them into the variable.
//
#ifndef BITMASK_BITS_ARE_SET
#define BITMASK_BITS_ARE_SET(A, BITMASK) ((A & BITMASK) == BITMASK)
#endif
