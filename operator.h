//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_OPERATOR_DOT_H
#define FCPP_OPERATOR_DOT_H

//////////////////////////////////////////////////////////////////////
// The goal here is to provide functoids for most C++ operators (e.g.
// Plus, Greater, ...) as well as conversions between representations.
// The conversions include ptr_to_fun, for turning function pointers into
// functoids, stl_to_funN, for turning STL functoids into our functoids,
// and monomophizeN, for converting polymorphic direct functoids into
// monomorphic ones.
//
// There's also some miscellaneous stuff at both the beginning and the 
// end of this file, for lack of a better place to put it.
//////////////////////////////////////////////////////////////////////

#include <utility>
#include <iterator>
#include <iostream>
#include "lambda.h"

namespace fcpp {

//////////////////////////////////////////////////////////////////////
// syntactic sugar for infix operators
//////////////////////////////////////////////////////////////////////
// The syntax
//    arg1 ^fun^ arg2     means     fun( arg1, arg2 )
// like Haskell's backquotes.
//
// I feel justified in this convenient abuse of operator overloading in
// that it's complete nonsense for someone to try to XOR a value with a
// 2-argument full functoid.  Put another way, I own Full2<F>s; I can do 
// whatever I want with them, darn it!  :)
//
// Note that it also works on Full3s (with currying).

template <class LHS, class Fun>
struct InfixOpThingy {
   // Note that storing const&s here relies on the fact that temporaries
   // are guaranteed to live for the duration of the full-expression in
   // which they are created.  There's no need to create copies.
   const LHS& lhs;
   const Fun& f;
   InfixOpThingy( const LHS& l, const Fun& ff ) : lhs(l), f(ff) {}
};

template <class LHS, class F>
inline InfixOpThingy<LHS,Full2<F> >
operator^( const LHS& lhs, const Full2<F>& f ) {
   return InfixOpThingy<LHS,Full2<F> >(lhs,f);
}

template <class LHS, class F>
inline InfixOpThingy<LHS,Full3<F> >
operator^( const LHS& lhs, const Full3<F>& f ) {
   return InfixOpThingy<LHS,Full3<F> >(lhs,f);
}

template <class LHS, class FF, class RHS>
inline typename RT<FF,LHS,RHS>::ResultType
operator^( const InfixOpThingy<LHS,FF>& x, const RHS& rhs ) {
   return x.f( x.lhs, rhs );
}

template <class LHS, class FF, class G>
inline typename RT<FF,LHS,Full2<G> >::ResultType
operator^( const InfixOpThingy<LHS,FF>& x, const Full2<G>& rhs ) {
   return x.f( x.lhs, rhs );
}

template <class LHS, class FF, class G>
inline typename RT<FF,LHS,Full3<G> >::ResultType
operator^( const InfixOpThingy<LHS,FF>& x, const Full3<G>& rhs ) {
   return x.f( x.lhs, rhs );
}

// Furthermore, I just can't help myself from making
//    arg1 %fun% arg2     mean     fun[ arg1 ][ arg2 ]
// for use in lambda expressions.  % is a good choice because it binds
// more tightly than <=, so it's less likely to interfere with "gets"
// bindings.

#ifdef FCPP_ENABLE_LAMBDA

template <class LHS, class Fun>
struct InfixOpWhatzit {
   // See comment in InfixOpThingy
   const LHS& lhs;
   const Fun& f;
   InfixOpWhatzit( const LHS& l, const Fun& ff ) : lhs(l), f(ff) {}
};

template <class LHS, class F>
inline InfixOpWhatzit<LHS,Full2<F> >
operator%( const LHS& lhs, const Full2<F>& f ) {
   return InfixOpWhatzit<LHS,Full2<F> >(lhs,f);
}

template <class LHS, class F>
inline InfixOpWhatzit<LHS,Full3<F> >
operator%( const LHS& lhs, const Full3<F>& f ) {
   return InfixOpWhatzit<LHS,Full3<F> >(lhs,f);
}

template <class LHS, class FF, class RHS>
inline typename LEType<CALL<CALL<FF,LHS>,RHS> >::Type
operator%( const InfixOpWhatzit<LHS,FF>& x, const RHS& rhs ) {
   return x.f[ x.lhs ][ rhs ];
}

template <class LHS, class FF, class G>
inline typename LEType<CALL<CALL<FF,LHS>,Full2<G> > >::Type
operator%( const InfixOpWhatzit<LHS,FF>& x, const Full2<G>& rhs ) {
   return x.f[ x.lhs ][ rhs ];
}

template <class LHS, class FF, class G>
inline typename LEType<CALL<CALL<FF,LHS>,Full3<G> > >::Type
operator%( const InfixOpWhatzit<LHS,FF>& x, const Full3<G>& rhs ) {
   return x.f[ x.lhs ][ rhs ];
}

#endif

//////////////////////////////////////////////////////////////////////
// operators
//////////////////////////////////////////////////////////////////////

namespace impl {
struct XMakePair {
   template <class A, class B>
   struct Sig : public FunType<A,B,std::pair<A,B> > {};

