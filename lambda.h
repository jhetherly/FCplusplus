//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

/*
List of things I know I can do better...
 - being more careful about LEs and 'wrapped' LEs (LambdaExp<LE>), so
   that I don't end up unnecessarily wrapping getses, etc.  Indeed,
   design so that comprehension commas make sense...
 - a way to ask for a fresh variable
 - have any LE which is sans-free-vars be a lambda--that is, callable
   via no-arg op().  this will make "compM<M>[ yadda ]()" work, yay!
    - this looks like a bit of work, and will need new LETypes like 
      COMPX or something, so I've decided not to do it

syntax summary:
   lambda(X,Y)[ LE ] //(a,b)
   let[ X==f[a], Y==g[X,b] ].in[ LExy ] //()     
   letrec[ x==f[x,y], y==g[x,y] ].in[ LExy ] //()
   compM<M>()[ LE | LE, x<=LE, guard[LE] ] //()
   doM[ LE, x<=LE, LE ] //()
   if0,if1,if2

Everything centers around LEs; whether LEs have free vars can be key to
whether they're callable; explicit lambdas are a special case of
callable LEs where certain bindings on params happen.
*/

#ifndef FCPP_LAMBDA_DOT_H
#define FCPP_LAMBDA_DOT_H

#include "full.h"

#ifdef FCPP_ENABLE_LAMBDA
namespace fcpp {
namespace fcpp_lambda {

//////////////////////////////////////////////////////////////////////
// Literate error messages look nicer when emitted as compiler
// diagnostics when they're not deeply nested inside classes, so we move
// them all out to the 'top' level of this namespace here.
//////////////////////////////////////////////////////////////////////
   template <class Actual, class Given, bool b>
   struct TheActualTypeOfTheLambdaExpressionIsNotConvertibleToItsGivenType;
   template <class Actual, class Given>
   struct TheActualTypeOfTheLambdaExpressionIsNotConvertibleToItsGivenType
   <Actual,Given,true> { typedef int Error; };

   template <class FreeVars>
   struct YouCannotInvokeALambdaContainingFreeVars {};

   template <class Dummy, bool b>
   struct TheTypeSpecYouHaveGivenIsIncompatibleWithItsLE;
   template <class Dummy>
   struct TheTypeSpecYouHaveGivenIsIncompatibleWithItsLE<Dummy,true>
   { typedef int Error; };

   template <class EE, bool b> struct IfExpressionMustHaveTypeConvertibleToBool;
   template <class EE> 
   struct IfExpressionMustHaveTypeConvertibleToBool<EE,true> 
   { typedef EE Type; };

   template <class TT, class FF, bool b> struct
   ActualTypeOfFalseBranchMustBeImplicitlyConvertibleToTypeOfTrueBranch;
   template <class TT, class FF> struct
   ActualTypeOfFalseBranchMustBeImplicitlyConvertibleToTypeOfTrueBranch
   <TT,FF,true> { typedef TT Error; };

   template <class FF, class TT, bool b> struct
   ActualTypeOfTrueBranchMustBeImplicitlyConvertibleToTypeOfFalseBranch;
   template <class FF, class TT> struct
   ActualTypeOfTrueBranchMustBeImplicitlyConvertibleToTypeOfFalseBranch
   <FF,TT,true> { typedef FF Error; };

   template <class TT, class FF> struct TrueAndFalseBranchOfIfMustHaveSameType;
   template <class TT> struct TrueAndFalseBranchOfIfMustHaveSameType<TT,TT>
   { typedef TT Type; };

   template <bool b> 
   struct YouCannotPassTheSameLambdaVarTo_lambda_MoreThanOnce;
   template <> 
   struct YouCannotPassTheSameLambdaVarTo_lambda_MoreThanOnce<false>
   { static inline void go() {} };

//////////////////////////////////////////////////////////////////////
// Useful helpers
//////////////////////////////////////////////////////////////////////

// some quick forward decls
namespace exp { 
   template <int i> class LambdaVar; 
   template <class T> class Value; 
}
using exp::LambdaVar;

template <class T>
struct ThunkifyType {
   struct Result {
      template <class Dummy> struct Go {
         typedef T Type;
      };
   };
};

//////////////////////////////////////////////////////////////////////
// Environment stuff
//////////////////////////////////////////////////////////////////////

// I seem to recall that
//   ET - Environment Thunk
//   BE - Binding Environment
//   TE - Type Environment
// Yes.  See pre_lambda.h for a little more explanation.

struct NIL_ET {
   template <class Dummy> struct Go {
      typedef NIL TE;
      typedef NIL BE;
   };
};

template <class OldET, class LVList>
struct EraseLVsFromET {
   struct Result {
      template <class Dummy> struct Go {
         typedef typename OldET::template Go<Dummy>::TE TE1;
         typedef typename OldET::template Go<Dummy>::BE BE1;

         struct BPred {
            template <class DumDum, class BEP> struct Go;
            template <class D, int i, class LE> struct Go<D,BEPair<i,LE> > { 
               static const bool value = 
                  !(Contains<LVList,LambdaVar<i> >::value);
            };
         };
         typedef Filter<BPred,BE1> BF;
         typedef typename BF::Result BE;
         static inline BE go( const BE1& x ) { return BF::go(x); }
   
         struct TPred {
            template <class DumDum, class TEP> struct Go;
            template <class D, int i, class T> struct Go<D,TEPair<i,T> > { 
               static const bool value = 
                  !(Contains<LVList,LambdaVar<i> >::value);
            };
         };
         typedef Filter<TPred,TE1> TF;
         typedef typename TF::Result TE;
      };
   };
};

template <class ET1, int i, class X>
struct ETUpdateX {
   struct Result {
      template <class Dummy> struct Go {
         typedef CONS<LambdaVar<i>,NIL> BoundVars;
         typedef typename ET1::template Go<Dummy>::BE BE1;
         typedef typename EraseLVsFromET<ET1,BoundVars>::Result ET2;
         typedef typename ET2::template Go<Dummy>::TE TE2;
         typedef typename ET2::template Go<Dummy>::BE BE2;
         typedef typename exp::Value<X> LEX; typedef BEPair<i,LEX> BEX;
         typedef CONS<BEX,BE2> BE;
         typedef typename ThunkifyType<X>::Result XTT;
         typedef TEPair<i,XTT> TEX;
         typedef CONS<TEX,TE2> TE;
         static inline BE go( const BE1& be, const X& x ) {
            return BE( BEX(LEX(x)), ET2::template Go<Dummy>::go(be) );
         }
      };
   };
};

template <class ET1, int i, class X, int j, class Y>
struct ETUpdateXY {
   struct Result {
      template <class Dummy> struct Go {
         typedef CONS<LambdaVar<i>,CONS<LambdaVar<j>,NIL> > BoundVars;
         typedef typename ET1::template Go<Dummy>::BE BE1;
         typedef typename EraseLVsFromET<ET1,BoundVars>::Result ET2;
         typedef typename ET2::template Go<Dummy>::TE TE2;
         typedef typename ET2::template Go<Dummy>::BE BE2;
         typedef typename exp::Value<X> LEX; typedef BEPair<i,LEX> BEX;
         typedef typename exp::Value<Y> LEY; typedef BEPair<j,LEY> BEY;
         typedef CONS<BEY,BE2> YBE;
         typedef CONS<BEX,YBE> BE;
         typedef typename ThunkifyType<X>::Result XTT;
         typedef TEPair<i,XTT> TEX;
         typedef typename ThunkifyType<Y>::Result YTT;
         typedef TEPair<j,YTT> TEY;
         typedef CONS<TEX,CONS<TEY,TE2> > TE;
         static inline BE go( const BE1& be, const X& x, const Y& y ) {
            return BE( BEX(LEX(x)), YBE( BEY(LEY(y)), 
                       ET2::template Go<Dummy>::go(be) ) );
         }
      };
   };
};

template <class ET1, int i, class X, int j, class Y, int k, class Z>
struct ETUpdateXYZ {
   struct Result {
      template <class Dummy> struct Go {
         typedef CONS<LambdaVar<i>,CONS<LambdaVar<j>,
                    CONS<LambdaVar<k>,NIL> > > BoundVars;
         typedef typename ET1::template Go<Dummy>::BE BE1;
         typedef typename EraseLVsFromET<ET1,BoundVars>::Result ET2;
         typedef typename ET2::template Go<Dummy>::TE TE2;
         typedef typename ET2::template Go<Dummy>::BE BE2;
         typedef typename exp::Value<X> LEX; typedef BEPair<i,LEX> BEX;
         typedef typename exp::Value<Y> LEY; typedef BEPair<j,LEY> BEY;
         typedef typename exp::Value<Z> LEZ; typedef BEPair<k,LEZ> BEZ;
         typedef CONS<BEZ,BE2> ZBE;
         typedef CONS<BEY,ZBE> YBE;
         typedef CONS<BEX,YBE> BE;
         typedef typename ThunkifyType<X>::Result XTT;
         typedef TEPair<i,XTT> TEX;
         typedef typename ThunkifyType<Y>::Result YTT;
         typedef TEPair<j,YTT> TEY;
         typedef typename ThunkifyType<Z>::Result ZTT;
         typedef TEPair<k,ZTT> TEZ;
         typedef CONS<TEX,CONS<TEY,CONS<TEZ,TE2> > > TE;
         static inline BE go( const BE1& be, const X& x, const Y& y,
                              const Z& z ) {
            return BE( BEX(LEX(x)), YBE( BEY(LEY(y)), ZBE( BEZ(LEZ(z)),
                       ET2::template Go<Dummy>::go(be) ) ) );
         }
      };
   };
};

//////////////////////////////////////////////////////////////////////
// The lambda expression types, all defined right together here
// in one big batch
//////////////////////////////////////////////////////////////////////

namespace exp {

template <class X>
class LambdaExp : public LEBase {
   X x;
public:
   LambdaExp( const X& xx ) : x(xx) { EnsureLE<X>::go(); }

