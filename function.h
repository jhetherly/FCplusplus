//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_FUNCTION_DOT_H
#define FCPP_FUNCTION_DOT_H

//////////////////////////////////////////////////////////////////////////
// Here is where FunN and FunNImpl classes are declared.  Also here are
//  - makeFunN    turn a monomorphic direct functoid into an indirect 
//                functoid
//  - convertN    for implicit conversions (subtype polymorphism)
//  - explicit_convertN   like convertN, but uses casts (non-implicit)
//////////////////////////////////////////////////////////////////////////

#include "ref_count.h"
#include "operator.h"

#ifndef FCPP_NO_USE_NAMESPACE
namespace fcpp {
#endif

// AnyType is the type to use when we don't care about the
// instantiation of a template. This usually happens when we are
// reading the signature of a monomorphic function as if it were
// polymorphic.
class AnyType {};

//////////////////////////////////////////////////////////////////////
// Ok, this file has been a mess, so I'm trying to clean it up.  The
// file is divided into 4 sections, for Fun0, Fun1, Fun2, and Fun3.
// The sections are all pretty redundant, except that
//  - Fun2 implements its own currying (it was written before Curryables were)
//  - Fun3 uses Curryable3 to implement its currying
// As a result, I'm removing some of the redundant comments from all the
// sections except Fun0.  So basically, use the comments in the Fun0
// section as a reference for the corresponding structures in the other
// FunN classes.
//////////////////////////////////////////////////////////////////////

template <class Result>
struct Fun0Impl;

template <class Result>
struct Fun0;

template <class Rd, class Rs>   // result of dest, result of src
Fun0Impl<Rd>* convert0( const IRef<const Fun0Impl<Rs> >& f );

template <class Rd, class DF>
struct Fun0Constructor;

template <class Result>
class Fun0 {
   typedef IRef<const Fun0Impl<Result> > RefImpl;

   RefImpl ref;
   template <class T> friend class Fun0; 
   template <class Rd, class Rs>
   friend Fun0<Rd> explicit_convert0( const Fun0<Rs>& f );

   template <class Rd, class DF>
   friend struct Fun0Constructor;
public:
   // See comment in Fun0Impl about g++2.95.2
   typedef Result ResultType;
   template <class Dummy1=Void, class Dummy2=Void>
   struct Sig : public FunType<WrongNumberOfSigArgs> {};
   template <class Dummy>
   struct Sig<Void,Dummy> : public FunType<Result> {};

   template <class Dummy, int i> struct crazy_accepts {
      static const bool args = false;
   };
   template <class Dummy> struct crazy_accepts<Dummy,0> {
      static const bool args = true;
   };
   static const int crazy_max_args = 0;

   typedef const Fun0Impl<Result>* Impl;
   // int is dummy arg to differentiate from the template constructor
   Fun0( int, Impl i ) : ref(i) {}

   Result operator()() const { return ref->operator()(); }

   template <class DF>   // direct functoid (or subtype polymorphism)
   Fun0( const DF& f ) : ref( Fun0Constructor<Result,DF>::make(f) ) {}

   Fun0( const Fun0& x ) : ref(x.ref) {}
   Fun0& operator=( const Fun0& x ) { ref = x.ref; return *this; }
#ifdef FCPP_ENABLE_LAMBDA
   typedef Fun0 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
};
// See comments below
template <class R> struct Inherits<Fun0<R>,CallableWithoutArguments> 
{ static const bool value = true; };
template <class R> struct Inherits<Fun0<R>,::fcpp::SmartFunctoid> 
{ static const bool value = true; };

template <class Result>
struct Fun0Impl : public IRefable {
   // g++2.95.2 doesn't implement the empty-base-class-optimization, so we
   // 'hand code' the Sig information rather than just inheriting it from a
   // CFunType class.  Fun0s are on the 'critical path' for lists, and need 
   // to be extra efficient.
   typedef Result ResultType;
   template <class Dummy1=Void, class Dummy2=Void>
   struct Sig : public FunType<WrongNumberOfSigArgs> {};
   template <class Dummy>
   struct Sig<Void,Dummy> : public FunType<Result> {};

