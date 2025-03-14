/***********************************************************************************************************************************
PostgreSQL Types That Do Not Vary By Version

Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
Portions Copyright (c) 1994, Regents of the University of California

For each supported release of PostgreSQL check the types in this file to see if they have changed. The easiest way to do this is to
copy and paste in place and check git to see if there are any diffs. Tabs should be copied as is to make this process easy even
though the pgBackRest project does not use tabs elsewhere.

Comments should be copied with the types they apply to, even if the comment has not changed. This does get repetitive, but has no
runtime cost and makes the rules a bit easier to follow.

If a comment is changed then the newer comment should be copied. If the *type* has changed then it must be moved to version.auto.c
which could have a large impact on dependencies. Hopefully that won't happen often.

Note when adding new types it is safer to add them to version.auto.c unless they are needed for code that must be compatible across
all versions of PostgreSQL supported by pgBackRest.
***********************************************************************************************************************************/
#ifndef POSTGRES_INTERFACE_STATICVENDOR_H
#define POSTGRES_INTERFACE_STATICVENDOR_H

#include "common/assert.h"
#include "postgres/interface.h"

/***********************************************************************************************************************************
Types from src/include/c.h
***********************************************************************************************************************************/

// uint16 type
// ---------------------------------------------------------------------------------------------------------------------------------
typedef uint16_t uint16;

// uint32 type
// ---------------------------------------------------------------------------------------------------------------------------------
typedef uint32_t uint32;

// uint64 type
// ---------------------------------------------------------------------------------------------------------------------------------
typedef uint64_t uint64;

// TransactionId type
// ---------------------------------------------------------------------------------------------------------------------------------
typedef uint32 TransactionId;

// FLEXIBLE_ARRAY_MEMBER macro
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 * We require C99, hence the compiler should understand flexible array
 * members.  However, for documentation purposes we still consider it to be
 * project style to write "field[FLEXIBLE_ARRAY_MEMBER]" not just "field[]".
 * When computing the size of such an object, use "offsetof(struct s, f)"
 * for portability.  Don't use "offsetof(struct s, f[0])", as this doesn't
 * work with MSVC and with C++ compilers.
 */
#define FLEXIBLE_ARRAY_MEMBER	/* empty */

/***********************************************************************************************************************************
Types from src/include/storage/itemid.h
***********************************************************************************************************************************/

// ItemIdData type
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 * A line pointer on a buffer page.  See buffer page definitions and comments
 * for an explanation of how line pointers are used.
 *
 * In some cases a line pointer is "in use" but does not have any associated
 * storage on the page.  By convention, lp_len == 0 in every line pointer
 * that does not have storage, independently of its lp_flags state.
 */
typedef struct ItemIdData
{
	unsigned	lp_off:15,		/* offset to tuple (from start of page) */
				lp_flags:2,		/* state of line pointer, see below */
				lp_len:15;		/* byte length of tuple */
} ItemIdData;

/***********************************************************************************************************************************
Types from src/include/storage/block.h
***********************************************************************************************************************************/

// BlockNumber type
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 * BlockNumber:
 *
 * each data file (heap or index) is divided into postgres disk blocks
 * (which may be thought of as the unit of i/o -- a postgres buffer
 * contains exactly one disk block).  the blocks are numbered
 * sequentially, 0 to 0xFFFFFFFE.
 *
 * InvalidBlockNumber is the same thing as P_NEW in bufmgr.h.
 *
 * the access methods, the buffer manager and the storage manager are
 * more or less the only pieces of code that should be accessing disk
 * blocks directly.
 */
typedef uint32 BlockNumber;

/***********************************************************************************************************************************
Types from src/include/storage/bufpage.h
***********************************************************************************************************************************/

// LocationIndex type
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 * location (byte offset) within a page.
 *
 * note that this is actually limited to 2^15 because we have limited
 * ItemIdData.lp_off and ItemIdData.lp_len to 15 bits (see itemid.h).
 */
typedef uint16 LocationIndex;

// PageXLogRecPtr type
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 * For historical reasons, the 64-bit LSN value is stored as two 32-bit
 * values.
 */
typedef struct
{
	uint32		xlogid;			/* high bits */
	uint32		xrecoff;		/* low bits */
} PageXLogRecPtr;

// PG_DATA_CHECKSUM_VERSION define
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 * As of Release 9.3, the checksum version must also be considered when
 * handling pages.
 */
#define PG_DATA_CHECKSUM_VERSION	1

