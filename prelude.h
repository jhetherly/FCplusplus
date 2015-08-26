//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_PRELUDE_DOT_H
#define FCPP_PRELUDE_DOT_H

//////////////////////////////////////////////////////////////////////
// Note that this header file includes all the other FC++ header files,
// so including this one (prelude.h) is sufficient to suck in the whole
// library. 
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Here we define a bunch of functions from the Haskell Standard
// Prelude (HSP).  For documentation of their behaviors, see 
//    http://haskell.org/onlinereport/standard-prelude.html
//
// A number of the functions are not from HSP, but seemed natural/useful.
//
// The implementations #ifdef-ed out (FCPP_SIMPLE_PRELUDE) are mostly
// just here to look at, as they are often more readable than their
// optimized counterparts.
//////////////////////////////////////////////////////////////////////

#include "list.h"

namespace fcpp {

namespace impl {
struct XId {
   template <class T> struct Sig : public FunType<T,T> {};

   template <class T>
   T operator()( const T& x ) const {
      return x;
   }
};
}
typedef Full1<impl::XId> Id;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Id id;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
template <class F, class G>
class XCompose0Helper : public CFunType<
typename F::template Sig<typename RT<G>::ResultType>::ResultType> {
   F f;
   G g;
public:
   XCompose0Helper( const F& a, const G& b ) : f(a), g(b) {}

   typename F::template Sig<typename RT<G>::ResultType>::ResultType
   operator()() const {
      return f( g() );
   }
};
template <class F, class G>
class XCompose1Helper {
   F f;
   G g;
public:
   XCompose1Helper( const F& a, const G& b ) : f(a), g(b) {}
   template <class X> struct Sig : public FunType<
      typename RT<G,X>::Arg1Type,
      typename RT<F,typename RT<G,X>::ResultType>::ResultType> {};
   template <class X>
   typename Sig<X>::ResultType operator()( const X& x ) const {
      return f( g(x) );
   }
};
template <class F, class G>
class XCompose2Helper {
   F f;
   G g;
public:
   XCompose2Helper( const F& a, const G& b ) : f(a), g(b) {}
   template <class X, class Y> struct Sig : public FunType<
      typename RT<G,X,Y>::Arg1Type,
      typename RT<G,X,Y>::Arg2Type,
      typename RT<F,typename RT<G,X,Y>::ResultType>::ResultType> {};
   template <class X, class Y>
   typename Sig<X,Y>::ResultType operator()( const X& x, const Y& y ) const {
      return f( g(x,y) );
   }
};
template <class F, class G>
class XCompose3Helper {
   F f;
   G g;
public:
   XCompose3Helper( const F& a, const G& b ) : f(a), g(b) {}
   template <class X, class Y, class Z> struct Sig : public FunType<
      typename RT<G,X,Y,Z>::Arg1Type,
      typename RT<G,X,Y,Z>::Arg2Type,
      typename RT<G,X,Y,Z>::Arg3Type,
      typename RT<F,typename RT<G,X,Y,Z>::ResultType>::ResultType> {};
   template <class X, class Y, class Z>
   typename Sig<X,Y,Z>::ResultType 
   operator()( const X& x, const Y& y, const Z& z ) const {
      return f( g(x,y,z) );
   }
};

// Compose is Haskell's operator (.) 
// compose(f,g)(x,y,z) = f( g(x,y,z) )
class XCompose {
   template <int i, class F, class G> struct Helper;
   template <class F, class G> 
   struct Helper<0,F,G> {
      typedef Full0<XCompose0Helper<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull0(XCompose0Helper<F,G>(f,g)); }
   };
   template <class F, class G> 
   struct Helper<1,F,G> {
      typedef Full1<XCompose1Helper<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull1(XCompose1Helper<F,G>(f,g)); }
   };
   template <class F, class G> 
   struct Helper<2,F,G> {
      typedef Full2<XCompose2Helper<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull2(XCompose2Helper<F,G>(f,g)); }
   };
   template <class F, class G> 
   struct Helper<3,F,G> {
      typedef Full3<XCompose3Helper<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull3(XCompose3Helper<F,G>(f,g)); }
   };
public:
   template <class F, class G> struct Sig : public 
   FunType<F,G,typename Helper<FunctoidTraits<G>::max_args,F,G>::Result> {};

   template <class F, class G>
   typename Sig<F,G>::ResultType operator()( const F& f, const G& g ) const {
      return Helper<FunctoidTraits<G>::max_args,F,G>::go( f, g );
   }
};
}
typedef Full2<impl::XCompose> Compose;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Compose compose;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// Now we can create "of", so that
//    f ^of^ g
// means
//    compose( funify(f), funify(g) )
//////////////////////////////////////////////////////////////////////

namespace impl {
struct XOf {
   template <class F, class G> struct Sig : public FunType<F,G,
      typename RT<Compose,typename RT<Funify,F>::ResultType,
      typename RT<Funify,G>::ResultType>::ResultType> {};
   template <class F, class G>
   typename Sig<F,G>::ResultType
   operator()( const F& f, const G& g ) const {
      return compose( funify(f), funify(g) );
   }
};
}
typedef Full2<impl::XOf> Of;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Of of;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
template <class F, class G, class H>
class XXCompose2Helper {
   F f;
   G g;
   H h;
public:
   XXCompose2Helper( const F& a, const G& b, const H& c) : f(a), g(b), h(c) {}

   template <class T>
   struct Sig : public FunType<
   typename G::template Sig<T>::Arg1Type,
   typename F::template Sig<typename G::template Sig<T>::ResultType, 
                            typename H::template Sig<T>::ResultType>::ResultType
   > {};

   template <class T>
   typename F::template Sig<typename G::template Sig<T>::ResultType, 
                            typename H::template Sig<T>::ResultType>::ResultType
   operator()( const T& x ) const {
      return f( g(x), h(x) );
   }
};
   
//      compose2(f,g,h)(x) == f( g(x), h(x) )
// Compose2 composes a two argument function with two one-argument
// functions (taking the same type). This is quite useful for the
// common case of binary operators.  Use lambda for more-complicated stuff.
struct XCompose2 {
   template <class F, class G, class H>
   struct Sig : public FunType<F,G,H,Full1<XXCompose2Helper<F,G,H> > > {};

   template <class F, class G, class H>
   Full1<XXCompose2Helper<F,G,H> > 
   operator()(const F& f, const G& g, const H& h) const {
      return makeFull1( XXCompose2Helper<F,G,H>( f, g, h ) );
   }
};
}
typedef Full3<impl::XCompose2> Compose2;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Compose2 compose2;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XUntil {
   template <class Pred, class Unary, class T>
   struct Sig : public FunType<Pred,Unary,T,T> {};

   template <class Pred, class Unary, class T>
   T operator()( const Pred& p, const Unary& op, T start ) const {
      while( !p(start) ) {
         T tmp( start );
         start.~T();
         new (&start) T( op(tmp) );
      }
      return start;
   }
};
}
typedef Full3<impl::XUntil> Until;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Until until;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XLast {
   template <class L>
   struct Sig : public FunType<L,typename L::ElementType> {};

   template <class L>
   typename L::ElementType operator()( const L& ll ) const {
      List<typename L::ElementType> l = ll;
      while( !null( tail(l) ) )
         l = tail(l);
      return head(l);
   }
};
}
typedef Full1<impl::XLast> Last;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Last last;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XInit {
   template <class L>
   struct Sig : public FunType<L,List<typename L::ElementType> > {};

   template <class T>
   List<T> operator()( const List<T>& l ) const {
      if( null( tail( l ) ) )
         return NIL;
      else
         return cons( head(l), curry( XInit(), tail(l) ) );
   }
};
#else
struct XInit {
   template <class L>
   struct Sig : public FunType<L,OddList<typename L::ElementType> > {};

