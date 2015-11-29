// deelx.h
//
// DEELX Regular Expression Engine (v1.2)
//
// Copyright 2006 (c) RegExLab.com
// All Rights Reserved.
//
// http://www.regexlab.com/deelx/
//
// Author:  ∑ ŸŒ∞ (sswater shi)
// sswater@gmail.com
//
// $Revision: 724 $
//

#ifndef __DEELX_REGEXP__H__
#define __DEELX_REGEXP__H__

#include <memory.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
	typedef int (*POSIX_FUNC)(int);
	int isblank(int c);
}

//
// Data Reference
//
template <class ELT> class CBufferRefT
{
public:
	CBufferRefT(const ELT * pcsz, int length);
	CBufferRefT(const ELT * pcsz);

public:
	int nCompare      (const ELT * pcsz) const;
	int nCompareNoCase(const ELT * pcsz) const;
	int  Compare      (const ELT * pcsz) const;
	int  CompareNoCase(const ELT * pcsz) const;
	int  Compare      (const CBufferRefT <ELT> &) const;
	int  CompareNoCase(const CBufferRefT <ELT> &) const;

	ELT At          (int nIndex, ELT def = 0) const;
	ELT operator [] (int nIndex) const;

	const ELT * GetBuffer() const;
	int GetSize() const;

public:
	virtual ~CBufferRefT();

// Content
protected:
	const ELT * m_pRef;
	int         m_nSize;
};

//
// Implemenation
//


























//
// Data Buffer
//
template <class ELT> class CBufferT : public CBufferRefT <ELT>
{
public:
	CBufferT(const ELT * pcsz, int length);
	CBufferT(const ELT * pcsz);
	CBufferT();

public:
	ELT & operator [] (int nIndex);
	const ELT & operator [] (int nIndex) const;
	void  Append(const ELT * pcsz, int length, int eol = 0);
	void  Append(ELT el, int eol = 0);

public:
	void  Push(ELT   el);
	int   Pop (ELT & el);
	int   Peek(ELT & el) const;

public:
	const ELT * GetBuffer() const;
	ELT * GetBuffer();
	ELT * Detach();
	void  Release();
	void  Prepare(int index, int fill = 0);
	void  Restore(int size);

public:
	virtual ~CBufferT();

// Content
protected:
	ELT * m_pBuffer;
	int   m_nMaxLength;
};

//
// Implemenation
//


































//
// Context
//
class CContext
{
public:
	CBufferT <int> m_stack;
	CBufferT <int> m_capturestack, m_captureindex;

public:
	int    m_nCurrentPos;
	int    m_nBeginPos;
	int    m_nLastBeginPos;
	int    m_nParenZindex;

	void * m_pMatchString;
	int    m_pMatchStringLength;
};

//
// Interface
//
class ElxInterface
{
public:
	virtual int Match    (CContext * pContext) const = 0;
	virtual int MatchNext(CContext * pContext) const = 0;

public:
	virtual ~ElxInterface() {};
};

//
// Alternative
//
template <int x> class CAlternativeElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CAlternativeElxT();

public:
	CBufferT <ElxInterface *> m_elxlist;
};

typedef CAlternativeElxT <0> CAlternativeElx;

//
// Assert
//
template <int x> class CAssertElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CAssertElxT(ElxInterface * pelx, int byes = 1);

public:
	ElxInterface * m_pelx;
	int m_byes;
};

typedef CAssertElxT <0> CAssertElx;

//
// Back reference elx
//
template <class CHART> class CBackrefElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CBackrefElxT(int nnumber, int brightleft, int bignorecase);

public:
	int m_nnumber;
	int m_brightleft;
	int m_bignorecase;

	CBufferT <CHART> m_szNamed;
};

//
// Implementation
//






// RCHART
#ifndef RCHART
	#define RCHART(ch) ((CHART)ch)
#endif

// BOUNDARY_TYPE
enum BOUNDARY_TYPE
{
	BOUNDARY_FILE_BEGIN, // begin of whole text
	BOUNDARY_FILE_END  , // end of whole text
	BOUNDARY_FILE_END_N, // end of whole text, or before newline at the end
	BOUNDARY_LINE_BEGIN, // begin of line
	BOUNDARY_LINE_END  , // end of line
	BOUNDARY_WORD_BEGIN, // begin of word
	BOUNDARY_WORD_END  , // end of word
	BOUNDARY_WORD_EDGE
};