   virtual Result operator()() const =0;
   virtual ~Fun0Impl() {}
};
// Since we cheated inheritance above, we need to inform our inheritance
// detector for the particular case of importance.
template <class R> struct Inherits<Fun0Impl<R>,CallableWithoutArguments> 
{ static const bool value = true; };

template <class Rd, class Rs>
class Fun0Converter : public Fun0Impl<Rd> {
   typedef IRef<const Fun0Impl<Rs> > MyFun;
   MyFun f;
public:
   Fun0Converter( const MyFun& g ) : f(g) {}
   Rd operator()() const {
      return f->operator()();
   }
};

template <class Rd, class Rs>
Fun0Impl<Rd>* convert0( const IRef<const Fun0Impl<Rs> >& f ) {
   return new Fun0Converter<Rd,Rs>( f );
}

template <class Rd, class Rs>
class Fun0ExplicitConverter : public Fun0Impl<Rd> {
   typedef IRef<const Fun0Impl<Rs> > MyFun;
   MyFun f;
public:
   Fun0ExplicitConverter( const MyFun& g ) : f(g) {}
   Rd operator()() const {
      return static_cast<Rd>( f->operator()() );
   }
};

template <class Rd, class Rs>
Fun0<Rd> explicit_convert0( const Fun0<Rs>& f ) {
   return Fun0<Rd>( 1, new Fun0ExplicitConverter<Rd,Rs>( f.ref ) );
}

template <class Gen>
class Gen0 : public Fun0Impl<typename RT<Gen>::ResultType> {
   Gen g;
public:
   Gen0( Gen x ) : g(x) {}
   typename RT<Gen>::ResultType operator()() const { return g(); }
};
   
template <class Gen>
Fun0<typename RT<Gen>::ResultType> makeFun0( const Gen& g ) {
   return Fun0<typename RT<Gen>::ResultType>(1,new Gen0<Gen>(g));
}

template <class Nullary>
Gen0<Nullary>* makeFun0Ref( const Nullary& g ) {
   return new Gen0<Nullary>(g);
}

// Note: conversion-from-direct-functoid and subtype-polymorphism are
// really two different kinds of functionality.  However, if we try to
// create two separate constructors in the Fun0 class, we end up with
// ambiguity (C++ can't tell which one to call).  As a result, there is
// one constructor that handles both cases by forwarding the work to
// this class, which uses partial specialization to distinguish between
// the two cases.
template <class Rd, class DF>
struct Fun0Constructor {
   static Fun0Impl<Rd>* make( const DF& df ) {
      return makeFun0Ref( ::fcpp::monomorphize0<Rd>(df) );
   }
};
template <class Rd, class Rs>
struct Fun0Constructor<Rd,Fun0<Rs> > {
   static Fun0Impl<Rd>* make( const Fun0<Rs>& f ) {
      return convert0<Rd>(f.ref);
   }
};

//////////////////////////////////////////////////////////////////////

template <class Arg1, class Result>
struct Fun1Impl;

template <class A1d, class Rd, class A1s, class Rs> 
Fun1Impl<A1d,Rd>* convert1( const IRef<const Fun1Impl<A1s,Rs> >& f );

template <class Ad, class Rd, class DF>
struct Fun1Constructor;

template <class Arg1, class Result> 
class Fun1 : public CFunType<Arg1,Result>, public ::fcpp::SmartFunctoid1 {
   typedef IRef<const Fun1Impl<Arg1,Result> > RefImpl;
   RefImpl ref;
   template <class A, class B> friend class Fun1;
   
   template <class Ad, class Rd, class DF>
   friend struct Fun1Constructor;

   template <class A1d, class Rd, class A1s, class Rs> 
   friend Fun1<A1d,Rd> explicit_convert1( const Fun1<A1s,Rs>& f );
public:
   typedef Fun1Impl<Arg1,Result>* Impl;

   Fun1( int, Impl i ) : ref(i) {}
   Result operator()( const Arg1& x ) const { return ref->operator()(x); }