   template <class L>
   OddList<typename L::ElementType> 
   operator()( const L& l, 
               Reuser1<Inv,Var,XInit,List<typename L::ElementType> >
               r = NIL ) const {
      if( null( tail( l ) ) )
         return NIL;
      else
         return cons( head(l), r( XInit(), tail(l) ) );
   }
};
#endif
}
typedef Full1<impl::XInit> Init;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Init init;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XLength {
   template <class L>
   struct Sig : public FunType<L,size_t> {};

   template <class L>
   size_t operator()( const L& ll ) const {
      List<typename L::ElementType> l = ll;
      size_t x = 0;
      while( !null(l) ) {
         l = tail(l);
         ++x;
      }
      return x;
   }
};
}
typedef Full1<impl::XLength> Length;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Length length;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
// At is Haskell's operator (!!)
struct XAt {
   template <class L, class N>
   struct Sig : public FunType<L,N,typename L::ElementType> {};

   template <class L>
   typename L::ElementType operator()( L l, size_t n ) const {
      while( n!=0 ) {
         l = tail(l);
         --n;
      }
      return head(l);
   }
};
}
typedef Full2<impl::XAt> At;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN At at;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XFilter {
   template <class P, class LT>
   struct Sig : public FunType<P,LT,List<typename LT::ElementType> > {};

   template <class P, class T>
   List<T> operator()( const P& p, const List<T>& l ) const {
      if( null(l) )
         return l;
      else if( p(head(l)) )
        return cons( head(l), curry2( XFilter(), p, tail(l) ) );
      else
         return XFilter()( p, tail(l) );
   }
};
#else
template <class P, class T>
struct XFilterHelp : public Fun0Impl< OddList<T> > {
   P p;
   mutable List<T> l;
   XFilterHelp( const P& pp, const List<T>& ll ) : p(pp), l(ll) {}
   OddList<T> operator()() const {
      while(1) {
         if( null(l) )
            return NIL;
         else if( p( head(l) ) ) {
            T x = head(l);
            l = tail(l);
            return cons( x, Fun0< OddList<T> >(1,this) );
         }
         else
            l = tail(l);
      }
   }
};
struct XFilter {
   template <class P, class L>
   struct Sig : public FunType<P,L,List<typename L::ElementType> > {};

   template <class P, class L>
   List<typename L::ElementType>
   operator()( const P& p, L l ) const {
      return Fun0< OddList<typename L::ElementType> >(1, 
               new XFilterHelp<P,typename L::ElementType>(p,l) );
   }
};
/* For filter, the version with a Reuser is just not as good as the
hand-coded reuse version, which is why this is commented out.
struct XFilter {
   template <class P, class L>
   struct Sig : public FunType<P,L,OddList<typename L::ElementType> > {};

   template <class P, class T>
   OddList<T> operator()( const P& p, List<T> l,
                            Reuser2<Inv,Inv,Var,XFilter,P,List<T> > 
                            r = NIL ) const {
      while(1) {
         if( null(l) )
            return NIL;
         else if( p(head(l)) )
            return cons( head(l), r( XFilter(), p, tail(l) ) );
         else
            r.iter( XFilter(), p, l = tail(l) );
      }
   }
};
*/
#endif
}
typedef Full2<impl::XFilter> Filter;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Filter filter;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XConcat {
   template <class LLT>
   struct Sig : public FunType<LLT, typename LLT::ElementType> {};

   template <class T>
   List<T> operator()( const List<List<T> >& l ) const {
      if( null(l) )
         return List<T>();
      else
         return cat( head(l), curry(XConcat(),tail(l)) );
   }
};
#else
struct XConcat {
   template <class LLT>
   struct Sig : public FunType<LLT, 
      OddList<typename LLT::ElementType::ElementType> > {};

   template <class L>
   OddList<typename L::ElementType::ElementType> 
   operator()( const L& l, 
               Reuser1<Inv,Var,XConcat,List<typename L::ElementType> > 
               r = NIL ) const {
      if( null(l) )
         return NIL;
      else
         return cat( head(l), r(XConcat(),tail(l)) );
   }
};
#endif
}
typedef Full1<impl::XConcat> Concat;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Concat concat;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
// Note: this isn't lazy (even if 'op' is 'cons').
struct XFoldr {
   template <class Op, class E, class L>
   struct Sig : public FunType<Op,E,L,E> {};

   template <class Op, class E, class L>
   E operator()( const Op& op, const E& e, const L& l ) const {
      if( null(l) )
         return e;
      else 
         return op( head(l), XFoldr()( op, e, tail(l) ) );
   }
};
}
typedef Full3<impl::XFoldr> Foldr;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Foldr foldr;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XFoldr1 {
   template <class Op, class L>
   struct Sig : public FunType<Op,L,typename L::ElementType> {};

   template <class Op, class L>
   typename L::ElementType operator()( const Op& op, const L& l ) const {
      return foldr( op, head(l), tail(l) );
   }
};
}
typedef Full2<impl::XFoldr1> Foldr1;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Foldr1 foldr1;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XFoldl {
   template <class Op, class E, class L>
   struct Sig : public FunType<Op,E,L,E> {};

   template <class Op, class E, class L>
   E operator()( const Op& op, E e, const L& ll ) const {
      List<typename L::ElementType> l = ll;
      while( !null(l) ) {
         E tmp( e );
         e.~E();
         new (&e) E( op(tmp,head(l)) );
         l = tail(l);
      }
      return e;
   }
};
}
typedef Full3<impl::XFoldl> Foldl;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Foldl foldl;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XFoldl1 {
   template <class Op, class L>
   struct Sig : public FunType<Op,L,typename L::ElementType> {};

   template <class Op, class L>
   typename L::ElementType operator()( const Op& op, const L& l ) const {
      return foldl( op, head(l), tail(l) );
   }
};
}
typedef Full2<impl::XFoldl1> Foldl1;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Foldl1 foldl1;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XScanr {
   template <class Op, class E, class L>
   struct Sig : public FunType<Op,E,L,OddList<E> > {};

   template <class Op, class E, class L>
   OddList<E> operator()( const Op& op, const E& e, const L& l ) const {
      if( null(l) )
         return cons( e, NIL );
      else {
         OddList<E> temp = XScanr()( op, e, tail(l) );
         return cons( op( head(l), head(temp) ), temp );
      }
   }
};
}
typedef Full3<impl::XScanr> Scanr;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Scanr scanr;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XScanr1 {
   template <class Op, class L>
   struct Sig : public FunType<Op,L,OddList<typename L::ElementType> > {};

   template <class Op, class L>
   OddList<typename L::ElementType> 
   operator()( const Op& op, const L& l ) const {
      if( null( tail(l) ) )
         return l.force();
      else {
         OddList<typename L::ElementType> temp = XScanr1()( op, tail(l) );
         return cons( op( head(l), head(temp) ), temp );
      }
   }
};
}
typedef Full2<impl::XScanr1> Scanr1;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Scanr1 scanr1;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XScanl {
   template <class Op, class E, class L>
   struct Sig : public FunType<Op,E,L,List<E> > {};

   template <class Op, class E, class T>
   List<E> operator()( const Op& op, const E& e, const List<T>& l ) const {
      if( null(l) )
         return cons( e, NIL );
      else
         return cons( e, curry3( XScanl(), op, op(e,head(l)), tail(l) ));
   }
};
#else
struct XScanl {
   template <class Op, class E, class L>
   struct Sig : public FunType<Op,E,L,OddList<E> > {};

   template <class Op, class E, class L>
   OddList<E> operator()( const Op& op, const E& e, const L& l,
         Reuser3<Inv,Inv,Var,Var,XScanl,Op,E,List<typename L::ElementType> >
         r = NIL ) const {
      if( null(l) )
         return cons( e, NIL );
      else
         return cons( e, r( XScanl(), op, op(e,head(l)), tail(l) ) );
   }
};
#endif
}
typedef Full3<impl::XScanl> Scanl;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Scanl scanl;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XScanl1 {
   template <class Op, class L>
   struct Sig : public FunType<Op,L,OddList<typename L::ElementType> > {};

   template <class Op, class L>
   OddList<typename L::ElementType> 
   operator()( const Op& op, const L& l ) const {
      return scanl( op, head(l), tail(l) );
   }
};
}
typedef Full2<impl::XScanl1> Scanl1;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Scanl1 scanl1;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XIterate {
   template <class F, class T>
   struct Sig : public FunType<F,T,List<T> > {};

   template <class F, class T>
   List<T> operator()( const F& f, const T& x ) const {
      return cons( x, curry2( XIterate(), f, f(x) ) );
   }
};
#else
struct XIterate {
   template <class F, class T>
   struct Sig : public FunType<F,T,OddList<T> > {};

