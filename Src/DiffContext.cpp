/////////////////////////////////////////////////////////////////////////////
//    WinMerge:  an interactive diff/merge utility
//    Copyright (C) 1997-2000  Thingamahoochie Software
//    Author: Dean Grimm
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////
/**
 *  @file DiffContext.cpp
 *
 *  @brief Implementation of CDiffContext
 */ 
// ID line follows -- this is updated by SVN
// $Id$
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Merge.h"
#include "CompareOptions.h"
#include "CompareStats.h"
#include "Common/version.h"
#include "FilterList.h"
#include "LineFiltersList.h"
#include "DiffContext.h"
#include "paths.h"
#include "coretools.h"
#include "codepage_detect.h"
#include "DiffItemList.h"
#include "IAbortable.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/**
 * @brief Construct CDiffContext.
 *
 * @param [in] pszLeft Initial left-side path.
 * @param [in] pszRight Initial right-side path.
 * @param [in] compareMethod Main compare method for this compare.
 */
CDiffContext::CDiffContext(LPCTSTR pszLeft, LPCTSTR pszRight)
: m_piFilterGlobal(NULL)
, m_nCompMethod(0)
, m_nCurrentCompMethod(0)
, m_bIgnoreSmallTimeDiff(false)
, m_pCompareStats(NULL)
, m_piAbortable(NULL)
, m_bStopAfterFirstDiff(false)
, m_pFilterList(new FilterList)
, m_nRecursive(0)
, m_bWalkUniques(true)
, m_paths(2)
{
	memset(&m_options, 0, sizeof m_options);
	m_paths[0] = paths_GetLongPath(pszLeft);
	m_paths[1] = paths_GetLongPath(pszRight);
}

/**
 * @brief Destructor.
 */
CDiffContext::~CDiffContext()
{
	delete m_pFilterList;
}

/**
 * @brief Update info in item in result list from disk.
 * This function updates result list item's file information from actual
 * file in the disk. This updates info like date, size and attributes.
 * @param [in] diffpos DIFFITEM to update.
 * @param [in] bLeft Update left-side info.
 * @param [in] bRight Update right-side info.
 */
void CDiffContext::UpdateStatusFromDisk(UINT_PTR diffpos, BOOL bLeft, BOOL bRight)
{
	DIFFITEM &di = GetDiffAt(diffpos);
	if (bLeft)
	{
		di.left.ClearPartial();
		if (!di.diffcode.isSideRightOnly())
			UpdateInfoFromDiskHalf(di, TRUE);
	}
	if (bRight)
	{
		di.right.ClearPartial();
		if (!di.diffcode.isSideLeftOnly())
			UpdateInfoFromDiskHalf(di, FALSE);
	}
}

/**
 * @brief Update file information from disk for DIFFITEM.
 * This function updates DIFFITEM's file information from actual file in
 * the disk. This updates info like date, size and attributes.
 * @param [in, out] di DIFFITEM to update (selected side, see bLeft param).
 * @param [in] bLeft If TRUE left side information is updated,
 *  right side otherwise.
 * @return TRUE if file exists
 */
BOOL CDiffContext::UpdateInfoFromDiskHalf(DIFFITEM & di, BOOL bLeft)
{
	String filepath;

	if (bLeft == TRUE)
		filepath = paths_ConcatPath(di.GetLeftFilepath(GetLeftPath()), di.left.filename);
	else
		filepath = paths_ConcatPath(di.GetRightFilepath(GetRightPath()), di.right.filename);

	DiffFileInfo & dfi = bLeft ? di.left : di.right;
	if (!dfi.Update(filepath.c_str()))
		return FALSE;
	UpdateVersion(di, bLeft);
	GuessCodepageEncoding(filepath.c_str(), &dfi.encoding, m_bGuessEncoding);
	return TRUE;
}

/**
 * @brief Determine if file is one to have a version information.
 * This function determines if the given file has a version information
 * attached into it in resource. This is done by comparing file extension to
 * list of known filename extensions usually to have a version information.
 * @param [in] ext Extension to check.
 * @return true if extension has version info, false otherwise.
 */
static bool CheckFileForVersion(LPCTSTR ext)
{
	if (!lstrcmpi(ext, _T(".EXE")) || !lstrcmpi(ext, _T(".DLL")) || !lstrcmpi(ext, _T(".SYS")) ||
	    !lstrcmpi(ext, _T(".DRV")) || !lstrcmpi(ext, _T(".OCX")) || !lstrcmpi(ext, _T(".CPL")) ||
	    !lstrcmpi(ext, _T(".SCR")) || !lstrcmpi(ext, _T(".LANG")))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief Load file version from disk.
 * Update fileversion for given item and side from disk. Note that versions
 * are read from only some filetypes. See CheckFileForVersion() function
 * for list of files to check versions.
 * @param [in,out] di DIFFITEM to update.
 * @param [in] bLeft If TRUE left-side file is updated, right-side otherwise.
 */
void CDiffContext::UpdateVersion(DIFFITEM & di, BOOL bLeft) const
{
	DiffFileInfo & dfi = bLeft ? di.left : di.right;
	// Check only binary files
	dfi.version.Clear();
	dfi.bVersionChecked = true;

	if (di.diffcode.isDirectory())
		return;
	
	String spath;
	if (bLeft)
	{
		if (di.diffcode.isSideRightOnly())
			return;
		LPCTSTR ext = PathFindExtension(di.left.filename.c_str());
		if (!CheckFileForVersion(ext))
			return;
		spath = di.GetLeftFilepath(GetLeftPath());
		spath = paths_ConcatPath(spath, di.left.filename);
	}
	else
	{
		if (di.diffcode.isSideLeftOnly())
			return;
		LPCTSTR ext = PathFindExtension(di.right.filename.c_str());
		if (!CheckFileForVersion(ext))
			return;
		spath = di.GetRightFilepath(GetRightPath());
		spath = paths_ConcatPath(spath, di.right.filename);
	}
	
	// Get version info if it exists
	CVersionInfo ver(spath.c_str());
	DWORD verMS = 0;
	DWORD verLS = 0;
	if (ver.GetFixedFileVersion(verMS, verLS))
		dfi.version.SetFileVersion(verMS, verLS);
}

/**
 * @brief Check if user has requested aborting the compare.
 * @return true if user has requested abort, false otherwise.
 */
bool CDiffContext::ShouldAbort() const
{
	return m_piAbortable && m_piAbortable->ShouldAbort();
}