   template <class A, class B>
   std::pair<A,B> operator()( const A& a, const B& b ) const {
      return std::make_pair(a,b);
   }
};
}
typedef Full2<impl::XMakePair> MakePair;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN MakePair makePair;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XMin {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      return std::less<T>()( x, y ) ? x : y;
   }
};
}
typedef Full2<impl::XMin> Min;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Min min;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XMax {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      return std::less<T>()( x, y ) ? y : x;
   }
};
}
typedef Full2<impl::XMax> Max;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Max max;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XPlus {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      return std::plus<T>()( x, y );
   }
};
}
typedef Full2<impl::XPlus> Plus;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Plus plus;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XMinus {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      return std::minus<T>()( x, y );
   }
};
}
typedef Full2<impl::XMinus> Minus;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Minus minus;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XMultiplies {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      return std::multiplies<T>()( x, y );
   }
};
}
typedef Full2<impl::XMultiplies> Multiplies;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Multiplies multiplies;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XDivides {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      return std::divides<T>()( x, y );
   }
};
}
typedef Full2<impl::XDivides> Divides;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Divides divides;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XModulus {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      return std::modulus<T>()( x, y );
   }
};
}
typedef Full2<impl::XModulus> Modulus;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Modulus modulus;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XNegate {
   template <class T>
   struct Sig : public FunType<T,T> {};

   template <class T>
   T operator()( const T& x ) const {
      return std::negate<T>()( x );
   }
};
}
typedef Full1<impl::XNegate> Negate;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Negate negate;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XEqual {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::equal_to<T>()( x, y );
   }
};
}
typedef Full2<impl::XEqual> Equal;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Equal equal;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XNotEqual {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::not_equal_to<T>()( x, y );
   }
};
}
typedef Full2<impl::XNotEqual> NotEqual;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN NotEqual notEqual;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XGreater {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::greater<T>()( x, y );
   }
};
}
typedef Full2<impl::XGreater> Greater;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Greater greater;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XLess {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::less<T>()( x, y );
   }
};
}
typedef Full2<impl::XLess> Less;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Less less;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XGreaterEqual {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::greater_equal<T>()( x, y );
   }
};
}
typedef Full2<impl::XGreaterEqual> GreaterEqual;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN GreaterEqual greaterEqual;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XLessEqual {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::less_equal<T>()( x, y );
   }
};
}
typedef Full2<impl::XLessEqual> LessEqual;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN LessEqual lessEqual;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XLogicalAnd {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::logical_and<T>()( x, y );
   }
};
}
typedef Full2<impl::XLogicalAnd> LogicalAnd;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN LogicalAnd logicalAnd;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XLogicalOr {
   template <class T, class U> struct Sig;

   template <class T>
   struct Sig<T,T> : public FunType<T,T,bool> {};

   template <class T>
   bool operator()( const T&x, const T&y ) const {
      return std::logical_or<T>()( x, y );
   }
};
}
typedef Full2<impl::XLogicalOr> LogicalOr;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN LogicalOr logicalOr;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XLogicalNot {
   template <class T>
   struct Sig : public FunType<T,bool> {};

   template <class T>
   bool operator()( const T&x ) const {
      return std::logical_not<T>()( x );
   }
};
}
typedef Full1<impl::XLogicalNot> LogicalNot;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN LogicalNot logicalNot;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XDereference {
   template <class T> struct Sig 
   : public FunType<T,typename std::iterator_traits<T>::value_type> {};

   template <class T>
   typename Sig<T>::ResultType operator()( const T& p ) const {
      return *p;
   }
};
}
typedef Full1<impl::XDereference> Dereference;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Dereference dereference;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XAddressOf {
   template <class T>
   struct Sig : public FunType<T,const T*> {};

   template <class T>
   const T* operator()( const T& x ) const {
      return &x;
   }
};
}
typedef Full1<impl::XAddressOf> AddressOf;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN AddressOf addressOf;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
   struct XDelete_ {
      template <class T> struct Sig : public FunType<T,void> {};
      template <class T> void operator()( T* p ) const { delete p; }
   };
}
typedef Full1<impl::XDelete_> Delete_;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Delete_ delete_;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
   template <class Dest>
   struct XDynamicCast {
      template <class Src> struct Sig : public FunType<Src,Dest> {};
      template <class Src>
      Dest operator()( const Src& s ) const {
         return dynamic_cast<Dest>(s);
      }
   };
}
template <class T>
struct DynamicCast { typedef Full1<impl::XDynamicCast<T> > Type; };
template <class T> Full1<impl::XDynamicCast<T> > dynamicCast()
{ return makeFull1( impl::XDynamicCast<T>() ); }

