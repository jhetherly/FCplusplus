//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_CURRY_DOT_H
#define FCPP_CURRY_DOT_H

#include "signature.h"

namespace fcpp {

//////////////////////////////////////////////////////////////////////
// This file implements currying for functoids.  Included here are
//  - bindMofN    for currying the Mth of N arguments
//  - Const       for turning a value into a constant function
//  - curryN      curries the first k arguments of an N-arg functoid, 
//                where k is the number of arguments "curryN" was 
//                called with
//  - CurryableN  new way to curry with underscores (e.g. f(_,y,_); )
//
// For more info, see
//    http://www.cc.gatech.edu/~yannis/fc++/currying.html
//////////////////////////////////////////////////////////////////////

// Important to implementation of CurryableN classes
struct AutoCurryType {};
FCPP_MAYBE_EXTERN AutoCurryType _;   // this is a legal identifier as fcpp::_

// Forward declarations; curryability and fullness are now somewhat
// inextricably intertwined...
template <class F> struct Full0;
template <class F> struct Full1;
template <class F> struct Full2;
template <class F> struct Full3;
template <class F> Full0<F> makeFull0( const F& f );
template <class F> Full1<F> makeFull1( const F& f );
template <class F> Full2<F> makeFull2( const F& f );
template <class F> Full3<F> makeFull3( const F& f );

namespace impl {

//////////////////////////////////////////////////////////////////////
// Const
//////////////////////////////////////////////////////////////////////

template <class T>
struct ConstHelper : public CFunType<T> {
   const T x;
public:
   ConstHelper( const T& a ) : x(a) {}
   T operator()() const { return x; }
};
struct Const {
   template <class T>
   struct Sig : public FunType<T,Full0<ConstHelper<T> > > {};

   template <class T>
   Full0<ConstHelper<T> > operator()( const T& x ) const {
      return makeFull0( ConstHelper<T>(x) );
   }
};

//////////////////////////////////////////////////////////////////////
// Binders (through "...of2")
//////////////////////////////////////////////////////////////////////

template <class Unary, class Arg>
class binder1of1 
: public CFunType<typename RT<Unary,Arg>::ResultType> {
   const Unary f;
   const Arg a;
public:
   binder1of1( const Unary& x, const Arg& y ) : f(x), a(y) {}
   typename RT<Unary,Arg>::ResultType operator()() const { return f(a); }
};

struct Bind1of1 {
   template <class Unary, class Arg>
   struct Sig : public FunType< Unary, Arg, Full0<binder1of1<Unary,Arg> > > {};
   
   template <class Unary, class Arg>
   inline Full0<binder1of1<Unary,Arg> > 
   operator()( const Unary& f, const Arg& a ) const {
      return makeFull0( binder1of1<Unary,Arg>(f,a) );
   }
};

template <class Binary, class Arg1>
class binder1of2 {
   const Binary f;
   const Arg1 x;
public:
   binder1of2( const Binary& a, const Arg1& b ) : f(a), x(b) {}

   template <class Arg2>
   struct Sig 
   : public FunType<typename Binary::template Sig<Arg1,Arg2>::Arg2Type,
                    typename Binary::template Sig<Arg1,Arg2>::ResultType> {};

   template <class Arg2>
   typename Binary::template Sig<Arg1,Arg2>::ResultType
   operator()( const Arg2& y ) const {
      return f(x,y);
   }
};

struct Bind1of2 {
   template <class Binary, class Arg1>
   struct Sig : public FunType<Binary,Arg1,Full1<binder1of2<Binary,Arg1> > > {};

   template <class Binary, class Arg1>
   Full1<binder1of2<Binary,Arg1> >
   operator()( const Binary& f, const Arg1& x ) const {
      return makeFull1( binder1of2<Binary,Arg1>(f,x) );
   }
};

template <class Binary, class Arg2>
class binder2of2 {
   const Binary f;
   const Arg2 y;
public:
   binder2of2( const Binary& a, const Arg2& b ) : f(a), y(b) {}

   template <class Arg1>
   struct Sig 
   : public FunType<typename Binary::template Sig<Arg1,Arg2>::Arg1Type,
                    typename Binary::template Sig<Arg1,Arg2>::ResultType> {};

   template <class Arg1>
   typename Binary::template Sig<Arg1,Arg2>::ResultType
   operator()( const Arg1& x ) const {
      return f(x,y);
   }
};

struct Bind2of2 {
   template <class Binary, class Arg2>
   struct Sig : public FunType<Binary,Arg2,Full1<binder2of2<Binary,Arg2> > > {};

