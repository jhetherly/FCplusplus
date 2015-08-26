//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_LIST_DOT_H
#define FCPP_LIST_DOT_H

///////////////////////////////////////////////////////////////////////////
// Here we implement (lazy) lists in the List class.  There are also a
// number of functions associated with lists:
//  - head, tail, cons, cat, null
///////////////////////////////////////////////////////////////////////////

// Order-of-initialization debugging help
// Note that you only might need this with the FCPP_1_3_LIST_IMPL version
#ifdef FCPP_OOI_DEBUG
#include <iostream>
#include <typeinfo>
#endif

#include <exception>
#include <new>
#include <cstdlib>

#include "reuse.h"

namespace fcpp {

struct fcpp_exception : public std::exception {
   const char* s;
   fcpp_exception( const char* ss ) : s(ss) {}
   const char* what() const throw() { return s; }
};

namespace impl {
struct XCons; struct XHead; struct XTail; struct XNull; struct XCat;

struct CacheEmpty {};
struct CacheDummy {};

template <class T> struct Cache;
template <class T> struct OddList;
template <class T> struct ListIterator;
template <class T, class It> struct ListItHelp;
template <class U,class F> struct cvt;
template <class T, class F, class R> struct ListHelp;
template <class T> Cache<T>* xempty_helper();
template <class T, class F, class R> struct ConsHelp;

struct ListRaw {};

template <class T> 
class List {
   IRef<Cache<T> > rep;   // never NIL, unless an empty OddList

   template <class U> friend class Cache;
   template <class U> friend class OddList;
   template <class U, class F, class R> friend struct ConsHelp;
   template <class U,class F> friend struct cvt;

   List( const IRef<Cache<T> >& p ) : rep(p) {}
   List( ListRaw, Cache<T>* p ) : rep(p) {}

   bool priv_isEmpty() const { 
      return rep->cache().second.rep == Cache<T>::XNIL(); 
   }
   T priv_head() const { 
#ifdef FCPP_DEBUG
      if( priv_isEmpty() )
         throw fcpp_exception("Tried to take head() of empty List");
#endif
      return rep->cache().first(); 
   }
   List<T> priv_tail() const { 
#ifdef FCPP_DEBUG
      if( priv_isEmpty() )
         throw fcpp_exception("Tried to take tail() of empty List");
#endif
      return rep->cache().second; 
   }
public:
   typedef T ElementType;

   List( AUniqueTypeForNil ) : rep( Cache<T>::XEMPTY() ) {}
   List() : rep( Cache<T>::XEMPTY() ) {}

   template <class F>  // works on both ()->OddList and ()->List 
   List( const F& f )
   : rep( ListHelp<T,F,typename F::ResultType>()(f) ) {}

   // Note:  this constructor is still part of List and thus still lazy;
   // the iterators may not get evaluated until much later.  This is a
   // feature, not a bug.  So if the iterators are going to be invalidated
   // before you finish using the list, then you'd better force evaluation 
   // of the entire list before the iterators go away.
   template <class It>
   List( const It& begin, const It& end )
   : rep( new Cache<T>( ListItHelp<T,It>(begin,end) ) ) {}

   List( const OddList<T>& e )
   : rep( (e.second.rep != Cache<T>::XNIL()) ? 
          new Cache<T>(e) : Cache<T>::XEMPTY() ) {}

#ifdef FCPP_SAFE_LIST
   // Long lists create long recursions of destructors that blow the
   // stack.  So we have an iterative destructor.  It is quite tricky to
   // get right.  The danger is that, when "bypassing" a node to be
   // unlinked and destructed, that node's 'next' pointer is, in fact, a
   // List object, whose destructor will be called.  As a result, as you
   // bypass a node, you need to see if its refC is down to 1, and if
   // so, mutate its next pointer so that when its destructor is called,
   // it won't cause a recursive cascade.  
   ~List() {
      while( rep != Cache<T>::XNIL() && rep != Cache<T>::XBAD() ) {
         if( rep->refC == 1 ) {
            // This is a rotate(), but this sequence is actually faster
            // than rotate(), so we do it explicitly
            IRef<Cache<T> > tmp( rep );
            rep = rep->val.second.rep;
            tmp->val.second.rep = Cache<T>::XNIL();
         }
         else
            rep = rep->val.second.rep;
      }
   }
#endif