//
// Boundary Elx
//
template <class CHART> class CBoundaryElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CBoundaryElxT(int ntype, int byes = 1);

protected:
	static int IsWordChar(CHART ch);

public:
	int m_ntype;
	int m_byes;
};

//
// Implementation
//








//
// Bracket
//
template <class CHART> class CBracketElxT : public ElxInterface  
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CBracketElxT(int nnumber, int bright);
	int CheckCaptureIndex(int & index, CContext * pContext) const;

public:
	int m_nnumber;
	int m_bright;

	CBufferT <CHART> m_szNamed;
};





//
// capturestack[index+0] => Group number
// capturestack[index+1] => Capture start pos
// capturestack[index+2] => Capture end pos
// capturestack[index+3] => Capture enclose z-index, zindex<0 means inner group with same name
//




//
// Deletage
//
template <class CHART> class CDelegateElxT : public ElxInterface  
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CDelegateElxT(int ndata = 0);

public:
	ElxInterface * m_pelx;
	int m_ndata; // +0 : recursive to
	             // -3 : named recursive

	CBufferT <CHART> m_szNamed;
};







//
// Empty
//
template <int x> class CEmptyElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CEmptyElxT();
};

typedef CEmptyElxT <0> CEmptyElx;

//
// Global
//
template <int x> class CGlobalElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CGlobalElxT();
};

typedef CGlobalElxT <0> CGlobalElx;

//
// Repeat
//
template <int x> class CRepeatElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CRepeatElxT(ElxInterface * pelx, int ntimes);

protected:
	int MatchFixed    (CContext * pContext) const;
	int MatchNextFixed(CContext * pContext) const;

public:
	ElxInterface * m_pelx;
	int m_nfixed;
};

typedef CRepeatElxT <0> CRepeatElx;

//
// Greedy
//
template <int x> class CGreedyElxT : public CRepeatElxT <x>
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CGreedyElxT(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);

protected:
	int MatchVart    (CContext * pContext) const;
	int MatchNextVart(CContext * pContext) const;

public:
	int m_nvart;
};

typedef CGreedyElxT <0> CGreedyElx;

//
// Independent
//
template <int x> class CIndependentElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CIndependentElxT(ElxInterface * pelx);

public:
	ElxInterface * m_pelx;
};

typedef CIndependentElxT <0> CIndependentElx;

//
// List
//
template <int x> class CListElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CListElxT(int brightleft);

public:
	CBufferT <ElxInterface *> m_elxlist;
	int m_brightleft;
};

typedef CListElxT <0> CListElx;

//
// Posix Elx
//
template <class CHART> class CPosixElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CPosixElxT(const char * posix, int brightleft);

public:
	POSIX_FUNC m_posixfun;
	int m_brightleft;
	int m_byes;
};

//
// Implementation
//


inline int isblank(int c)
{
	return c == 0x20 || c == '\t';
}





//
// Possessive
//
template <int x> class CPossessiveElxT : public CGreedyElxT <x>
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CPossessiveElxT(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);
};

typedef CPossessiveElxT <0> CPossessiveElx;

//
// Range Elx
//
template <class CHART> class CRangeElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CRangeElxT(int brightleft, int byes);

public:
	int IsContainChar(CHART ch) const;

public:
	CBufferT <CHART> m_ranges;
	CBufferT <CHART> m_chars;
	CBufferT <ElxInterface *> m_embeds;

public:
	int m_brightleft;
	int m_byes;
};

//
// Implementation
//








//
// Reluctant
//
template <int x> class CReluctantElxT : public CRepeatElxT <x>
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CReluctantElxT(ElxInterface * pelx, int nmin = 0, int nmax = INT_MAX);

protected:
	int MatchVart    (CContext * pContext) const;
	int MatchNextVart(CContext * pContext) const;

public:
	int m_nvart;
};

typedef CReluctantElxT <0> CReluctantElx;

//
// String Elx
//
template <class CHART> class CStringElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CStringElxT(const CHART * fixed, int nlength, int brightleft, int bignorecase);