   template <class F, class T>
   OddList<T> operator()( const F& f, const T& x,
                          Reuser2<Inv,Inv,Var,XIterate,F,T> r = NIL ) const {
      return cons( x, r( XIterate(), f, f(x) ) );
   }
};
#endif
}
typedef Full2<impl::XIterate> Iterate;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Iterate iterate;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XRepeat {
   template <class T>
   struct Sig : public FunType<T,List<T> > {};

   template <class T>
   List<T> operator()( const T& x ) const {
      return cons( x, curry( XRepeat(), x ) );
   }
};
#else
struct XRepeat {
   template <class T>
   struct Sig : public FunType<T,OddList<T> > {};

   template <class T>
   OddList<T> operator()( const T& x, 
                          Reuser1<Inv,Inv,XRepeat,T> r = NIL ) const {
      return cons( x, r( XRepeat(), x ) );
   }
};
#endif
}
typedef Full1<impl::XRepeat> Repeat;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Repeat repeat;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XMap {
   template <class F, class L>
   struct Sig : public FunType<F,L,
      List<typename F::template Sig<typename L::ElementType>::ResultType> > {};

   template <class F, class T>
   List<typename F::template Sig<T>::ResultType>
   operator()( const F& f, const List<T>& l ) const {
      if( null(l) )
         return NIL;
      else
         return cons( f(head(l)), curry2( XMap(), f, tail(l) ) );
   }
};
#else
struct XMap {
   template <class F, class L>
   struct Sig : public FunType<F,L,
      OddList<typename RT<F,typename L::ElementType>::ResultType> > {};

   template <class F, class L>
   OddList<typename RT<F,typename L::ElementType>::ResultType> 
   operator()( const F& f, const L& l, 
               Reuser2<Inv,Inv,Var,XMap,F,List<typename L::ElementType> >
               r = NIL ) const {
      if( null(l) )
         return NIL;
      else
         return cons( f(head(l)), r( XMap(), f, tail(l) ) );
   }
};
#endif
}
typedef Full2<impl::XMap> Map;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Map map;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XTake {
   template <class N,class L>
   struct Sig : public FunType<N,L,List<typename L::ElementType> > {};

   template <class T>
   List<T> operator()( size_t n, const List<T>& l ) const {
      if( n==0 || null(l) )
         return NIL;
      else
         return cons( head(l), curry2( XTake(), n-1, tail(l) ) );
   }
};
#else
struct XTake {
   template <class N,class L>
   struct Sig : public FunType<N,L,OddList<typename L::ElementType> > {};

   template <class L>
   OddList<typename L::ElementType> 
   operator()( size_t n, const L& l,
               Reuser2<Inv,Var,Var,XTake,size_t,List<typename L::ElementType> > 
               r = NIL ) const {
      if( n==0 || null(l) )
         return NIL;
      else
         return cons( head(l), r( XTake(), n-1, tail(l) ) );
   }
};
#endif
}
typedef Full2<impl::XTake> Take;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Take take;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XDrop {
   template <class Dummy, class L>
   struct Sig : public FunType<size_t,L,List<typename L::ElementType> > {};
   
   template <class L>
   List<typename L::ElementType> operator()( size_t n, const L& ll ) const {
      List<typename L::ElementType> l = ll;
      while( n!=0 && !null(l) ) {
         --n;
         l = tail(l);
      }
      return l;
   }
};
}
typedef Full2<impl::XDrop> Drop;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Drop drop;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XTakeWhile {
   template <class P, class L>
   struct Sig : public FunType<P,L,List<typename L::ElementType> > {};

   template <class P, class T>
   List<T> operator()( const P& p, const List<T>& l ) const {
      if( null(l) || !p( head(l) ) )
         return NIL;
      else
         return cons( head(l), curry2( XTakeWhile(), p, tail(l) ) );
   }
};
#else
struct XTakeWhile {
   template <class P, class L>
   struct Sig : public FunType<P,L,OddList<typename L::ElementType> > {};

   template <class P, class L>
   OddList<typename L::ElementType>
   operator()( const P& p, const L& l,
               Reuser2<Inv,Inv,Var,XTakeWhile,P,List<typename L::ElementType> >
               r = NIL ) const {
      if( null(l) || !p( head(l) ) )
         return NIL;
      else
         return cons( head(l), r( XTakeWhile(), p, tail(l) ) );
   }
};
#endif
}
typedef Full2<impl::XTakeWhile> TakeWhile;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN TakeWhile takeWhile;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XDropWhile {
   template <class P, class L>
   struct Sig : public FunType<P,L,List<typename L::ElementType> > {};

   template <class P, class L>
   List<typename L::ElementType> operator()( const P& p, const L& ll ) const {
      List<typename L::ElementType> l = ll;
      while( !null(l) && p( head(l) ) )
         l = tail(l);
      return l;
   }
};
}
typedef Full2<impl::XDropWhile> DropWhile;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN DropWhile dropWhile;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XReplicate {
   template <class N, class T>
   struct Sig : public FunType<N,T,OddList<T> > {};

   template <class T>
   OddList<T> operator()( size_t n, const T& x ) const {
      return take( n, repeat(x) );
   }
};
}
typedef Full2<impl::XReplicate> Replicate;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Replicate replicate;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XCycle {
   template <class L>
   struct Sig : public FunType<L,List<typename L::ElementType> > {};

   template <class T>
   List<T> operator()( const List<T>& l ) const {
      return cat( l, curry( XCycle(), l ) );
   }
};
#else
struct XCycle {
   template <class L>
   struct Sig : public FunType<L,OddList<typename L::ElementType> > {};

   template <class L>
   OddList<typename L::ElementType> 
   operator()( const L& l, Reuser1<Inv,Inv,XCycle,L> r = NIL ) const {
      return cat( l, r( XCycle(), l ) );
   }
};
#endif
}
typedef Full1<impl::XCycle> Cycle;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Cycle cycle;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XSplitAt {
   template <class N, class L>
   struct Sig : public FunType<N,L,std::pair<List<typename
      L::ElementType>,List<typename L::ElementType> > > {};

   template <class T>
   std::pair<List<T>,List<T> > operator()( size_t n, const List<T>& l ) const {
      if( n==0 || null(l) )
         return std::make_pair( List<T>(), l );
      else {
         std::pair<List<T>,List<T> > temp = XSplitAt()( n-1, tail(l) );
         List<T> tl = cons( head(l), temp.first );
         return std::make_pair( tl, temp.second );
      }
   }
};
}
typedef Full2<impl::XSplitAt> SplitAt;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN SplitAt splitAt;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XSpan {
   template <class P, class L>
   struct Sig : public FunType<P,L,std::pair<List<typename
      L::ElementType>,List<typename L::ElementType> > > {};

   template <class P, class T>
   std::pair<List<T>,List<T> > 
   operator()( const P& p, const List<T>& l ) const {
      if( null(l) || !p(head(l)) )
         return std::make_pair( List<T>(), l );
      else {
         std::pair<List<T>,List<T> > temp = XSpan()( p, tail(l) );
         List<T> tl = cons(head(l),temp.first);
         return std::make_pair( tl, temp.second );
      }
   }
};
}
typedef Full2<impl::XSpan> Span;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Span span;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XBreak {
   template <class P, class L>
   struct Sig : public FunType<P,L,std::pair<List<typename
      L::ElementType>,List<typename L::ElementType> > > {};

   template <class P, class T>
   std::pair<List<T>,List<T> > 
   operator()( const P& p, const List<T>& l ) const {
      return span( Compose()( LogicalNot(), p ), l );
   }
};
}
typedef Full2<impl::XBreak> Break;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Break break_; // C++ keyword, so add trailing underscore
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
template <class Binary>
class XFlipHelper {
   Binary op;
public:
   XFlipHelper( const Binary& b ) : op(b) {}
   
   template <class Y, class X>
   struct Sig : public FunType<Y,X, 
      typename Binary::template Sig<X,Y>::ResultType > {};