   operator bool() const { return !priv_isEmpty(); }
   const OddList<T>& force() const { return rep->cache(); }
   const List<T>& delay() const { return *this; }
   // Note: force returns a reference; implicit conversion now returns a copy.
   operator OddList<T>() const { return force(); }

   // VC++7.1 says line below makes "return l;" (when l is a List and
   // function returns an OddList) illegal, and I think it's right.
   //operator const OddList<T>&() const { return force(); }

   T head() const { return priv_head(); }
   List<T> tail() const { return priv_tail(); }

   // The following helps makes List almost an STL "container"
   typedef T value_type;
   typedef ListIterator<T> const_iterator;
   typedef const_iterator iterator;         // List is immutable
   iterator begin() const { return ListIterator<T>( *this ); }
   iterator end() const   { return ListIterator<T>(); }
};

struct OddListDummyX {};
struct OddListDummyY {};

namespace misc_types {
   struct Argh { virtual int f() {return 0;} };
   typedef int (*PtrToFxn)();
   typedef int (Argh::*PtrToMember);
   typedef int (Argh::*PtrToMemberFxn)();
}

template <class T> 
class OddList {
   // We need to make sure that "fst" is properly aligned to hold a "T"
   // object, so we do the 'standard' hack.
   union {
      unsigned char fst[ sizeof(T) ];   // The real variable
      // a bunch of dummies of every conceivable type
      long z1, *pz1; long double z2, *pz2; void *z3, **pz3; 
      misc_types::PtrToFxn z4, *pz4;
      misc_types::Argh *pz5;
      int z6, *pz6; char z7, *pz7; double z8, *pz8; 
      misc_types::PtrToMember z9, *pz9;
      misc_types::PtrToMemberFxn z10, *pz10;
   };

   const T& first() const { 
      return *static_cast<const T*>(static_cast<const void*>(&fst)); 
   }
   T& first() { 
      return *static_cast<T*>(static_cast<void*>(&fst));
   }
   List<T>  second;   // If XNIL, then this OddList is NIL

   template <class U> friend class List;
   template <class U> friend class Cache;

   OddList( OddListDummyX ) : second( Cache<T>::XNIL() ) { }

   OddList( OddListDummyY )
   : second( Cache<T>::XBAD() ) { }

   void init( const T& x ) {
      new (static_cast<void*>(&fst)) T(x);
   } 

   bool fst_is_valid() const {
      if( second.rep != Cache<T>::XNIL() )
         if( second.rep != Cache<T>::XBAD() )
            return true;
      return false;
   }

   bool priv_isEmpty() const { return second.rep == Cache<T>::XNIL(); }
   T priv_head() const { 
#ifdef FCPP_DEBUG
      if( priv_isEmpty() )
         throw fcpp_exception("Tried to take head() of empty OddList");
#endif
      return first(); 
   }
   List<T> priv_tail() const { 
#ifdef FCPP_DEBUG
      if( priv_isEmpty() )
         throw fcpp_exception("Tried to take tail() of empty OddList");
#endif
      return second; 
   }

public:
   typedef T ElementType;

   OddList() : second( Cache<T>::XNIL() ) { }
   OddList( AUniqueTypeForNil ) : second( Cache<T>::XNIL() ) { }
   OddList( const T& x, const List<T>& y ) : second(y) { init(x); }
   OddList( const T& x, AUniqueTypeForNil ) 
   : second(Cache<T>::XEMPTY()) { init(x); }

