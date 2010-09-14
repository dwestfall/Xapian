/* flint_termlist.cc: Termlists in a flint database
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006,2007,2008,2010 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "flint_termlist.h"

#include "xapian/error.h"

#include "expandweight.h"
#include "flint_positionlist.h"
#include "flint_utils.h"
#include "debuglog.h"
#include "omassert.h"
#include "str.h"

using namespace std;

FlintTermList::FlintTermList(Xapian::Internal::RefCntPtr<const FlintDatabase> db_,
			     Xapian::docid did_)
	: db(db_), did(did_), current_wdf(0), current_termfreq(0)
{
    LOGCALL_CTOR(DB, "FlintTermList", db_ | did_);

    if (!db->termlist_table.get_exact_entry(flint_docid_to_key(did), data))
	throw Xapian::DocNotFoundError("No termlist for document " + str(did));

    pos = data.data();
    end = pos + data.size();

    if (pos == end) {
	doclen = 0;
	termlist_size = 0;
	return;
    }

    // Read doclen
    if (!F_unpack_uint(&pos, end, &doclen)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for doclen in termlist";
	} else {
	    msg = "Overflowed value for doclen in termlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    // Read termlist_size
    if (!F_unpack_uint(&pos, end, &termlist_size)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for list size in termlist";
	} else {
	    msg = "Overflowed value for list size in termlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    // See comment in FlintTermListTable::set_termlist() in
    // flint_termlisttable.cc for an explanation of this!
    if (pos != end && *pos == '0') ++pos;
}

flint_doclen_t
FlintTermList::get_doclength() const
{
    LOGCALL(DB, flint_doclen_t, "FlintTermList::get_doclength", NO_ARGS);
    RETURN(doclen);
}

Xapian::termcount
FlintTermList::get_approx_size() const
{
    LOGCALL(DB, Xapian::termcount, "FlintTermList::get_approx_size", NO_ARGS);
    RETURN(termlist_size);
}

void
FlintTermList::accumulate_stats(Xapian::Internal::ExpandStats & stats) const
{
    LOGCALL_VOID(DB, "FlintTermList::accumulate_stats", stats);
    Assert(!at_end());
    stats.accumulate(current_wdf, doclen, get_termfreq(), db->get_doccount());
}

string
FlintTermList::get_termname() const
{
    LOGCALL(DB, string, "FlintTermList::get_termname", NO_ARGS);
    RETURN(current_term);
}

Xapian::termcount
FlintTermList::get_wdf() const
{
    LOGCALL(DB, Xapian::termcount, "FlintTermList::get_wdf", NO_ARGS);
    RETURN(current_wdf);
}

Xapian::doccount
FlintTermList::get_termfreq() const
{
    LOGCALL(DB, Xapian::doccount, "FlintTermList::get_termfreq", NO_ARGS);
    if (current_termfreq == 0)
	current_termfreq = db->get_termfreq(current_term);
    RETURN(current_termfreq);
}

TermList *
FlintTermList::next()
{
    LOGCALL(DB, TermList *, "FlintTermList::next", NO_ARGS);
    Assert(!at_end());
    if (pos == end) {
	pos = NULL;
	RETURN(NULL);
    }

    // Reset to 0 to indicate that the termfreq needs to be read.
    current_termfreq = 0;

    bool wdf_in_reuse = false;
    if (!current_term.empty()) {
	// Find out how much of the previous term to reuse.
	size_t len = static_cast<unsigned char>(*pos++);
	if (len > current_term.size()) {
	    // The wdf is also stored in the "reuse" byte.
	    wdf_in_reuse = true;
	    size_t divisor = current_term.size() + 1;
	    current_wdf = len / divisor - 1;
	    len %= divisor;
	}
	current_term.resize(len);
    }

    // Append the new tail to form the next term.
    size_t append_len = static_cast<unsigned char>(*pos++);
    current_term.append(pos, append_len);
    pos += append_len;

    // Read the wdf if it wasn't packed into the reuse byte.
    if (!wdf_in_reuse && !F_unpack_uint(&pos, end, &current_wdf)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for wdf in termlist";
	} else {
	    msg = "Overflowed value for wdf in termlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    RETURN(NULL);
}

TermList *
FlintTermList::skip_to(const string & term)
{
    LOGCALL(API, TermList *, "FlintTermList::skip_to", term);
    while (pos != NULL && current_term < term) {
	(void)FlintTermList::next();
    }
    RETURN(NULL);
}

bool
FlintTermList::at_end() const
{
    LOGCALL(DB, bool, "FlintTermList::at_end", NO_ARGS);
    RETURN(pos == NULL);
}

Xapian::termcount
FlintTermList::positionlist_count() const
{
    LOGCALL(DB, Xapian::termcount, "FlintTermList::positionlist_count", NO_ARGS);
    RETURN(db->position_table.positionlist_count(did, current_term));
}

Xapian::PositionIterator
FlintTermList::positionlist_begin() const
{
    LOGCALL(DB, Xapian::PositionIterator, "FlintTermList::positionlist_begin", NO_ARGS);
    return Xapian::PositionIterator(
	    new FlintPositionList(&db->position_table, did, current_term));
}