   template <class Y, class X>
   typename Binary::template Sig<X,Y>::ResultType
   operator()( const Y& y, const X& x ) const {
      return op( x, y );
   }
};
struct XFlip {
   template <class Binary>
   struct Sig : public FunType<Binary,Full2<XFlipHelper<Binary> > > {};

   template <class Binary>
   Full2<XFlipHelper<Binary> > operator()( const Binary& op ) const {
      return makeFull2( XFlipHelper<Binary>( op ) );
   }
};
}
typedef Full1<impl::XFlip> Flip;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Flip flip;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XReverse {
   template <class L>
   struct Sig : public FunType<L,List<typename L::ElementType> > {};

   template <class T>
   List<T> operator()( const List<T>& l ) const {
      return curry3( foldl, flip(cons), List<T>(), l );
   }
};
}
typedef Full1<impl::XReverse> Reverse;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Reverse reverse;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// Not HSP but close
//////////////////////////////////////////////////////////////////////

// These next two are defined as _lazy_ versions of these operators on lists
namespace impl {
struct XAnd : public CFunType<List<bool>,bool> {
   bool operator()( const List<bool>& l ) const {
      return null(dropWhile( equal(true), l ));
   }
};
}
typedef Full1<impl::XAnd> And;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN And and_;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XOr : public CFunType<List<bool>,bool> {
   bool operator()( const List<bool>& l ) const {
      return !null(dropWhile( equal(false), l ));
   }
};
}
typedef Full1<impl::XOr> Or;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Or or_;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// Back to HSP
//////////////////////////////////////////////////////////////////////

namespace impl {
struct XAll {
   template <class P, class L>
   struct Sig : public FunType<P,L,bool> {};

   template <class P, class L>
   bool operator()( const P& p, const L& l ) const {
      return and_( map( p, l ) );
   }
};
}
typedef Full2<impl::XAll> All;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN All all;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XAny {
   template <class P, class L>
   struct Sig : public FunType<P,L,bool> {};

   template <class P, class L>
   bool operator()( const P& p, const L& l ) const {
      return or_( map( p, l ) );
   }
};
}
typedef Full2<impl::XAny> Any;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Any any;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XElem {
   template <class T, class L>
   struct Sig : public FunType<T,L,bool> {};

   template <class T, class L>
   bool operator()( const T& x, const L& l ) const {
      return any( equal(x), l );
   }
};
}
typedef Full2<impl::XElem> Elem;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Elem elem;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XNotElem {
   template <class T, class L>
   struct Sig : public FunType<T,L,bool> {};

   template <class T, class L>
   bool operator()( const T& x, const L& l ) const {
      return all( notEqual(x), l );
   }
};
}
typedef Full2<impl::XNotElem> NotElem;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN NotElem notElem;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XSum {
   template <class L>
   struct Sig : public FunType<L,typename L::ElementType> {};

   template <class L>
   typename L::ElementType operator()( const L& l ) const {
      return foldl( plus, 0, l );
   }
};
}
typedef Full1<impl::XSum> Sum;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Sum sum;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XProduct {
   template <class L>
   struct Sig : public FunType<L,typename L::ElementType> {};

   template <class L>
   typename L::ElementType operator()( const L& l ) const {
      return foldl( multiplies, 1, l );
   }
};
}
typedef Full1<impl::XProduct> Product;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Product product;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XMinimum {
   template <class L>
   struct Sig : public FunType<L,typename L::ElementType> {};

   template <class L>
   typename L::ElementType operator()( const L& l ) const {
      return foldl1( min, l );
   }
};
}
typedef Full1<impl::XMinimum> Minimum;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Minimum minimum;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XMaximum {
   template <class L>
   struct Sig : public FunType<L,typename L::ElementType> {};

   template <class L>
   typename L::ElementType operator()( const L& l ) const {
      return foldl1( max, l );
   }
};
}
typedef Full1<impl::XMaximum> Maximum;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Maximum maximum;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_SIMPLE_PRELUDE
struct XZipWith {
   template <class Z, class LA, class LB>
   struct Sig : public FunType<Z,LA,LB,
   List<typename Z::template Sig<typename LA::ElementType,
                                 typename LB::ElementType>::ResultType> > {};

   template <class Z, class A, class B>
   List<typename RT<Z,A,B>::ResultType> 
   operator()( const Z& z, const List<A>& a, const List<B>& b) const {
      if( null(a) || null(b) )
         return List<typename RT<Z,A,B>::ResultType>();
      else
         return cons( z(head(a),head(b)),
            curry3( XZipWith(), z, tail(a), tail(b) ) );
   }
};
#else
struct XZipWith {
   template <class Z, class LA, class LB>
   struct Sig : public FunType<Z,LA,LB,
   OddList<typename RT<Z,typename LA::ElementType,
                         typename LB::ElementType>::ResultType> > {};

   template <class Z, class LA, class LB>
   OddList<typename RT<Z,typename LA::ElementType,
                         typename LB::ElementType>::ResultType> 
   operator()( const Z& z, const LA& a, const LB& b,
               Reuser3<Inv,Inv,Var,Var,XZipWith,Z,
                  List<typename LA::ElementType>,
                  List<typename LB::ElementType> > r = NIL ) const {
      if( null(a) || null(b) )
         return NIL;
      else
         return cons( z(head(a),head(b)),
            r( XZipWith(), z, tail(a), tail(b) ) );
   }
};
#endif
}
typedef Full3<impl::XZipWith> ZipWith;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN ZipWith zipWith;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XZip {
   template <class LA, class LB>
   struct Sig 
   : public FunType<LA,LB,OddList<std::pair<typename LA::ElementType,
                                              typename LB::ElementType> > > {};

   template <class LA, class LB>
   OddList<std::pair<typename LA::ElementType, typename LB::ElementType> >
   operator()( const LA& a, const LB& b ) const {
      return zipWith( MakePair(), a, b );
   }
};
}
typedef Full2<impl::XZip> Zip;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Zip zip;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XFst {
   template <class P>
   struct Sig : public FunType<P,typename P::first_type> {};

   template <class A, class B>
   A operator()( const std::pair<A,B>& p ) const {
      return p.first;
   }
};
}
typedef Full1<impl::XFst> Fst;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Fst fst;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XSnd {
   template <class P>
   struct Sig : public FunType<P,typename P::second_type> {};

   template <class A, class B>
   B operator()( const std::pair<A,B>& p ) const {
      return p.second;
   }
};
}
typedef Full1<impl::XSnd> Snd;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Snd snd;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XUnzip {
   template <class LPT>
   struct Sig : public FunType<LPT,std::pair<
      List<typename LPT::ElementType::first_type>,
      List<typename LPT::ElementType::second_type> > > {};

   template <class LPT>
   std::pair<
      List<typename LPT::ElementType::first_type>,
      List<typename LPT::ElementType::second_type> >
   operator()( const LPT& l ) const {
      typedef typename LPT::ElementType::first_type F;
      typedef typename LPT::ElementType::second_type S;
      return std::make_pair( List<F>(curry2(map,fst,l)), 
                             List<S>(curry2(map,snd,l))  );
   }
};
}
typedef Full1<impl::XUnzip> Unzip;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Unzip unzip;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XGcdPrime {
   template <class T, class U> struct Sig;
   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( T x, T y ) const {
      while( y!=0 ) {
         T tmp( x%y );
         x = y;
         y = tmp;
      }
      return x;
   }
};
struct XGcd {
   template <class T, class U> struct Sig;
   template <class T>
   struct Sig<T,T> : public FunType<T,T,T> {};

   template <class T>
   T operator()( const T& x, const T& y ) const {
      if( x==0 && y==0 )
         throw fcpp_exception("Gcd error: x and y both 0");
      return XGcdPrime()( x<0?-x:x, y<0?-y:y );
   }
};
}
typedef Full2<impl::XGcd> Gcd;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Gcd gcd;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XOdd {
   template <class T>
   struct Sig : public FunType<T,bool> {};

   template <class T>
   bool operator()( const T& x ) const {
      return x%2==1;
   }
};
}
typedef Full1<impl::XOdd> Odd;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Odd odd;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XEven {
   template <class T>
   struct Sig : public FunType<T,bool> {};

   template <class T>
   bool operator()( const T& x ) const {
      return x%2==0;
   }
};
}
typedef Full1<impl::XEven> Even;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Even even;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// Not HSP but close
//////////////////////////////////////////////////////////////////////