   template <class DF>
   Fun1( const DF& df ) : ref( Fun1Constructor<Arg1,Result,DF>::make(df) ) {}

   Fun1( const Fun1& x ) : ref(x.ref) {}
   Fun1& operator=( const Fun1& x ) { ref = x.ref; return *this; }
#ifdef FCPP_ENABLE_LAMBDA
   typedef Fun1 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
};

template <class Arg1, class Result>
struct Fun1Impl : public CFunType<Arg1,Result>, public IRefable {
   virtual Result operator()( const Arg1& ) const =0;
   virtual ~Fun1Impl() {}
};

template <class A1d, class Rd, class A1s, class Rs> 
class Fun1Converter : public Fun1Impl<A1d,Rd> {
   typedef IRef<const Fun1Impl<A1s,Rs> > MyFun;
   MyFun f;
public:
   Fun1Converter( const MyFun& g ) : f(g) {}
   Rd operator()( const A1d& x ) const {
      return f->operator()( x );
   }
};

template <class A1d, class Rd, class A1s, class Rs> 
Fun1Impl<A1d,Rd>* convert1( const IRef<const Fun1Impl<A1s,Rs> >& f ) {
   return new Fun1Converter<A1d,Rd,A1s,Rs>( f );
}

template <class A1d, class Rd, class A1s, class Rs> 
class Fun1ExplicitConverter : public Fun1Impl<A1d,Rd> {
   typedef IRef<const Fun1Impl<A1s,Rs> > MyFun;
   MyFun f;
public:
   Fun1ExplicitConverter( const MyFun& g ) : f(g) {}
   Rd operator()( const A1d& x ) const {
      return static_cast<Rd>( f->operator()(  static_cast<A1s>(x)  ) );
   }
};

template <class A1d, class Rd, class A1s, class Rs> 
Fun1<A1d,Rd> explicit_convert1( const Fun1<A1s,Rs>& f ) {
   return Fun1<A1d,Rd>( 1, new Fun1ExplicitConverter<A1d,Rd,A1s,Rs>(f.ref) );
}


template <class Gen>
class Gen1 : public Fun1Impl<typename Gen::template Sig<AnyType>::Arg1Type, 
                             typename Gen::template Sig<AnyType>::ResultType> {
   Gen g;
public:
   Gen1( Gen x ) : g(x) {}
   typename Gen::template Sig<AnyType>::ResultType 
   operator()( const typename Gen::template Sig<AnyType>::Arg1Type& x ) const {
      return g(x); 
   }
};
   

template <class Unary>
Fun1<typename Unary::template Sig<AnyType>::Arg1Type, 
     typename Unary::template Sig<AnyType>::ResultType> 
makeFun1( const Unary& g ) {
   return 
     Fun1<typename Unary::template Sig<AnyType>::Arg1Type,
          typename Unary::template Sig<AnyType>::ResultType> 
        (1,new Gen1<Unary>(g));
}

template <class Unary>
Gen1<Unary>* makeFun1Ref( const Unary& g ) {
   return new Gen1<Unary>(g);
}

template <class Ad, class Rd, class DF>
struct Fun1Constructor {
   static Fun1Impl<Ad,Rd>* make( const DF& df ) {
      return makeFun1Ref( ::fcpp::monomorphize1<Ad,Rd>(df) );
   }
};
template <class Ad, class Rd, class As, class Rs>
struct Fun1Constructor<Ad,Rd,Fun1<As,Rs> > {
   static Fun1Impl<Ad,Rd>* make( const Fun1<As,Rs>& f ) {
      return convert1<Ad,Rd>(f.ref);
   }
};

//////////////////////////////////////////////////////////////////////

template <class Arg1, class Arg2, class Result>
struct Fun2Impl;

template <class A1d, class A2d, class Rd, class A1s, class A2s, class Rs>
Fun2Impl<A1d,A2d,Rd>* convert2( const IRef<const Fun2Impl<A1s,A2s,Rs> >& f );

template <class A1d, class A2d, class Rd, class DF>
struct Fun2Constructor;

// Note that this class has two signatures: it can be used either as
// a two argument function or as a single argument function (currying).
template <class Arg1, class Arg2, class Result>
class Fun2 : public ::fcpp::SmartFunctoid2 {
   typedef IRef<const Fun2Impl<Arg1, Arg2, Result> > RefImpl;
   RefImpl ref;
   template <class A1, class A2, class R> friend class Fun2;
   template <class A1d, class A2d, class Rd, class A1s, class A2s, class Rs>
   friend Fun2<A1d,A2d,Rd> explicit_convert2( const Fun2<A1s,A2s,Rs>& f );