   typedef typename X::FreeVars FreeVars;
   template <class EnvThunk> struct MyType {
      typedef typename X::template MyType<EnvThunk>::TypeThunk TypeThunk;
   };
   template <class EnvThunk> struct RecheckType {
      typedef typename X::template RecheckType<EnvThunk>::Ok Ok;
   };

   template <class EnvThunk>
   typename MyType<EnvThunk>::TypeThunk::template Go<int>::Type 
   eval( const typename EnvThunk::template Go<int>::BE& be ) const {
      return x.template eval<EnvThunk>( be );
   }

   typedef LambdaExp This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

template <class T>
class Value : public LEBase {
   T data;
public:
   Value( const T& x ) : data(x) {}

   typedef NIL FreeVars;
   template <class EnvThunk> struct MyType { 
      struct TypeThunk {
         template <class Dummy> struct Go {
            typedef T Type; 
         };
      };
   };
   template <class EnvThunk> struct RecheckType { typedef int Ok; };
   template <class EnvThunk>
   typename MyType<EnvThunk>::TypeThunk::template Go<int>::Type 
   eval( const typename EnvThunk::template Go<int>::BE& ) const
   { return data; }

   typedef Value This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

template <int i>
class LambdaVar : public LEBase {
   // 'ii' is used to get around what appears to be a g++ bug...
public:
   template <class Dummy, int ii, class TE> struct Lookup;
   template <class D, int ii, class T, class Rest> 
   struct Lookup<D,ii,CONS<TEPair<ii,T>,Rest> >
   { typedef typename T::template Go<int>::Type Type; };
   template <class D, int ii, class TEP, class Rest> 
   struct Lookup<D,ii,CONS<TEP,Rest> >
   { typedef typename Lookup<D,ii,Rest>::Type Type; };

private:
   template <class Dummy, int ii, class Result, class EnvThunk, 
             class BE, class WBE> struct Find;
   template <class D, int ii, class R, class ET, class LE, class Rest, 
             class WBE>
   struct Find<D,ii,R,ET,CONS<BEPair<ii,LE>,Rest>,WBE> { 
      static inline R go( const CONS<BEPair<ii,LE>,Rest>& be,
                          const WBE& wbe ) 
      { return be.head.value.template eval<ET>(wbe); }
   };
   template <class D, int ii, class R, class ET, class BEP, 
             class Rest, class WBE> 
   struct Find<D,ii,R,ET,CONS<BEP,Rest>,WBE> { 
      static inline R go( const CONS<BEP,Rest>& be, const WBE& wbe ) {
         return Find<D,ii,R,ET,Rest,WBE>::go( be.tail, wbe );
      }
   };
public:
   typedef CONS<LambdaVar,NIL> FreeVars;
   template <class EnvThunk> struct MyType { 
      struct TypeThunk {
         template <class Dummy> struct Go {
            typedef typename EnvThunk::template Go<Dummy>::TE TE;
            typedef typename Lookup<int,i,TE>::Type Type;
         };
      };
   };
   template <class EnvThunk> struct RecheckType { typedef int Ok; };
   template <class EnvThunk>
   typename MyType<EnvThunk>::TypeThunk::template Go<int>::Type 
   eval( const typename EnvThunk::template Go<int>::BE& be ) const {
      typedef 
         typename MyType<EnvThunk>::TypeThunk::template Go<int>::Type Result;
      typedef typename EnvThunk::template Go<int>::BE BE;
      return Find<int,i,Result,EnvThunk,BE,BE>::go(be,be);
   }

   typedef LambdaVar This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

template <class Fun, class Args>
class Call : public LEBase {
   Fun fun;  Args args;
public:  
   template <class ET, class F, class A> struct TypeHelper;
   template <class ET, class F> 
   struct TypeHelper<ET,F,NIL> {
      typedef typename RT<F>::ResultType Type;
   };
   template <class ET, class F, class A1> 
   struct TypeHelper<ET,F,CONS<A1,NIL> > {
      typedef typename A1::template MyType<ET>::TypeThunk XTT;
      typedef typename XTT::template Go<int>::Type X;
      typedef typename RT<F,X>::ResultType Type;
   };
   template <class ET, class F, class A1, class A2> 
   struct TypeHelper<ET,F,CONS<A2,CONS<A1,NIL> > > {
      typedef typename A1::template MyType<ET>::TypeThunk XTT;
      typedef typename XTT::template Go<int>::Type X;
      typedef typename A2::template MyType<ET>::TypeThunk YTT;
      typedef typename YTT::template Go<int>::Type Y;
      typedef typename RT<F,X,Y>::ResultType Type;
   };
   template <class ET, class F, class A1, class A2, class A3> 
   struct TypeHelper<ET,F,CONS<A3,CONS<A2,CONS<A1,NIL> > > > {
      typedef typename A1::template MyType<ET>::TypeThunk XTT;
      typedef typename XTT::template Go<int>::Type X;
      typedef typename A2::template MyType<ET>::TypeThunk YTT;
      typedef typename YTT::template Go<int>::Type Y;
      typedef typename A3::template MyType<ET>::TypeThunk ZTT;
      typedef typename ZTT::template Go<int>::Type Z;
      typedef typename RT<F,X,Y,Z>::ResultType Type;
   };