// For some unknown reason, g++2.95.2 (for Solaris, at least) generates
// poor code when these next two functoids are templates.  (g++3 does
// fine, regardless.)  As a result, we make them just work with ints,
// unless the user #defines the flag below.
namespace impl {
#ifdef FCPP_TEMPLATE_ENUM
template <class T>
struct XEFH : public Fun0Impl< OddList<T> > {
   mutable T x;
   XEFH( const T& xx ) : x(xx) {}
   OddList<T> operator()() const {
      ++x;
      return cons( x-1, Fun0<OddList<T> >(1,this) );
   }
};
struct XEnumFrom {
   template <class T>
   struct Sig : FunType<T,List<T> > {};

   template <class T>
   List<T> operator()( const T& x ) const {
      return Fun0<OddList<T> >(1, new XEFH<T>(x) );
   }
};
#else
struct XEFH : public Fun0Impl< OddList<int> > {
   mutable int x;
   XEFH( int xx ) : x(xx) {}
   OddList<int> operator()() const {
      ++x;
      return cons( x-1, Fun0<OddList<int> >(1,this) );
   }
};
struct XEnumFrom : CFunType<int,List<int> > {
   List<int> operator()( int x ) const {
      return Fun0<OddList<int> >(1, new XEFH(x) );
   }
};
#endif
}
typedef Full1<impl::XEnumFrom> EnumFrom;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN EnumFrom enumFrom;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
#ifdef FCPP_TEMPLATE_ENUM
template <class T>
struct XEFTH : public Fun0Impl<OddList<T> > {
   mutable T x;
   T y;
   XEFTH( const T& xx, const T& yy ) : x(xx), y(yy) {}
   OddList<T> operator()() const {
      if( x > y )
         return NIL;
      ++x;
      return cons( x-1, Fun0<OddList<T> >( 1, this ) );
   }
};
struct XEnumFromTo {
   template <class T, class U> struct Sig;
   template <class T>
   struct Sig<T,T> : FunType<T,T,List<T> > {};

   template <class T>
   List<T> operator()( const T& x, const T& y ) const {
      return Fun0<OddList<T> >( 1, new XEFTH<T>(x,y) );
   }
};
#else
struct XEFTH : public Fun0Impl<OddList<int> > {
   mutable int x;
   int y;
   XEFTH( const int& xx, const int& yy ) : x(xx), y(yy) {}
   OddList<int> operator()() const {
      if( x > y )
         return NIL;
      ++x;
      return cons( x-1, Fun0<OddList<int> >( 1, this ) );
   }
};
struct XEnumFromTo : CFunType<int,int,List<int> > {
   List<int> operator()( const int& x, const int& y ) const {
      return Fun0<OddList<int> >( 1, new XEFTH(x,y) );
   }
};
#endif
}
typedef Full2<impl::XEnumFromTo> EnumFromTo;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN EnumFromTo enumFromTo;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
// Not HSP
struct XListUntil {
   template <class P, class F, class T>
   struct Sig : public FunType<P,F,T,List<T> > {};

   template <class Pred, class Unary, class T>
   List<T> operator()( const Pred& p, const Unary& f, const T& x ) const {
      return takeWhile( Compose()(logicalNot,p), iterate(f,x) );
   }
};
}
typedef Full3<impl::XListUntil> ListUntil;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN ListUntil listUntil;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// The "Maybe" type, from Haskell
//////////////////////////////////////////////////////////////////////

struct AUniqueTypeForNothing {};
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN AUniqueTypeForNothing NOTHING;
FCPP_MAYBE_NAMESPACE_CLOSE

template <class T>
class Maybe {
   OddList<T> rep;
public:
   typedef T ElementType;

   Maybe( AUniqueTypeForNothing ) {}
   Maybe() {}                                    // the Nothing constructor
   Maybe( const T& x ) : rep( cons(x,NIL) ) {}   // the Just constructor

   bool is_nothing() const { return null(rep); }
   T value() const { return head(rep); }
};

namespace impl {
   struct XJust {
      template <class T> struct Sig : public FunType<T,Maybe<T> > {};
   
      template <class T>
      typename Sig<T>::ResultType
      operator()( const T& x ) const {
         return Maybe<T>( x );
      }
   };
}
typedef Full1<impl::XJust> Just;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Just just;
FCPP_MAYBE_NAMESPACE_CLOSE

// Haskell's "()" type/value
struct Empty {};
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Empty empty;
FCPP_MAYBE_NAMESPACE_CLOSE

// Haskell has curry() and uncurry() for pairs; we call these hCurry()
// and hUncurry().
namespace impl {
   struct XHCurry {
      template <class F, class X, class Y> struct Sig : public FunType<F,X,Y,
         typename RT<F,std::pair<X,Y> >::ResultType> {};
      template <class F, class X, class Y>
      typename Sig<F,X,Y>::ResultType
      operator()( const F& f, const X& x, const Y& y ) const {
         return f( makePair(x,y) );
      }
   };
}
typedef Full3<impl::XHCurry> HCurry;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN HCurry hCurry;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
   struct XHUncurry {
      template <class F, class P> struct Sig : public FunType<F,P,typename 
         RT<F,typename P::first_type,typename P::second_type>::ResultType> {};
      template <class F, class P>
      typename Sig<F,P>::ResultType
      operator()( const F& f, const P& p ) const {
         return f( fst(p), snd(p) );
      }
   };
}
typedef Full2<impl::XHUncurry> HUncurry;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN HUncurry hUncurry;
FCPP_MAYBE_NAMESPACE_CLOSE

// That's the end of the Haskell stuff; on to made-just-for-FC++

//////////////////////////////////////////////////////////////////////
// Useful effect combinators
//////////////////////////////////////////////////////////////////////
// Here we define some combinators for statement sequencing:
//    before(f,g)(args) = { f(); return g(args); }
//    after(f,g)(args)  = { r = f(args); g(); return r; }
// That is, before() prepends a thunk onto a functoid, and after()
// appends the thunk onto the back of a functoid.  Finally, noOp
// results in a thunk that does nothing, and serves as the left/right
// identity element for before/after thusly:
//    f  =  before( noOp, f )  =  after( f, noOp )
// Note: the effect happens when all of the functoid's expected
// arguments finally arrive (which, thanks to currying, might not be at
// the "next call".  So if g() expects two arguments, note that
//    before( f, g )( x )
// will not call f() now, rather it waits for another argument.

namespace impl {
struct XNoOp : public CFunType<void> {
   void operator()() const {}
};
}
typedef Full0<impl::XNoOp> NoOp;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN NoOp noOp;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
template <class F, class G>
class XBeforer0 : public CFunType<typename RT<G>::ResultType> {
   F f;
   G g;
public:
   XBeforer0( const F& ff, const G& gg ) : f(ff), g(gg) {}
   typename RT<G>::ResultType 
   operator()() const { f(); return g(); }
};
template <class F, class G>
class XBeforer1 { 
   F f;
   G g;
public:
   XBeforer1( const F& ff, const G& gg ) : f(ff), g(gg) {}
   template <class X> struct Sig 
   : public FunType<typename RT<G,X>::Arg1Type,typename RT<G,X>::ResultType> {};
   template <class X>
   typename Sig<X>::ResultType 
   operator()( const X& x ) const { f(); return g(x); }
};
template <class F, class G>
class XBeforer2 { 
   F f;
   G g;
public:
   XBeforer2( const F& ff, const G& gg ) : f(ff), g(gg) {}
   template <class X, class Y> struct Sig 
   : public FunType<typename RT<G,X,Y>::Arg1Type,
                    typename RT<G,X,Y>::Arg2Type,
                    typename RT<G,X,Y>::ResultType> {};
   template <class X, class Y>
   typename Sig<X,Y>::ResultType 
   operator()( const X& x, const Y& y ) const { f(); return g(x,y); }
};
template <class F, class G>
class XBeforer3 { 
   F f;
   G g;
public:
   XBeforer3( const F& ff, const G& gg ) : f(ff), g(gg) {}
   template <class X, class Y, class Z> struct Sig 
   : public FunType<typename RT<G,X,Y,Z>::Arg1Type,
                    typename RT<G,X,Y,Z>::Arg2Type,
                    typename RT<G,X,Y,Z>::Arg3Type,
                    typename RT<G,X,Y,Z>::ResultType> {};
   template <class X, class Y, class Z>
   typename Sig<X,Y,Z>::ResultType 
   operator()( const X& x, const Y& y, const Z& z ) const 
   { f(); return g(x,y,z); }
};

