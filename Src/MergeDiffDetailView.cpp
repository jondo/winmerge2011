//////////////////////////////////////////////////////////////////////
/** 
 * @file  MergeDiffDetailView.cpp
 *
 * @brief Implementation file for CMergeDiffDetailView
 *
 */
// ID line follows -- this is updated by SVN
// $Id$
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Merge.h"
#include "LanguageSelect.h"
#include "MergeDiffDetailView.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "SyntaxColors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using stl::vector;

/////////////////////////////////////////////////////////////////////////////
// CMergeDiffDetailView

/**
 * @brief Constructor.
 */
CMergeDiffDetailView::CMergeDiffDetailView(CChildFrame *pDocument, int nThisPane)
: CGhostTextView(sizeof *this)
, m_pDocument(pDocument)
, m_nThisPane(nThisPane)
{
}

CMergeDiffDetailView::~CMergeDiffDetailView()
{
}

LRESULT CMergeDiffDetailView::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		SetDisplayHeight(SHORT HIWORD(lParam));
		break;
	case WM_CONTEXTMENU:
		OnContextMenu(lParam);
		break;
	}
	return CCrystalTextView::WindowProc(uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// CMergeDiffDetailView message handlers

/**
 * @brief Get TextBuffer associated with view.
 * @return pointer to textbuffer associated with the view.
 */
CCrystalTextBuffer *CMergeDiffDetailView::LocateTextBuffer()
{
	return m_pDocument->m_ptBuf[m_nThisPane];
}

/**
 * @brief Update any resources necessary after a GUI language change
 */
void CMergeDiffDetailView::UpdateResources()
{
}

/**
 * @brief Set view's height.
 * @param [in] h new view height in pixels
 */
void CMergeDiffDetailView::SetDisplayHeight(int h) 
{
	int lineHeight = GetLineHeight();
	if (h >= lineHeight)
		h -= lineHeight / 2;
	m_displayLength = h / lineHeight;
}

/// virtual, ensure we remain in diff
void CMergeDiffDetailView::SetSelection(const POINT &ptStart, const POINT &ptEnd)
{
	POINT ptStartNew = ptStart;
	EnsureInDiff(ptStartNew);
	POINT ptEndNew = ptEnd;
	EnsureInDiff(ptEndNew);
	EnsureInDiff(m_ptCursorPos);
	CCrystalTextView::SetSelection(ptStartNew, ptEndNew);
}

void CMergeDiffDetailView::OnInitialUpdate()
{
	SetFont(theApp.m_pMainWnd->m_lfDiff);
}

int CMergeDiffDetailView::GetAdditionalTextBlocks (int nLineIndex, TEXTBLOCK *pBuf)
{
	if (nLineIndex < m_lineBegin || nLineIndex >= m_lineEnd)
		return 0;

	DWORD dwLineFlags = GetLineFlags(nLineIndex);
	if ((dwLineFlags & LF_DIFF) != LF_DIFF)
		return 0; // No diff

	if (!COptionsMgr::Get(OPT_WORDDIFF_HIGHLIGHT))
		return 0; // No coloring

	int nLineLength = GetLineLength(nLineIndex);
	vector<wdiff> worddiffs;
	m_pDocument->GetWordDiffArray(nLineIndex, worddiffs);

	if (nLineLength == 0 || worddiffs.size() == 0 || // Both sides are empty
		IsSide0Empty(worddiffs, nLineLength) || IsSide1Empty(worddiffs, nLineLength))
	{
		return 0;
	}

	int nWordDiffs = worddiffs.size();

	pBuf[0].m_nCharPos = 0;
	pBuf[0].m_nColorIndex = COLORINDEX_NONE;
	pBuf[0].m_nBgColorIndex = COLORINDEX_NONE;
	for (int i = 0; i < nWordDiffs; i++)
	{
		const wdiff &wd = worddiffs[i];
		++pBuf;
		pBuf->m_nCharPos = wd.start[m_nThisPane];
		pBuf->m_nColorIndex = COLORINDEX_HIGHLIGHTTEXT1 | COLORINDEX_APPLYFORCE;
		if (wd.start[0] == wd.end[0] + 1 || wd.start[1] == wd.end[1] + 1)
			// Case on one side char/words are inserted or deleted 
			pBuf->m_nBgColorIndex = COLORINDEX_HIGHLIGHTBKGND3 | COLORINDEX_APPLYFORCE;
		else
			pBuf->m_nBgColorIndex = COLORINDEX_HIGHLIGHTBKGND2 | COLORINDEX_APPLYFORCE;
		++pBuf;
		pBuf->m_nCharPos = wd.end[m_nThisPane] + 1;
		pBuf->m_nColorIndex = COLORINDEX_NONE;
		pBuf->m_nBgColorIndex = COLORINDEX_NONE;
	}
	return nWordDiffs * 2 + 1;
}

COLORREF CMergeDiffDetailView::GetColor(int nColorIndex)
{
	switch (nColorIndex & ~COLORINDEX_APPLYFORCE)
	{
	case COLORINDEX_HIGHLIGHTBKGND1:
		return COptionsMgr::Get(OPT_CLR_SELECTED_WORDDIFF);
	case COLORINDEX_HIGHLIGHTTEXT1:
		return COptionsMgr::Get(OPT_CLR_SELECTED_WORDDIFF_TEXT);
	case COLORINDEX_HIGHLIGHTBKGND2:
		return COptionsMgr::Get(OPT_CLR_WORDDIFF);
	case COLORINDEX_HIGHLIGHTTEXT2:
		return COptionsMgr::Get(OPT_CLR_WORDDIFF_TEXT);
	case COLORINDEX_HIGHLIGHTBKGND3:
		return COptionsMgr::Get(OPT_CLR_WORDDIFF_DELETED);
	default:
		return CCrystalTextView::GetColor(nColorIndex);
	}
}

/// virtual, avoid coloring the whole diff with diff color 
void CMergeDiffDetailView::GetLineColors(int nLineIndex, COLORREF &crBkgnd, COLORREF &crText)
{
	DWORD dwLineFlags = GetLineFlags(nLineIndex);
	// Line with WinMerge flag, 
	// Lines with only the LF_DIFF/LF_TRIVIAL flags are not colored with Winmerge colors
	if (dwLineFlags & (LF_WINMERGE_FLAGS & ~LF_DIFF & ~LF_TRIVIAL & ~LF_MOVED))
	{
		crText = COptionsMgr::Get(OPT_CLR_DIFF);
		if (dwLineFlags & LF_GHOST)
		{
			crBkgnd = COptionsMgr::Get(OPT_CLR_DIFF_DELETED);
		}
	}
	else
	{
		// If no syntax hilighting
		if (!COptionsMgr::Get(OPT_SYNTAX_HIGHLIGHT))
		{
			crBkgnd = GetColor(COLORINDEX_BKGND);
			crText = GetColor(COLORINDEX_NORMALTEXT);
		}
		else
		{
			// Line not inside diff, get colors from CrystalEditor
			CCrystalTextView::GetLineColors(nLineIndex, crBkgnd, crText);
		}
	}
	if (nLineIndex < m_lineBegin || nLineIndex >= m_lineEnd)
	{
		crBkgnd = GetColor(COLORINDEX_WHITESPACE);
		crText = GetColor(COLORINDEX_WHITESPACE);
	}
}

void CMergeDiffDetailView::OnDisplayDiff(int nDiff)
{
	int newlineBegin, newlineEnd;
	if (nDiff < 0 || nDiff >= m_pDocument->m_diffList.GetSize())
	{
		newlineBegin = 0;
		newlineEnd = 0;
	}
	else
	{
		DIFFRANGE curDiff;
		VERIFY(m_pDocument->m_diffList.GetDiff(nDiff, curDiff));

		newlineBegin = curDiff.dbegin0;
		ASSERT(newlineBegin >= 0);
		newlineEnd = curDiff.dend0 + 1;
	}

	if (newlineBegin == m_lineBegin && newlineEnd == m_lineEnd)
		return;

	m_lineBegin = newlineBegin;
	m_lineEnd = newlineEnd;

	// scroll to the first line of the diff
	ScrollToLine(m_lineBegin);
	// update the width of the horizontal scrollbar
	RecalcHorzScrollBar();
}

/**
 * @brief Adjust the point to remain in the displayed diff
 *
 * @return Tells if the point has been changed
 */
bool CMergeDiffDetailView::EnsureInDiff(POINT &pt)
{
	const POINT cpt = pt;
	// not above diff
	if (pt.y < m_lineBegin)
	{
		pt.y = m_lineBegin;
		pt.x = 0;
	}
	// diff is defined and not below diff
	else if (pt.y >= m_lineEnd)
	{
		pt.y = m_lineEnd;
		pt.x = 0;
	}
	return pt != cpt;
}

/// virtual, ensure we remain in diff
void CMergeDiffDetailView::ScrollToSubLine(int nNewTopLine, BOOL bNoSmoothScroll, BOOL bTrackScrollBar)
{
	if (nNewTopLine > m_lineEnd - m_displayLength)
		nNewTopLine = m_lineEnd - m_displayLength;
	if (nNewTopLine < m_lineBegin)
		nNewTopLine = m_lineBegin;

	m_nTopLine = nNewTopLine;
	
	POINT pt = GetCursorPos();
	if (EnsureInDiff(pt))
		SetCursorPos(pt);
	
	POINT ptSelStart, ptSelEnd;
	GetSelection(ptSelStart, ptSelEnd);
	if (EnsureInDiff(ptSelStart) || EnsureInDiff(ptSelEnd))
		SetSelection(ptSelStart, ptSelEnd);
	
	CCrystalTextView::ScrollToSubLine(nNewTopLine, bNoSmoothScroll, bTrackScrollBar);
}

/**
 * @brief Same purpose as the one as in MergeEditView.cpp
 * @note Nearly the same code also
 */
void CMergeDiffDetailView::UpdateSiblingScrollPos(BOOL bHorz)
{
	HWindow *pSender = reinterpret_cast<HWindow *>(m_hWnd);
	LPCTSTR pcwAtom = MAKEINTATOM(pSender->GetClassAtom());
	HWindow *pParent = pSender->GetParent();
	HWindow *pChild = NULL;
	// limit the TopLine : must be smaller than GetLineCount for all the panels
	int nNewTopLine = m_nTopLine;
	while ((pChild = pParent->FindWindowEx(pChild, pcwAtom)) != NULL)
	{
		if (pChild == pSender)
			continue;
		CMergeDiffDetailView *pSiblingView = static_cast<CMergeDiffDetailView *>(FromHandle(pChild));
		if (pSiblingView->GetLineCount() <= nNewTopLine)
			nNewTopLine = pSiblingView->GetLineCount() - 1;

	}
	if (m_nTopLine != nNewTopLine) 
	{
		// only modification from code in MergeEditView.cpp
		// Where are we now, are we still in a diff ? So set to no diff
		nNewTopLine = m_lineBegin = m_lineEnd = 0;
		ScrollToLine(nNewTopLine);
	}
	while ((pChild = pParent->FindWindowEx(pChild, pcwAtom)) != NULL)
	{
		if (pChild == pSender) // We don't need to update ourselves
			continue;
		CMergeDiffDetailView *pSiblingView = static_cast<CMergeDiffDetailView *>(FromHandle(pChild));
		pSiblingView->OnUpdateSibling(this, bHorz);
	}
}

/**
 * @brief Same purpose as the one as in MergeDiffView.cpp
 * @note Code is the same except we cast to a pointer to a CMergeDiffDetailView
 */
void CMergeDiffDetailView::OnUpdateSibling(CCrystalTextView *pUpdateSource, BOOL bHorz)
{
	if (pUpdateSource != this)
	{
		ASSERT(pUpdateSource != NULL);
		// only modification from code in MergeEditView.cpp
		CMergeDiffDetailView *pSrcView = static_cast<CMergeDiffDetailView*>(pUpdateSource);
		if (!bHorz)  // changed this so bHorz works right
		{
			ASSERT(pSrcView->m_nTopLine >= 0);
			if (pSrcView->m_nTopLine != m_nTopLine)
			{
				ScrollToLine(pSrcView->m_nTopLine, TRUE, FALSE);
				UpdateCaret();
				RecalcVertScrollBar(TRUE);
			}
		}
		else
		{
			ASSERT(pSrcView->m_nOffsetChar >= 0);
			if (pSrcView->m_nOffsetChar != m_nOffsetChar)
			{
				ScrollToChar(pSrcView->m_nOffsetChar, TRUE, FALSE);
				UpdateCaret();
				RecalcHorzScrollBar(TRUE);
			}
		}
	}
}

/*
 * @brief Compute the max length of the lines inside the displayed diff
 */
int CMergeDiffDetailView::GetDiffLineLength()
{
	int nMaxLineLength = 0;

	// we can not use GetLineActualLength below nLineCount
	// diff info (and lineBegin/lineEnd) are updated only during Rescan
	// they may get invalid just after we delete some text
	int validLineEnd = m_lineEnd;
	if (m_lineEnd > GetLineCount())
		validLineEnd = GetLineCount();

	for (int i = m_lineBegin; i < validLineEnd; ++i)
	{
		int nActualLength = GetLineActualLength(i);
		if (nMaxLineLength < nActualLength)
			nMaxLineLength = nActualLength;
	}
	return nMaxLineLength;
}


/**
 * @brief Update the horizontal scrollbar
 *
 * @note The scrollbar width is the one needed for the largest view
 * @sa ccrystaltextview::RecalcHorzScrollBar()
 */
void CMergeDiffDetailView::RecalcHorzScrollBar(BOOL bPositionOnly)
{
	// Again, we cannot use nPos because it's 16-bit
	SCROLLINFO si;
	si.cbSize = sizeof si;

	const int nScreenChars = GetScreenChars();

	// note : this value differs from the value in CCrystalTextView::RecalcHorzScrollBar
	int nMaxLineLen = 0;
	if (CMergeDiffDetailView *pView = m_pDocument->GetRightDetailView())
		nMaxLineLen = pView->GetDiffLineLength();
	if (CMergeDiffDetailView *pView = m_pDocument->GetLeftDetailView())
		nMaxLineLen = max(nMaxLineLen, pView->GetDiffLineLength());
	
	if (bPositionOnly)
	{
		si.fMask = SIF_POS;
		si.nPos = m_nOffsetChar;
	}
	else
	{
		if (nScreenChars >= nMaxLineLen && m_nOffsetChar > 0)
		{
			m_nOffsetChar = 0;
			Invalidate();
			UpdateCaret();
		}
		si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nMin = 0;
		
		// Horiz scroll limit to longest line + one screenwidth 
		si.nMax = nMaxLineLen + nScreenChars;
		si.nPage = nScreenChars;
		si.nPos = m_nOffsetChar;
	}
	if (GetStyle() & WS_HSCROLL)
		SetScrollInfo(SB_HORZ, &si);
}

void CMergeDiffDetailView::PushCursors()
{
	CGhostTextView::PushCursors();
	// push lineBegin and lineEnd
	m_lineBeginPushed = m_lineBegin;
	m_lineEndPushed = m_lineEnd;
}

void CMergeDiffDetailView::PopCursors()
{
	CGhostTextView::PopCursors();
	// pop lineBegin and lineEnd
	m_lineBegin = m_lineBeginPushed;
	m_lineEnd = m_lineEndPushed;
	// clip the involved coordinates to their valid ranges
	int nLineCount = GetLineCount();
	if (m_lineBegin >= nLineCount)
	{
		// even the first line is invalid, stop displaying the diff
		m_lineBegin = m_lineEnd = m_nTopLine = m_nTopSubLine = 0;
	}
	else
	{
		// just check that all positions all valid
		if (m_lineEnd > nLineCount)
			m_lineEnd = nLineCount;
		if (m_ptCursorPos.y >= nLineCount)
			m_ptCursorPos.y = nLineCount - 1;
		int nLineLength = GetLineLength(m_ptCursorPos.y);
		if (m_ptCursorPos.x > nLineLength)
			m_ptCursorPos.x = nLineLength;
	}
}

/**
 * @brief Offer a context menu
 */
void CMergeDiffDetailView::OnContextMenu(LPARAM lParam)
{
	POINT point;
	POINTSTOPOINT(point, lParam);
	// Create the menu and populate it with the available functions
	HMenu *const pMenu = LanguageSelect.LoadMenu(IDR_POPUP_MERGEDETAILVIEW);
	HMenu *const pSub = pMenu->GetSubMenu(0);
	// Context menu opened using keyboard has no coordinates
	if (point.x == -1 && point.y == -1)
	{
		point.x = point.y = 5;
		ClientToScreen(&point);
	}
	pSub->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		point.x, point.y, theApp.m_pMainWnd->m_pWnd);
	pMenu->DestroyMenu();
}

void CMergeDiffDetailView::OnUpdateCaret(bool bMove)
{
	CCrystalTextView::OnUpdateCaret(bMove);
	m_pDocument->UpdateCmdUI();
}

/**
 * @brief Called after document is loaded.
 * This function is called from CChildFrame::OpenDocs() after documents are
 * loaded. So this is good place to set View's options etc.
 */
void CMergeDiffDetailView::DocumentsLoaded()
{
	SetTabSize(COptionsMgr::Get(OPT_TAB_SIZE),
		COptionsMgr::Get(OPT_SEPARATE_COMBINING_CHARS));
	SetViewTabs(COptionsMgr::Get(OPT_VIEW_WHITESPACE));
	bool bMixedEOL = COptionsMgr::Get(OPT_ALLOW_MIXED_EOL) ||
		m_pDocument->IsMixedEOL(m_nThisPane);
	SetViewEols(COptionsMgr::Get(OPT_VIEW_WHITESPACE), bMixedEOL);
	SetWordWrapping(FALSE);
	SetFont(theApp.m_pMainWnd->m_lfDiff);
}