   template <class ET, class LEL> struct CheckHelper;
   template <class ET> 
   struct CheckHelper<ET,NIL> { typedef int Result; };
   template <class ET, class H, class T> 
   struct CheckHelper<ET,CONS<H,T> > { 
      typedef typename AlwaysFirst<typename H::template RecheckType<ET>::Ok,
         typename CheckHelper<ET,T>::Result>::Type Result; 
   };

   template <class Result, class ET, class BE, class Args_> struct EvalHelp;
   template <class R, class ET, class BE> 
   struct EvalHelp<R,ET,BE,NIL> {
      static inline R go( const Fun& f, const NIL&, const BE& be ) {
         return (f.template eval<ET>(be))();
      }
   };
   template <class R, class ET, class BE, class A1> 
   struct EvalHelp<R,ET,BE,CONS<A1,NIL> > {
      static inline R go( const Fun& f, const CONS<A1,NIL>& a, const BE& be ) {
         return (f.template eval<ET>(be))(a.head.template eval<ET>(be));
      }
   };
   template <class R, class ET, class BE, class A1, class A2> 
   struct EvalHelp<R,ET,BE,CONS<A2,CONS<A1,NIL> > > {
      static inline R go( const Fun& f, const CONS<A2,CONS<A1,NIL> >& a, 
                          const BE& be ) {
         return (f.template eval<ET>(be))(
            a.tail.head.template eval<ET>(be),
            a.head.template eval<ET>(be));
      }
   };
   template <class R, class ET, class BE, class A1, class A2, class A3> 
   struct EvalHelp<R,ET,BE,CONS<A3,CONS<A2,CONS<A1,NIL> > > > {
      static inline R go( const Fun& f, 
                          const CONS<A3,CONS<A2,CONS<A1,NIL> > >& a, 
                          const BE& be ) {
         return (f.template eval<ET>(be))(
            a.tail.tail.head.template eval<ET>(be),
            a.tail.head.template eval<ET>(be),
            a.head.template eval<ET>(be));
      }
   };
   
   struct FoldrOp {
      template <class LE, class FVList> struct Go {
         typedef typename AppendList<typename LE::FreeVars,FVList>::Result 
            Result;
      };
   };
   typedef typename Foldr<FoldrOp,NIL,CONS<Fun,Args> >::Result
      AccumulatedFreeVars;

   Call( const Fun& f, const Args& a ) : fun(f), args(a) 
   { EnsureLE<Fun>::go(); EnsureLEList<Args>::go(); }

   typedef typename RemoveDuplicates<AccumulatedFreeVars>::Result FreeVars;
   template <class EnvThunk> struct MyType { 
      struct TypeThunk {
         template <class Dummy> struct Go {
            typedef typename Fun::template MyType<EnvThunk>::TypeThunk FTT;
            typedef typename FTT::template Go<Dummy>::Type F;
            typedef typename TypeHelper<EnvThunk,F,Args>::Type Type; 
         };
      };
   };
   template <class EnvThunk> struct RecheckType 
   { typedef typename CheckHelper<EnvThunk,CONS<Fun,Args> >::Result Ok; };
   template <class ET>
   typename MyType<ET>::TypeThunk::template Go<int>::Type 
   eval( const typename ET::template Go<int>::BE& be ) const {
      typedef typename ET::template Go<int>::BE BE;
      typedef typename MyType<ET>::TypeThunk::template Go<int>::Type Result;
      return EvalHelp<Result,ET,BE,Args>::go(fun,args,be);
   }

   typedef Call This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

// This is a weird special class only BindingEnvExp below uses...
template <class RealET, class LE, class FV, class BE1, class B>
class AddEnvLE : public LEBase {
   LE exp;
   BE1 be1;
   B b;
public:
   AddEnvLE( const LE& e, const BE1& x, const B& y ) : exp(e), be1(x), b(y) {}

   typedef FV FreeVars;

   template <class EnvThunk> struct MyType { 
      struct TypeThunk {
         template <class Dummy> struct Go {
            typedef typename LE::template MyType<RealET>::TypeThunk LETT;
            typedef typename LETT::template Go<Dummy>::Type Type; 
         };
      };
   };
   template <class EnvThunk> struct RecheckType { 
      typedef typename LE::template RecheckType<RealET>::Ok Ok; 
   };
   template <class EnvThunk>
   typename MyType<RealET>::TypeThunk::template Go<int>::Type 
   eval( const typename EnvThunk::template Go<int>::BE& ) const { 
      return exp.template eval<RealET>( RealET::template Go<int>::go(be1,b) );
   }

   typedef AddEnvLE This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

template <class B, class LE>
class BindingEnvExp : public LEBase {    // letrec semantics
   B binders;
   LE exp;

   template <class Dummy, class BinderList> struct AccumFree;
   template <class D> struct AccumFree<D,NIL> { typedef NIL Result; };
   template <class D, int i, class LEa, class Rest> 
   struct AccumFree<D,CONS<Binder<i,LEa>,Rest> >
   { typedef typename AppendList<typename LEa::FreeVars,
        typename AccumFree<D,Rest>::Result>::Result Result; };
   
   template <class Dummy, class BinderList> struct AccumBound;
   template <class D> struct AccumBound<D,NIL> { typedef NIL Result; };
   template <class D, int i, class LEa, class Rest> 
   struct AccumBound<D,CONS<Binder<i,LEa>,Rest> >
   { typedef CONS<LambdaVar<i>,typename AccumBound<D,Rest>::Result> Result; };
   
public:
   typedef typename AccumBound<int,B>::Result NewlyBoundVars;
   typedef typename AccumFree<int,B>::Result FreeVarsInBinders;

   // In order to compute the environment for "exp", we must follow
   // these steps:
   //  - Call the outer env E1
   //  - foreach lv in lhs of binders, erase lv-entries from E1 => E2
   //  - foreach binder in binders, add <binder.lhs,AddEnvLE<E3>(binder.rhs)> 
   //    BE/TE pair to E2 => E3
   // Now exp should be evaled in E3.

   template <class Inner, class ET3>
   struct UnusualTTHelper {
      template <class DumDum> struct Go {
         typedef typename Inner::template MyType<ET3>::TypeThunk ITT;
         typedef typename ITT::template Go<DumDum>::Type Type;
      };
   };

   // Note the trick; we take the very result we are computing (ET3) as
   // a template parameter!  Ha ha!  Darn I'm clever.  :)
   template <class ET2, class ET3, class BinderList, class NBV, 
             class BE1, class BB> struct Env3FromEnv2;
   template <class ET2, class ET3, class NBV, class BE1, class BB> 
   struct Env3FromEnv2<ET2,ET3,NIL,NBV,BE1,BB> {
      struct Result {
         template <class Dummy> struct Go {
            typedef typename ET2::template Go<Dummy>::TE TE;
            typedef typename ET2::template Go<Dummy>::BE BE;
            static inline BE go( const BE& x, const NIL&, const BE1&,
                                 const BB& ) { return x; }
         };
      };
   };
   template <class ET2, class ET3, int i, class LEa, class Rest, class NBV, 
             class BE1, class BB> 
   struct Env3FromEnv2<ET2,ET3,CONS<Binder<i,LEa>,Rest>,NBV,BE1,BB> {
      struct Result {
         template <class Dummy> struct Go {
            typedef typename ET2::template Go<Dummy>::BE BE2;
            typedef typename 
               Env3FromEnv2<ET2,ET3,Rest,NBV,BE1,BB>::Result Recurse;
            typedef typename 
               ListDifference<typename LEa::FreeVars,NBV>::Result LEFV;
            // Inner will capture the letrec environment (by value)
            typedef AddEnvLE<ET3,LEa,LEFV,BE1,BB> Inner;
            typedef BEPair<i,Inner> BEP;
            typedef CONS<BEP,typename Recurse::template Go<Dummy>::BE> BE;
            static inline BE go( const BE2& be, 
                                 const CONS<Binder<i,LEa>,Rest>& binders,
                                 const BE1& be1, const BB& b ) { 
               return BE( BEP(Inner(binders.head.exp,be1,b)),
                  Recurse::template Go<Dummy>::go(be,binders.tail,be1,b) );
            }
            typedef UnusualTTHelper<Inner,ET3> TT;
            typedef TEPair<i,TT> TEP;
            typedef CONS<TEP,typename Recurse::template Go<Dummy>::TE> TE;
         };
      };
   };