   OddList( const OddList<T>& x ) : second(x.second) {
      if( fst_is_valid() ) {
         init( x.first() );
      }
   }

   OddList<T>& operator=( const OddList<T>& x ) {
      if( this == &x ) return *this;  
      if( fst_is_valid() ) {
         if( x.fst_is_valid() )
            first() = x.first();
         else
            first().~T();
      }
      else {
         if( x.fst_is_valid() )
            init( x.first() );
      }
      second = x.second;
      return *this;
   }
      
   ~OddList() {
      if( fst_is_valid() ) {
         first().~T(); 
      }
   }

   operator bool() const { return !priv_isEmpty(); }
   const OddList<T>& force() const { return *this; }
   List<T> delay() const { return List<T>(*this); }

   T head() const { return priv_head(); }
   List<T> tail() const { return priv_tail(); }
};

// This converts ()->List<T> to ()->OddList<T>.
// In other words, here is the 'extra work' done when using the
// unoptimized interface.
template <class U,class F>
struct cvt : public CFunType<OddList<U> > {
   F f;
   cvt( const F& ff ) : f(ff) {}
   OddList<U> operator()() const {
      List<U> l = f();
      return l.force();
   }
};

// I malloc a RefCountType to hold the refCount and init it to 1 to ensure the
// refCount will never get to 0, so the destructor-of-global-object
// order at the end of the program is a non-issue.  In other words, the
// memory allocated here is only reclaimed by the operating system.
template <class T> 
Cache<T>* xnil_helper() {
   void *p = std::malloc( sizeof(RefCountType) );
#ifdef FCPP_OOI_DEBUG
   std::cout << "making a nil/bad:" << typeid(T).name() 
             << " at address " << p << std::endl;
#endif
   *((RefCountType*)p) = 1;
   return static_cast<Cache<T>*>( p );
}

template <class T> 
Cache<T>* xempty_helper() {
#ifdef FCPP_1_3_LIST_IMPL
   (void) Cache<T>::xnil;   // Make sure xnil exists before moving forward
#endif
   return new Cache<T>( CacheEmpty() );
}

template <class T> 
class Cache {
   RefCountType refC;
   mutable Fun0<OddList<T> >   fxn;
   mutable OddList<T>          val;
   // val.second.rep can be XBAD, XNIL, or a valid ptr
   //  - XBAD: val is invalid (fxn is valid)
   //  - XNIL: this is the empty list
   //  - anything else: val.first() is head, val.second is tail()

   // Caches are not copyable or assignable
   Cache( const Cache<T>& );
   void operator=( Cache<T> );

   // This functoid should never be called; it represents a
   // self-referent Cache, which should be impossible under the current
   // implementation.  Nonetheless, we need a 'dummy' function object to
   // represent invalid 'fxn's (val.second.rep!=XBAD), and this
   // implementation seems to be among the most reasonable.
   struct blackhole_helper : CFunType< OddList<T> > {
      OddList<T> operator()() const {
         throw fcpp_exception("You have entered a black hole.");
      }
   };
#ifdef FCPP_1_3_LIST_IMPL
   static IRef<Cache<T> > xnil, xbad;
   static IRef<Cache<T> > xempty;
#endif