// outStream is the << stream operator, but takes a stream*
//    e.g.   &cout ^outStream^ x
namespace impl {
   struct XOutStream {
      template <class StreamP, class Data> struct Sig
         : public FunType<StreamP,Data,StreamP> {};
      template <class StreamP, class Data>
      StreamP operator()( StreamP s, const Data& x ) const {
         (*s) << x;
         return s;
      }
   };
}
typedef Full2<impl::XOutStream> OutStream;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN OutStream outStream;
FCPP_MAYBE_NAMESPACE_CLOSE

// inStream is the >> stream operator, but takes stream* and data*
//    e.g.   &cin ^inStream^ &x
namespace impl {
   struct XInStream {
      template <class StreamP, class DataP> struct Sig
         : public FunType<StreamP,DataP,StreamP> {};
      template <class StreamP, class DataP>
      StreamP operator()( StreamP s, DataP x ) const {
         (*s) >> (*x);
         return s;
      }
   };
}
typedef Full2<impl::XInStream> InStream;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN InStream inStream;
FCPP_MAYBE_NAMESPACE_CLOSE

#ifndef FCPP_I_AM_GCC2
// makeManip(aStream)(aManip) returns the manipulator for that stream
// (The C++ std stream manipulators have a crazy interface which
// necessitates this ugliness.)
//    e.g.  &cout ^outStream^ makeManip(cout)(endl)
template <class C, class T>
struct ManipMaker {
   std::basic_ostream<C,T>& (*
   operator()( std::basic_ostream<C,T>& (*pfn)( std::basic_ostream<C,T>&) )
         const )( std::basic_ostream<C,T>& ) { return pfn; }
   std::basic_ios<C,T>& (*
   operator()( std::basic_ios<C,T>& (*pfn)( std::basic_ios<C,T>& ) )
         const )( std::basic_ios<C,T>& ) { return pfn; }
   std::ios_base& (*
   operator()( std::ios_base& (*pfn)( std::ios_base& ) )
         const )( std::ios_base& ) { return pfn; }
};
template <class C, class T>
ManipMaker<C,T> makeManip( std::basic_ios<C,T>& ) { return ManipMaker<C,T>(); }
#endif

//////////////////////////////////////////////////////////////////////
// STL conversions
//////////////////////////////////////////////////////////////////////
// Note that these are template functions, not functoids.  I'm lazy.  

namespace impl {
template <class Op>
class Xstl1 : public CFunType<typename Op::argument_type,
                       typename Op::result_type> {
   Op f;
public:
   Xstl1( const Op& o ) : f(o) {}
   typename Op::result_type 
   operator()( const typename Op::argument_type& x ) const {
      return f(x);
   }
};
}
template <class Op>
Full1<impl::Xstl1<Op> > stl_to_fun1( const Op& o ) {
   return makeFull1( impl::Xstl1<Op>(o) );
}