   template <class OrigET>
   struct MakeNewET {
      struct Result {
         template <class Dummy> struct Go {
            typedef typename OrigET::template Go<Dummy>::BE BE1;
            typedef typename EraseLVsFromET<OrigET,NewlyBoundVars>::Result E2;
            // Here is the trick to tie the recursive knot:
            typedef typename 
               Env3FromEnv2<E2,Result,B,NewlyBoundVars,BE1,B>::Result E3;
            typedef typename OrigET::template Go<Dummy>::BE BE1;
            typedef typename E3::template Go<Dummy>::BE BE;
            typedef typename E3::template Go<Dummy>::TE TE;
            static inline BE go( const BE1& be, const B& binders ) { 
               return E3::template Go<Dummy>::go( 
                  E2::template Go<Dummy>::go(be), binders, be, binders );
            }
         };
      };
   };
      
private:
   template <class NewET, class BinderList> struct CheckHelp;
   template <class ET> struct CheckHelp<ET,NIL> { typedef int Result; };
   template <class ET, int i, class LEa, class Rest> 
   struct CheckHelp<ET,CONS<Binder<i,LEa>,Rest> > { 
      typedef typename LEa::template RecheckType<ET>::Ok ThisOne;
      typedef typename AlwaysFirst<ThisOne,
         typename CheckHelp<ET,Rest>::Result>::Type Result;
   };
public:
   BindingEnvExp( const B& g, const LE& e ) : binders(g), exp(e) 
   { EnsureBinderList<B>::go(); EnsureLE<LE>::go(); }

   typedef typename RemoveDuplicates<typename ListDifference<
      typename AppendList<FreeVarsInBinders,typename LE::FreeVars>::Result,
      NewlyBoundVars>::Result>::Result FreeVars;

   template <class ET> struct MyType { 
      typedef typename LE::template MyType<typename
         MakeNewET<ET>::Result>::TypeThunk TypeThunk;
   };

   template <class ET> struct RecheckType { 
      typedef typename MakeNewET<ET>::Result NET;
      typedef typename AlwaysFirst<
         typename LE::template RecheckType<NET>::Ok,
         typename CheckHelp<NET,B>::Result>::Type Ok; 
   };

   template <class EnvThunk>
   typename MyType<EnvThunk>::TypeThunk::template Go<int>::Type 
   eval( const typename EnvThunk::template Go<int>::BE& be ) const {
      typedef typename MakeNewET<EnvThunk>::Result NET;
      return exp.template eval<NET>( NET::template Go<int>::go(be,binders) );
   }

   typedef BindingEnvExp This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

template <class LE, class TBBV>  // To-Be-Bound Vars, in correct order
class LambdaExpWithFreeVars : public LEBase {
   // This class's name isn't great; nearly every kind of lambda
   // expression has free variables. Specifically, this class is about
   // such LEs which are created like so:
   //    lambda(X)[ f[X,Y] ]
   // LEs which, when evaluated, will result in a functoid which
   // captures a particular outer binding environment.

   LE exp;
public:
   template <class LEa, class ET>
   class Lambda0WithFreeVars : public CFunType<typename LEa::template 
            MyType<ET>::TypeThunk::template Go<int>::Type> {
      typedef typename ET::template Go<int>::BE BE;
      LEa exp;
      BE env;
   public:
      Lambda0WithFreeVars( const LEa& e, const BE& be ) : exp(e), env(be) {}
       
      typename AlwaysFirst<typename LEa::template MyType<ET>::TypeThunk
         ::template Go<int>::Type, typename LEa::template 
         RecheckType<ET>::Ok>::Type
      operator()() const {
         return exp.template eval<ET>( env );
      }
   };

   template <class LEa, class ET, int i>
   class Lambda1WithFreeVars {
      typedef typename ET::template Go<int>::BE BE;
      LEa exp;
      BE env;
      template <class X> struct NewET {
         typedef typename ETUpdateX<ET,i,X>::Result Result;
      };
   public:
      Lambda1WithFreeVars( const LEa& e, const BE& be ) : exp(e), env(be) {}
      template <class X> struct Sig : public FunType<X,
         typename LEa::template MyType<typename NewET<X>::Result>
         ::TypeThunk::template Go<int>::Type> {};
      template <class X>
      typename Sig<X>::ResultType
      operator()( const X& x ) const {
         return exp.template eval<typename NewET<X>::Result>
            ( NewET<X>::Result::template Go<int>::go( env, x ) );
      }
   };

   template <class LEa, class ET, int i, int j>
   class Lambda2WithFreeVars {
      typedef typename ET::template Go<int>::BE BE;
      LEa exp;
      BE env;
      template <class X, class Y> struct NewET {
         typedef typename ETUpdateXY<ET,i,X,j,Y>::Result Result;
      };
   public:
      Lambda2WithFreeVars( const LEa& e, const BE& be ) : exp(e), env(be) {}
      template <class X, class Y> struct Sig : public FunType<X,Y,
         typename LEa::template MyType<typename NewET<X,Y>::Result>
         ::TypeThunk::template Go<int>::Type> {};
      template <class X, class Y>
      typename Sig<X,Y>::ResultType
      operator()( const X& x, const Y& y ) const {
         return exp.template eval<typename NewET<X,Y>::Result>
            ( NewET<X,Y>::Result::template Go<int>::go( env, x, y ) );
      }
   };

   template <class LEa, class ET, int i, int j, int k>
   class Lambda3WithFreeVars {
      typedef typename ET::template Go<int>::BE BE;
      LEa exp;
      BE env;
      template <class X, class Y, class Z> struct NewET {
         typedef typename ETUpdateXYZ<ET,i,X,j,Y,k,Z>::Result Result;
      };
   public:
      Lambda3WithFreeVars( const LEa& e, const BE& be ) : exp(e), env(be) {}
      template <class X, class Y, class Z> struct Sig : public FunType<X,Y,Z,
         typename LEa::template MyType<typename NewET<X,Y,Z>::Result>
         ::TypeThunk::template Go<int>::Type> {};
      template <class X, class Y, class Z>
      typename Sig<X,Y,Z>::ResultType
      operator()( const X& x, const Y& y, const Z& z ) const {
         return exp.template eval<typename NewET<X,Y,Z>::Result>
            ( NewET<X,Y,Z>::Result::template Go<int>::go( env, x, y, z ) );
      }
   };