   template <class Binary, class Arg2>
   Full1<binder2of2<Binary,Arg2> >
   operator()( const Binary& f, const Arg2& y ) const {
      return makeFull1( binder2of2<Binary,Arg2>(f,y) );
   }
};

template <class Binary, class Arg1, class Arg2>
class binder1and2of2 
: public CFunType<typename RT<Binary,Arg1,Arg2>::ResultType > {
   const Binary f;
   const Arg1 a1;
   const Arg2 a2;
public:
   binder1and2of2( const Binary& x, const Arg1& y, const Arg2& z ) 
   : f(x), a1(y), a2(z) {}
   typename RT<Binary,Arg1,Arg2>::ResultType  
   operator()() const { return f(a1,a2); }
};

struct Bind1and2of2 {
   template <class Binary, class Arg1, class Arg2>
   struct Sig 
   : public FunType< Binary, Arg1, Arg2, 
      Full0<binder1and2of2<Binary,Arg1,Arg2> > > {};

   template <class Binary, class Arg1, class Arg2>
   Full0<binder1and2of2<Binary,Arg1,Arg2> >
   operator()( const Binary& f, const Arg1& a1, const Arg2& a2 ) const {
      return makeFull0( binder1and2of2<Binary,Arg1,Arg2>(f,a1,a2) );
   }
};

//////////////////////////////////////////////////////////////////////
// Now that bindNof2 are defined, we can define Curryable2, which then
// some of the later binders can use.
//////////////////////////////////////////////////////////////////////

template <class R, class F, class X, class Y>
struct Curryable2Helper {
   static inline R go( const F& f, const X& x, const Y& y ) {
      return f(x,y); 
   }
};

template <class R, class F, class Y>
struct Curryable2Helper<R,F,AutoCurryType,Y> {
   static R go( const F& f, const AutoCurryType& , const Y& y ) {
      return makeFull1( binder2of2<F,Y>(f,y) );
   }
};

template <class R, class F, class X>
struct Curryable2Helper<R,F,X,AutoCurryType> {
   static R go( const F& f, const X& x, const AutoCurryType& ) {
      return makeFull1( binder1of2<F,X>(f,x) );
   }
};

template <class F>
class Curryable2 {
   const F f;
public:
   Curryable2( const F& ff ) : f(ff) {}

   template <class X, class Y=void>
   struct Sig
   : public FunType<typename F::template Sig<X,Y>::Arg1Type,
                    typename F::template Sig<X,Y>::Arg2Type,
                    typename RT<F,X,Y>::ResultType> {};

   template <class X>
   struct Sig<X,void> : public FunType<X,Full1<binder1of2<F,X> > > {};

   template <class Y>
   struct Sig<AutoCurryType,Y> 
   : public FunType<AutoCurryType,Y,Full1<binder2of2<F,Y> > > {};

   template <class X>
   struct Sig<X,AutoCurryType> 
   : public FunType<X,AutoCurryType,Full1<binder1of2<F,X> > > {};

   template <class X>
   typename Sig<X>::ResultType operator()( const X& x ) const {
      return makeFull1( binder1of2<F,X>(f,x) );
   }
   template <class X, class Y>
   inline typename Sig<X,Y>::ResultType 
   operator()( const X& x, const Y& y ) const {
      // need partial specialization, so defer to a class helper
      return Curryable2Helper<typename Sig<X,Y>::ResultType,F,X,Y>::go(f,x,y);
   }
};

//////////////////////////////////////////////////////////////////////
// With Curryable2 out of the way, we can go back to the 3-arg binders.
//////////////////////////////////////////////////////////////////////

template <class Ternary, class A1, class A2, class A3>
class binder1and2and3of3
: public CFunType<typename RT<Ternary,A1,A2,A3>::ResultType> {
   const Ternary f;
   const A1 a1;
   const A2 a2;
   const A3 a3;
public:
   binder1and2and3of3( const Ternary& w, const A1& x, const A2& y, const A3& z )
   : f(w), a1(x), a2(y), a3(z) {}
   typename RT<Ternary,A1,A2,A3>::ResultType 
   operator()() const { return f(a1,a2,a3); }
};

struct Bind1and2and3of3 {
   template <class Ternary, class A1, class A2, class A3>
   struct Sig 
   : public FunType<Ternary,A1,A2,A3,
                    Full0<binder1and2and3of3<Ternary,A1,A2,A3> > > {};