   // Don't get rid of these XFOO() functions; they impose no overhead,
   // and provide a useful place to add debugging code for tracking down
   // before-main()-order-of-initialization problems.
   static const IRef<Cache<T> >& XEMPTY() {
#ifndef FCPP_1_3_LIST_IMPL
      static IRef<Cache<T> > xempty( xempty_helper<T>() );
#endif
#ifdef FCPP_OOI_DEBUG
      static bool b = true;
      if(b) {
         std::cout << "access xempty:" << typeid(T).name() << std::endl;
         b = false;
      }
#endif
      return xempty;
   }
   static const IRef<Cache<T> >& XNIL() {    // this list is nil
#ifndef FCPP_1_3_LIST_IMPL
      static IRef<Cache<T> > xnil( xnil_helper<T>() );
#endif
#ifdef FCPP_OOI_DEBUG
      static bool b = true;
      if(b) {
         std::cout << "access xnil:" << typeid(T).name() << std::endl;
         b = false;
      }
#endif
      return xnil;
   }
   static const IRef<Cache<T> >& XBAD() {    // the pair is invalid; use fxn
#ifndef FCPP_1_3_LIST_IMPL
      static IRef<Cache<T> > xbad( xnil_helper<T>() );
#endif
#ifdef FCPP_OOI_DEBUG
      static bool b = true;
      if(b) {
         std::cout << "access xbad:" << typeid(T).name() << std::endl;
         b = false;
      }
#endif
      return xbad;
   }
   static Fun0<OddList<T> > the_blackhole;
   static Fun0<OddList<T> >& blackhole() {
#ifndef FCPP_1_3_LIST_IMPL
      static Fun0<OddList<T> > the_blackhole( makeFun0( blackhole_helper() ) );
#endif
      return the_blackhole;
   }

   OddList<T>& cache() const {
      if( val.second.rep == XBAD() ) {
         val = fxn();
         fxn = blackhole();
      }
      return val;
   }

   template <class U> friend class List;
   template <class U> friend class OddList;
   template <class U, class F, class R> friend struct ConsHelp;
   template <class U,class F> friend struct cvt;
   template <class U, class F, class R> friend struct ListHelp;
   template <class U> friend Cache<U>* xempty_helper();

   Cache( CacheEmpty ) : refC(0), fxn(blackhole()), val() {}
   Cache( const OddList<T>& x ) : refC(0), fxn(blackhole()), val(x) {}
   Cache( const T& x, const List<T>& l ) : refC(0),fxn(blackhole()),val(x,l) {}
   Cache( CacheDummy ) : refC(0), fxn(blackhole()), val( OddListDummyX() ) {}

   Cache( const Fun0<OddList<T> >& f )
   : refC(0), fxn(f), val( OddListDummyY() ) {}

   template <class F>
   Cache( const F& f )    // ()->OddList
   : refC(0), fxn(makeFun0(f)), val( OddListDummyY() ) {}

   // This is for ()->List<T> to ()->OddList<T>
   struct CvtFxn {};
   template <class F>
   Cache( CvtFxn, const F& f )    // ()->List
   : refC(0), fxn(makeFun0(cvt<T,F>(f))), val( OddListDummyY() ) {}

public:
   void incref() { ++refC; }
   void decref() { if (!--refC) delete this; }
};

#ifdef FCPP_1_3_LIST_IMPL
template <class T>
Fun0<OddList<T> > Cache<T>::the_blackhole( makeFun0( blackhole_helper() ) );

template <class T> IRef<Cache<T> > Cache<T>::xnil( xnil_helper<T>() );
template <class T> IRef<Cache<T> > Cache<T>::xbad( xnil_helper<T>() );
template <class T> IRef<Cache<T> > Cache<T>::xempty( xempty_helper<T>() );
#endif

// Rest of List's stuff

template <class T, class F> struct ListHelp<T,F,List<T> > {
   IRef<Cache<T> > operator()( const F& f ) const {
      return IRef<Cache<T> >(new Cache<T>(Cache<T>::CvtFxn(),f));
   }
};
template <class T, class F> struct ListHelp<T,F,OddList<T> > {
   IRef<Cache<T> > operator()( const F& f ) const {
      return IRef<Cache<T> >(new Cache<T>(f));
   }
};

template <class T, class It>
struct ListItHelp : public CFunType<OddList<T> > {
   It begin, end;
   ListItHelp( const It& b, const It& e ) : begin(b), end(e) {}
   OddList<T> operator()() const;
};
   
template <class T>
#ifdef FCPP_NO_STD_ITER
class ListIterator : public std::input_iterator<T,ptrdiff_t> {
#else
class ListIterator : public std::iterator<std::input_iterator_tag,T,ptrdiff_t> {
#endif
   List<T> l;
   bool is_nil;
   void advance() {
      l = l.tail();
      if( !l )
         is_nil = true;
   }
   class Proxy {  // needed for operator->
      const T x;
      friend class ListIterator;
      Proxy( const T& xx ) : x(xx) {}
   public:
      const T* operator->() const { return &x; }
   };
public:
   ListIterator() : l(), is_nil(true) {}
   explicit ListIterator( const List<T>& ll ) : l(ll), is_nil(!ll) {}

