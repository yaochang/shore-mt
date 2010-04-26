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

/*<std-header orig-src='shore' incl-file-exclusion='SCAN_H'>

 $Id: scan.h,v 1.88.2.7 2010/03/19 22:20:26 nhall Exp $

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

#ifndef SCAN_H
#define SCAN_H

#include "w_defines.h"

/*  -- do not edit anything above this line --   </std-header>*/

#ifdef __GNUG__
#pragma interface
#endif

#ifndef XCT_DEPENDENT_H
#include <xct_dependent.h>
#endif /* XCT_DEPENDENT_H */

#include <page_alias.h>

/**\defgroup SSMSCAN Scanning
 * \ingroup SSMSTG
 * Scans can be performed on B+tree and R-tree indexes and on files
 * of records.  Iterator classes scan_index_i, scan_rt_i and scan_file_i are 
 * used for Btree's, Rtree's and files, respectively. 
 * 
 * Scans begin by creating an iterator to specify what
 * to scan and the range of the scan.
 * 
 * An iterator's next() function is used to retrieve values from
 * the scan (including the first).  Next() will set the eof
 * parameter to true only when no value can be retrieved for the
 * current call, so
 * if a file contains 2 records and next() has been called
 * twice, eof will return false on the first and second calls, but
 * true on the third.
 * 
 * The eof() function reports the value of eof returned
 * from the \b last call to next().  This means you might find
 * eof to be false, call next(), and get nothing back from next()
 * (eof is now true).
 * 
 * The scan destructor frees and un-fixes (un-pins) all resources
 * used by the scan.
 * 
 * The finish() function frees (and un-fixes) resources used
 * by the scan.  finish() is called by the destructor,
 * but it is sometimes useful to call it early for scan objects
 * on the stack when the destructor might not be called until
 * the end of the function.
 * 
 * All of the scan constructors have a concurrency control
 * parameter.  This controls the granularity of locking
 * done by the scan.  The possible values of cc are:
 * 
 * - t_cc_none:    IS lock the file/index and 
 *    obtain no other locks during the scan
 * - t_cc_kvl:    IS lock the index and obtain SH key-value locks
 *    on every entry as encountered;  used only for btrees (scan_index_i)
 * - t_cc_record:IS lock the file and obtain SH locks on every 
 *   record as encountered
 * - t_cc_page:  IS lock the file/index and obtain SH locks on pages 
 *   (leaf pages, used only for rtrees)
 * - t_cc_file:  SH lock the file/index 
 *
 * \todo scan_file_i
 * \todo append_file_i
 */

/**\defgroup SSMSCANF Scanning Files
 * \ingroup SSMSCAN
 */

/**\defgroup SSMSCANI Scanning B+-Tree Indexes
 * \ingroup SSMSCAN
 * \sa SSMBTREE
 * scan_index_i
 */
class bt_cursor_t;

class scan_index_i : public smlevel_top, public xct_dependent_t {
public:
    NORET            scan_index_i(
        const stid_t&             stid,
        cmp_t                     c1,
        const cvec_t&             bound1, 
        cmp_t                     c2,
        const cvec_t&             bound2,
        bool                      include_nulls = false,
        concurrency_t             cc = t_cc_kvl
        );


    NORET            ~scan_index_i();

    rc_t            curr(
        vec_t*           key, 
        smsize_t&        klen,
        vec_t*           el,
        smsize_t&        elen)  {
            return _fetch(key, &klen, el, &elen, false);
        }

    rc_t             next(bool& eof)  {
        rc_t rc = _fetch(0, 0, 0, 0, true);
        eof = _eof;
        return rc.reset();
    }

    void             finish();
    
    bool             eof()    { return _eof; }
    tid_t            xid() const { return tid; }
    ndx_t            ndx() const { return ntype; }
    const rc_t &     error_code() const { return _error_occurred; }

private:
    stid_t               _stid;
    tid_t                tid;
    ndx_t                ntype;
    cmp_t                cond2;
    bool                 _eof;

