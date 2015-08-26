//
// Copyright (c) 2000-2003 Brian McNamara and Yannis Smaragdakis
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is granted without fee,
// provided that the above copyright notice and this permission notice
// appear in all source code copies and supporting documentation. The
// software is provided "as is" without any express or implied
// warranty.

#ifndef FCPP_REF_DOT_H
#define FCPP_REF_DOT_H

#include "config.h"

#ifndef FCPP_NO_USE_NAMESPACE
namespace fcpp {
#endif

//////////////////////////////////////////////////////////////////////
// Ref<T> can be used in place of T* most anywhere.  Ref<T>s are garbage
// collected (using reference-couting); they are used in place of
// pointers throughout the library to get automatic memory management.
// It is notable that the library parts that use Ref (the indirect functoids
// in function.h and the list in list.h) do not allow the creation of
// "circular" (self-referencing) data structures.
//
// Ref<T> should work exactly as T*, except that instead of dynamic_cast
// you must use ref_dynamic_cast.
//////////////////////////////////////////////////////////////////////
// IRef<T> is an intrusive reference count.  All components of the
// library use IRefs instead of Refs now, as IRefs are more
// efficient.  The disadvantage of IRefs is that they require
// assistance from the objects they reference.
//
// Here is a minial example of class that supports intrusive reference
// counting:
//    struct Foo {
//       mutable RefCountType refC_;
//    public:
//       Foo() : refC_(0) {}
//       void incref() const { ++refC_; }
//       void decref() const { if (!--refC_) delete this; }
//    };
// To create a reference-counted Foo, we can just say
//    IRef<Foo> p = new Foo;
// and we're done.
//
// Alternatively, you can say
//    struct Foo : public IRefAble {};
//    IRef<Foo> p = new Foo;
// the IRefable "mixin" has all the functionality, but adds the overhead
// of a virtual destructor.
//////////////////////////////////////////////////////////////////////

typedef unsigned int RefCountType;

// This is a helper; it will probably be in next version of the C++ standard
template<class T, class U>
T implicit_cast( const U& x ) {
   return x;
}

// Here's the Ref class
template<class T>
class Ref;

template <class U, class T>
Ref<U> ref_dynamic_cast( const Ref<T>& r );

template<class T>
class Ref {
protected:
   T* ptr;
   RefCountType* count;
   
   void new_ref() { count = new RefCountType(1); }
   void inc()     { ++(*count); }
   bool dec()     { return 0 == --(*count); }

   template <class U> friend class Ref;

   template <class U, class V>
   friend Ref<U> ref_dynamic_cast( const Ref<V>& r );

public:
   typedef T WrappedType;

   explicit Ref(T* p=0) : ptr(p), count(0) {
      if(ptr) new_ref();
   }
   Ref(const Ref<T>& other) : ptr(other.ptr), count(0) {
      if(ptr) { count = other.count; inc(); }
   }
   ~Ref() {
      if (ptr && dec()) { delete count; delete ptr; }
   }
   Ref<T>& operator=(const Ref<T>& other) {
      T* tp = other.ptr;
      RefCountType* tc = other.count;
      if( tp ) { ++(*tc); }
      if (ptr && dec()) { delete count; delete ptr; }
      ptr = tp;
      count = tc;
      return *this;
   }

   operator T* () const  { return ptr; }
   T* operator->() const { return ptr; }

   template <class U>
   Ref(const Ref<U>& other) 
   : ptr(implicit_cast<T*>(other.ptr)), count(0) {
      if(ptr) { count = other.count; inc(); }
   }
   bool operator==(const Ref<T>& other) const {
      return ptr==other.ptr;
   }
   bool operator!=(const Ref<T>& other) const {
      return ptr!=other.ptr;
   }
};

// dynamic_cast; can't overload the operator (why?!?) so we create our own

template <class U, class T>
Ref<U> ref_dynamic_cast( const Ref<T>& r ) {
   Ref<U> temp;
   temp.ptr = dynamic_cast<U*>( r.ptr );
   if( temp.ptr ) { temp.count = r.count; temp.inc(); }
   return temp;
}

template<class T>
class IRef {
   T* ptr;
public:
   static void swap( IRef<T>& x, IRef<T>& y ) {
      T* tmp = x.ptr;
      x.ptr = y.ptr;
      y.ptr = tmp;
   }
   static void rotate( IRef<T>& x, IRef<T>& y, IRef<T>& z ) {
      T* tmp = x.ptr;
      x.ptr = y.ptr;
      y.ptr = z.ptr;
      z.ptr = tmp;
   }

   typedef T WrappedType;

   explicit IRef(T* p=0) : ptr(p) {
#ifndef FCPP_LEAK
      if(ptr) ptr->incref();
#endif
   }
   IRef(const IRef<T>& other) : ptr(other.ptr) {
#ifndef FCPP_LEAK
      if(ptr) { ptr->incref(); }
#endif
   }
   ~IRef() {
#ifndef FCPP_LEAK
      if (ptr) { ptr->decref(); }
#endif
   }
   IRef<T>& operator=(const IRef<T>& other) {
#ifndef FCPP_LEAK
      if (other.ptr) { other.ptr->incref(); }
      if (ptr) { ptr->decref(); }
#endif
      ptr = other.ptr;
      return *this;
   }
   operator T* () const  { return ptr; }
   T* operator->() const { return ptr; }
   bool operator==(const IRef<T>& other) const {
      return ptr==other.ptr;
   }
   bool operator!=(const IRef<T>& other) const {
      return ptr!=other.ptr;
   }
};

// A convenient "mixin"

struct IRefable {
   mutable RefCountType refC_;
public:
   IRefable(RefCountType x = 0) : refC_(x) {}
   void incref() const { ++refC_; }
   void decref() const { if (!--refC_) delete this; }
   virtual ~IRefable() {}
};

#ifndef FCPP_NO_USE_NAMESPACE
} // end namespace fcpp 
#endif

#endif