namespace impl {
template <class Op>
class Xstl2 : public CFunType<typename Op::first_argument_type,
                       typename Op::second_argument_type,
                       typename Op::result_type> {
   Op f;
public:
   Xstl2( const Op& o ) : f(o) {}
   typename Op::result_type 
   operator()( const typename Op::first_argument_type& x, 
               const typename Op::second_argument_type& y ) const {
      return f(x,y);
   }
};
}
template <class Op>
Full2<impl::Xstl2<Op> > stl_to_fun2( const Op& o ) {
   return makeFull2(impl::Xstl2<Op>(o));
}

//////////////////////////////////////////////////////////////////////
// monomorphizing conversions
//////////////////////////////////////////////////////////////////////
// Note that these are template functions, not functoids.  I'm lazy.  

namespace impl {
template <class Arg1, class Arg2, class Arg3, class Res, class F>
class XMonomorphicWrapper3 : public CFunType<Arg1,Arg2,Arg3,Res> {
   F f;
public:
   XMonomorphicWrapper3( const F& g ) : f(g) {}
   Res operator()( const Arg1& x, const Arg2& y, const Arg3& z ) const {
      return f(x,y,z);
   }
};
}
template <class Arg1, class Arg2, class Arg3, class Res, class F>
Full3<impl::XMonomorphicWrapper3<Arg1,Arg2,Arg3,Res,F> > 
monomorphize3( const F& f ) {
   return makeFull3(impl::XMonomorphicWrapper3<Arg1,Arg2,Arg3,Res,F>( f ));
}

namespace impl {
template <class Arg1, class Arg2, class Res, class F>
class XMonomorphicWrapper2 : public CFunType<Arg1,Arg2,Res> {
   F f;
public:
   XMonomorphicWrapper2( const F& g ) : f(g) {}
   Res operator()( const Arg1& x, const Arg2& y ) const {
      return f(x,y);
   }
};
}
template <class Arg1, class Arg2, class Res, class F>
Full2<impl::XMonomorphicWrapper2<Arg1,Arg2,Res,F> > 
monomorphize2( const F& f ) {
   return makeFull2(impl::XMonomorphicWrapper2<Arg1,Arg2,Res,F>( f ));
}

namespace impl {
template <class Arg1, class Res, class F>
class XMonomorphicWrapper1 : public CFunType<Arg1,Res> {
   F f;
public:
   XMonomorphicWrapper1( const F& g ) : f(g) {}
   Res operator()( const Arg1& x ) const {
      return f(x);
   }
};
}
template <class Arg1, class Res, class F>
Full1<impl::XMonomorphicWrapper1<Arg1,Res,F> > monomorphize1( const F& f ) {
   return makeFull1( impl::XMonomorphicWrapper1<Arg1,Res,F>( f ) );
}

namespace impl {
template <class Res, class F>
class XMonomorphicWrapper0 : public CFunType<Res> {
   F f;
public:
   XMonomorphicWrapper0( const F& g ) : f(g) {}
   Res operator()() const {
      return f();
   }
};
}
template <class Res, class F>
Full0<impl::XMonomorphicWrapper0<Res,F> > monomorphize0( const F& f ) {
   return makeFull0( impl::XMonomorphicWrapper0<Res,F>( f ) );
}

//////////////////////////////////////////////////////////////////////
// ptr_fun
//////////////////////////////////////////////////////////////////////
// ptr_to_fun is now a functoid -- hurray!