   const T operator*() const { return l.head(); }
   const Proxy operator->() const { return Proxy(l.head()); }
   ListIterator<T>& operator++() {
      advance();
      return *this;
   }
   const ListIterator<T> operator++(int) {
      ListIterator<T> i( *this );
      advance();
      return i;
   }
   bool operator==( const ListIterator<T>& i ) const {
      return is_nil && i.is_nil;
   }
   bool operator!=( const ListIterator<T>& i ) const {
      return ! this->operator==(i);
   }
};
}

using impl::List;
using impl::OddList;
using impl::ListIterator;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

namespace impl{
struct XHead {
   template <class LT>
   struct Sig : public FunType<LT,typename LT::ElementType> {};

   template <class T>
   T operator()( const List<T>& l ) const {
      return l.head();
   }
   template <class T>
   T operator()( const OddList<T>& l ) const {
      return l.head();
   }
};
}
typedef Full1<impl::XHead> Head;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Head head;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XTail {
   template <class LT>
   struct Sig : public FunType<LT,List<typename LT::ElementType> > {};

   template <class T>
   List<T> operator()( const List<T>& l ) const {
      return l.tail();
   }
   template <class T>
   List<T> operator()( const OddList<T>& l ) const {
      return l.tail();
   }
};
}
typedef Full1<impl::XTail> Tail;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Tail tail;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XNull {
   template <class LT>
   struct Sig : public FunType<LT,bool> {};

   template <class T>
   bool operator()( const List<T>& l ) const {
      return !l;
   }
   template <class T>
   bool operator()( const OddList<T>& l ) const {
      return !l;
   }
};
}
typedef Full1<impl::XNull> Null;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Null null;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
template <class T, class F> struct ConsHelp<T,F,List<T> > {
   OddList<T> operator()( const T& x, const F& f ) const {
      return OddList<T>(x, List<T>(
         IRef<Cache<T> >(new Cache<T>(Cache<T>::CvtFxn(),f))));
   }
};
template <class T, class F> struct ConsHelp<T,F,OddList<T> > {
   OddList<T> operator()( const T& x, const F& f ) const {
      return OddList<T>(x, List<T>( ListRaw(), new Cache<T>(f) ));
   }
};
struct XCons {
   template <class T, class L>
   struct Sig : public FunType<T,L,OddList<T> > {};

   template <class T>
   OddList<T> operator()( const T& x, const List<T>& l ) const {
      return OddList<T>(x,l);
   }
   template <class T>
   OddList<T> operator()( const T& x, const OddList<T>& l ) const {
      return OddList<T>(x,l);
   }
   template <class T>
   OddList<T> operator()( const T& x, const AUniqueTypeForNil& ) const {
      return OddList<T>(x,NIL);
   }

   template <class T, class F>
   OddList<T> operator()( const T& x, const F& f ) const {
      return ConsHelp<T,F,typename F::ResultType>()(x,f);
   }
};
}
typedef Full2<impl::XCons> Cons;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Cons cons;
FCPP_MAYBE_NAMESPACE_CLOSE

template <class T, class It>
OddList<T> impl::ListItHelp<T,It>::operator()() const {
   if( begin == end ) return NIL;
   It tmp = begin;
   T x( *begin );
   return cons( x, ListItHelp<T,It>( ++tmp, end ) );
}

