/** @file api_unicode.cc
 * @brief Test the Unicode and UTF-8 classes and functions.
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "api_unicode.h"

#include <xapian.h>

#include "apitest.h"
#include "testutils.h"

#include <cctype>

using namespace std;

struct testcase {
    const char * a, * b;
};

static const testcase testcases[] = {
    { "abcd", "abcd" }, // Sanity check!
    { "a\x80""bcd", "a\xc2\x80""bcd" },
    { "a\xa0", "a\xc2\xa0" },
    { 0, 0 }
};

// Test handling of invalid UTF-8 is as desired.
DEFINE_TESTCASE(utf8iterator1,!backend) {
    const testcase * p;
    for (p = testcases; p->a; ++p) {
	tout << '"' << p->a << "\" and \"" << p->b << '"' << endl;
	size_t a_len = strlen(p->a);
	Xapian::Utf8Iterator a(p->a, a_len);

	size_t b_len = strlen(p->b);
	Xapian::Utf8Iterator b(p->b, b_len);

	while (a != Xapian::Utf8Iterator() && b != Xapian::Utf8Iterator()) {
	    TEST_EQUAL(*a, *b);
	    ++a;
	    ++b;
	}

	// Test that we don't reach the end of one before the other.
	TEST(a == Xapian::Utf8Iterator());
	TEST(b == Xapian::Utf8Iterator());
    }
    return true;
}

struct testcase2 {
    const char * a;
    unsigned long n;
};

static const testcase2 testcases2[] = {
    { "a", 97 },
    { "\x80", 128 },
    { "\xa0", 160 },
    { "\xc2\x80", 128 },
    { "\xc2\xa0", 160 },
    { "\xf0\xa8\xa8\x8f", 166415 },
    { 0, 0 }
};

// Test decoding of UTF-8.
DEFINE_TESTCASE(utf8iterator2,!backend) {
    const testcase2 * p;
    for (p = testcases2; p->a; ++p) {
	Xapian::Utf8Iterator a(p->a, strlen(p->a));

	TEST(a != Xapian::Utf8Iterator());
	TEST_EQUAL(*a, p->n);
	TEST(++a == Xapian::Utf8Iterator());
    }
    return true;
}

// Test Unicode categorisation.
DEFINE_TESTCASE(unicode1,!backend) {
    using namespace Xapian;
    TEST_EQUAL(Unicode::get_category('a'), Unicode::LOWERCASE_LETTER);
    TEST_EQUAL(Unicode::get_category('0'), Unicode::DECIMAL_DIGIT_NUMBER);
    TEST_EQUAL(Unicode::get_category('$'), Unicode::CURRENCY_SYMBOL);
    TEST_EQUAL(Unicode::get_category(0xa3), Unicode::CURRENCY_SYMBOL);
    // U+0242 was added in Unicode 5.0.0.
    TEST_EQUAL(Unicode::get_category(0x242), Unicode::LOWERCASE_LETTER);
    // U+11A7 was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0x11A7), Unicode::OTHER_LETTER);
    // U+9FCB was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0x9FCB), Unicode::OTHER_LETTER);
    // U+FA6C was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0xFA6C), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0xFFFF), Unicode::UNASSIGNED);
    // Test characters outside BMP.
    TEST_EQUAL(Unicode::get_category(0x10345), Unicode::OTHER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x10FFFD), Unicode::PRIVATE_USE);
    TEST_EQUAL(Unicode::get_category(0x10FFFF), Unicode::UNASSIGNED);
    // U+1109A was added in Unicode 5.2.0.
    TEST_EQUAL(Unicode::get_category(0x1109a), Unicode::OTHER_LETTER);
    // Test some invalid Unicode values.
    TEST_EQUAL(Unicode::get_category(0x110000), Unicode::UNASSIGNED);
    TEST_EQUAL(Unicode::get_category(0xFFFFFFFF), Unicode::UNASSIGNED);
    return true;
}

DEFINE_TESTCASE(caseconvert1,!backend) {
    using namespace Xapian;
    for (unsigned ch = 0; ch < 128; ++ch) {
	if (isupper((char)ch)) {
	    TEST_EQUAL(Unicode::tolower(ch), unsigned(tolower((char)ch)));
	} else {
	    TEST_EQUAL(Unicode::tolower(ch), ch);
	}
	if (islower((char)ch)) {
	    TEST_EQUAL(Unicode::toupper(ch), unsigned(toupper((char)ch)));
	} else {
	    TEST_EQUAL(Unicode::toupper(ch), ch);
	}
    }

    // U+0242 was added in Unicode 5.0.0 as a lowercase form of U+0241.
    TEST_EQUAL(Unicode::tolower(0x242), 0x242);
    TEST_EQUAL(Unicode::toupper(0x242), 0x241);
    TEST_EQUAL(Unicode::toupper(0x241), 0x241);
    TEST_EQUAL(Unicode::tolower(0x241), 0x242);

    // Pound currency symbol:
    TEST_EQUAL(Unicode::tolower(0xa3), 0xa3);
    TEST_EQUAL(Unicode::toupper(0xa3), 0xa3);
    // Unassigned:
    TEST_EQUAL(Unicode::tolower(0xFFFF), 0xFFFF);
    TEST_EQUAL(Unicode::toupper(0xFFFF), 0xFFFF);
    // Test characters outside BMP.
    TEST_EQUAL(Unicode::tolower(0x10345), 0x10345);
    TEST_EQUAL(Unicode::toupper(0x10345), 0x10345);
    TEST_EQUAL(Unicode::tolower(0x10FFFD), 0x10FFFD);
    TEST_EQUAL(Unicode::toupper(0x10FFFD), 0x10FFFD);
    TEST_EQUAL(Unicode::tolower(0x10FFFF), 0x10FFFF);
    TEST_EQUAL(Unicode::toupper(0x10FFFF), 0x10FFFF);
    // Test some invalid Unicode values.
    TEST_EQUAL(Unicode::tolower(0x110000), 0x110000);
    TEST_EQUAL(Unicode::toupper(0x110000), 0x110000);
    TEST_EQUAL(Unicode::tolower(0xFFFFFFFF), 0xFFFFFFFF);
    TEST_EQUAL(Unicode::toupper(0xFFFFFFFF), 0xFFFFFFFF);

    return true;
}

/// Test Unicode 5.1 support.
DEFINE_TESTCASE(caseconvert2,!backend) {
    using namespace Xapian;

    TEST_EQUAL(Unicode::toupper(0x250), 0x2c6f);
    TEST_EQUAL(Unicode::toupper(0x251), 0x2c6d);
    TEST_EQUAL(Unicode::toupper(0x271), 0x2c6e);

    TEST_EQUAL(Unicode::get_category(0x2ec), Unicode::MODIFIER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x374), Unicode::MODIFIER_LETTER);
    TEST_EQUAL(Unicode::get_category(0x487), Unicode::NON_SPACING_MARK);
    TEST_EQUAL(Unicode::get_category(0x5be), Unicode::DASH_PUNCTUATION);
    TEST_EQUAL(Unicode::get_category(0x1f093), Unicode::OTHER_SYMBOL);

    TEST_EQUAL(Unicode::tolower(0x370), 0x371);
    TEST_EQUAL(Unicode::toupper(0x371), 0x370);
    TEST_EQUAL(Unicode::tolower(0x372), 0x373);
    TEST_EQUAL(Unicode::toupper(0x373), 0x372);
    TEST_EQUAL(Unicode::tolower(0x376), 0x377);
    TEST_EQUAL(Unicode::toupper(0x377), 0x376);
    TEST_EQUAL(Unicode::tolower(0x3cf), 0x3d7);
    TEST_EQUAL(Unicode::toupper(0x3d7), 0x3cf);

    unsigned u;
    for (u = 0x514; u < 0x524; u += 2) {
	TEST_EQUAL(Unicode::get_category(u), Unicode::UPPERCASE_LETTER);
	TEST_EQUAL(Unicode::get_category(u + 1), Unicode::LOWERCASE_LETTER);
	TEST_EQUAL(Unicode::tolower(u), u + 1);
	TEST_EQUAL(Unicode::toupper(u + 1), u);
    }
	
    return true;
}

DEFINE_TESTCASE(utf8convert1,!backend) {
    string s;
    Xapian::Unicode::append_utf8(s, 'a');
    Xapian::Unicode::append_utf8(s, 128);
    Xapian::Unicode::append_utf8(s, 160);
    Xapian::Unicode::append_utf8(s, 0xFFFF);
    Xapian::Unicode::append_utf8(s, 166415);
    Xapian::Unicode::append_utf8(s, 0x10345);
    Xapian::Unicode::append_utf8(s, 0x10FFFD);
    Xapian::Unicode::append_utf8(s, 0xFFFFFFFF);
    Xapian::Unicode::append_utf8(s, 'z');
    TEST_STRINGS_EQUAL(s, "a"
			  "\xc2\x80"
			  "\xc2\xa0"
			  "\xef\xbf\xbf"
			  "\xf0\xa8\xa8\x8f"
			  "\xf0\x90\x8d\x85"
			  "\xf4\x8f\xbf\xbd"
			  ""
			  "z"
			  );

    return true;
}

DEFINE_TESTCASE(unicodepredicates1,!backend) {
    const unsigned wordchars[] = {
	// DECIMAL_DIGIT_NUMER
	'0', '7', '9',
	// LOWERCASE_LETTER
	'a', 'z', 0x250, 0x251, 0x271, 0x3d7,
	0x242, // (added in Unicode 5.0.0)
	// LOWERCASE_LETTER (added in Unicode 5.1.0)
	0x371, 0x373, 0x377, 0x514, 0x516, 0x518, 0x51a, 0x51c, 0x51e,
	0x520, 0x522,
	// UPPERCASE_LETTER
	'A', 'Z', 0x241,
	// UPPERCASE_LETTER (added in Unicode 5.1.0)
	0x370, 0x372, 0x376, 0x3cf, 0x515, 0x517, 0x519, 0x51b, 0x51d, 0x51f,
	0x521, 0x523, 0x2c6d, 0x2c6e, 0x2c6f,
	// OTHER_LETTER
	0x10345,
	// MODIFIER_LETTER (added in Unicode 5.1.0)
	0x2ec, 0x374,
	// NON_SPACING_MARK (added to is_wordchar() in 1.1.0)
	0x651,
	0x487, // Added in Unicode 5.1.0
	0
    };
    const unsigned currency[] = {
	// CURRENCY_SYMBOL
	'$', 0xa3,
	0
    };
    const unsigned whitespace[] = {
	// CONTROL
	'\t', '\n', '\f', '\r',
	// SPACE_SEPARATOR
	' ',
	0
    };
    const unsigned other[] = {
	// DASH_PUNCTUATION (added in Unicode 5.1.0)
	0x5be,
	// OTHER_SYMBOL (added in Unicode 5.1.0)
	0x1f093,
	// UNASSIGNED
	0xffff, 0x10ffff, 0x110000, 0xFFFFFFFF,
	// PRIVATE_USE
	0x10fffd,
	0
    };

    for (const unsigned * p = wordchars; *p; ++p) {
	TEST(Xapian::Unicode::is_wordchar(*p));
	TEST(!Xapian::Unicode::is_currency(*p));
	TEST(!Xapian::Unicode::is_whitespace(*p));
    }

    for (const unsigned * p = currency; *p; ++p) {
	TEST(!Xapian::Unicode::is_wordchar(*p));
	TEST(Xapian::Unicode::is_currency(*p));
	TEST(!Xapian::Unicode::is_whitespace(*p));
    }

    for (const unsigned * p = whitespace; *p; ++p) {
	TEST(!Xapian::Unicode::is_wordchar(*p));
	TEST(!Xapian::Unicode::is_currency(*p));
	TEST(Xapian::Unicode::is_whitespace(*p));
    }

    for (const unsigned * p = other; *p; ++p) {
	TEST(!Xapian::Unicode::is_wordchar(*p));
	TEST(!Xapian::Unicode::is_currency(*p));
	TEST(!Xapian::Unicode::is_whitespace(*p));
    }

    return true;
}