   template <class A1d, class A2d, class Rd, class DF>
   friend struct Fun2Constructor;

   // kludge while this file not in namepsace (FCPP_NO_USE_NAMESPACE)
   typedef fcpp::AutoCurryType AutoCurryType;   
public:
   // Signature for normal use of the functoid (2 args)
   template <class P1, class P2 = void>
   struct Sig : public FunType<Arg1, Arg2, Result> {};

   // Signature for using this function with automatic currying (1-arg)
   template <class P1>
   struct Sig<P1,void> : public FunType<Arg1, Fun1<Arg2, Result> > {};

   // Signatures for using this function with underscore currying (1-arg)
   template <class P2>
   struct Sig<AutoCurryType,P2> 
   : public FunType<AutoCurryType, P2, Fun1<Arg1, Result> > {};
   template <class P1>
   struct Sig<P1,AutoCurryType> 
   : public FunType<P1, AutoCurryType, Fun1<Arg2, Result> > {};

   typedef Fun2Impl<Arg1,Arg2,Result>* Impl;
   Fun2( int, Impl i ) : ref(i) {}
   
   template <class DF>
   Fun2( const DF& df ) 
   : ref( Fun2Constructor<Arg1,Arg2,Result,DF>::make(df) ) {}

   Fun2( const Fun2& x ) : ref(x.ref) {}
   Fun2& operator=( const Fun2& x ) { ref = x.ref; return *this; }

   // normal call
   Result operator()( const Arg1& x, const Arg2& y ) const { 
      return ref->operator()(x,y); 
   }
  
   // inheritable underscore currying!
   Fun1<Arg1, Result> operator()( const AutoCurryType&, const Arg2& y ) const { 
      return makeFun1(bind2of2(*this, y));
   }
   Fun1<Arg2, Result> operator()( const Arg1& x, const AutoCurryType& ) const { 
      return makeFun1(bind1of2(*this, x));
   }
  

   // REVIEW: Note that this could return a direct functoid, too, which
   // might be more efficient.  Same with other currying calls.