namespace impl {
class XCat {
   // The Intel compiler doesn't like it when I overload this function,
   // so I just used class template partial specialization in a nested
   // helper class to code around it.
   template <class L, class M>
   struct Helper : public CFunType<L,M,OddList<typename L::ElementType> > {
      OddList<typename L::ElementType> 
      operator()( const L& l, const M& m, 
             Reuser2<Inv,Var,Inv,Helper,List<typename L::ElementType>,M>
             r = NIL ) const {
         if( null(l) ) 
            return m().force();
         else
            return cons( head(l), r( *this, tail(l), m ) );
      }
   };
   template <class L, class T>
   struct Helper<L,List<T> >
   : public CFunType<L,List<T>,OddList<typename L::ElementType> > {
      OddList<T> 
      operator()( const L& l, const List<T>& m,
             Reuser2<Inv,Var,Inv,Helper,List<typename L::ElementType>,List<T> >
             r = NIL ) const {
         if( null(l) ) 
            return m.force();
         else
            return cons( head(l), r( *this, tail(l), m ) );
      }
   };
   template <class L, class T>
   struct Helper<L,OddList<T> > 
   : public CFunType<L,OddList<T>,OddList<typename L::ElementType> > {
      OddList<T> 
      operator()( const L& l, const OddList<T>& m,
           Reuser2<Inv,Var,Inv,Helper,List<typename L::ElementType>,OddList<T> >
           r = NIL ) const {
         if( null(l) ) 
            return m;
         else
            return cons( head(l), r( *this, tail(l), m ) );
      }
   };
   template <class L>
   struct Helper<L,AUniqueTypeForNil> 
   : public CFunType<L,AUniqueTypeForNil,OddList<typename L::ElementType> > {
      OddList<typename L::ElementType> 
      operator()( const L& l, const AUniqueTypeForNil& ) const {
         return l;
      }
   };
public:
   template <class L, class M>
   struct Sig : public FunType<L,M,OddList<typename L::ElementType> > {};

   // Note: first arg must be a list, but second arg can be either a list 
   // or a function that returns a list.
   template <class L, class M>
   OddList<typename L::ElementType> 
   operator()( const L& l, const M& m ) const {
      return Helper<L,M>()(l,m);
   }
};
}
typedef Full2<impl::XCat> Cat;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Cat cat;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XDelay {
   template <class L>
   struct Sig : public FunType<L,List<typename L::ElementType> > {};

   template <class L>
   List<typename L::ElementType> operator()( const L& l ) const {
      return l.delay();
   }
};
}
typedef Full1<impl::XDelay> Delay;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Delay delay;
FCPP_MAYBE_NAMESPACE_CLOSE

namespace impl {
struct XForce {
   template <class L>
   struct Sig : public FunType<L,OddList<typename L::ElementType> > {};

   template <class L>
   OddList<typename L::ElementType> operator()( const L& l ) const {
      return l.force();
   }
};
}
typedef Full1<impl::XForce> Force;
FCPP_MAYBE_NAMESPACE_OPEN
FCPP_MAYBE_EXTERN Force force;
FCPP_MAYBE_NAMESPACE_CLOSE

//////////////////////////////////////////////////////////////////////
// op== and op<, overloaded for all combos of List, OddList, and NIL
//////////////////////////////////////////////////////////////////////