   template <class Ternary, class A1, class A2, class A3>
   Full0<binder1and2and3of3<Ternary,A1,A2,A3> >
   operator()( const Ternary& f, const A1& a1, 
               const A2& a2, const A3& a3 ) const {
      return makeFull0( binder1and2and3of3<Ternary,A1,A2,A3>(f,a1,a2,a3) );
   }
};

template <class Ternary, class Arg1, class Arg2>
class binder1and2of3 {
   const Ternary f;
   const Arg1 a1;
   const Arg2 a2;
public:
   template <class Arg3>
   struct Sig 
   : public FunType<typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg3Type,
                typename Ternary::template Sig<Arg1,Arg2,Arg3>::ResultType> {};

   binder1and2of3(const Ternary& w, const Arg1& x, const Arg2& y) : 
     f(w), a1(x), a2(y) {}
   template <class Arg3>
   typename Sig<Arg3>::ResultType 
   operator()(const Arg3& z) const { return f(a1,a2,z); }
};

struct Bind1and2of3 {
   template <class Ternary, class A1, class A2>
   struct Sig 
   : public FunType<Ternary,A1,A2,Full1<binder1and2of3<Ternary,A1,A2> > > {};

   template <class Ternary, class A1, class A2>
   Full1<binder1and2of3<Ternary,A1,A2> >
   operator()( const Ternary& f, const A1& a1, const A2& a2 ) const {
      return makeFull1( binder1and2of3<Ternary,A1,A2>(f,a1,a2) );
   }
};

template <class Ternary, class Arg2, class Arg3>
class binder2and3of3 {
   const Ternary f;
   const Arg2 a2;
   const Arg3 a3;
public:
   template <class Arg1>
   struct Sig 
   : public FunType<typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg1Type,
                    typename RT<Ternary,Arg1,Arg2,Arg3>::ResultType> {};
                    // need RT above due to g++ bug on line below
                    // typename Ternary::Sig<Arg1,Arg2,Arg3>::ResultType> {};

   binder2and3of3(const Ternary& w, const Arg2& y, const Arg3& z) : 
     f(w), a2(y), a3(z) {}
   template <class Arg1>
   typename Sig<Arg1>::ResultType 
   operator()(const Arg1& x) const { return f(x,a2,a3); }
};

struct Bind2and3of3 {
   template <class Ternary, class A2, class A3>
   struct Sig 
   : public FunType<Ternary,A2,A3,Full1<binder2and3of3<Ternary,A2,A3> > > {};

   template <class Ternary, class A2, class A3>
   Full1<binder2and3of3<Ternary,A2,A3> >
   operator()( const Ternary& f, const A2& a2, const A3& a3 ) const {
      return makeFull1( binder2and3of3<Ternary,A2,A3>(f,a2,a3) );
   }
};

template <class Ternary, class Arg1, class Arg3>
class binder1and3of3 {
   const Ternary f;
   const Arg1 a1;
   const Arg3 a3;
public:
   template <class Arg2>
   struct Sig 
   : public FunType<typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg2Type,
                    typename RT<Ternary,Arg1,Arg2,Arg3>::ResultType> {};
                    // need RT above due to g++ bug on line below
                    // typename Ternary::Sig<Arg1,Arg2,Arg3>::ResultType> {};

   binder1and3of3(const Ternary& w, const Arg1& x, const Arg3& z) : 
     f(w), a1(x), a3(z) {}
   template <class Arg2>
   typename Sig<Arg2>::ResultType 
   operator()(const Arg2& y) const { return f(a1,y,a3); }
};

struct Bind1and3of3 {
   template <class Ternary, class A1, class A3>
   struct Sig 
   : public FunType<Ternary,A1,A3,Full1<binder1and3of3<Ternary,A1,A3> > > {};

   template <class Ternary, class A1, class A3>
   Full1<binder1and3of3<Ternary,A1,A3> >
   operator()( const Ternary& f, const A1& a1, const A3& a3 ) const {
      return makeFull1( binder1and3of3<Ternary,A1,A3>(f,a1,a3) );
   }
};

template <class Ternary, class Arg1>
class binder1of3 {
   const Ternary f;
   const Arg1 x;
public:
   binder1of3( const Ternary& a, const Arg1& b ) : f(a), x(b) {}