   // inheritable automatic currying!
   Fun1<Arg2,Result> operator()( const Arg1& x) const {
      return makeFun1(bind1of2(*this, x));
   }
#ifdef FCPP_ENABLE_LAMBDA
   typedef Fun2 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
};

template <class Arg1, class Arg2, class Result>
struct Fun2Impl : public CFunType<Arg1,Arg2,Result>, public IRefable {
   virtual Result operator()( const Arg1&, const Arg2& ) const =0;
   virtual ~Fun2Impl() {}
};

template <class A1d, class A2d, class Rd, class A1s, class A2s, class Rs>
class Fun2Converter : public Fun2Impl<A1d,A2d,Rd> {
   typedef IRef<const Fun2Impl<A1s,A2s,Rs> > MyFun;
   MyFun f;
public:
   Fun2Converter( const MyFun& g ) : f(g) {}
   Rd operator()( const A1d& x, const A2d& y ) const {
      return f->operator()( x, y );
   }
};

template <class A1d, class A2d, class Rd, class A1s, class A2s, class Rs>
Fun2Impl<A1d,A2d,Rd>* convert2( const IRef<const Fun2Impl<A1s,A2s,Rs> >& f ) {
   return new Fun2Converter<A1d,A2d,Rd,A1s,A2s,Rs>( f );
}

template <class A1d, class A2d, class Rd, class A1s, class A2s, class Rs>
class Fun2ExplicitConverter : public Fun2Impl<A1d,A2d,Rd> {
   typedef IRef<const Fun2Impl<A1s,A2s,Rs> > MyFun;
   MyFun f;
public:
   Fun2ExplicitConverter( const MyFun& g ) : f(g) {}
   Rd operator()( const A1d& x, const A2d& y ) const {
      return static_cast<Rd>( f->operator()( static_cast<A1s>(x), 
                                             static_cast<A2s>(y) ) );
   }
};

template <class A1d, class A2d, class Rd, class A1s, class A2s, class Rs>
Fun2<A1d,A2d,Rd> explicit_convert2( const Fun2<A1s,A2s,Rs>& f ) {
   return Fun2<A1d,A2d,Rd>( 1, 
      new Fun2ExplicitConverter<A1d,A2d,Rd,A1s,A2s,Rs>(f.ref) );
}

template <class Gen>
class Gen2 : public Fun2Impl<
      typename Gen::template Sig<AnyType, AnyType>::Arg1Type, 
      typename Gen::template Sig<AnyType, AnyType>::Arg2Type,
      typename Gen::template Sig<AnyType, AnyType>::ResultType>
{
   Gen g;
public:
   Gen2( Gen x ) : g(x) {}
   typename Gen::template Sig<AnyType, AnyType>::ResultType 
   operator()( 
      const typename Gen::template Sig<AnyType,AnyType>::Arg1Type& x,
      const typename Gen::template Sig<AnyType,AnyType>::Arg2Type& y ) const {
      return g(x,y); 
   }
};
   
template <class Binary>
Fun2<typename Binary::template Sig<AnyType, AnyType>::Arg1Type, 
     typename Binary::template Sig<AnyType, AnyType>::Arg2Type,
     typename Binary::template Sig<AnyType, AnyType>::ResultType> 
makeFun2( const Binary& g ) {
   return Fun2<typename Binary::template Sig<AnyType, AnyType>::Arg1Type,
               typename Binary::template Sig<AnyType, AnyType>::Arg2Type,
               typename Binary::template Sig<AnyType, AnyType>::ResultType> 
     (1,new Gen2<Binary>(g));
}

template <class Binary>
Gen2<Binary>* makeFun2Ref( const Binary& g ) {
   return new Gen2<Binary>(g);
}

template <class A1d, class A2d, class Rd, class DF>
struct Fun2Constructor {
   static Fun2Impl<A1d,A2d,Rd>* make( const DF& df ) {
      return makeFun2Ref( ::fcpp::monomorphize2<A1d,A2d,Rd>(df) );
   }
};
template <class A1d, class A2d, class Rd, class A1s, class A2s, class Rs>
struct Fun2Constructor<A1d,A2d,Rd,Fun2<A1s,A2s,Rs> > {
   static Fun2Impl<A1d,A2d,Rd>* make( const Fun2<A1s,A2s,Rs>& f ) {
      return convert2<A1d,A2d,Rd>(f.ref);
   }
};

//////////////////////////////////////////////////////////////////////

template <class Arg1, class Arg2, class Arg3, class Result>
struct Fun3Impl;

template <class Arg1, class Arg2, class Arg3, class Result>
struct Fun3;

template <class A1d, class A2d, class A3d, class Rd, 
          class A1s, class A2s, class A3s, class Rs> 
Fun3Impl<A1d,A2d,A3d,Rd>* 
convert3( const IRef<const Fun3Impl<A1s,A2s,A3s,Rs> >& f );

template <class A1d, class A2d, class A3d, class Rd, class DF>
struct Fun3Constructor;

// The "Guts" class helps us implement currying; Fun3 floats gently atop
// Fun3Guts and adds currying.  
template <class Arg1, class Arg2, class Arg3, class Result>
class Fun3Guts : public CFunType<Arg1,Arg2,Arg3,Result> {
   typedef IRef<const Fun3Impl<Arg1,Arg2,Arg3,Result> > RefImpl;
   RefImpl ref;
   template <class A, class B, class C, class D> friend class Fun3Guts;
   template <class A, class B, class C, class D> friend class Fun3;
   
   template <class A1d, class A2d, class A3d, class Rd, class DF>
   friend struct Fun3Constructor;