namespace impl {

template <class Result>
class Xptr_to_nullary_function : public CFunType<Result> {
    Result (*ptr)();
public:
    explicit Xptr_to_nullary_function(Result (*x)()) : ptr(x) {}
    Result operator()() const { return ptr(); }
};

template <class Arg, class Result>
class Xptr_to_unary_function : public CFunType<Arg, Result> {
    Result (*ptr)(Arg);
public:
    explicit Xptr_to_unary_function(Result (*x)(Arg)) : ptr(x) {}
    Result operator()(Arg x) const { return ptr(x); }
};

template <class Arg1, class Arg2, class Result>
class Xptr_to_binary_function : public CFunType<Arg1, Arg2, Result> {
    Result (*ptr)(Arg1, Arg2);
public:
    explicit Xptr_to_binary_function(Result (*x)(Arg1, Arg2)) : ptr(x) {}
    Result operator()(Arg1 x, Arg2 y) const { return ptr(x, y); }
};

template <class Arg1, class Arg2, class Arg3, class Result>
class Xptr_to_ternary_function : public CFunType<Arg1, Arg2, Arg3, Result> {
    Result (*ptr)(Arg1, Arg2, Arg3);
public:
    explicit Xptr_to_ternary_function(Result (*x)(Arg1, Arg2, Arg3)) : ptr(x) {}
    Result operator()(Arg1 x, Arg2 y, Arg3 z) const { return ptr(x,y,z); }
};

//////////////////////////////////////////////////////////////////////
// Turn member functions into normal functions which take a Receiver*
// (or a smart pointer) as their first (extra) argument.  Note that we 
// disallow reference parameters.
//////////////////////////////////////////////////////////////////////

template <class Arg1, class Arg2, class Arg3, class Result>
class Xptr_to_mem_binary_function {
    Result (Arg1::*ptr)(Arg2,Arg3);
public:
    explicit Xptr_to_mem_binary_function(Result (Arg1::*x)(Arg2,Arg3)) 
       : ptr(x) {}
    template <class P, class Y, class Z> 
    struct Sig : public FunType<P,Arg2,Arg3,Result> {};
    template <class P> 
    Result operator()(const P& x, const Arg2& y, const Arg3& z) const 
       //{ return (x->*ptr)(y,z); }
       { return ((*x).*ptr)(y,z); }
};

template <class Arg1, class Arg2, class Arg3, class Result>
class Xptr_to_const_mem_binary_function {
    Result (Arg1::*ptr)(Arg2,Arg3) const;
public:
    explicit Xptr_to_const_mem_binary_function(
          Result (Arg1::*x)(Arg2,Arg3) const) : ptr(x) {}
    template <class P, class Y, class Z> 
    struct Sig : public FunType<P,Arg2,Arg3,Result> {};
    template <class P> 
    Result operator()(const P& x, const Arg2& y, const Arg3& z) const 
       //{ return (x->*ptr)(y,z); }
       { return ((*x).*ptr)(y,z); }
};

template <class Arg1, class Arg2, class Result>
class Xptr_to_mem_unary_function {
    Result (Arg1::*ptr)(Arg2);
public:
    explicit Xptr_to_mem_unary_function(Result (Arg1::*x)(Arg2)) : ptr(x) {}
    template <class P, class Y> 
    struct Sig : public FunType<P,Arg2,Result> {};
    template <class P> 
    Result operator()(const P& x, const Arg2& y) const 
    //{ return (x->*ptr)(y); }
    { return ((*x).*ptr)(y); }
};

template <class Arg1, class Arg2, class Result>
class Xptr_to_const_mem_unary_function {
    Result (Arg1::*ptr)(Arg2) const;
public:
    explicit Xptr_to_const_mem_unary_function(Result (Arg1::*x)(Arg2) const) 
       : ptr(x) {}
    template <class P, class Y> 
    struct Sig : public FunType<P,Arg2,Result> {};
    template <class P> 
    Result operator()(const P& x, const Arg2& y) const 
       //{ return (x->*ptr)(y); }
       { return ((*x).*ptr)(y); }
};

template <class Arg1, class Result>
class Xptr_to_mem_nullary_function {
    Result (Arg1::*ptr)();
public:
    explicit Xptr_to_mem_nullary_function(Result (Arg1::*x)()) : ptr(x) {}
    template <class P> 
    struct Sig : public FunType<P,Result> {};
    template <class P>
    //Result operator()(const P& x) const { return (x->*ptr)(); }
    Result operator()(const P& x) const { return ((*x).*ptr)(); }
};

template <class Arg1, class Result>
class Xptr_to_const_mem_nullary_function {
    Result (Arg1::*ptr)() const;
public:
    explicit Xptr_to_const_mem_nullary_function(Result (Arg1::*x)() const) 
       : ptr(x) {}
    template <class P> 
    struct Sig : public FunType<P,Result> {};
    template <class P>
    //Result operator()(const P& x) const { return (x->*ptr)(); }
    Result operator()(const P& x) const { return ((*x).*ptr)(); }
};

struct XPtrToFun {
   template <class P> struct Sig;