class XBefore {
   template <int i, class F, class G> struct Helper;
   template <class F, class G> 
   struct Helper<0,F,G> {
      typedef Full0<XBeforer0<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull0( XBeforer0<F,G>(f,g) ); }
   };
   template <class F, class G> 
   struct Helper<1,F,G> {
      typedef Full1<XBeforer1<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull1( XBeforer1<F,G>(f,g) ); }
   };
   template <class F, class G> 
   struct Helper<2,F,G> {
      typedef Full2<XBeforer2<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull2( XBeforer2<F,G>(f,g) ); }
   };
   template <class F, class G> 
   struct Helper<3,F,G> {
      typedef Full3<XBeforer3<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull3( XBeforer3<F,G>(f,g) ); }
   };
public:
   template <class F, class G> struct Sig : public FunType<F,G,
      typename Helper<FunctoidTraits<G>::max_args,F,G>::Result> {};
   template <class F, class G>
   typename Sig<F,G>::ResultType operator()( const F& f, const G& g ) const {
      return Helper<FunctoidTraits<G>::max_args,F,G>::go( f, g );
   }
};
}
typedef Full2<impl::XBefore> Before;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Before before;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
template <class F, class G>
class XAfterer0 : public CFunType<typename RT<F>::ResultType> {
   F f;
   G g;
public:
   XAfterer0( const F& ff, const G& gg ) : f(ff), g(gg) {}
   typename RT<F>::ResultType operator()() const 
   { typename RT<F>::ResultType tmp = f(); g();
return tmp; }
};
template <class F, class G>
class XAfterer1 { 
   F f;
   G g;
public:
   XAfterer1( const F& ff, const G& gg ) : f(ff), g(gg) {}
   template <class X> struct Sig 
   : public FunType<typename RT<F,X>::Arg1Type,
                    typename RT<F,X>::ResultType> {};
   template <class X>
   typename Sig<X>::ResultType 
   operator()( const X& x ) const 
   { typename Sig<X>::ResultType tmp = f(x); g(); return tmp; }
};
template <class F, class G>
class XAfterer2 { 
   F f;
   G g;
public:
   XAfterer2( const F& ff, const G& gg ) : f(ff), g(gg) {}
   template <class X, class Y> struct Sig 
   : public FunType<typename RT<F,X,Y>::Arg1Type,
                    typename RT<F,X,Y>::Arg2Type,
                    typename RT<F,X,Y>::ResultType> {};
   template <class X, class Y>
   typename Sig<X,Y>::ResultType 
   operator()( const X& x, const Y& y ) const 
   { typename Sig<X,Y>::ResultType tmp = f(x,y); g(); return tmp; }
};
template <class F, class G>
class XAfterer3 { 
   F f;
   G g;
public:
   XAfterer3( const F& ff, const G& gg ) : f(ff), g(gg) {}
   template <class X, class Y, class Z> struct Sig 
   : public FunType<typename RT<F,X,Y,Z>::Arg1Type,
                    typename RT<F,X,Y,Z>::Arg2Type,
                    typename RT<F,X,Y,Z>::Arg3Type,
                    typename RT<F,X,Y,Z>::ResultType> {};
   template <class X, class Y, class Z>
   typename Sig<X,Y,Z>::ResultType 
   operator()( const X& x, const Y& y, const Z& z ) const 
   { typename Sig<X,Y,Z>::ResultType tmp = f(x,y,z); g(); return tmp; }
};

class XAfter {
   template <int i, class F, class G> struct Helper;
   template <class F, class G> 
   struct Helper<0,F,G> {
      typedef Full0<XAfterer0<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull0( XAfterer0<F,G>(f,g) ); }
   };
   template <class F, class G> 
   struct Helper<1,F,G> {
      typedef Full1<XAfterer1<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull1( XAfterer1<F,G>(f,g) ); }
   };
   template <class F, class G> 
   struct Helper<2,F,G> {
      typedef Full2<XAfterer2<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull2( XAfterer2<F,G>(f,g) ); }
   };
   template <class F, class G> 
   struct Helper<3,F,G> {
      typedef Full3<XAfterer3<F,G> > Result;
      static Result go( const F& f, const G& g ) 
      { return makeFull3( XAfterer3<F,G>(f,g) ); }
   };
public:
   template <class F, class G> struct Sig : public FunType<F,G,
      typename Helper<FunctoidTraits<F>::max_args,F,G>::Result> {};
   template <class F, class G>
   typename Sig<F,G>::ResultType operator()( const F& f, const G& g ) const {
      return Helper<FunctoidTraits<F>::max_args,F,G>::go( f, g );
   }
};
}
typedef Full2<impl::XAfter> After;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN After after;
FCPP_MAYBE_NAMESPACE_CLOSE

// emptify(f)(yadda) == f(yadda), but throws away f's result (even if
//    it's void) and returns Empty instead.  This is useful because
//       length( map( emptify(effectFunctoid), someList ) )
//    is an easy way to do something with each element of someList.
namespace impl {
template <class F>
class XEmptifier0 : public CFunType<Empty> {
   F f;
public:
   XEmptifier0( const F& ff ) : f(ff) {}
   Empty operator()() const { f(); return Empty(); }
};
template <class F>
class XEmptifier1 { 
   F f;
public:
   XEmptifier1( const F& ff ) : f(ff) {}
   template <class X> struct Sig 
   : public FunType<typename RT<F,X>::Arg1Type,Empty> {};
   template <class X>
   typename Sig<X>::ResultType 
   operator()( const X& x ) const { f(x); return Empty(); }
};
template <class F>
class XEmptifier2 { 
   F f;
public:
   XEmptifier2( const F& ff ) : f(ff) {}
   template <class X, class Y> struct Sig 
   : public FunType<typename RT<F,X,Y>::Arg1Type,
                    typename RT<F,X,Y>::Arg2Type,
                    Empty> {};
   template <class X, class Y>
   typename Sig<X,Y>::ResultType 
   operator()( const X& x, const Y& y ) const { f(x,y); return Empty(); }
};
template <class F>
class XEmptifier3 { 
   F f;
public:
   XEmptifier3( const F& ff ) : f(ff) {}
   template <class X, class Y, class Z> struct Sig 
   : public FunType<typename RT<F,X,Y,Z>::Arg1Type,
                    typename RT<F,X,Y,Z>::Arg2Type,
                    typename RT<F,X,Y,Z>::Arg3Type,
                    Empty> {};
   template <class X, class Y, class Z>
   typename Sig<X,Y,Z>::ResultType 
   operator()( const X& x, const Y& y, const Z& z ) const 
   { f(x,y,z); return Empty(); }
};

class XEmptify {
   template <int i, class F> struct Helper;
   template <class F> struct Helper<0,F> {
      typedef Full0<XEmptifier0<F> > Result;
      static Result go( const F& f ) 
      { return makeFull0( XEmptifier0<F>(f) ); }
   };
   template <class F> struct Helper<1,F> {
      typedef Full1<XEmptifier1<F> > Result;
      static Result go( const F& f ) 
      { return makeFull1( XEmptifier1<F>(f) ); }
   };
   template <class F> struct Helper<2,F> {
      typedef Full2<XEmptifier2<F> > Result;
      static Result go( const F& f ) 
      { return makeFull2( XEmptifier2<F>(f) ); }
   };
   template <class F> struct Helper<3,F> {
      typedef Full3<XEmptifier3<F> > Result;
      static Result go( const F& f ) 
      { return makeFull3( XEmptifier3<F>(f) ); }
   };
public:
   template <class F> struct Sig : public FunType<F,
      typename Helper<FunctoidTraits<F>::max_args,F>::Result> {};
   template <class F>
   typename Sig<F>::ResultType operator()( const F& f ) const {
      return Helper<FunctoidTraits<F>::max_args,F>::go( f );
   }
};
}
typedef Full1<impl::XEmptify> Emptify;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Emptify emptify;
FCPP_MAYBE_NAMESPACE_CLOSE

