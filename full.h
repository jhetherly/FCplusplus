//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_FULL_DOT_H
#define FCPP_FULL_DOT_H

#include "smart.h"
#include "curry.h"
#include "pre_lambda.h"

namespace fcpp {

//////////////////////////////////////////////////////////////////////
// Full functoids
//////////////////////////////////////////////////////////////////////
// Just as Curryable2/Curryable3 serve as wrappers which decorate
// functoids to give them 'curryability', FullN are wrappers to give
// functoids _all_ the 'features' that functoids can have.  The current
// "extra feature set" for functoids is
//   - curryability     (ability to call with fewer args)
//   - lambda-awareness (operator[] for use inside lambda)
//   - smartness        (inherited typedefs to answer introspective questions)
// The FullN classes just combine all of the features into one
// uber-wrapper which does it all.

// Don't forget that any features added here need to be added to 
// the indirect functoids in function.h, too.
// And also to the Uncurryable class in prelude.h.
// FIX THIS take a good look at Uncurry; it may have pre-dated smartness
// and can now be written more sanely...?
template <class F>
class Full0 
: public SmartFunctoid0, public CFunType<typename RT<F>::ResultType> {
   F f;
public:
   Full0() : f() {}
   Full0( const F& ff ) : f(ff) {}
#ifdef FCPP_ENABLE_LAMBDA
   typedef Full0 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
   inline typename RT<F>::ResultType operator()() const {
      return f();
   }
};

template <class F>
class Full1 : public SmartFunctoid1 {
   F f;
public:
   Full1() : f() {}
   Full1( const F& ff ) : f(ff) {}
#ifdef FCPP_ENABLE_LAMBDA
   typedef Full1 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
   template <class T> struct Sig 
      : public FunType<typename RT<F,T>::Arg1Type,
                       typename RT<F,T>::ResultType> {};
   template <class T>
   inline typename Sig<T>::ResultType operator()( const T& x ) const {
      return f(x);
   }
};

template <class F>
class Full2 : public SmartFunctoid2 {
   F f;
public:
   Full2() : f() {}
   Full2( const F& ff ) : f(ff) {}
#ifdef FCPP_ENABLE_LAMBDA
   typedef Full2 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
/////////////  copied from Curryable2; added impl:: to binders  //////
   template <class X, class Y=void>
   struct Sig
   : public FunType<typename F::template Sig<X,Y>::Arg1Type,
                    typename F::template Sig<X,Y>::Arg2Type,
                    typename RT<F,X,Y>::ResultType> {};

   template <class X>
   struct Sig<X,void> : public FunType<X,Full1<impl::binder1of2<F,X> > > {};

   template <class Y>
   struct Sig<AutoCurryType,Y>
   : public FunType<AutoCurryType,Y,Full1<impl::binder2of2<F,Y> > > {};

   template <class X>
   struct Sig<X,AutoCurryType>
   : public FunType<X,AutoCurryType,Full1<impl::binder1of2<F,X> > > {};

   template <class X>
   typename Sig<X>::ResultType operator()( const X& x ) const {
      return makeFull1( impl::binder1of2<F,X>(f,x) );
   }
   template <class X, class Y>
   inline typename Sig<X,Y>::ResultType
   operator()( const X& x, const Y& y ) const {
      // need partial specialization, so defer to a class helper
return impl::Curryable2Helper<typename Sig<X,Y>::ResultType,F,X,Y>::go(f,x,y);
   }
//////////////////////////////////////////////////////////////////////
};

template <class F>
class Full3 : public SmartFunctoid3 {
   F f;
public:
   Full3() : f() {}
   Full3( const F& ff ) : f(ff) {}
#ifdef FCPP_ENABLE_LAMBDA
   typedef Full3 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
/////////////  copied from Curryable3; added impl:: to all binders  //
   template <class X, class Y=void, class Z=void>
   struct Sig
   : public FunType<typename F::template Sig<X,Y,Z>::Arg1Type,
                    typename F::template Sig<X,Y,Z>::Arg2Type,
                    typename F::template Sig<X,Y,Z>::Arg3Type,
                    typename RT<F,X,Y,Z>::ResultType> {};

   template <class X,class Y> struct Sig<X,Y,void>
   : public FunType<X,Y,Full1<impl::binder1and2of3<F,X,Y> > > {};

   template <class X> struct Sig<X,AutoCurryType,void>
   : public FunType<X,AutoCurryType,Full2<impl::binder1of3<F,X> > > {};

   template <class Y> struct Sig<AutoCurryType,Y,void>
   : public FunType<AutoCurryType,Y,Full2<impl::binder2of3<F,Y> > > {};