   template <class Arg2, class Arg3>
   struct Sig 
   : public FunType<typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg2Type,
                    typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg3Type,
                    typename Ternary::template Sig<Arg1,Arg2,Arg3>::ResultType> {};

   template <class Arg2, class Arg3>
   typename RT<Ternary,Arg1,Arg2,Arg3>::ResultType
   operator()( const Arg2& y, const Arg3& z ) const {
      return f(x,y,z);
   }
};

struct Bind1of3 {
   template <class Ternary, class Arg1>
   struct Sig 
   : public FunType<Ternary,Arg1,Full2<binder1of3<Ternary,Arg1> > > {};

   template <class Ternary, class Arg1>
   typename Sig<Ternary,Arg1>::ResultType
   operator()( const Ternary& f, const Arg1& x ) const {
      return makeFull2( binder1of3<Ternary,Arg1>(f,x) );
   }
};

template <class Ternary, class Arg2>
class binder2of3 {
   const Ternary f;
   const Arg2 x;
public:
   binder2of3( const Ternary& a, const Arg2& b ) : f(a), x(b) {}

   template <class Arg1, class Arg3>
   struct Sig 
   : public FunType<typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg1Type,
                    typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg3Type,
                    typename Ternary::template Sig<Arg1,Arg2,Arg3>::ResultType> {};

   template <class Arg1, class Arg3>
   typename RT<Ternary,Arg1,Arg2,Arg3>::ResultType
   operator()( const Arg1& y, const Arg3& z ) const {
      return f(y,x,z);
   }
};

struct Bind2of3 {
   template <class Ternary, class Arg2>
   struct Sig 
   : public FunType<Ternary,Arg2,Full2<binder2of3<Ternary,Arg2> > > {};

   template <class Ternary, class Arg2>
   typename Sig<Ternary,Arg2>::ResultType
   operator()( const Ternary& f, const Arg2& x ) const {
      return makeFull2( binder2of3<Ternary,Arg2>(f,x) );
   }
};

template <class Ternary, class Arg3>
class binder3of3 {
   const Ternary f;
   const Arg3 x;
public:
   binder3of3( const Ternary& a, const Arg3& b ) : f(a), x(b) {}

   template <class Arg1, class Arg2>
   struct Sig 
   : public FunType<typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg1Type,
                    typename Ternary::template Sig<Arg1,Arg2,Arg3>::Arg2Type,
                    typename Ternary::template Sig<Arg1,Arg2,Arg3>::ResultType> {};

   template <class Arg1, class Arg2>
   typename RT<Ternary,Arg1,Arg2,Arg3>::ResultType
   operator()( const Arg1& y, const Arg2& z ) const {
      return f(y,z,x);
   }
};

struct Bind3of3 {
   template <class Ternary, class Arg3>
   struct Sig 
   : public FunType<Ternary,Arg3,Full2<binder3of3<Ternary,Arg3> > > {};

   template <class Ternary, class Arg3>
   typename Sig<Ternary,Arg3>::ResultType
   operator()( const Ternary& f, const Arg3& x ) const {
      return makeFull2( binder3of3<Ternary,Arg3>(f,x) );
   }
};

//////////////////////////////////////////////////////////////////////
// "curry" versions. Now that we have auto-currying, you rarely need to
// call curry() explicitly, unless you are trying to get lazy evaulation
// in examples like
//    curry2( map, f, l ).
// It used to be the case that you might also do
//    curry2( map, f ),
// but nowadays you can say the same thing with just
//    map(f).
//////////////////////////////////////////////////////////////////////

struct Curry3 {
  template <class Ternary, class A1, class A2 = void, class A3 = void>
  struct Sig 
    : public FunType<Ternary, A1, A2, A3, 
                     Full0<binder1and2and3of3<Ternary,A1,A2,A3> > > {};

  template <class Ternary, class A1, class A2>
  struct Sig<Ternary, A1, A2, void> 
  : public FunType<Ternary, A1, A2, Full1<binder1and2of3<Ternary,A1,A2> > > {};  

  template <class Ternary, class A1>
  struct Sig<Ternary, A1, void, void> 
    : public FunType<Ternary, A1, Full2<binder1of3<Ternary,A1> > > {};  