public:
	CBufferT <CHART> m_szPattern;
	int m_brightleft;
	int m_bignorecase;
};

//
// Implementation
//






//
// CConditionElx
//
template <class CHART> class CConditionElxT : public ElxInterface
{
public:
	int Match    (CContext * pContext) const;
	int MatchNext(CContext * pContext) const;

public:
	CConditionElxT();

public:
	// backref condition
	int m_nnumber;
	CBufferT <CHART> m_szNamed;

	// elx condition
	ElxInterface * m_pelxask;

	// selection
	ElxInterface * m_pelxyes, * m_pelxno;
};







//
// MatchResult
//
template <int x> class MatchResultT
{
public:
	int IsMatched() const;

public:
	int GetStart() const;
	int GetEnd  () const;

public:
	int MaxGroupNumber() const;
	int GetGroupStart(int nGroupNumber) const;
	int GetGroupEnd  (int nGroupNumber) const;

public:
	MatchResultT(const MatchResultT <x> & from) { *this = from; }
	MatchResultT(CContext * pContext = 0, int nMaxNumber = -1);
	MatchResultT <x> & operator = (const MatchResultT <x> &);
	inline operator int() const { return IsMatched(); }

public:
	CBufferT <int> m_result;
};

typedef MatchResultT <0> MatchResult;

// Stocked Elx IDs
enum STOCKELX_ID_DEFINES
{
	STOCKELX_EMPTY = 0,

	///////////////////////

	STOCKELX_DOT_ALL,
	STOCKELX_DOT_NOT_ALL,

	STOCKELX_WORD,
	STOCKELX_WORD_NOT,

	STOCKELX_SPACE,
	STOCKELX_SPACE_NOT,

	STOCKELX_DIGITAL,
	STOCKELX_DIGITAL_NOT,

	//////////////////////

	STOCKELX_DOT_ALL_RIGHTLEFT,
	STOCKELX_DOT_NOT_ALL_RIGHTLEFT,

	STOCKELX_WORD_RIGHTLEFT,
	STOCKELX_WORD_RIGHTLEFT_NOT,

	STOCKELX_SPACE_RIGHTLEFT,
	STOCKELX_SPACE_RIGHTLEFT_NOT,

	STOCKELX_DIGITAL_RIGHTLEFT,
	STOCKELX_DIGITAL_RIGHTLEFT_NOT,

	/////////////////////

	STOCKELX_COUNT
};

// REGEX_FLAGS
#ifndef _REGEX_FLAGS_DEFINED
	enum REGEX_FLAGS
	{
		NO_FLAG        = 0,
		SINGLELINE     = 0x01,
		MULTILINE      = 0x02,
		GLOBAL         = 0x04,
		IGNORECASE     = 0x08,
		RIGHTTOLEFT    = 0x10,
		EXTENDED       = 0x20
	};
	#define _REGEX_FLAGS_DEFINED
#endif

