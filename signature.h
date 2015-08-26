//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_SIGNATURE_DOT_H
#define FCPP_SIGNATURE_DOT_H

#include "config.h"
#include <functional>

#ifndef FCPP_NO_USE_NAMESPACE
namespace fcpp {
#endif

//////////////////////////////////////////////////////////////////////
// Inheritance detection

// Code based on Andrei Alexandrescu's article:
//    http://www.cuj.com/experts/1810/alexandr.html

template <class T, class U>
struct ConversionHelper {
   typedef char Small;
   struct Big { char dummy[2]; };
   static Small Test( const U* );
   static Big   Test(...);
   static const T* MakeT();
};

template <class T, class U>
struct Conversion {
   typedef ConversionHelper<T,U> H;
   static const int lhs = sizeof(H::Test((T*) H::MakeT()));
   static const int rhs = sizeof(typename H::Small);
   static const bool exists = (lhs==rhs);
   static const bool sameType = false;
};

template <class T>
struct Conversion<T, T> {
   static const bool exists = true;
   static const bool sameType = true;
};

template <class Derived, class Base>
struct Inherits {
   static const bool value = 
      Conversion<Derived,Base>::exists && 
      !Conversion<Base,void>::sameType;
};

//////////////////////////////////////////////////////////////////////
// Here are the classes with "nested typedefs" which just help us use
// our own type system; these classes are just inherited.
//
// Note that although the sigs support a large number of arguments, most 
// of the rest of the library only supports functions of 0-3 arguments.
//////////////////////////////////////////////////////////////////////

// Handy helper 'nothing' class.
struct Void {};

// This set names functoid arguments and results

template <class A1, class A2 = Void, class A3 = Void, class A4 = Void, 
                    class A5 = Void, class A6 = Void, class R = Void>
struct FunType {
   typedef R ResultType;
   typedef A1 Arg1Type;
   typedef A2 Arg2Type;
   typedef A3 Arg3Type;
   typedef A4 Arg4Type;
   typedef A5 Arg5Type;
   typedef A6 Arg6Type;
};

template <class A1, class A2, class A3, class A4, class A5, class R>
struct FunType<A1, A2, A3, A4, A5, R, Void> {
   typedef R ResultType;
   typedef A1 Arg1Type;
   typedef A2 Arg2Type;
   typedef A3 Arg3Type;
   typedef A4 Arg4Type;
   typedef A5 Arg5Type;
};

template <class A1, class A2, class A3, class A4, class R>
struct FunType<A1, A2, A3, A4, R, Void, Void> {
   typedef R ResultType;
   typedef A1 Arg1Type;
   typedef A2 Arg2Type;
   typedef A3 Arg3Type;
   typedef A4 Arg4Type;
};

template <class A1, class A2, class A3, class R>
struct FunType<A1, A2, A3, R, Void, Void, Void> {
   typedef R ResultType;
   typedef A1 Arg1Type;
   typedef A2 Arg2Type;
   typedef A3 Arg3Type;
};

template <class A1, class A2, class R>
struct FunType<A1, A2, R, Void, Void, Void, Void> {
  typedef R ResultType;
  typedef A1 Arg1Type;
  typedef A2 Arg2Type;
};

template <class A1, class R>
struct FunType<A1, R, Void, Void, Void, Void, Void> {
  typedef R ResultType;
  typedef A1 Arg1Type;
};

template <class R>
struct FunType<R, Void, Void, Void, Void, Void, Void> {
  typedef R ResultType;
};

//////////////////////////////////////////////////////////////////////
// Concrete versions
//////////////////////////////////////////////////////////////////////
// This set is used for monomorphic direct functoids; the type names
// are inherited as-is, and also a template-Sig is defined so that
// monomorphic direct functoids can mix freely with polymorphic functoids 
// since the latter require a template-Sig member

template < class A1, class A2 = Void, class A3 = Void, class A4 = Void, 
                     class A5 = Void, class A6 = Void, class R = Void >
struct CFunType : public FunType<A1,A2,A3,A4,A5,A6,R> {
   template <class P1, class P2, class P3, class P4, class P5, class P6>
   struct Sig : public FunType<A1,A2,A3,A4,A5,A6,R> {};
};

template <class A1, class A2, class A3, class A4, class A5, class R>
struct CFunType<A1, A2, A3, A4, A5, R, Void> : 
  public FunType<A1,A2,A3,A4,A5,R> {
   template <class P1, class P2, class P3, class P4, class P5>
   struct Sig : public FunType<A1,A2,A3,A4,A5,R> {};
};

template <class A1, class A2, class A3, class A4, class R>
struct CFunType<A1, A2, A3, A4, R, Void, Void> :
  public FunType<A1,A2,A3,A4,R> {
  template <class P1, class P2, class P3, class P4>
  struct Sig : public FunType<A1,A2,A3,A4,R> {};
};

template <class A1, class A2, class A3, class R>
struct CFunType<A1, A2, A3, R, Void, Void, Void> :
  public FunType<A1,A2,A3,R> {
  template <class P1, class P2, class P3>
  struct Sig : public FunType<A1,A2,A3,R> {};
};

template <class A1, class A2, class R>
struct CFunType<A1, A2, R, Void, Void, Void, Void> :
  public FunType<A1,A2,R>, public std::binary_function<A1,A2,R> {
  template <class P1, class P2>
  struct Sig : public FunType<A1,A2,R> {};
};

template <class A1, class R>
struct CFunType<A1, R, Void, Void, Void, Void, Void> :
  public FunType<A1,R>, public std::unary_function<A1,R> {
  template <class P1>
  struct Sig : public FunType<A1,R> {};
};

struct CallableWithoutArguments {};
struct WrongNumberOfSigArgs {};

template <class R>
struct CFunType<R, Void, Void, Void, Void, Void, Void> :
  public CallableWithoutArguments, public FunType<R> {
  template <class Dummy1=Void, class Dummy2=Void>
  struct Sig : public FunType<WrongNumberOfSigArgs> {};
  template <class Dummy>
  struct Sig<Void,Dummy> : public FunType<R> {};
};

//////////////////////////////////////////////////////////////////////
// Icky helpers
//////////////////////////////////////////////////////////////////////
// These are strictly unnecessary, but they avoid a bug in the g++
// compiler and also make some things shorter to type.  
// RT<T, args> means "return type of T when passed argument types <args>"

template <class T, class A1 = Void, class A2 = Void, class A3 = Void,
                   class A4 = Void, class A5 = Void, class A6 = Void>
struct RT {
typedef typename T::template Sig<A1, A2, A3, A4, A5, A6>::ResultType ResultType;
typedef typename T::template Sig<A1, A2, A3, A4, A5, A6>::Arg1Type Arg1Type;
typedef typename T::template Sig<A1, A2, A3, A4, A5, A6>::Arg2Type Arg2Type;
typedef typename T::template Sig<A1, A2, A3, A4, A5, A6>::Arg3Type Arg3Type;
typedef typename T::template Sig<A1, A2, A3, A4, A5, A6>::Arg4Type Arg4Type;
typedef typename T::template Sig<A1, A2, A3, A4, A5, A6>::Arg5Type Arg5Type;
typedef typename T::template Sig<A1, A2, A3, A4, A5, A6>::Arg6Type Arg6Type;
};

template <class T, class A1, class A2, class A3, class A4, class A5>
struct RT<T, A1, A2, A3, A4, A5, Void> {
   typedef typename T::template Sig<A1, A2, A3, A4, A5>::ResultType ResultType;
   typedef typename T::template Sig<A1, A2, A3, A4, A5>::Arg1Type Arg1Type;
   typedef typename T::template Sig<A1, A2, A3, A4, A5>::Arg2Type Arg2Type;
   typedef typename T::template Sig<A1, A2, A3, A4, A5>::Arg3Type Arg3Type;
   typedef typename T::template Sig<A1, A2, A3, A4, A5>::Arg4Type Arg4Type;
   typedef typename T::template Sig<A1, A2, A3, A4, A5>::Arg5Type Arg5Type;
};

template <class T, class A1, class A2, class A3, class A4>
struct RT<T, A1, A2, A3, A4, Void, Void> {
   typedef typename T::template Sig<A1, A2, A3, A4>::ResultType ResultType;
   typedef typename T::template Sig<A1, A2, A3, A4>::Arg1Type Arg1Type;
   typedef typename T::template Sig<A1, A2, A3, A4>::Arg2Type Arg2Type;
   typedef typename T::template Sig<A1, A2, A3, A4>::Arg3Type Arg3Type;
   typedef typename T::template Sig<A1, A2, A3, A4>::Arg4Type Arg4Type;
};

template <class T, class A1, class A2, class A3>
struct RT<T, A1, A2, A3, Void, Void, Void> {
   typedef typename T::template Sig<A1, A2, A3>::ResultType ResultType;
   typedef typename T::template Sig<A1, A2, A3>::Arg1Type Arg1Type;
   typedef typename T::template Sig<A1, A2, A3>::Arg2Type Arg2Type;
   typedef typename T::template Sig<A1, A2, A3>::Arg3Type Arg3Type;
};

template <class T, class A1, class A2>
struct RT<T, A1, A2, Void, Void, Void, Void> {
   typedef typename T::template Sig<A1, A2>::ResultType ResultType;
   typedef typename T::template Sig<A1, A2>::Arg1Type Arg1Type;
   typedef typename T::template Sig<A1, A2>::Arg2Type Arg2Type;
};

template <class T, class A1>
struct RT<T, A1, Void, Void, Void, Void, Void> {
   typedef typename T::template Sig<A1>::ResultType ResultType;
   typedef typename T::template Sig<A1>::Arg1Type Arg1Type;
};

template <class T>
struct RT<T, Void, Void, Void, Void, Void, Void> {
   typedef typename T::template Sig<>::ResultType ResultType;
};

#ifndef FCPP_NO_USE_NAMESPACE
} // end namespace fcpp
#endif

#endif