   template <class A1d, class A2d, class A3d, class Rd, 
             class A1s, class A2s, class A3s, class Rs> 
   friend Fun3<A1d,A2d,A3d,Rd> 
   explicit_convert3( const Fun3<A1s,A2s,A3s,Rs>& f );
public:
   typedef Fun3Impl<Arg1,Arg2,Arg3,Result>* Impl;

   Fun3Guts( int, Impl i ) : ref(i) {}
   Result operator()( const Arg1& x, const Arg2& y, const Arg3& z ) const 
      { return ref->operator()(x,y,z); }

   template <class DF>
   Fun3Guts( const DF& df )
   : ref( Fun3Constructor<Arg1,Arg2,Arg3,Result,DF>::make(df) ) {}

   Fun3Guts( const Fun3Guts& x ) : ref(x.ref) {}
   Fun3Guts& operator=( const Fun3Guts& x ) { ref = x.ref; return *this; }
};

template <class Arg1, class Arg2, class Arg3, class Result>
class Fun3 : public ::fcpp::SmartFunctoid3 {
   template <class A1d, class A2d, class A3d, class Rd, class DF>
   friend struct Fun3Constructor;

   template <class A1d, class A2d, class A3d, class Rd, 
             class A1s, class A2s, class A3s, class Rs> 
   friend Fun3<A1d,A2d,A3d,Rd> 
   explicit_convert3( const Fun3<A1s,A2s,A3s,Rs>& f );

   Fun3Guts<Arg1,Arg2,Arg3,Result> rep;
public:
   typedef Fun3Impl<Arg1,Arg2,Arg3,Result>* Impl;

   Fun3( int, Impl i ) : rep(1,i) {}

   template <class DF> 
   Fun3( const DF& df ) : rep(df) {}

   Fun3( const Fun3& x ) : rep(x.rep) {}
   Fun3& operator=( const Fun3& x ) { rep = x.rep; return *this; }
   
   typedef fcpp::Curryable3<Fun3Guts<Arg1,Arg2,Arg3,Result> > SigHelp;
   template <class A, class B=void, class C=void>
   struct Sig : public SigHelp::template Sig<A,B,C> {};
 
   template <class A, class B, class C>
   typename Sig<A,B,C>::ResultType
   operator()( const A& x, const B& y, const C& z ) const 
      { return ::fcpp::makeCurryable3(rep)(x,y,z); }

   template <class A, class B>
   typename Sig<A,B>::ResultType
   operator()( const A& x, const B& y ) const 
      { return ::fcpp::curry3(rep,x,y); }

   template <class A>
   typename Sig<A>::ResultType
   operator()( const A& x ) const 
      { return ::fcpp::curry3(rep,x); }
#ifdef FCPP_ENABLE_LAMBDA
   typedef Fun3 This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
};

template <class Arg1, class Arg2, class Arg3, class Result>
struct Fun3Impl : public CFunType<Arg1,Arg2,Arg3,Result>, public IRefable {
public:
  Fun3Impl() {}