// PageHeaderData type
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 * disk page organization
 *
 * space management information generic to any page
 *
 *		pd_lsn		- identifies xlog record for last change to this page.
 *		pd_checksum - page checksum, if set.
 *		pd_flags	- flag bits.
 *		pd_lower	- offset to start of free space.
 *		pd_upper	- offset to end of free space.
 *		pd_special	- offset to start of special space.
 *		pd_pagesize_version - size in bytes and page layout version number.
 *		pd_prune_xid - oldest XID among potentially prunable tuples on page.
 *
 * The LSN is used by the buffer manager to enforce the basic rule of WAL:
 * "thou shalt write xlog before data".  A dirty buffer cannot be dumped
 * to disk until xlog has been flushed at least as far as the page's LSN.
 *
 * pd_checksum stores the page checksum, if it has been set for this page;
 * zero is a valid value for a checksum. If a checksum is not in use then
 * we leave the field unset. This will typically mean the field is zero
 * though non-zero values may also be present if databases have been
 * pg_upgraded from releases prior to 9.3, when the same byte offset was
 * used to store the current timelineid when the page was last updated.
 * Note that there is no indication on a page as to whether the checksum
 * is valid or not, a deliberate design choice which avoids the problem
 * of relying on the page contents to decide whether to verify it. Hence
 * there are no flag bits relating to checksums.
 *
 * pd_prune_xid is a hint field that helps determine whether pruning will be
 * useful.  It is currently unused in index pages.
 *
 * The page version number and page size are packed together into a single
 * uint16 field.  This is for historical reasons: before PostgreSQL 7.3,
 * there was no concept of a page version number, and doing it this way
 * lets us pretend that pre-7.3 databases have page version number zero.
 * We constrain page sizes to be multiples of 256, leaving the low eight
 * bits available for a version number.
 *
 * Minimum possible page size is perhaps 64B to fit page header, opaque space
 * and a minimal tuple; of course, in reality you want it much bigger, so
 * the constraint on pagesize mod 256 is not an important restriction.
 * On the high end, we can only support pages up to 32KB because lp_off/lp_len
 * are 15 bits.
 */
typedef struct PageHeaderData
{
	/* XXX LSN is member of *any* block, not only page-organized ones */
	PageXLogRecPtr pd_lsn;		/* LSN: next byte after last byte of xlog
								 * record for last change to this page */
	uint16		pd_checksum;	/* checksum */
	uint16		pd_flags;		/* flag bits, see below */
	LocationIndex pd_lower;		/* offset to start of free space */
	LocationIndex pd_upper;		/* offset to end of free space */
	LocationIndex pd_special;	/* offset to start of special space */
	uint16		pd_pagesize_version;
	TransactionId pd_prune_xid; /* oldest prunable XID, or zero if none */
	ItemIdData	pd_linp[FLEXIBLE_ARRAY_MEMBER]; /* line pointer array */
} PageHeaderData;

// PageHeader type
// ---------------------------------------------------------------------------------------------------------------------------------
typedef PageHeaderData *PageHeader;

/***********************************************************************************************************************************
Types from src/include/access/transam.h
***********************************************************************************************************************************/

// FirstNormalObjectId define
// ---------------------------------------------------------------------------------------------------------------------------------
/*
 *		Object ID (OID) zero is InvalidOid.
 *
 *		OIDs 1-9999 are reserved for manual assignment (see .dat files in
 *		src/include/catalog/).  Of these, 8000-9999 are reserved for
 *		development purposes (such as in-progress patches and forks);
 *		they should not appear in released versions.
 *
 *		OIDs 10000-11999 are reserved for assignment by genbki.pl, for use
 *		when the .dat files in src/include/catalog/ do not specify an OID
 *		for a catalog entry that requires one.  Note that genbki.pl assigns
 *		these OIDs independently in each catalog, so they're not guaranteed
 *		to be globally unique.  Furthermore, the bootstrap backend and
 *		initdb's post-bootstrap processing can also assign OIDs in this range.
 *		The normal OID-generation logic takes care of any OID conflicts that
 *		might arise from that.
 *
 *		OIDs 12000-16383 are reserved for unpinned objects created by initdb's
 *		post-bootstrap processing.  initdb forces the OID generator up to
 *		12000 as soon as it's made the pinned objects it's responsible for.
 *
 *		OIDs beginning at 16384 are assigned from the OID generator
 *		during normal multiuser operation.  (We force the generator up to
 *		16384 as soon as we are in normal operation.)
 *
 * The choices of 8000, 10000 and 12000 are completely arbitrary, and can be
 * moved if we run low on OIDs in any category.  Changing the macros below,
 * and updating relevant documentation (see bki.sgml and RELEASE_CHANGES),
 * should be sufficient to do this.  Moving the 16384 boundary between
 * initdb-assigned OIDs and user-defined objects would be substantially
 * more painful, however, since some user-defined OIDs will appear in
 * on-disk data; such a change would probably break pg_upgrade.
 *
 * NOTE: if the OID generator wraps around, we skip over OIDs 0-16383
 * and resume with 16384.  This minimizes the odds of OID conflict, by not
 * reassigning OIDs that might have been assigned during initdb.  Critically,
 * it also ensures that no user-created object will be considered pinned.
 */
#define FirstNormalObjectId		16384

/***********************************************************************************************************************************
Types from src/include/access/xlog_internal.h
***********************************************************************************************************************************/

// WalSegMinSize/WalSegMinSize macros
// ---------------------------------------------------------------------------------------------------------------------------------
/* wal_segment_size can range from 1MB to 1GB */
#define WalSegMinSize 1024 * 1024
#define WalSegMaxSize 1024 * 1024 * 1024

// IsPowerOf2/IsValidWalSegSize macros
// ---------------------------------------------------------------------------------------------------------------------------------
/* check that the given size is a valid wal_segment_size */
#define IsPowerOf2(x) (x > 0 && ((x) & ((x)-1)) == 0)
#define IsValidWalSegSize(size) \
	 (IsPowerOf2(size) && \
	 ((size) >= WalSegMinSize && (size) <= WalSegMaxSize))

#endif