   template <class LEa, class ET, class TBBVars> 
   struct NumBoundVarsHelp;
   template <class LEa, class ET> 
   struct NumBoundVarsHelp<LEa,ET,NIL> {
      typedef Lambda0WithFreeVars<LEa,ET> Lam;
      typedef Full0<Lam> Full;
   };
   template <class LEa, class ET, int i> 
   struct NumBoundVarsHelp<LEa,ET,CONS<LambdaVar<i>,NIL> > {
      typedef Lambda1WithFreeVars<LEa,ET,i> Lam;
      typedef Full1<Lam> Full;
   };
   template <class LEa, class ET, int i, int j> 
   struct NumBoundVarsHelp<LEa,ET,CONS<LambdaVar<i>,CONS<LambdaVar<j>,
            NIL> > > {
      typedef Lambda2WithFreeVars<LEa,ET,i,j> Lam;
      typedef Full2<Lam> Full;
   };
   template <class LEa, class ET, int i, int j, int k> 
   struct NumBoundVarsHelp<LEa,ET,CONS<LambdaVar<i>,CONS<LambdaVar<j>,
            CONS<LambdaVar<k>,NIL> > > > {
      typedef Lambda3WithFreeVars<LEa,ET,i,j,k> Lam;
      typedef Full3<Lam> Full;
   };

   LambdaExpWithFreeVars( const LE& e ) : exp(e) { EnsureLE<LE>::go(); }

   typedef typename RemoveDuplicates<typename ListDifference<typename 
      LE::FreeVars,TBBV>::Result>::Result FreeVars;

   template <class ET> struct MyType { 
      typedef typename NumBoundVarsHelp<LE,ET,TBBV>::Full Full;
      typedef typename ThunkifyType<Full>::Result TypeThunk; 
   };

   template <class ET> struct RecheckType 
   { typedef typename LE::template RecheckType<ET>::Ok Ok; };

   template <class EnvThunk>
   typename MyType<EnvThunk>::TypeThunk::template Go<int>::Type 
   eval( const typename EnvThunk::template Go<int>::BE& be ) const {
      typedef NumBoundVarsHelp<LE,EnvThunk,TBBV> NBVH; 
      typedef typename NBVH::Lam Lam;
      typedef typename NBVH::Full Full;
      return Full( Lam(exp,be) );
   }

   YouCannotInvokeALambdaContainingFreeVars<FreeVars>
   operator()() const { return 0; }
   template <class X> 
   YouCannotInvokeALambdaContainingFreeVars<FreeVars>
   operator()(const X&) const { return 0; }
   template <class X, class Y> 
   YouCannotInvokeALambdaContainingFreeVars<FreeVars>
   operator()(const X&, const Y&) const { return 0; }
   template <class X, class Y, class Z> 
   YouCannotInvokeALambdaContainingFreeVars<FreeVars>
   operator()(const X&, const Y&, const Z&) const { return 0; }
 
   template <class AA=Void, class BB=Void, class CC=Void> struct Sig {
      typedef typename YouCannotInvokeALambdaContainingFreeVars<FreeVars>
         ::AndThusYouShouldNotBeTryingToUseItsSigEither Arg1Type;
      typedef typename YouCannotInvokeALambdaContainingFreeVars<FreeVars>
         ::AndThusYouShouldNotBeTryingToUseItsSigEither Arg2Type;
      typedef typename YouCannotInvokeALambdaContainingFreeVars<FreeVars>
         ::AndThusYouShouldNotBeTryingToUseItsSigEither Arg3Type;
      typedef typename YouCannotInvokeALambdaContainingFreeVars<FreeVars>
         ::AndThusYouShouldNotBeTryingToUseItsSigEither ResultType;
   };

   typedef LambdaExpWithFreeVars This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

template <class E, class T, class F, IfKind deduction_method>
class IfLE : public LEBase {
   E e;
   T t;
   F f;
public:
   template <class ET, IfKind deduct_method> struct XType;
   template <class ET, IfKind deduct_method> struct XRecheckType;

   // Normal if type deduction
   template <class ET> struct XType<ET,IfNormal> { 
      typedef typename T::template MyType<ET>::TypeThunk TypeThunk;
   };
   template <class ET> struct XRecheckType<ET,IfNormal> { 
      typedef typename E::template MyType<ET>::TypeThunk ETT; 
      typedef typename T::template MyType<ET>::TypeThunk TTT; 
      typedef typename F::template MyType<ET>::TypeThunk FTT; 
      typedef typename ETT::template Go<int>::Type EType;
      typedef typename TTT::template Go<int>::Type TType;
      typedef typename FTT::template Go<int>::Type FType;
      static const bool b = ImplicitlyConvertible<EType,bool>::value;
      typedef typename AlwaysFirst<typename AlwaysFirst<typename AlwaysFirst<
         typename AlwaysFirst<typename
         TrueAndFalseBranchOfIfMustHaveSameType<TType,FType>::Type, typename 
         IfExpressionMustHaveTypeConvertibleToBool<EType,b>::Type>::Type,
         typename E::template RecheckType<ET>::Ok>::Type,
         typename T::template RecheckType<ET>::Ok>::Type,
         typename F::template RecheckType<ET>::Ok>::Type Ok; 
   };

   // Type deduction based on true-branch
   template <class ET> struct XType<ET,IfTrue> { 
      typedef typename T::template MyType<ET>::TypeThunk TypeThunk;
   };
   template <class ET> struct XRecheckType<ET,IfTrue> { 
      typedef typename E::template MyType<ET>::TypeThunk ETT; 
      typedef typename T::template MyType<ET>::TypeThunk TTT; 
      typedef typename F::template MyType<ET>::TypeThunk FTT; 
      typedef typename ETT::template Go<int>::Type EType;
      typedef typename TTT::template Go<int>::Type TType;
      typedef typename FTT::template Go<int>::Type FType;
      static const bool bx = ImplicitlyConvertible<EType,bool>::value;
      typedef typename 
         IfExpressionMustHaveTypeConvertibleToBool<EType,bx>::Type Foo;
      static const bool b = ImplicitlyConvertible<FType,TType>::value;
      typedef typename
         ActualTypeOfFalseBranchMustBeImplicitlyConvertibleToTypeOfTrueBranch
         <FType,TType,b>::Error Tmp;
      typedef typename AlwaysFirst<typename AlwaysFirst<
         typename AlwaysFirst<typename AlwaysFirst<Tmp,Foo>::Type,
         typename E::template RecheckType<ET>::Ok>::Type,
         typename T::template RecheckType<ET>::Ok>::Type,
         typename F::template RecheckType<ET>::Ok>::Type Ok; 
   };

   // Type deduction based on false-branch
   template <class ET> struct XType<ET,IfFalse> { 
      typedef typename F::template MyType<ET>::TypeThunk TypeThunk;
   };
   template <class ET> struct XRecheckType<ET,IfFalse> { 
      typedef typename E::template MyType<ET>::TypeThunk ETT; 
      typedef typename T::template MyType<ET>::TypeThunk TTT; 
      typedef typename F::template MyType<ET>::TypeThunk FTT; 
      typedef typename ETT::template Go<int>::Type EType;
      typedef typename TTT::template Go<int>::Type TType;
      typedef typename FTT::template Go<int>::Type FType;
      static const bool bx = ImplicitlyConvertible<EType,bool>::value;
      typedef typename 
         IfExpressionMustHaveTypeConvertibleToBool<EType,bx>::Type Foo;
      static const bool b = ImplicitlyConvertible<TType,FType>::value;
      typedef typename
         ActualTypeOfTrueBranchMustBeImplicitlyConvertibleToTypeOfFalseBranch
         <TType,FType,b>::Error Tmp;
      typedef typename AlwaysFirst<typename AlwaysFirst<
         typename AlwaysFirst<typename AlwaysFirst<Tmp,Foo>::Type,
         typename E::template RecheckType<ET>::Ok>::Type,
         typename T::template RecheckType<ET>::Ok>::Type,
         typename F::template RecheckType<ET>::Ok>::Type Ok; 
   };

   IfLE( const E& ee, const T& tt, const F& ff ) : e(ee), t(tt), f(ff) 
   { EnsureLE<E>::go(); EnsureLE<T>::go(); EnsureLE<F>::go(); }

