/** 
 * @file  PropCompareFolder.cpp
 *
 * @brief Implementation of PropCompareFolder propertysheet
 */
// ID line follows -- this is updated by SVN
// $Id$

#include "StdAfx.h"
#include "OptionsPanel.h"
#include "resource.h"
#include "LanguageSelect.h"
#include "PropCompareFolder.h"
#include "OptionsDef.h"
#include "OptionsMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int Mega = 1024 * 1024;

/** 
 * @brief Constructor.
 * @param [in] optionsMgr Pointer to CCOptionsMgr::
 */
PropCompareFolder::PropCompareFolder()
: OptionsPanel(IDD_PROPPAGE_COMPARE_FOLDER)
, m_compareMethod(-1)
, m_bStopAfterFirst(FALSE)
, m_bIgnoreSmallTimeDiff(FALSE)
, m_bIncludeUniqFolders(TRUE)
, m_nQuickCompareLimit(4 * Mega)
{
}

template<ODialog::DDX_Operation op>
bool PropCompareFolder::UpdateData()
{
	DDX_CBIndex<op>(IDC_COMPAREMETHODCOMBO, m_compareMethod);
	DDX_Check<op>(IDC_COMPARE_STOPFIRST, m_bStopAfterFirst);
	DDX_Check<op>(IDC_IGNORE_SMALLTIMEDIFF, m_bIgnoreSmallTimeDiff);
	DDX_Check<op>(IDC_COMPARE_WALKSUBDIRS, m_bIncludeUniqFolders);
	DDX_Text<op>(IDC_COMPARE_QUICKC_LIMIT, m_nQuickCompareLimit);
	return true;
}

LRESULT PropCompareFolder::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		if (IsUserInputCommand(wParam))
			UpdateData<Get>();
		switch (wParam)
		{
		case MAKEWPARAM(IDC_COMPAREFOLDER_DEFAULTS, BN_CLICKED):
			OnDefaults();
			break;
		case MAKEWPARAM(IDC_COMPAREMETHODCOMBO, CBN_SELCHANGE):
			UpdateScreen();
			break;
		}
		break;
	}
	return OptionsPanel::WindowProc(message, wParam, lParam);
}

/** 
 * @brief Reads options values from storage to UI.
 * Property sheet calls this before displaying GUI to load values
 * into members.
 */
void PropCompareFolder::ReadOptions()
{
	m_compareMethod = COptionsMgr::Get(OPT_CMP_METHOD);
	m_bStopAfterFirst = COptionsMgr::Get(OPT_CMP_STOP_AFTER_FIRST);
	m_bIgnoreSmallTimeDiff = COptionsMgr::Get(OPT_IGNORE_SMALL_FILETIME);
	m_bIncludeUniqFolders = COptionsMgr::Get(OPT_CMP_WALK_UNIQUE_DIRS);
	m_nQuickCompareLimit = COptionsMgr::Get(OPT_CMP_QUICK_LIMIT) / Mega;
}

/** 
 * @brief Writes options values from UI to storage.
 * Property sheet calls this after dialog is closed with OK button to
 * store values in member variables.
 */
void PropCompareFolder::WriteOptions()
{
	COptionsMgr::SaveOption(OPT_CMP_METHOD, (int)m_compareMethod);
	COptionsMgr::SaveOption(OPT_CMP_STOP_AFTER_FIRST, m_bStopAfterFirst == TRUE);
	COptionsMgr::SaveOption(OPT_IGNORE_SMALL_FILETIME, m_bIgnoreSmallTimeDiff == TRUE);
	COptionsMgr::SaveOption(OPT_CMP_WALK_UNIQUE_DIRS, m_bIncludeUniqFolders == TRUE);

	if (m_nQuickCompareLimit > 2000)
		m_nQuickCompareLimit = 2000;
	COptionsMgr::SaveOption(OPT_CMP_QUICK_LIMIT, m_nQuickCompareLimit * Mega);
}

/** 
 * @brief Called before propertysheet is drawn.
 */
BOOL PropCompareFolder::OnInitDialog()
{
	if (HComboBox *combo = static_cast<HComboBox *>(GetDlgItem(IDC_COMPAREMETHODCOMBO)))
	{
		String item = LanguageSelect.LoadString(IDS_COMPMETHOD_FULL_CONTENTS);
		combo->AddString(item.c_str());
		item = LanguageSelect.LoadString(IDS_COMPMETHOD_QUICK_CONTENTS);
		combo->AddString(item.c_str());
		item = LanguageSelect.LoadString(IDS_COMPMETHOD_MODDATE);
		combo->AddString(item.c_str());
		item = LanguageSelect.LoadString(IDS_COMPMETHOD_DATESIZE);
		combo->AddString(item.c_str());
		item = LanguageSelect.LoadString(IDS_COMPMETHOD_SIZE);
		combo->AddString(item.c_str());
	}
	return OptionsPanel::OnInitDialog();
}

/** 
 * @brief Sets options to defaults
 */
void PropCompareFolder::OnDefaults()
{
	m_compareMethod = COptionsMgr::GetDefault(OPT_CMP_METHOD);
	m_bStopAfterFirst = COptionsMgr::GetDefault(OPT_CMP_STOP_AFTER_FIRST);
	m_bIncludeUniqFolders = COptionsMgr::GetDefault(OPT_CMP_WALK_UNIQUE_DIRS);
	m_nQuickCompareLimit = COptionsMgr::GetDefault(OPT_CMP_QUICK_LIMIT) / Mega;
	UpdateScreen();
}

/** 
 * @brief Called when compare method dropdown selection is changed.
 * Enables / disables "Stop compare after first difference" checkbox.
 * That checkbox is valid only for quick contents compare method.
 */
void PropCompareFolder::UpdateScreen()
{
	UpdateData<Set>();
	HComboBox *combo = static_cast<HComboBox *>(GetDlgItem(IDC_COMPAREMETHODCOMBO));
	GetDlgItem(IDC_COMPARE_STOPFIRST)->EnableWindow(combo->GetCurSel() == 1);
}
