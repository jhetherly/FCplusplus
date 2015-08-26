//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_SMART_DOT_H
#define FCPP_SMART_DOT_H

#include "signature.h"

namespace fcpp {

//////////////////////////////////////////////////////////////////////
// Feature: Smartness
//////////////////////////////////////////////////////////////////////
// If F is smart, then we can refer to these entities:
//
//    FunctoidTraits<F>::template accepts<N>::args
//       A bool which says whether F can accept N arguments
//
//    FunctoidTraits<F>::max_args
//       An int which says what the most arguments F can accept is
//
//    FunctoidTraits<F>::template ensure_accepts<N>::args()
//       A no-op call that compiles only if F can accept N args
//
// We use traits so that if you happen to ask a non-smart functoid these
// questions, you will hopefully get a literate error message.

struct SmartFunctoid {};

// We add crazy identifiers to ensure that users don't accidentally talk
// to functoids directly; they should always be going through the traits
// class to ask for info.
struct SmartFunctoid0 : public SmartFunctoid {
   template <class Dummy, int i> struct crazy_accepts {
      static const bool args = false;
   };
   template <class Dummy> struct crazy_accepts<Dummy,0> {
      static const bool args = true;
   };
   static const int crazy_max_args = 0;
};

struct SmartFunctoid1 : public SmartFunctoid {
   template <class Dummy, int i> struct crazy_accepts {
      static const bool args = false;
   };
   template <class Dummy> struct crazy_accepts<Dummy,1> {
      static const bool args = true;
   };
   static const int crazy_max_args = 1;
};

struct SmartFunctoid2 : public SmartFunctoid {
   template <class Dummy, int i> struct crazy_accepts {
      static const bool args = false;
   };
   template <class Dummy> struct crazy_accepts<Dummy,1> {
      static const bool args = true;
   };
   template <class Dummy> struct crazy_accepts<Dummy,2> {
      static const bool args = true;
   };
   static const int crazy_max_args = 2;
};

struct SmartFunctoid3 : public SmartFunctoid {
   template <class Dummy, int i> struct crazy_accepts {
      static const bool args = false;
   };
   template <class Dummy> struct crazy_accepts<Dummy,1> {
      static const bool args = true;
   };
   template <class Dummy> struct crazy_accepts<Dummy,2> {
      static const bool args = true;
   };
   template <class Dummy> struct crazy_accepts<Dummy,3> {
      static const bool args = true;
   };
   static const int crazy_max_args = 3;
};

namespace impl {
   template <class F, bool b> struct NeededASmartFunctoidButInsteadGot {};
   template <class F> struct NeededASmartFunctoidButInsteadGot<F,true> {
      typedef F Type;
   };
   template <bool b> struct Ensure;
   template <> struct Ensure<true> {};
} // end namespace impl

template <class MaybeASmartFunctoid>
struct FunctoidTraits {
   typedef 
      typename impl::NeededASmartFunctoidButInsteadGot<MaybeASmartFunctoid,
         Inherits<MaybeASmartFunctoid,SmartFunctoid>::value>::Type
      F;
   template <int i> struct accepts {
      static const bool args = F::template crazy_accepts<int,i>::args;
   };
   template <int i> struct ensure_accepts {
      static const bool ok = F::template crazy_accepts<int,i>::args;
      inline static void args() { (void) impl::Ensure<ok>(); }
   };
   static const int max_args = F::crazy_max_args;
};

} // end namespace fcpp

#endif