    w_rc_t               _error_occurred;
    bt_cursor_t*         _btcursor;
    bool                 _finished;
    bool                 _skip_nulls;
    concurrency_t        _cc;
    lock_mode_t          _mode;

    rc_t            _fetch(
        vec_t*                key, 
        smsize_t*             klen, 
        vec_t*                el, 
        smsize_t*             elen,
        bool                  skip);

    void             _init(
        cmp_t                 cond,
        const cvec_t&         bound,
        cmp_t                 c2,
        const cvec_t&         b2,
        lock_mode_t           mode = SH);

    void            xct_state_changed(
        xct_state_t            old_state,
        xct_state_t            new_state);

    // disabled
    NORET            scan_index_i(const scan_index_i&);
    scan_index_i&    operator=(const scan_index_i&);
};


// R-Tree Scanning
/**\defgroup SSMSCANRT Scanning R*-Tree Indexes
 * \ingroup SSMSCAN
 * \sa SSMRTREE
 * scan_rt_i
 */
class rt_cursor_t;
/**\brief Iterator for scanning an R-Tree.
 * \todo example of rtree scan.
 */
class scan_rt_i : public smlevel_top, public xct_dependent_t {
public:

    /// Store id of the R-Tree.
    stid_t            stid;
    /// Transaction ID of the transaction that initialized this iterator.
    tid_t             tid;
    /// Type of this index.
    ndx_t             ntype;
    
    /**\brief Construct an iterator.
     * \details
     * @param[in] stid   ID of the R-Tree to be scanned.
     * @param[in] c   Comparison type to be used in the scan : t_exact,
     *       t_overlap, t_cover, t_inside.
     * @param[in] box   Box to reference for the comparison above.
     * @param[in] include_nulls   If true, we will consider null keys
     *        as satisfying the condition.
     * @param[in] cc   Must be t_cc_none, t_cc_page or t_cc_file.
     *       In the first two cases, an IS lock is acquired; in the
     *       last, an SH lock is acquired; this lock applies to the
     *       entire index.
     */
    NORET            scan_rt_i(
        const stid_t&             stid, 
        nbox_t::sob_cmp_t         c,
        const nbox_t&             box,
        bool                      include_nulls = false,
        concurrency_t             cc = t_cc_page);

    NORET            ~scan_rt_i();
    
    rc_t            next(
        nbox_t&               key,
        void*                 el,
        smsize_t&             elen, 
        bool&                 eof) {
            return _fetch(key, el, elen, eof, true);
        }

/*
    curr(nbox_t& key, void* el, smsize_t& elen, bool& eof) {
    return _fetch(key, el, elen, eof, false);
    }
*/
    void            finish();
    
    bool            eof()    { return _eof; }
    const rc_t &     error_code() const { return _error_occurred; }
private:
    bool            _eof;
    rc_t            _error_occurred;
    rt_cursor_t*         _cursor;
    bool            _finished;
    bool            _skip_nulls;
    concurrency_t        _cc;

    rc_t            _fetch(
    nbox_t&             key, 
    void*                el,
    smsize_t&            elen, 
    bool&             eof, 
    bool                 skip);
    void             _init(
    nbox_t::sob_cmp_t         c, 
    const nbox_t&             qbox);

    void            xct_state_changed(
    xct_state_t            old_state,
    xct_state_t            new_state);

    // disabled
    NORET            scan_rt_i(const scan_rt_i&);
    scan_rt_i&            operator=(const scan_rt_i&);
};

class bf_prefetch_thread_t;


/** \ingroup SSMSCANF
 * The file-scanning iterator class is scan_file_i.  The next() function
 * returns a pointer to a pin_i object which "points" to the next record
 * in the file.  The start_offset argument to next() indicates the first
 * byte in the record to pin.
 */