  template <class Ternary, class A1, class A2, class A3>
  typename Sig<Ternary, A1, A2, A3>::ResultType
  operator()( const Ternary& f, const A1& a1, const A2& a2, 
	      const A3& a3 ) const {
    return makeFull0( binder1and2and3of3<Ternary,A1,A2,A3>(f,a1,a2,a3) );
  }

  template <class Ternary, class A1, class A2>
  typename Sig<Ternary, A1, A2>::ResultType
  operator()( const Ternary& f, const A1& a1, const A2& a2 ) const {
    return makeFull1( binder1and2of3<Ternary,A1,A2>(f,a1,a2) );
  }

  template <class Ternary, class A1>
  typename Sig<Ternary, A1>::ResultType
  operator()( const Ternary& f, const A1& a1 ) const {
    return makeFull2( binder1of3<Ternary,A1>(f,a1) );
  }
};

struct Curry2 {
  template <class Binary, class A1, class A2 = void>
  struct Sig 
  : public FunType<Binary, A1, A2, Full0<binder1and2of2<Binary,A1,A2> > > {};

  template <class Binary, class A1>
  struct Sig<Binary, A1, void> 
    : public FunType<Binary, A1, Full1<binder1of2<Binary,A1> > > {};  


  template <class Binary, class A1, class A2>
  typename Sig<Binary, A1, A2>::ResultType
  operator()( const Binary& f, const A1& a1, const A2& a2 ) const {
    return makeFull0( binder1and2of2<Binary,A1,A2>(f,a1,a2) );
  }

  template <class Binary, class A1>
  typename Sig<Binary, A1>::ResultType
  operator()( const Binary& f, const A1& a1 ) const {
    return makeFull1( binder1of2<Binary,A1>(f,a1) );
  }
};

struct Curry1 {
  template <class Unary, class A1>
  struct Sig : public FunType<Unary, A1, Full0<binder1of1<Unary,A1> > > {};  

  template <class Unary, class A1>
  typename Sig<Unary, A1>::ResultType
  operator()( const Unary& f, const A1& a1 ) const {
    return makeFull0( binder1of1<Unary,A1>(f,a1) );
  }
};

//////////////////////////////////////////////////////////////////////
// Finally, Curryable3 (what a huge beast)
//////////////////////////////////////////////////////////////////////

template <class R, class F, class X, class Y, class Z>
struct Curryable3Helper {
   static inline R go( const F& f, const X& x, const Y& y, const Z& z ) {
      return f(x,y,z); 
   }
};

template <class R, class F, class X>
struct Curryable3Helper<R,F,X,AutoCurryType,AutoCurryType> {
   static R go( const F& f, const X& x, const AutoCurryType&, 
                 const AutoCurryType& ) {
      return makeFull2( binder1of3<F,X>(f,x) );
   }
};

template <class R, class F, class Y>
struct Curryable3Helper<R,F,AutoCurryType,Y,AutoCurryType> {
   static R go( const F& f, const AutoCurryType&, const Y& y, 
                 const AutoCurryType& ) {
      return makeFull2( binder2of3<F,Y>(f,y) );
   }
};

template <class R, class F, class Z>
struct Curryable3Helper<R,F,AutoCurryType,AutoCurryType,Z> {
   static R go( const F& f, const AutoCurryType&, const AutoCurryType&,
                 const Z& z ) {
      return makeFull2( binder3of3<F,Z>(f,z) );
   }
};

template <class R, class F, class Y, class Z>
struct Curryable3Helper<R,F,AutoCurryType,Y,Z> {
   static R go( const F& f, const AutoCurryType&, const Y& y, 
                 const Z& z ) {
      return makeFull1( binder2and3of3<F,Y,Z>(f,y,z) );
   }
};

template <class R, class F, class X, class Z>
struct Curryable3Helper<R,F,X,AutoCurryType,Z> {
   static R go( const F& f, const X& x, const AutoCurryType&, 
                 const Z& z ) {
      return makeFull1( binder1and3of3<F,X,Z>(f,x,z) );
   }
};

template <class R, class F, class X, class Y>
struct Curryable3Helper<R,F,X,Y,AutoCurryType> {
   static R go( const F& f, const X& x, const Y& y, 
                 const AutoCurryType& ) {
      return makeFull1( binder1and2of3<F,X,Y>(f,x,y) );
   }
};

template <class R, class F, class X, class Y>
struct Curryable3Helper2 {
   static R go( const F& f, const X& x, const Y& y ) {
      return makeFull1( binder1and2of3<F,X,Y>(f,x,y) );
   }
};

template <class R, class F, class Y>
struct Curryable3Helper2<R,F,AutoCurryType,Y> {
   static R go( const F& f, const AutoCurryType&, const Y& y ) {
      return makeFull2(binder2of3<F,Y>(f,y));
   }
};

template <class R, class F, class X>
struct Curryable3Helper2<R,F,X,AutoCurryType> {
   static R go( const F& f, const X& x, const AutoCurryType& ) {
      return makeFull2(binder1of3<F,X>(f,x));
   }
};

template <class F>
class Curryable3 {
   const F f;
public:
   Curryable3( const F& ff ) : f(ff) {}