//
// Builder T
//
template <class CHART> class CBuilderT
{
public:
	typedef CDelegateElxT  <CHART> CDelegateElx;
	typedef CBracketElxT   <CHART> CBracketElx;
	typedef CBackrefElxT   <CHART> CBackrefElx;
	typedef CConditionElxT <CHART> CConditionElx;

// Methods
public:
	ElxInterface * Build(const CBufferRefT <CHART> & pattern, int flags);
	int GetNamedNumber(const CBufferRefT <CHART> & named) const;
	void Clear();

public:
	 CBuilderT();
	~CBuilderT();

// Public Attributes
public:
	ElxInterface * m_pTopElx;
	int            m_nFlags;
	int            m_nMaxNumber;
	int            m_nNextNamed;
	int            m_nGroupCount;

	CBufferT <ElxInterface  *> m_objlist;
	CBufferT <ElxInterface  *> m_grouplist;
	CBufferT <CDelegateElx  *> m_recursivelist;
	CBufferT <CListElx      *> m_namedlist;
	CBufferT <CBackrefElx   *> m_namedbackreflist;
	CBufferT <CConditionElx *> m_namedconditionlist;

// CHART_INFO
protected:
	struct CHART_INFO
	{
	public:
		CHART ch;
		int   type;
		int   pos;
		int   len;

	public:
		CHART_INFO(CHART c, int t, int p = 0, int l = 0) { ch = c; type = t; pos = p; len = l;    }
		inline int operator == (const CHART_INFO & ci)   { return ch == ci.ch && type == ci.type; }
		inline int operator != (const CHART_INFO & ci)   { return ! operator == (ci);             }
	};

protected:
	static unsigned int Hex2Int(const CHART * pcsz, int length, int & used);
	static int ReadDec(char * & str, unsigned int & dec);
	void MoveNext();
	int  GetNext2();

	ElxInterface * BuildAlternative(int vaflags);
	ElxInterface * BuildList       (int & flags);
	ElxInterface * BuildRepeat     (int & flags);
	ElxInterface * BuildSimple     (int & flags);
	ElxInterface * BuildCharset    (int & flags);
	ElxInterface * BuildRecursive  (int & flags);
	ElxInterface * BuildBoundary   (int & flags);
	ElxInterface * BuildBackref    (int & flags);

	ElxInterface * GetStockElx     (int nStockId);
	ElxInterface * Keep(ElxInterface * pElx);

// Private Attributes
protected:
	CBufferRefT <CHART> m_pattern;
	CHART_INFO prev, curr, next, nex2;
	int m_nNextPos;
	int m_nCharsetDepth;
	int m_bQuoted;
	POSIX_FUNC m_quote_fun;

	ElxInterface * m_pStockElxs[STOCKELX_COUNT];
};

//
// Implementation
//










//
// hex to int
//


















#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))











//
// Regexp
//
template <class CHART> class CRegexpT
{
public:
	CRegexpT(const CHART * pattern = 0, int flags = 0);
	CRegexpT(const CHART * pattern, int length, int flags);
	void Compile(const CHART * pattern, int flags = 0);
	void Compile(const CHART * pattern, int length, int flags);

public:
	MatchResult MatchExact(const CHART * tstring, CContext * pContext = 0) const;
	MatchResult MatchExact(const CHART * tstring, int length, CContext * pContext = 0) const;
	MatchResult Match(const CHART * tstring, int start = -1, CContext * pContext = 0) const;
	MatchResult Match(const CHART * tstring, int length, int start, CContext * pContext = 0) const;
	MatchResult Match(CContext * pContext) const;
	CContext * PrepareMatch(const CHART * tstring, int start = -1, CContext * pContext = 0) const;
	CContext * PrepareMatch(const CHART * tstring, int length, int start, CContext * pContext = 0) const;
	CHART * Replace(const CHART * tstring, const CHART * replaceto, int start = -1, int ntimes = -1, MatchResult * result = 0, CContext * pContext = 0) const;
	CHART * Replace(const CHART * tstring, int string_length, const CHART * replaceto, int to_length, int & result_length, int start = -1, int ntimes = -1, MatchResult * result = 0, CContext * pContext = 0) const;
	int GetNamedGroupNumber(const CHART * group_name) const;

public:
	static void ReleaseString (CHART    * tstring );
	static void ReleaseContext(CContext * pContext);

public:
	CBuilderT <CHART> m_builder;
};

//
// Implementation
//
































//
// All implementations
//






// assertx.cpp: implementation of the CAssertElx class.
//






// emptyelx.cpp: implementation of the CEmptyElx class.
//






// globalx.cpp: implementation of the CGlobalElx class.
//






// greedelx.cpp: implementation of the CGreedyElx class.
//










// indepelx.cpp: implementation of the CIndependentElx class.
//






// listelx.cpp: implementation of the CListElx class.
//






// mresult.cpp: implementation of the MatchResult class.
//
















// posselx.cpp: implementation of the CPossessiveElx class.
//






// reluctx.cpp: implementation of the CReluctantElx class.
//










// repeatx.cpp: implementation of the CRepeatElx class.
//










// Regexp
typedef CRegexpT <char> CRegexpA;
typedef CRegexpT <unsigned short> CRegexpW;

#if defined(_UNICODE) || defined(UNICODE)
	typedef CRegexpW CRegexp;
#else
	typedef CRegexpA CRegexp;
#endif

#endif//__DEELX_REGEXP__H__