   template <class X> struct Sig<X,void,void>
   : public FunType<X,Full2<impl::binder1of3<F,X> > > {};

   template <class X> struct Sig<X,AutoCurryType,AutoCurryType>
   : public FunType<X,AutoCurryType,AutoCurryType,
                    Full2<impl::binder1of3<F,X> > > {};

   template <class Y> struct Sig<AutoCurryType,Y,AutoCurryType>
   : public FunType<AutoCurryType,Y,AutoCurryType,
                    Full2<impl::binder2of3<F,Y> > > {};

   template <class Z> struct Sig<AutoCurryType,AutoCurryType,Z>
   : public FunType<AutoCurryType,AutoCurryType,Z,
                    Full2<impl::binder3of3<F,Z> > > {};

   template <class X,class Z> struct Sig<X,AutoCurryType,Z>
   : public FunType<X,AutoCurryType,Z,Full1<impl::binder1and3of3<F,X,Z> > > {};

   template <class Y,class Z> struct Sig<AutoCurryType,Y,Z>
   : public FunType<AutoCurryType,Y,Z,Full1<impl::binder2and3of3<F,Y,Z> > > {};

   template <class X,class Y> struct Sig<X,Y,AutoCurryType>
   : public FunType<X,Y,AutoCurryType,Full1<impl::binder1and2of3<F,X,Y> > > {};

   template <class X,class Y>
   typename Sig<X,Y>::ResultType operator()( const X& x, const Y& y ) const {
      // need partial specialization, so defer to a class helper
return impl::Curryable3Helper2<typename Sig<X,Y>::ResultType,F,X,Y>::go(f,x,y);
   }
   template <class X>
   typename Sig<X>::ResultType operator()( const X& x ) const {
      return makeFull2(impl::binder1of3<F,X>(f,x));
   }

   template <class X, class Y, class Z>
   inline typename Sig<X,Y,Z>::ResultType
   operator()( const X& x, const Y& y, const Z& z ) const {
      // need partial specialization, so defer to a class helper
      return impl::Curryable3Helper<typename Sig<X,Y,Z>::ResultType,F,X,Y,Z>
      ::go(f,x,y,z);
   }
//////////////////////////////////////////////////////////////////////
};

template <class F> Full0<F> makeFull0( const F& f ) { return Full0<F>(f); }
template <class F> Full1<F> makeFull1( const F& f ) { return Full1<F>(f); }
template <class F> Full2<F> makeFull2( const F& f ) { return Full2<F>(f); }
template <class F> Full3<F> makeFull3( const F& f ) { return Full3<F>(f); }

//////////////////////////////////////////////////////////////////////
// Definitions of stuff heretofore put-off...
//////////////////////////////////////////////////////////////////////
// from curry.h:
typedef Full1<impl::Const> Const;
typedef Full2<impl::Bind1of1> Bind1of1;

typedef Full2<impl::Bind1of2> Bind1of2;
typedef Full2<impl::Bind2of2> Bind2of2;
typedef Full3<impl::Bind1and2of2> Bind1and2of2;

// FIX THIS? I never noticed: the only 4-arg functoid in the entire library
typedef impl::Bind1and2and3of3 Bind1and2and3of3;

typedef Full3<impl::Bind1and2of3> Bind1and2of3;
typedef Full3<impl::Bind2and3of3> Bind2and3of3;
typedef Full3<impl::Bind1and3of3> Bind1and3of3;
typedef Full2<impl::Bind1of3> Bind1of3;
typedef Full2<impl::Bind2of3> Bind2of3;
typedef Full2<impl::Bind3of3> Bind3of3;

FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Const const_;   // C++ keyword, so add trailing underscore
FCPP_MAYBE_EXTERN Bind1of1 bind1of1;

FCPP_MAYBE_EXTERN Bind1of2 bind1of2;
FCPP_MAYBE_EXTERN Bind2of2 bind2of2;
FCPP_MAYBE_EXTERN Bind1and2of2 bind1and2of2;

FCPP_MAYBE_EXTERN Bind1and2and3of3 bind1and2and3of3;
FCPP_MAYBE_EXTERN Bind1and2of3 bind1and2of3;
FCPP_MAYBE_EXTERN Bind2and3of3 bind2and3of3;
FCPP_MAYBE_EXTERN Bind1and3of3 bind1and3of3;
FCPP_MAYBE_EXTERN Bind1of3 bind1of3;
FCPP_MAYBE_EXTERN Bind2of3 bind2of3;
FCPP_MAYBE_EXTERN Bind3of3 bind3of3;
FCPP_MAYBE_NAMESPACE_CLOSE

} // end namespace fcpp

#endif
