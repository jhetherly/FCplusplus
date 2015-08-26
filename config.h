//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_CONFIG_DOT_H
#define FCPP_CONFIG_DOT_H

// Here we just do the kludges necessary to make stuff work with some
// compilers.

#ifdef __GNUC__
   // __GNUC_MINOR__ is also available

#   if __GNUC__ == 2

       // g++2.95.2 has a non-conforming <iterator> library
#      define FCPP_NO_STD_ITER

       // g++2.95.2 has bugs that prevent namespace use in some cases
#      define FCPP_NO_USE_NAMESPACE

       // g++2.95.2 doesn't always support sucking in base-class names 
       // with 'using' declarations
#      define FCPP_NO_USING_DECLS

       // g++2.95.2 has other misc bugs
#      define FCPP_I_AM_GCC2

#   endif

#endif

// Here we do a little macro setting-up for compile-time options
#ifdef FCPP_DEFER_DEFINITIONS
#   define FCPP_MAYBE_NAMESPACE_OPEN
#   define FCPP_MAYBE_NAMESPACE_CLOSE
#   define FCPP_MAYBE_EXTERN extern
#   define FCPP_MAYBE_DEFINE(x)
#else
#   define FCPP_MAYBE_NAMESPACE_OPEN namespace {
#   define FCPP_MAYBE_NAMESPACE_CLOSE }
#   define FCPP_MAYBE_EXTERN 
#   define FCPP_MAYBE_DEFINE(x) x
#endif


#endif