   typedef typename RemoveDuplicates<typename AppendList<typename E::FreeVars, 
      typename AppendList<typename T::FreeVars, typename F::FreeVars>::Result
      >::Result>::Result FreeVars;

   template <class ET> struct MyType 
   { typedef typename XType<ET,deduction_method>::TypeThunk TypeThunk; };

   template <class ET> struct RecheckType
   { typedef typename XRecheckType<ET,deduction_method>::Ok Ok; };

   template <class ET>
   typename MyType<ET>::TypeThunk::template Go<int>::Type 
   eval( const typename ET::template Go<int>::BE& be ) const {
      if( e.template eval<ET>(be) )
         return t.template eval<ET>(be);
      else
         return f.template eval<ET>(be);
   }

   typedef IfLE This;
   template <class A> typename BracketCallable<This,A>::Result
   operator[]( const A& a ) const 
   { return BracketCallable<This,A>::go( *this, a ); }
};

// operator, overloads
//
// Koenig lookup will only find these overloads if one of the arguments
// to comma is an LE (a type defined in this namespace).

// Either the LHS is already a CONS...
template <class H, class T, class RHS>
CONS<typename LEify<RHS>::Type,CONS<H,T> >
operator,( const CONS<H,T>& lhs, const RHS& rhs ) {
   return CONS<typename LEify<RHS>::Type,CONS<H,T> >
      ( LEify<RHS>::go(rhs), lhs );
}

// ... or it's not
template <class LHS, class RHS>
CONS<typename LEify<RHS>::Type,typename LEListify<LHS>::Type>
operator,( const LHS& lhs, const RHS& rhs ) {
   return CONS<typename LEify<RHS>::Type,typename LEListify<LHS>::Type>
      ( LEify<RHS>::go(rhs), LEListify<LHS>::go(lhs) );
}

} // end namespace exp

//////////////////////////////////////////////////////////////////////
// lambda() and the functoids that get made when all the vars are bound
// and we make it back out into "C++ space"
//////////////////////////////////////////////////////////////////////

template <class LE>
class Lambda0 : public CFunType<typename LE::template
                   MyType<NIL_ET>::TypeThunk::template Go<int>::Type> {
   LE exp;
public:
   Lambda0( const LE& e ) : exp(e) { EnsureLE<LE>::go(); }
   typename LE::template MyType<NIL_ET>::TypeThunk::template Go<int>::Type
   operator()() const {
      return exp.template eval<NIL_ET>( NIL() );
   }
};
template <class LE, int i>
class Lambda1 {
   LE exp;
   template <class X> struct NewET 
   { typedef typename ETUpdateX<NIL_ET,i,X>::Result Result; };
public:
   Lambda1( const LE& e ) : exp(e) { EnsureLE<LE>::go(); }
   template <class X> struct Sig : public FunType<X,typename LE::template 
      MyType<typename NewET<X>::Result>::TypeThunk::template Go<int>::Type> {};
   template <class X>
   typename Sig<X>::ResultType
   operator()( const X& x ) const {
      typedef typename NewET<X>::Result NET;
      return exp.template eval<NET>( NET::template Go<int>::go(NIL(),x) );
   }
};
template <class LE, int i, int j>
class Lambda2 {
   LE exp;
   template <class X, class Y> struct NewET 
   { typedef typename ETUpdateXY<NIL_ET,i,X,j,Y>::Result Result; };
public:
   Lambda2( const LE& e ) : exp(e) { EnsureLE<LE>::go(); }
   template <class X, class Y> struct Sig : public FunType<X,Y, 
      typename LE::template MyType<typename NewET<X,Y>::Result>
      ::TypeThunk::template Go<int>::Type> {};
   template <class X, class Y>
   typename Sig<X,Y>::ResultType
   operator()( const X& x, const Y& y ) const {
      typedef typename NewET<X,Y>::Result NET;
      return exp.template eval<NET>( NET::template Go<int>::go(NIL(),x,y) );
   }
};
template <class LE, int i, int j, int k>
class Lambda3 {
   LE exp;
   template <class X, class Y, class Z> struct NewET 
   { typedef typename ETUpdateXYZ<NIL_ET,i,X,j,Y,k,Z>::Result Result; };
public:
   Lambda3( const LE& e ) : exp(e) { EnsureLE<LE>::go(); }
   template <class X, class Y, class Z> struct Sig : public FunType<X,Y,Z,
      typename LE::template MyType<typename NewET<X,Y,Z>::Result>
      ::TypeThunk::template Go<int>::Type> {};
   template <class X, class Y, class Z>
   typename Sig<X,Y,Z>::ResultType
   operator()( const X& x, const Y& y, const Z& z ) const {
      typedef typename NewET<X,Y,Z>::Result NET;
      return exp.template eval<NET>( NET::template Go<int>::go(NIL(),x,y,z) );
   }
};

// LambdaThingy is the temporary object that lambda() returns which 
// understands operator[] calls.
template <class TBBV>
struct LambdaThingy {
   typedef TBBV VarsThisLambdaBinds;

   template <class LE, class FV, class TBBVars> struct Help2 { 
      typedef exp::LambdaExpWithFreeVars<LE,VarsThisLambdaBinds> Result; 
      typedef Result Full;
   };
   template <class LE> struct Help2<LE,NIL,NIL> { 
      typedef Lambda0<LE> Result; 
      typedef Full0<Result> Full;
   };
   template <class LE, int i> 
   struct Help2<LE,NIL,CONS<LambdaVar<i>,NIL> > { 
      typedef Lambda1<LE,i> Result; 
      typedef Full1<Result> Full;
   };
   template <class LE, int i, int j> 
   struct Help2<LE,NIL,CONS<LambdaVar<i>,CONS<LambdaVar<j>,NIL> > > { 
      typedef Lambda2<LE,i,j> Result; 
      typedef Full2<Result> Full;
   };
   template <class LE, int i, int j, int k> 
   struct Help2<LE,NIL,CONS<LambdaVar<i>,CONS<LambdaVar<j>,
                       CONS<LambdaVar<k>,NIL> > > > { 
      typedef Lambda3<LE,i,j,k> Result; 
      typedef Full3<Result> Full;
   };

   template <class LE> struct Helper {
      typedef typename ListDifference<typename LE::FreeVars,
         VarsThisLambdaBinds>::Result FreeVars;
      typedef typename Help2<LE,FreeVars,TBBV>::Result Result;
      typedef typename Help2<LE,FreeVars,TBBV>::Full Full;
   };
public:
   template <class E> struct RT
   { typedef typename Helper<typename LEify<E>::Type>::Full Type; };