#ifndef FCPP_I_AM_GCC2
namespace impl {
struct XThunkFuncToFunc {
   // tftf(f)(yadda) = f()(yadda)
   // f is a thunk that returns a functoid; this combinator 'hides' the thunk.
   // can be useful to break what would otherwise be infinite recursion.
   template <class F> class TFTFH0 
   : CFunType<typename RT<typename RT<F>::ResultType>::ResultType> {
      F f;
   public:
      TFTFH0( const F& ff ) : f(ff) {}
      template <class X>
      typename RT<typename RT<F>::ResultType>::ResultType
      operator()() const {
         return f()();
      }
   };
   template <class F> class TFTFH1 {
      F f;
   public:
      TFTFH1( const F& ff ) : f(ff) {}
      template <class X> struct Sig : public FunType<
         typename RT<typename RT<F>::ResultType,X>::Arg1Type,
         typename RT<typename RT<F>::ResultType,X>::ResultType   > {};
      template <class X>
      typename Sig<X>::ResultType
      operator()( const X& x ) const {
         return f()( x );
      }
   };
   template <class F> class TFTFH2 {
      F f;
   public:
      TFTFH2( const F& ff ) : f(ff) {}
      template <class X, class Y> struct Sig : public FunType<
         typename RT<typename RT<F>::ResultType,X,Y>::Arg1Type,
         typename RT<typename RT<F>::ResultType,X,Y>::Arg2Type,
         typename RT<typename RT<F>::ResultType,X,Y>::ResultType   > {};
      template <class X, class Y>
      typename Sig<X,Y>::ResultType
      operator()( const X& x, const Y& y ) const {
         return f()( x, y );
      }
   };
   template <class F> class TFTFH3 {
      F f;
   public:
      TFTFH3( const F& ff ) : f(ff) {}
      template <class X, class Y, class Z> struct Sig : public FunType<
         typename RT<typename RT<F>::ResultType,X,Y,Z>::Arg1Type,
         typename RT<typename RT<F>::ResultType,X,Y,Z>::Arg2Type,
         typename RT<typename RT<F>::ResultType,X,Y,Z>::Arg3Type,
         typename RT<typename RT<F>::ResultType,X,Y,Z>::ResultType   > {};
      template <class X, class Y, class Z>
      typename Sig<X,Y,Z>::ResultType
      operator()( const X& x, const Y& y, const Z& z ) const {
         return f()( x, y, z );
      }
   };
   template <int i, class F> struct Helper;
   template <class F> struct Helper<0,F> {
      typedef Full0<TFTFH0<F> > Result;
      static Result go( const F& f )
      { return makeFull0( TFTFH0<F>(f) ); }
   };
   template <class F> struct Helper<1,F> {
      typedef Full1<TFTFH1<F> > Result;
      static Result go( const F& f )
      { return makeFull1( TFTFH1<F>(f) ); }
   };
   template <class F> struct Helper<2,F> {
      typedef Full2<TFTFH2<F> > Result;
      static Result go( const F& f )
      { return makeFull2( TFTFH2<F>(f) ); }
   };
   template <class F> struct Helper<3,F> {
      typedef Full3<TFTFH3<F> > Result;
      static Result go( const F& f )
      { return makeFull3( TFTFH3<F>(f) ); }
   };

   template <class F> struct Sig : public FunType<F, typename Helper<
      FunctoidTraits<typename RT<F>::ResultType>::max_args,F>::Result> {};
   template <class F>
   typename Sig<F>::ResultType
   operator()( const F& f ) const {
      return Helper<FunctoidTraits<typename RT<F>::ResultType>
         ::max_args,F>::go(f);
   }
};
}
typedef Full1<impl::XThunkFuncToFunc> ThunkFuncToFunc;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN ThunkFuncToFunc thunkFuncToFunc;
FCPP_MAYBE_NAMESPACE_CLOSE
#endif
   
//////////////////////////////////////////////////////////////////////
// uncurry
//////////////////////////////////////////////////////////////////////
// Sometimes FC++ expressions result in functoids where, for example,
//    f(x)(y)
// is legal but
//    f(x,y)
// is not, owing to the fact that (in the example) f() is a one-argument
// functoid the returns another one-argument functoid, which is 
// different from a two-argument functoid.  (In Haskell, the two types
// are identical.)  uncurry() wraps a functoid in a magical cloak which
// splits up its arguments, so that, for example,
//    uncurry(f)(x,y,z) = f(x)(y)(z)
// It rarely arises that you need this, but when you do, you can't live
// without it.
//
// FIX THIS: Note that uncurry() (as well as curryN()) means something 
// different in FC++ than what it does in Haskell.

namespace impl {
template <class F>
class XUncurryable : public SmartFunctoid3 {
   F f;
public:
   XUncurryable( const F& ff ) : f(ff) { 
      FunctoidTraits<F>::template ensure_accepts<1>::args();
   }

   template <class X, class Y=Void, class Z=Void, class Dummy=Void>
   struct Sig : public FunType<typename RT<F,X>::Arg1Type,
            typename RT<typename RT<F,X>::ResultType,Y>::Arg1Type,
typename RT<typename RT<typename RT<F,X>::ResultType,Y>::ResultType,Z>
   ::Arg1Type,
typename RT<typename RT<typename RT<F,X>::ResultType,Y>::ResultType,Z>
   ::ResultType> {};

   template <class X, class Y, class Dummy>
   struct Sig<X,Y,Void,Dummy> : public FunType<typename RT<F,X>::Arg1Type,
            typename RT<typename RT<F,X>::ResultType,Y>::Arg1Type,
            typename RT<typename RT<F,X>::ResultType,Y>::ResultType> {};

   template <class X, class Dummy>
   struct Sig<X,Void,Void,Dummy> : public FunType<typename RT<F,X>::Arg1Type,
            typename RT<F,X>::ResultType> {};

   template <class X, class Y, class Z>
   typename Sig<X,Y,Z>::ResultType
   operator()( const X& x, const Y& y, const Z& z ) const { return f(x)(y)(z); }

   template <class X, class Y>
   typename Sig<X,Y>::ResultType
   operator()( const X& x, const Y& y ) const { return f(x)(y); }

   template <class X>
   typename Sig<X>::ResultType
   operator()( const X& x ) const { return f(x); }
#ifdef FCPP_ENABLE_LAMBDA
   typedef XUncurryable This;
   template <class A> typename fcpp_lambda::BracketCallable<This,A>::Result
   operator[]( const A& a ) const
   { return fcpp_lambda::BracketCallable<This,A>::go( *this, a ); }
#endif
};
struct XUncurry {
   template <class F>
   struct Sig : FunType<F,XUncurryable<F> > {};

   template <class F>
   XUncurryable<F> operator()( const F& f ) const { return XUncurryable<F>(f); }
};
}
typedef Full1<impl::XUncurry> Uncurry;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Uncurry uncurry;
FCPP_MAYBE_NAMESPACE_CLOSE

// Uncurry0 is truly a different case from uncurry:
//    uncurry0(f)(x,y,z) = f()(x)(y)(z)
namespace impl {
template <class F>
class XUncurryable0 {
   F f;
public:
   XUncurryable0( const F& ff ) : f(ff) { 
      FunctoidTraits<F>::template ensure_accepts<0>::args();
   }

   template <class X> struct Sig
   : public FunType<typename RT<typename RT<F>::ResultType,X>::Arg1Type,
            typename RT<typename RT<F>::ResultType,X>::ResultType> {};

   template <class X>
   typename Sig<X>::ResultType
   operator()( const X& x ) const { return f()(x); }
};
struct XUncurry0 {
   template <class F>
   struct Sig : FunType<F,XUncurryable<XUncurryable0<F> > > {};