template <class T>
bool operator==( const OddList<T>& a, AUniqueTypeForNil ) {
   return null(a);
}
template <class T>
bool operator==( const List<T>& a, AUniqueTypeForNil ) {
   return null(a);
}
template <class T>
bool operator==( AUniqueTypeForNil, const OddList<T>& a ) {
   return null(a);
}
template <class T>
bool operator==( AUniqueTypeForNil, const List<T>& a ) {
   return null(a);
}
template <class T>
bool operator==( const List<T>& a, const List<T>& b ) {
   if( null(a) && null(b) )
      return true;
   if( null(a) || null(b) )
      return false;
   return (head(a)==head(b)) && (tail(a)==tail(b));
}
template <class T>
bool operator==( const OddList<T>& a, const OddList<T>& b ) {
   if( null(a) && null(b) )
      return true;
   if( null(a) || null(b) )
      return false;
   return (head(a)==head(b)) && (tail(a)==tail(b));
}
template <class T>
bool operator==( const List<T>& a, const OddList<T>& b ) {
   if( null(a) && null(b) )
      return true;
   if( null(a) || null(b) )
      return false;
   return (head(a)==head(b)) && (tail(a)==tail(b));
}
template <class T>
bool operator==( const OddList<T>& a, const List<T>& b ) {
   if( null(a) && null(b) )
      return true;
   if( null(a) || null(b) )
      return false;
   return (head(a)==head(b)) && (tail(a)==tail(b));
}

template <class T>
bool operator<( const List<T>& a, const List<T>& b ) {
   if( null(a) && !null(b) )  return true;
   if( null(b) )              return false;
   if( head(b) < head(a) )    return false;
   if( head(a) < head(b) )    return true;
   return (tail(a) < tail(b));
}
template <class T>
bool operator<( const OddList<T>& a, const List<T>& b ) {
   if( null(a) && !null(b) )  return true;
   if( null(b) )              return false;
   if( head(b) < head(a) )    return false;
   if( head(a) < head(b) )    return true;
   return (tail(a) < tail(b));
}
template <class T>
bool operator<( const List<T>& a, const OddList<T>& b ) {
   if( null(a) && !null(b) )  return true;
   if( null(b) )              return false;
   if( head(b) < head(a) )    return false;
   if( head(a) < head(b) )    return true;
   return (tail(a) < tail(b));
}
template <class T>
bool operator<( const OddList<T>& a, const OddList<T>& b ) {
   if( null(a) && !null(b) )  return true;
   if( null(b) )              return false;
   if( head(b) < head(a) )    return false;
   if( head(a) < head(b) )    return true;
   return (tail(a) < tail(b));
}
template <class T>
bool operator<( const OddList<T>&, AUniqueTypeForNil ) {
   return false;
}
template <class T>
bool operator<( const List<T>&, AUniqueTypeForNil ) {
   return false;
}
template <class T>
bool operator<( AUniqueTypeForNil, const OddList<T>& b ) {
   return !null(b);
}
template <class T>
bool operator<( AUniqueTypeForNil, const List<T>& b ) {
   return !null(b);
}

//////////////////////////////////////////////////////////////////////
// Handy functions for making list literals
//////////////////////////////////////////////////////////////////////
// Yes, these aren't functoids, they're just template functions.  I'm
// lazy and created these mostly to make it easily to make little lists
// in the sample code snippets that appear in papers.

template <class T>
List<T> list_with( const T& a ) {
   List<T> l;
   l = cons( a, l );
   return l;
}

template <class T>
List<T> list_with( const T& a, const T& b ) {
   List<T> l;
   l = cons( b, l );
   l = cons( a, l );
   return l;
}

template <class T>
List<T> list_with( const T& a, const T& b, const T& c ) {
   List<T> l;
   l = cons( c, l );
   l = cons( b, l );
   l = cons( a, l );
   return l;
}

template <class T>
List<T> list_with( const T& a, const T& b, const T& c, const T& d ) {
   List<T> l;
   l = cons( d, l );
   l = cons( c, l );
   l = cons( b, l );
   l = cons( a, l );
   return l;
}

template <class T>
List<T> 
list_with( const T& a, const T& b, const T& c, const T& d, const T& e ) {
   List<T> l;
   l = cons( e, l );
   l = cons( d, l );
   l = cons( c, l );
   l = cons( b, l );
   l = cons( a, l );
   return l;
}

} // namespace fcpp

#endif