   template <class E>
   typename RT<E>::Type
   operator[]( const E& e ) const {
      typedef typename Helper<typename LEify<E>::Type>::Result Result;
      typedef typename Helper<typename LEify<E>::Type>::Full Full;
      return Full( Result( LEify<E>::go(e) ) );
   }
};

inline LambdaThingy<NIL> lambda() { return LambdaThingy<NIL>(); }

template <int i> 
LambdaThingy<CONS<LambdaVar<i>,NIL> > 
lambda( const LambdaVar<i>& ) 
{ return LambdaThingy<CONS<LambdaVar<i>,NIL> >(); }

template <int i, int j> 
LambdaThingy<CONS<LambdaVar<i>,CONS<LambdaVar<j>,NIL> > > 
lambda( const LambdaVar<i>&, const LambdaVar<j>& ) { 
   YouCannotPassTheSameLambdaVarTo_lambda_MoreThanOnce<(i==j)>::go();
   return LambdaThingy<CONS<LambdaVar<i>,CONS<LambdaVar<j>,NIL> > >(); 
}

template <int i, int j, int k> 
LambdaThingy<CONS<LambdaVar<i>,CONS<LambdaVar<j>,CONS<LambdaVar<k>,NIL> > > > 
lambda( const LambdaVar<i>&, const LambdaVar<j>&, const LambdaVar<k>& ) { 
   YouCannotPassTheSameLambdaVarTo_lambda_MoreThanOnce<
      (i==j || i==k || j==k) >::go();
   return LambdaThingy<CONS<LambdaVar<i>,CONS<LambdaVar<j>,
            CONS<LambdaVar<k>,NIL> > > >(); 
}

//////////////////////////////////////////////////////////////////////
// lambda language constructs 
//////////////////////////////////////////////////////////////////////

template <IfKind k>
struct IfLambdaoid {
   template <class E, class T, class F>
   exp::IfLE<E,T,F,k>
   operator[]( const CONS<F,CONS<T,CONS<E,NIL> > >& x ) const {
      EnsureLE<E>::go(); EnsureLE<T>::go(); EnsureLE<F>::go();
      return exp::IfLE<E,T,F,k>( x.tail.tail.head, x.tail.head, x.head );
   }
};

template <int i, class E>
Binder<i,typename LEify<E>::Type>
operator==( exp::LambdaVar<i>, const E& e ) {
   return Binder<i,typename LEify<E>::Type>( LEify<E>::go(e) );
}

template <class LHS, int i, class LE>
CONS<Binder<i,LE>,typename BinderListify<LHS>::Type>
operator,( const LHS& lhs, const Binder<i,LE>& b ) {
   return CONS<Binder<i,LE>,typename BinderListify<LHS>::Type>
      ( b, BinderListify<LHS>::go(lhs) );
}

template <class H, class T, int i, class LE>
CONS<Binder<i,LE>,CONS<H,T> >
operator,( const CONS<H,T>& lhs, const Binder<i,LE>& b ) {
   return CONS<Binder<i,LE>,CONS<H,T> >( b, lhs );
}

//////////////////////////////////////////////////////////////////////
// LEType stuff
//////////////////////////////////////////////////////////////////////

template <class F, class X=Void, class Y=Void, class Z=Void> struct CALL;
template <int i> struct LV;
template <class E, class T, class F> struct IF0;
template <class E, class T, class F> struct IF1;
template <class E, class T, class F> struct IF2;
template <class A, class B=Void, class C=Void, class D=Void> struct LAM;

template <class T> struct LEType { typedef T Type; };

template <class A, class B=Void, class C=Void, class D=Void> 
struct LET_LEListify {
   typedef typename LEify<typename LEType<D>::Type>::Type LE_D;
   typedef CONS<LE_D,typename LET_LEListify<A,B,C>::Type> Type;
};
template <class A, class B, class C>
struct LET_LEListify<A,B,C,Void> {
   typedef typename LEify<typename LEType<C>::Type>::Type LE_C;
   typedef CONS<LE_C,typename LET_LEListify<A,B>::Type> Type;
};
template <class A, class B>
struct LET_LEListify<A,B,Void,Void> {
   typedef typename LEify<typename LEType<B>::Type>::Type LE_B;
   typedef CONS<LE_B,typename LET_LEListify<A>::Type> Type;
};
template <class A>
struct LET_LEListify<A,Void,Void,Void> {
   typedef typename LEListify<typename LEType<A>::Type>::Type Type;
};

template <class F, class X, class Y, class Z> 
struct LEType< CALL<F,X,Y,Z> > {
   typedef typename LEify<typename LEType<F>::Type>::Type FF;
   typedef exp::Call<FF,typename LET_LEListify<X,Y,Z>::Type> Type;
}; 
template <class F, class X, class Y>
struct LEType< CALL<F,X,Y,Void> > {
   typedef typename LEify<typename LEType<F>::Type>::Type FF;
   typedef exp::Call<FF,typename LET_LEListify<X,Y>::Type> Type;
}; 
template <class F, class X>
struct LEType< CALL<F,X,Void,Void> > {
   typedef typename LEify<typename LEType<F>::Type>::Type FF;
   typedef exp::Call<FF,typename LET_LEListify<X>::Type> Type;
}; 
template <class F>
struct LEType< CALL<F,Void,Void,Void> > {
   typedef typename LEify<typename LEType<F>::Type>::Type FF;
   typedef exp::Call<FF,NIL> Type;
}; 

template <int i> struct LEType< LV<i> > { typedef LambdaVar<i> Type; };

template <class E, class T, class F>
struct LEType< IF0<E,T,F> > {
   typedef typename LEify<typename LEType<E>::Type>::Type EE;
   typedef typename LEify<typename LEType<T>::Type>::Type TT;
   typedef typename LEify<typename LEType<F>::Type>::Type FF;
   typedef exp::IfLE<EE,TT,FF,IfNormal> Type;
};
template <class E, class T, class F>
struct LEType< IF1<E,T,F> > {
   typedef typename LEify<typename LEType<E>::Type>::Type EE;
   typedef typename LEify<typename LEType<T>::Type>::Type TT;
   typedef typename LEify<typename LEType<F>::Type>::Type FF;
   typedef exp::IfLE<EE,TT,FF,IfTrue> Type;
};
template <class E, class T, class F>
struct LEType< IF2<E,T,F> > {
   typedef typename LEify<typename LEType<E>::Type>::Type EE;
   typedef typename LEify<typename LEType<T>::Type>::Type TT;
   typedef typename LEify<typename LEType<F>::Type>::Type FF;
   typedef exp::IfLE<EE,TT,FF,IfFalse> Type;
};

template <int i, int j, int k, class E>
struct LEType< LAM<LV<i>,LV<j>,LV<k>,E> > {
   typedef typename LEType<E>::Type EE;
   typedef typename LambdaThingy<CONS<LambdaVar<i>,CONS<LambdaVar<j>,
      CONS<LambdaVar<k>,NIL> > > >::template RT<EE>::Type Type;
};
template <int i, int j, class E>
struct LEType< LAM<LV<i>,LV<j>,E,Void> > {
   typedef typename LEType<E>::Type EE;
   typedef typename LambdaThingy<CONS<LambdaVar<i>,CONS<LambdaVar<j>,
      NIL> > >::template RT<EE>::Type Type;
};
template <int i, class E>
struct LEType< LAM<LV<i>,E,Void,Void> > {
   typedef typename LEType<E>::Type EE;
   typedef typename LambdaThingy<CONS<LambdaVar<i>,
      NIL> >::template RT<EE>::Type Type;
};
template <class E>
struct LEType< LAM<E,Void,Void,Void> > {
   typedef typename LEType<E>::Type EE;
   typedef typename LambdaThingy<NIL>::template RT<EE>::Type Type;
};

//////////////////////////////////////////////////////////////////////
// more lambda language constructs
//////////////////////////////////////////////////////////////////////

struct LetLambdaoid {
   template <class BL>
   struct InLambdaoid {
      class Help {
         BL bl;
      public:
         template <class B, class LE> struct Lambdify;
         template <int i, class LEa, class LE> 
         struct Lambdify<CONS<Binder<i,LEa>,NIL>,LE> {
            typedef typename LEType<CALL<LAM<LV<i>,LE>,LEa> >::Type R;
            static inline R go( const CONS<Binder<i,LEa>,NIL>& binders, 
                                const LE& le ) {
               LambdaVar<i> X;
               return lambda(X)[ le ][ binders.head.exp ];
            }
         };
         template <int i, class LEa, class Rest, class LE> 
         struct Lambdify<CONS<Binder<i,LEa>,Rest>,LE> {
            typedef typename LEType<CALL<LAM<LV<i>,LE>,LEa> >::Type Inner;
            typedef typename Lambdify<Rest,Inner>::R R;
            static inline R go( const CONS<Binder<i,LEa>,Rest>& binders, 
                                const LE& le ) {
               LambdaVar<i> X;
               return Lambdify<Rest,Inner>::go( binders.tail,
                  lambda(X)[ le ][ binders.head.exp ] );
            }
         };