   // non-member functions
   template <class Result>
   struct Sig< Result (*)() > : public FunType< Result (*)(), 
      Full0<Xptr_to_nullary_function<Result> > > {};
   template <class A1, class Result>
   struct Sig< Result (*)(A1) > : public FunType< Result (*)(A1), 
      Full1<Xptr_to_unary_function<A1,Result> > > {};
   template <class A1, class A2, class Result>
   struct Sig< Result (*)(A1,A2) > : public FunType< Result (*)(A1,A2), 
      Full2<Xptr_to_binary_function<A1,A2,Result> > > {};
   template <class A1, class A2, class A3, class Result>
   struct Sig< Result (*)(A1,A2,A3) > : public FunType< Result (*)(A1,A2,A3), 
      Full3<Xptr_to_ternary_function<A1,A2,A3,Result> > > {};

   // member functions
   template <class A1, class A2, class A3, class Result>
   struct Sig< Result (A1::*)(A2,A3) > : public FunType<
      Result (A1::*)(A2,A3),
      Full3<Xptr_to_mem_binary_function<A1, A2, A3, Result> > > {};
   template <class A1, class A2, class A3, class Result>
   struct Sig< Result (A1::*)(A2,A3) const > : public FunType<
      Result (A1::*)(A2,A3) const,
      Full3<Xptr_to_const_mem_binary_function<A1, A2, A3, Result> > > {};
   template <class A1, class A2, class Result>
   struct Sig< Result (A1::*)(A2) > : public FunType<
      Result (A1::*)(A2),
      Full2<Xptr_to_mem_unary_function<A1, A2, Result> > > {};
   template <class A1, class A2, class Result>
   struct Sig< Result (A1::*)(A2) const > : public FunType<
      Result (A1::*)(A2) const,
      Full2<Xptr_to_const_mem_unary_function<A1, A2, Result> > > {};
   template <class A1, class Result>
   struct Sig< Result (A1::*)() > : public FunType<
      Result (A1::*)(),
      Full1<Xptr_to_mem_nullary_function<A1, Result> > > {};
   template <class A1, class Result>
   struct Sig< Result (A1::*)() const > : public FunType<
      Result (A1::*)() const,
      Full1<Xptr_to_const_mem_nullary_function<A1, Result> > > {};

   // non-member functions
   template <class Result>
   inline Full0<Xptr_to_nullary_function<Result> >
   operator()(Result (*x)()) const {
     return makeFull0( Xptr_to_nullary_function<Result>(x) );
   }
   template <class A, class Result>
   inline Full1<Xptr_to_unary_function<A, Result> >
   operator()(Result (*x)(A)) const {
     return makeFull1( Xptr_to_unary_function<A, Result>(x) );
   }
   template <class A1, class A2, class Result>
   inline Full2<Xptr_to_binary_function<A1, A2, Result> >
   operator()(Result (*x)(A1, A2)) const {
     return makeFull2( Xptr_to_binary_function<A1, A2, Result>(x) );
   }
   template <class A1, class A2, class A3, class Result>
   inline Full3<Xptr_to_ternary_function<A1, A2, A3, Result> >
   operator()(Result (*x)(A1, A2, A3)) const {
     return makeFull3( Xptr_to_ternary_function<A1,A2,A3,Result>(x) );
   }

