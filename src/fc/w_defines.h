/* -*- mode:C++; c-basic-offset:4 -*-
     Shore-MT -- Multi-threaded port of the SHORE storage manager
   
                       Copyright (c) 2007-2009
      Data Intensive Applications and Systems Labaratory (DIAS)
               Ecole Polytechnique Federale de Lausanne
   
                         All Rights Reserved.
   
   Permission to use, copy, modify and distribute this software and
   its documentation is hereby granted, provided that both the
   copyright notice and this permission notice appear in all copies of
   the software, derivative works or modified versions, and any
   portions thereof, and that both notices appear in supporting
   documentation.
   
   This code is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS
   DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER
   RESULTING FROM THE USE OF THIS SOFTWARE.
*/

/*<std-header orig-src='shore' incl-file-exclusion='W_DEFINES_H' no-defines='true'>

 $Id: w_defines.h,v 1.5 2010/06/08 22:27:22 nhall Exp $

SHORE -- Scalable Heterogeneous Object REpository

Copyright (c) 1994-99 Computer Sciences Department, University of
                      Wisconsin -- Madison
All Rights Reserved.

Permission to use, copy, modify and distribute this software and its
documentation is hereby granted, provided that both the copyright
notice and this permission notice appear in all copies of the
software, derivative works or modified versions, and any portions
thereof, and that both notices appear in supporting documentation.

THE AUTHORS AND THE COMPUTER SCIENCES DEPARTMENT OF THE UNIVERSITY
OF WISCONSIN - MADISON ALLOW FREE USE OF THIS SOFTWARE IN ITS
"AS IS" CONDITION, AND THEY DISCLAIM ANY LIABILITY OF ANY KIND
FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.

This software was developed with support by the Advanced Research
Project Agency, ARPA order number 018 (formerly 8230), monitored by
the U.S. Army Research Laboratory under contract DAAB07-91-C-Q518.
Further funding for this work was provided by DARPA through
Rome Research Laboratory Contract No. F30602-97-2-0247.

*/

#ifndef W_DEFINES_H
#define W_DEFINES_H

/*  -- do not edit anything above this line --   </std-header>*/

#include "shore.def"
/* shore-config.h does not have duplicate-include protection, but
   this file does, and we don't include shore-config.h anywhere else

   Before including it, erase the existing autoconf #defines (if any)
   to avoid warnings about redefining macros.
*/
#ifdef PACKAGE
#undef PACKAGE
#endif
#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif
#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif
#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif
#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif
#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif
#ifdef VERSION
#undef VERSION
#endif
#include "shore-config.h"

/*
 *  Valgrind/Purify support
 *  note: ZERO_INIT makes data structures clear out or
 *  otherwise initialize their contents on
 *  init so that you don't get certain benign uninit read/write errors  
 */
#if defined(HAVE_PURIFY_H) || defined(PURIFY)
#include <purify.h>
#define ZERO_INIT 1
#endif

#ifdef HAVE_VALGRIND_H
#define USING_VALGRIND 1
#include <valgrind.h>
#define ZERO_INIT 1
#elif HAVE_VALGRIND_VALGRIND_H
#define USING_VALGRIND 1
#include <valgrind/valgrind.h>
#define ZERO_INIT 1
#endif

#ifdef USING_VALGRIND
#include "valgrind_help.h"
#endif

#ifdef __SUNPRO_CC
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#else
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <climits>
#endif

#include <unistd.h>

#include <inttypes.h>

/* the following cannot be "verbatim" included in shore-config.def,
 * unfortunately. The  #undef ARCH_LP64 gets mangled
 * by autoconf.
 */
#ifdef ARCH_LP64
/* enabled LP64 - let's make sure the environment can handle it */
#if _SC_V6_LP64_OFF64 || _XBS5_LP64_OFF64 || _SC_V6_LPBIG_OFFBIG || _XBS5_LPBIG_OFFBIG 
#else
#warning Turning off ARCH_LP64
#undef ARCH_LP64
#endif
/* ARCH_LP64 was defined (might no longer be) */
#endif

/* Issue warning if we don't have large file offsets with ILP32 */
#ifndef ARCH_LP64

#if _SC_V6_ILP32_OFFBIG || _XBS5_ILP32_OFFBIG
#else
#warning large file off_t support seems to be missing accoring to sysconf !
#endif

/* ARCH_LP64 not defined */
#endif


#if SM_PAGESIZE > 32768 
#error SM does not support pages this large.
#endif

#ifdef __GNUC__
#define MAYBE_UNUSED __attribute__((unused))
#else
#define MAYBE_UNUSED
#endif

#include <sys/types.h>

/* Alignment functions.

   WARNING: undefined behavior results if k is zero or not a power of
   two.
 */

/// aligns a pointer p on a size a
template <typename T>
static inline
T alignon(T p, size_t a) {
    size_t n = (size_t) p;
    size_t rval = (n + a - 1) & -a;
    return (T) rval;
}
 

///  We now support only 8-byte alignment of records
#define ALIGNON 0x8

/// align to 8-byte boundary
template <typename T>
static inline
T align(T sz) { return alignon(sz, ALIGNON); }

// sometimes we need compile-time constant alignment...
template <size_t N, size_t K=ALIGNON>
struct static_align {
    enum { value=(N+K-1) & -K };
};

// avoid nasty bus errors...
template<class T>
static inline T* aligned_cast(char const* ptr) 
{
  // bump the pointer up to the next proper alignment (always a power of 2)
  return (T*) alignon(ptr, __alignof__(T));
}


/** \brief constructs a blob of N bytes.
*
*  When instantiated on the stack, allocates enough
*  bytes on the stack to hold an object of size N, aligned.
*  Specialize this to handle arbitrary sizes (sizeof(a)+sizeof(b), etc).
*/
template<int N>
class allocaN {
  char _buf[N+__alignof__(uintptr_t)];
public:
  operator void*() { return aligned_cast<uintptr_t>(_buf); }
  // no destructor because we don't know what's going on...
};

/*<std-footer incl-file-exclusion='W_DEFINES_H'>  -- do not edit anything below this line -- */

#endif          /*</std-footer>*/