         Help( const BL& l ) : bl(l) {}
         template <class E> struct RT
         { typedef typename Lambdify<BL,typename LEify<E>::Type>::R Type; };
         template <class E>
         typename RT<E>::Type
         operator[]( const E& e ) {
            return Lambdify<BL,typename LEify<E>::Type>::go
               ( bl, LEify<E>::go(e) );
         }
      };
      Help in;
      InLambdaoid( const BL& l ) : in(l) {}
   };

   template <class BL> struct RT
   { typedef InLambdaoid<typename BinderListify<BL>::Type> Type; };
   template <class BL>
   typename RT<BL>::Type
   operator[]( const BL& bl ) const {
      EnsureBinderList<typename BinderListify<BL>::Type>::go();
      return InLambdaoid<typename BinderListify<BL>::Type>
         ( BinderListify<BL>::go(bl) );
   }
};

struct LetRecLambdaoid {
   template <class BL>
   struct InLambdaoid {
      class Help {
         BL bl;
      public:
         Help( const BL& l ) : bl(l) {}
         template <class E> struct RT 
         { typedef exp::BindingEnvExp<BL,typename LEify<E>::Type> Type; };
         template <class E>
         typename RT<E>::Type
         operator[]( const E& e ) {
            return exp::BindingEnvExp<BL,typename LEify<E>::Type>
               ( bl, LEify<E>::go(e) );
         }
      };
      Help in;
      InLambdaoid( const BL& l ) : in(l) {}
   };

   template <class BL> struct RT;
   template <class BL> friend struct RT;
   template <class BL> struct RT
   { typedef InLambdaoid<typename BinderListify<BL>::Type> Type; };
   template <class BL>
   typename RT<BL>::Type
   operator[]( const BL& bl ) const {
      EnsureBinderList<typename BinderListify<BL>::Type>::go();
      return InLambdaoid<typename BinderListify<BL>::Type>
         ( BinderListify<BL>::go(bl) );
   }
};

//////////////////////////////////////////////////////////////////////
// more LEType stuff
//////////////////////////////////////////////////////////////////////

template <int i, class LE> struct BIND;
template <class A, class B, class C=Void, class D=Void> struct LET;
template <class A, class B, class C=Void, class D=Void> struct LETREC;

template <class A, class B=Void, class C=Void, class D=Void> 
struct LET_BinderListify {
   typedef CONS<D,typename LET_BinderListify<A,B,C>::Type> Type;
};
template <class A, class B, class C>
struct LET_BinderListify<A,B,C,Void> {
   typedef CONS<C,typename LET_BinderListify<A,B>::Type> Type;
};
template <class A, class B>
struct LET_BinderListify<A,B,Void,Void> {
   typedef CONS<B,typename LET_BinderListify<A>::Type> Type;
};
template <class A>
struct LET_BinderListify<A,Void,Void,Void> {
   typedef typename BinderListify<A>::Type Type;
};

template <int i, class A, class LE>
struct LEType< LET<BIND<i,A>,LE,Void,Void> > {
   typedef typename LEType<LE>::Type LELE;
   typedef Binder<i,typename LEify<typename LEType<A>::Type>::Type> AA;
   typedef typename LetLambdaoid::template RT<typename
      LET_BinderListify<AA>::Type>::Type::Help::template RT<LELE>::Type Type;
};
template <int i, class A, int j, class B, class LE>
struct LEType< LET<BIND<i,A>,BIND<j,B>,LE,Void> > {
   typedef typename LEType<LE>::Type LELE;
   typedef Binder<i,typename LEify<typename LEType<A>::Type>::Type> AA;
   typedef Binder<j,typename LEify<typename LEType<B>::Type>::Type> BB;
   typedef typename LetLambdaoid::template RT<typename
      LET_BinderListify<AA,BB>::Type>::Type::Help::template RT<LELE>::Type Type;
};
template <int i, class A, int j, class B, int k, class C, class LE>
struct LEType< LET<BIND<i,A>,BIND<j,B>,BIND<k,C>,LE> > {
   typedef typename LEType<LE>::Type LELE;
   typedef Binder<i,typename LEify<typename LEType<A>::Type>::Type> AA;
   typedef Binder<j,typename LEify<typename LEType<B>::Type>::Type> BB;
   typedef Binder<k,typename LEify<typename LEType<C>::Type>::Type> CC;
   typedef typename LetLambdaoid::template RT<typename
      LET_BinderListify<AA,BB,CC>::Type>::Type::Help::template 
      RT<LELE>::Type Type;
};

template <int i, class A, class LE>
struct LEType< LETREC<BIND<i,A>,LE,Void,Void> > {
   typedef typename LEType<LE>::Type LELE;
   typedef Binder<i,typename LEify<typename LEType<A>::Type>::Type> AA;
   typedef typename LetRecLambdaoid::template RT<typename
      LET_BinderListify<AA>::Type>::Type::Help::template RT<LELE>::Type Type;
};
template <int i, class A, int j, class B, class LE>
struct LEType< LETREC<BIND<i,A>,BIND<j,B>,LE,Void> > {
   typedef typename LEType<LE>::Type LELE;
   typedef Binder<i,typename LEify<typename LEType<A>::Type>::Type> AA;
   typedef Binder<j,typename LEify<typename LEType<B>::Type>::Type> BB;
   typedef typename LetRecLambdaoid::template RT<typename
      LET_BinderListify<AA,BB>::Type>::Type::Help::template RT<LELE>::Type Type;
};
template <int i, class A, int j, class B, int k, class C, class LE>
struct LEType< LETREC<BIND<i,A>,BIND<j,B>,BIND<k,C>,LE> > {
   typedef typename LEType<LE>::Type LELE;
   typedef Binder<i,typename LEify<typename LEType<A>::Type>::Type> AA;
   typedef Binder<j,typename LEify<typename LEType<B>::Type>::Type> BB;
   typedef Binder<k,typename LEify<typename LEType<C>::Type>::Type> CC;
   typedef typename LetRecLambdaoid::template RT<typename
      LET_BinderListify<AA,BB,CC>::Type>::Type::Help::template 
      RT<LELE>::Type Type;
};

} // end namespace fcpp_lambda

FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN fcpp_lambda::IfLambdaoid<fcpp_lambda::IfNormal> if0;
FCPP_MAYBE_EXTERN fcpp_lambda::IfLambdaoid<fcpp_lambda::IfTrue>   if1;
FCPP_MAYBE_EXTERN fcpp_lambda::IfLambdaoid<fcpp_lambda::IfFalse>  if2;
FCPP_MAYBE_EXTERN fcpp_lambda::LetRecLambdaoid letrec;
FCPP_MAYBE_EXTERN fcpp_lambda::LetLambdaoid let;
FCPP_MAYBE_NAMESPACE_CLOSE

using fcpp_lambda::LambdaVar;
using fcpp_lambda::lambda;  // all that work for _one_ exported function :)

using fcpp_lambda::LEType;
using fcpp_lambda::CALL;
using fcpp_lambda::LV;
using fcpp_lambda::IF0;
using fcpp_lambda::IF1;
using fcpp_lambda::IF2;
using fcpp_lambda::LAM;
using fcpp_lambda::BIND;
using fcpp_lambda::LET;
using fcpp_lambda::LETREC;

} // end namespace fcpp

#endif
#endif