   virtual Result operator()( const Arg1&, const Arg2&, const Arg3& ) const =0;
   virtual ~Fun3Impl() {}
};

template <class A1d, class A2d, class A3d, class Rd, 
          class A1s, class A2s, class A3s, class Rs> 
class Fun3Converter : public Fun3Impl<A1d,A2d,A3d,Rd> {
   typedef IRef<const Fun3Impl<A1s,A2s,A3s,Rs> > MyFun;
   MyFun f;
public:
   Fun3Converter( const MyFun& g ) : f(g) {}
   Rd operator()( const A1d& x, const A2d& y, const A3d& z ) const {
      return f->operator()( x, y, z );
   }
};

template <class A1d, class A2d, class A3d, class Rd, 
          class A1s, class A2s, class A3s, class Rs> 
Fun3Impl<A1d,A2d,A3d,Rd>* 
convert3( const IRef<const Fun3Impl<A1s,A2s,A3s,Rs> >& f ) {
   return new Fun3Converter<A1d,A2d,A3d,Rd,A1s,A2s,A3s,Rs>( f );
}

template <class A1d, class A2d, class A3d, class Rd, 
          class A1s, class A2s, class A3s, class Rs> 
class Fun3ExplicitConverter : public Fun3Impl<A1d,A2d,A3d,Rd> {
   typedef IRef<const Fun3Impl<A1s,A2s,A3s,Rs> > MyFun;
   MyFun f;
public:
   Fun3ExplicitConverter( const MyFun& g ) : f(g) {}
   Rd operator()( const A1d& x, const A2d& y, const A3d& z ) const {
      return static_cast<Rd>( f->operator()(  static_cast<A1s>(x),
         static_cast<A2s>(y), static_cast<A3s>(z)  ) );
   }
};

template <class A1d, class A2d, class A3d, class Rd, 
          class A1s, class A2s, class A3s, class Rs> 
Fun3<A1d,A2d,A3d,Rd> explicit_convert3( const Fun3<A1s,A2s,A3s,Rs>& f ) {
   return Fun3<A1d,A2d,A3d,Rd>( 1, 
      new Fun3ExplicitConverter<A1d,A2d,A3d,Rd,A1s,A2s,A3s,Rs>(f.rep.ref) );
}

template <class Gen>
class Gen3 : public Fun3Impl<
   typename Gen::template Sig<AnyType,AnyType,AnyType>::Arg1Type, 
   typename Gen::template Sig<AnyType,AnyType,AnyType>::Arg2Type,
   typename Gen::template Sig<AnyType,AnyType,AnyType>::Arg3Type,
   typename Gen::template Sig<AnyType,AnyType,AnyType>::ResultType> {
   Gen g;
public:
   Gen3( Gen x ) : g(x) {}
   typename Gen::template Sig<AnyType,AnyType,AnyType>::ResultType 
   operator()(
const typename Gen::template Sig<AnyType,AnyType,AnyType>::Arg1Type& x,
const typename Gen::template Sig<AnyType,AnyType,AnyType>::Arg2Type& y,
const typename Gen::template Sig<AnyType,AnyType,AnyType>::Arg3Type& z 
             ) const { 
      return g(x,y,z); 
   }
};

template <class Ternary>
Fun3<typename Ternary::template Sig<AnyType,AnyType,AnyType>::Arg1Type, 
     typename Ternary::template Sig<AnyType,AnyType,AnyType>::Arg2Type,
     typename Ternary::template Sig<AnyType,AnyType,AnyType>::Arg3Type,
     typename Ternary::template Sig<AnyType,AnyType,AnyType>::ResultType> 
makeFun3( const Ternary& g ) {
   return 
   Fun3<typename Ternary::template Sig<AnyType,AnyType,AnyType>::Arg1Type,
        typename Ternary::template Sig<AnyType,AnyType,AnyType>::Arg2Type,
        typename Ternary::template Sig<AnyType,AnyType,AnyType>::Arg3Type,
        typename Ternary::template Sig<AnyType,AnyType,AnyType>::ResultType> 
        (1,new Gen3<Ternary>(g));
}

template <class Ternary>
Gen3<Ternary>* makeFun3Ref( const Ternary& g ) {
   return new Gen3<Ternary>(g);
}

template <class A1d, class A2d, class A3d, class Rd, class DF>
struct Fun3Constructor {
   static Fun3Impl<A1d,A2d,A3d,Rd>* make( const DF& df ) {
      return makeFun3Ref( ::fcpp::monomorphize3<A1d,A2d,A3d,Rd>(df) );
   }
};
template <class A1d, class A2d, class A3d, class Rd, 
          class A1s, class A2s, class A3s, class Rs> 
struct Fun3Constructor<A1d,A2d,A3d,Rd,Fun3<A1s,A2s,A3s,Rs> > {
   static Fun3Impl<A1d,A2d,A3d,Rd>* make( const Fun3<A1s,A2s,A3s,Rs>& f ) {
      return convert3<A1d,A2d,A3d,Rd>(f.rep.ref);
   }
};

#ifndef FCPP_NO_USE_NAMESPACE
} // end namespace fcpp
#endif

#endif