   // member functions
   template <class A1, class A2, class A3, class Result>
   inline Full3<Xptr_to_mem_binary_function<A1, A2, A3, Result> >
   operator()(Result (A1::*x)(A2,A3)) const {
     return makeFull3(
               Xptr_to_mem_binary_function<A1, A2, A3, Result>(x) );
   }
   template <class A1, class A2, class A3, class Result>
   inline Full3<Xptr_to_const_mem_binary_function<A1, A2, A3, Result> >
   operator()(Result (A1::*x)(A2,A3) const) const {
     return makeFull3(
        Xptr_to_const_mem_binary_function<A1, A2, A3, Result>(x) );
   }
   template <class A1, class A2, class Result>
   inline Full2<Xptr_to_mem_unary_function<A1, A2, Result> >
   operator()(Result (A1::*x)(A2)) const {
     return makeFull2( Xptr_to_mem_unary_function<A1, A2, Result>(x) );
   }
   template <class A1, class A2, class Result>
   inline Full2<Xptr_to_const_mem_unary_function<A1, A2, Result> >
   operator()(Result (A1::*x)(A2) const) const {
     return makeFull2( 
       Xptr_to_const_mem_unary_function<A1, A2, Result>(x) );
   }
   template <class A1, class Result>
   inline Full1<Xptr_to_mem_nullary_function<A1, Result> >
   operator()(Result (A1::*x)()) const {
     return makeFull1( Xptr_to_mem_nullary_function<A1, Result>(x) );
   }
   template <class A1, class Result>
   inline Full1<Xptr_to_const_mem_nullary_function<A1, Result> >
   operator()(Result (A1::*x)() const) const {
     return makeFull1( Xptr_to_const_mem_nullary_function<A1, Result>(x) );
   }
};
   
}  // end namespace impl
typedef Full1<impl::XPtrToFun> PtrToFun;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN PtrToFun ptr_to_fun;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// funify is an identity for FullNs, but calls ptr_to_fun otherwise
//////////////////////////////////////////////////////////////////////

namespace impl {
struct XFunify {
   template <class P> struct Sig : public FunType<P,
      typename RT<PtrToFun,P>::ResultType> {};
   template <class F> struct Sig< Full0<F> > 
   : public FunType< Full0<F>, Full0<F> > {};
   template <class F> struct Sig< Full1<F> > 
   : public FunType< Full1<F>, Full1<F> > {};
   template <class F> struct Sig< Full2<F> > 
   : public FunType< Full2<F>, Full2<F> > {};
   template <class F> struct Sig< Full3<F> > 
   : public FunType< Full3<F>, Full3<F> > {};

   template <class P>
   typename Sig<P>::ResultType
   operator()( const P& p ) const { return ptr_to_fun(p); }
   template <class F>
   typename Sig<Full0<F> >::ResultType
   operator()( const Full0<F>& f ) const { return f; }
   template <class F>
   typename Sig<Full1<F> >::ResultType
   operator()( const Full1<F>& f ) const { return f; }
   template <class F>
   typename Sig<Full2<F> >::ResultType
   operator()( const Full2<F>& f ) const { return f; }
   template <class F>
   typename Sig<Full3<F> >::ResultType
   operator()( const Full3<F>& f ) const { return f; }
};
}
typedef Full1<impl::XFunify> Funify;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Funify funify;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// Misc stuff
//////////////////////////////////////////////////////////////////////
// FIX THIS: add dec?  pre/post versions?

namespace impl {
struct XInc {
    template <class T>
    struct Sig : public FunType<T,T> {};

    template <class T>
    T operator()(const T& x) const { T y = x; return ++y; }
};
}
typedef Full1<impl::XInc> Inc;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Inc inc;
FCPP_MAYBE_NAMESPACE_CLOSE

// These are obsolete; ignore(const_(true)) and ignore(const_(false)) 
// do the same thing.  Hurray for combinators!
namespace impl {
struct XAlways1 {
    template <class T>
    struct Sig : public FunType<T,bool> {};

    template <class T>
    bool operator()(const T&) const { return true; }
};
}
typedef Full1<impl::XAlways1> Always1;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Always1 always1;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XNever1 {
    template <class T>
    struct Sig : public FunType<T,bool> {};

    template <class T>
    bool operator()(const T&) const { return false; }
};
}
typedef Full1<impl::XNever1> Never1;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Never1 never1;
FCPP_MAYBE_NAMESPACE_CLOSE

} // end namespace fcpp

#endif