   template <class F>
   XUncurryable<XUncurryable0<F> > operator()( const F& f ) const 
   { return ::fcpp::uncurry( XUncurryable0<F>(f) ); }
};
}
typedef Full1<impl::XUncurry0> Uncurry0;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Uncurry0 uncurry0;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// duplicate() and ignore()
//////////////////////////////////////////////////////////////////////
// duplicate() duplicates the first argument of a functoid, whereas
// ignore() ignores it:
//    duplicate(f)(x)    = f(x)(x)
//    ignore(f)(x)(args) = f(args)

namespace impl {
template <class F>
class XDuplicater {
   F f;
public:
   XDuplicater( const F& ff ) : f(ff) {}

   template <class X>
   struct Sig : public FunType<X,typename RT<typename RT<F,X>::ResultType,
                                             X>::ResultType> {};
   template <class X>
   typename Sig<X>::ResultType
   operator()( const X& x ) const {
      return f(x)(x);
   }
};
struct XDuplicate {
   template <class F>
   struct Sig : public FunType<F,Full1<XDuplicater<F> > > {};

   template <class F>
   Full1<XDuplicater<F> > operator()( const F& f ) const {
      return makeFull1( XDuplicater<F>(f) );
   }
};
}
typedef Full1<impl::XDuplicate> Duplicate;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Duplicate duplicate;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
template <class F>
class XIgnorer1 {
   F f;
public:
   XIgnorer1( const F& ff ) : f(ff) {}

   template <class X> 
   struct Sig : public FunType<X,typename RT<F>::ResultType> {};

   template <class X>
   typename Sig<X>::ResultType operator()( const X& ) const {
      return f();
   }
};
template <class F>
class XIgnorer2 {
   F f;
public:
   XIgnorer2( const F& ff ) : f(ff) {}

   template <class X, class Y> 
   struct Sig : public FunType<X,
      typename RT<F,Y>::Arg1Type,typename RT<F,Y>::ResultType> {};

   template <class X, class Y>
   typename Sig<X,Y>::ResultType 
   operator()( const X&, const Y& y ) const {
      return f(y);
   }
};
template <class F>
class XIgnorer3 {
   F f;
public:
   XIgnorer3( const F& ff ) : f(ff) {}

   template <class X, class Y, class Z> 
   struct Sig : public FunType<X,
      typename RT<F,Y,Z>::Arg1Type,
      typename RT<F,Y,Z>::Arg2Type,
      typename RT<F,Y,Z>::ResultType> {};

   template <class X, class Y, class Z>
   typename Sig<X,Y,Z>::ResultType 
   operator()( const X&, const Y& y, const Z& z ) const {
      return f(y,z);
   }
};
class XIgnore {
   template<int n, class F> struct Helper;
   template<class F> struct Helper<0,F> {
      typedef Full1< XIgnorer1<F> > Result;
      static inline Result go( const F& f ) {
         return makeFull1( XIgnorer1<F>(f) );
      }
   };
   template<class F> struct Helper<1,F> {
      typedef Full2< XIgnorer2<F> > Result;
      static inline Result go( const F& f ) {
         return makeFull2( XIgnorer2<F>(f) );
      }
   };
   template<class F> struct Helper<2,F> {
      typedef Full3< XIgnorer3<F> > Result;
      static inline Result go( const F& f ) {
         return makeFull3( XIgnorer3<F>(f) );
      }
   };
public:
   template <class F>
   struct Sig : public FunType<F,
      typename Helper<FunctoidTraits<F>::max_args,F>::Result> {};

   template <class F>
   typename Sig<F>::ResultType operator()( const F& f ) const {
      return Helper<FunctoidTraits<F>::max_args,F>::go(f);
   }
};
}
typedef Full1<impl::XIgnore> Ignore;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Ignore ignore;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// ConstructN
//////////////////////////////////////////////////////////////////////
// C++ constructors are not functions, and thus cannot easily be turned
// into functoids.  So we write these helpers.  For example,
//    construct2<Foo>()(x,y) = Foo(x,y)    // Foo is a type name
// Note also that construct1 also serves the role of an explicit
// converter; if Foos (or any type) can be converted into Bars, then we 
// could use a construct1 functoid to capture the conversion function:
//    construct1<Bar>()       // functoid that converts arg into a Bar
//    construct1<Bar>()(x) = Bar(x)
// Note also that these are template functions returning full functoids,
// and we have template structs which name the types of the functoids.

namespace impl {
template <class T>
struct XConstruct0 : public CFunType<T> {
   T operator()() const { return T(); }
};
}
template <class T> Full0<impl::XConstruct0<T> > construct0() 
{ return makeFull0( impl::XConstruct0<T>() ); }
template <class T> struct Construct0
{ typedef Full0<impl::XConstruct0<T> > Type; };

namespace impl {
template <class T>
struct XConstruct1 {
   template <class X> struct Sig : FunType<X,T> {};
   template <class X>
   T operator()( const X& x ) const { return T(x); }
};
}
template <class T> Full1<impl::XConstruct1<T> > construct1() 
{ return makeFull1( impl::XConstruct1<T>() ); }
template <class T> struct Construct1
{ typedef Full1<impl::XConstruct1<T> > Type; };
   
namespace impl {
template <class T>
struct XConstruct2 {
   template <class X, class Y> struct Sig : FunType<X,Y,T> {};
   template <class X, class Y>
   T operator()( const X& x, const Y& y ) const { return T(x,y); }
};
}
template <class T> Full2<impl::XConstruct2<T> > construct2() 
{ return makeFull2( impl::XConstruct2<T>() ); }
template <class T> struct Construct2
{ typedef Full2<impl::XConstruct2<T> > Type; };
   
namespace impl {
template <class T>
struct XConstruct3 {
   template <class X, class Y, class Z> struct Sig : FunType<X,Y,Z,T> {};
   template <class X, class Y, class Z>
   T operator()( const X& x, const Y& y, const Z& z ) const { return T(x,y,z); }
};
}
template <class T> Full3<impl::XConstruct3<T> > construct3() 
{ return makeFull3( impl::XConstruct3<T>() ); }
template <class T> struct Construct3
{ typedef Full3<impl::XConstruct3<T> > Type; };
   
//////////////////////////////////////////////////////////////////////
// NewN works like ConstructN but "new"s it and returns the ptr
//////////////////////////////////////////////////////////////////////
namespace impl {
template <class T>
struct XNew0 : public CFunType<T*> {
   T* operator()() const { return new T(); }
};
}
template <class T> Full0<impl::XNew0<T> > new0() 
{ return makeFull0( impl::XNew0<T>() ); }
template <class T> struct New0
{ typedef Full0<impl::XNew0<T> > Type; };

namespace impl {
template <class T>
struct XNew1 {
   template <class X> struct Sig : FunType<X,T*> {};
   template <class X>
   T* operator()( const X& x ) const { return new T(x); }
};
}
template <class T> Full1<impl::XNew1<T> > new1() 
{ return makeFull1( impl::XNew1<T>() ); }
template <class T> struct New1
{ typedef Full1<impl::XNew1<T> > Type; };
   
namespace impl {
template <class T>
struct XNew2 {
   template <class X, class Y> struct Sig : FunType<X,Y,T*> {};
   template <class X, class Y>
   T* operator()( const X& x, const Y& y ) const { return new T(x,y); }
};
}
template <class T> Full2<impl::XNew2<T> > new2() 
{ return makeFull2( impl::XNew2<T>() ); }
template <class T> struct New2
{ typedef Full2<impl::XNew2<T> > Type; };
   
namespace impl {
template <class T>
struct XNew3 {
   template <class X, class Y, class Z> struct Sig : FunType<X,Y,Z,T*> {};
   template <class X, class Y, class Z>
   T* operator()( const X& x, const Y& y, const Z& z ) const 
   { return new T(x,y,z); }
};
}
template <class T> Full3<impl::XNew3<T> > new3() 
{ return makeFull3( impl::XNew3<T>() ); }
template <class T> struct New3
{ typedef Full3<impl::XNew3<T> > Type; };

} // end namespace fcpp

#ifdef FCPP_ENABLE_LAMBDA
#include "monad.h"
#endif

#endif