class scan_file_i : public smlevel_top, public xct_dependent_t {
public:
    stid_t                stid;
    rid_t                curr_rid;
    
    NORET            scan_file_i(
        const stid_t&            stid, 
        const rid_t&             start,
        concurrency_t            cc = t_cc_file,
        bool                     prefetch=false,
        lock_mode_t              mode = SH);

    NORET            scan_file_i(
        const stid_t&            stid,
        concurrency_t            cc = t_cc_file,
        bool                     prefetch=false,
        lock_mode_t              mode = SH);

    NORET            ~scan_file_i();
    
    /* needed for tcl scripts */
    void            cursor(
                      pin_i*&    pin_ptr,
                    bool&       eof
                ) { pin_ptr = &_cursor; eof = _eof; }

    rc_t            next(
    pin_i*&                pin_ptr,
    smsize_t             start_offset, 
    bool&                     eof);

    /*
     * The next_page function advances the scan to the first
     * record on the next page in the file.  If next_page is
     * called after the scan_file_i is initialized it will advance
     * the cursor to the first record in the file, just as
     * next() would do.
     */
    rc_t            next_page(
        pin_i*&                pin_ptr,
        smsize_t               start_offset, 
        bool&                  eof);

    void            finish();
    bool            eof()        { return _eof; }
    const rc_t &    error_code() const { return _error_occurred; }
    tid_t           xid() const { return tid; }

protected:
    tid_t            tid;
    bool             _eof;
    w_rc_t           _error_occurred;
    pin_i            _cursor;
    lpid_t           _next_pid;
    concurrency_t    _cc;  // concurrency control
    lock_mode_t      _page_lock_mode;
    lock_mode_t      _rec_lock_mode;

    rc_t             _init(bool for_append=false);

    rc_t            _next(
        pin_i*&                pin_ptr,
        smsize_t               start_offset, 
        bool&                   eof);

    void            xct_state_changed(
        xct_state_t            old_state,
        xct_state_t            new_state);

private:
    bool              _do_prefetch;
    bf_prefetch_thread_t*    _prefetch;

    // disabled
    NORET            scan_file_i(const scan_file_i&);
    scan_file_i&        operator=(const scan_file_i&);
};

#include <cstring>
#ifndef SDESC_H
#include "sdesc.h"
#endif

/**\brief Pseudo-iterator used to append to a file.
 * \details
 * Just creating a record doesn't ensure that it appears at the
 * end of the file; when new pages are allocated, they could be
 * in extents in the middle of the file, and unused slots in
 * pages in the middle of the file can be scavenged for create_rec.
 *
 * If you want to ensure that the records are appended to a file, 
 * use this.
 */
class append_file_i : public scan_file_i 
{
public:
    /**\brief Construct an append_file_i for a given file.
     * @param[in] stid  ID of the file in which to create records.
     */
    NORET            append_file_i( const stid_t& stid);

    NORET            ~append_file_i();

    rc_t            next(
        pin_i*&         pin_ptr,
        smsize_t        start_offset, 
        bool&           eof);

    /**\brief Append a new record to the end of the file.
     * \ingroup SSMFILE
     * @param[in] hdr The client-defined record header.
     * @param[in] len_hint The length of the record, more or less. More
     *                 accuracy here helps the sm reduce its work.
     * @param[in] data The client-defined record contents.
     * @param[out] rid  The identifier of the new record.
     */
    rc_t            create_rec(
        const vec_t&    hdr,
        smsize_t        len_hint, 
        const vec_t&    data,
        rid_t&          rid);
private:
    void             _init_constructor();

    // See comments in pin.h, which does the same thing
    // file_p                page;
     file_p&             _page();
    char                 _page_alias[PAGE_ALIAS_FILE];
    sdesc_t              _cached_sdesc;
};

/*<std-footer incl-file-exclusion='SCAN_H'>  -- do not edit anything below this line -- */

#endif          /*</std-footer>*/