   template <class X, class Y=void, class Z=void>
   struct Sig
   : public FunType<typename F::template Sig<X,Y,Z>::Arg1Type,
                    typename F::template Sig<X,Y,Z>::Arg2Type,
                    typename F::template Sig<X,Y,Z>::Arg3Type,
                    typename RT<F,X,Y,Z>::ResultType> {};

   template <class X,class Y> struct Sig<X,Y,void> 
   : public FunType<X,Y,Full1<binder1and2of3<F,X,Y> > > {};

   template <class X> struct Sig<X,AutoCurryType,void> 
   : public FunType<X,AutoCurryType,Full2<binder1of3<F,X> > > {};

   template <class Y> struct Sig<AutoCurryType,Y,void> 
   : public FunType<AutoCurryType,Y,Full2<binder2of3<F,Y> > > {};

   template <class X> struct Sig<X,void,void> 
   : public FunType<X,Full2<binder1of3<F,X> > > {};

   template <class X> struct Sig<X,AutoCurryType,AutoCurryType> 
   : public FunType<X,AutoCurryType,AutoCurryType,
                    Full2<binder1of3<F,X> > > {};

   template <class Y> struct Sig<AutoCurryType,Y,AutoCurryType> 
   : public FunType<AutoCurryType,Y,AutoCurryType,
                    Full2<binder2of3<F,Y> > > {};

   template <class Z> struct Sig<AutoCurryType,AutoCurryType,Z> 
   : public FunType<AutoCurryType,AutoCurryType,Z,
                    Full2<binder3of3<F,Z> > > {};

   template <class X,class Z> struct Sig<X,AutoCurryType,Z> 
   : public FunType<X,AutoCurryType,Z,Full1<binder1and3of3<F,X,Z> > > {};

   template <class Y,class Z> struct Sig<AutoCurryType,Y,Z> 
   : public FunType<AutoCurryType,Y,Z,Full1<binder2and3of3<F,Y,Z> > > {};

   template <class X,class Y> struct Sig<X,Y,AutoCurryType> 
   : public FunType<X,Y,AutoCurryType,Full1<binder1and2of3<F,X,Y> > > {};

   template <class X,class Y>
   typename Sig<X,Y>::ResultType operator()( const X& x, const Y& y ) const {
      // need partial specialization, so defer to a class helper
      return Curryable3Helper2<typename Sig<X,Y>::ResultType,F,X,Y>::go(f,x,y);
   }
   template <class X>
   typename Sig<X>::ResultType operator()( const X& x ) const {
      return makeFull2(binder1of3<F,X>(f,x));
   }
   template <class X, class Y, class Z>
   inline typename Sig<X,Y,Z>::ResultType 
   operator()( const X& x, const Y& y, const Z& z ) const {
      // need partial specialization, so defer to a class helper
return Curryable3Helper<typename Sig<X,Y,Z>::ResultType,F,X,Y,Z>::go(f,x,y,z);
   }
};

} // end namespace impl

using impl::Curryable2;
using impl::Curryable3;
using impl::Curry3;
using impl::Curry2;
using impl::Curry1;

// FIX THIS: the CurryNs are not Full functoids.  Oh well.
FCPP_MAYBE_EXTERN Curry1 curry1, curry; // "curry" is the same as "curry1"
FCPP_MAYBE_EXTERN Curry2 curry2;
FCPP_MAYBE_EXTERN Curry3 curry3; 

// FIX THIS These are deprecated; use makeFullN instead
template <class F>
Curryable2<F> makeCurryable2( const F& f ) {
   return Curryable2<F>( f );
}
template <class F>
Curryable3<F> makeCurryable3( const F& f ) {
   return Curryable3<F>( f );
}

} // end namespace fcpp

#endif
