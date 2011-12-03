/**
 * @file  ProjectFilePathsDlg.cpp
 *
 * @brief Implementation file for ProjectFilePaths dialog
 */
// ID line follows -- this is updated by SVN
// $Id$

#include "stdafx.h"
#include "Merge.h"
#include "LanguageSelect.h"
#include "MainFrm.h"
#include "paths.h"
#include "OptionsDef.h"
#include "ProjectFile.h"
#include "coretools.h"
#include "ProjectFilePathsDlg.h"
#include "FileOrFolderSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/** 
 * @brief Standard constructor.
 */
ProjectFilePathsDlg::ProjectFilePathsDlg()
: ODialog(IDD_PROJFILES_PATHS)
, m_bLeftPathReadOnly(FALSE)
, m_bRightPathReadOnly(FALSE)
{
	m_strCaption = LanguageSelect.LoadDialogCaption(m_idd);
}

template<ODialog::DDX_Operation op>
bool ProjectFilePathsDlg::UpdateData()
{
	DDX_Text<op>(IDC_PROJ_LFILE_EDIT, m_sLeftFile);
	DDX_Text<op>(IDC_PROJ_RFILE_EDIT, m_sRightFile);
	DDX_Text<op>(IDC_PROJ_FILTER_EDIT, m_sFilter);
	DDX_Check<op>(IDC_PROJ_INC_SUBFOLDERS, m_bIncludeSubfolders);
	DDX_Check<op>(IDC_PROJFILE_LREADONLY, m_bLeftPathReadOnly);
	DDX_Check<op>(IDC_PROJFILE_RREADONLY, m_bRightPathReadOnly);
	return true;
}

LRESULT ProjectFilePathsDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case MAKEWPARAM(IDC_PROJ_LFILE_BROWSE, BN_CLICKED):
			OnBnClickedProjLfileBrowse();
			break;
		case MAKEWPARAM(IDC_PROJ_RFILE_BROWSE, BN_CLICKED):
			OnBnClickedProjRfileBrowse();
			break;
		case MAKEWPARAM(IDC_PROJ_FILTER_SELECT, BN_CLICKED):
			OnBnClickedProjFilterSelect();
			break;
		case MAKEWPARAM(IDC_PROJ_OPEN, BN_CLICKED):
			OnBnClickedProjOpen();
			break;
		case MAKEWPARAM(IDC_PROJ_SAVE, BN_CLICKED):
			OnBnClickedProjSave();
			break;
		}
		break;
	}
	return ODialog::WindowProc(message, wParam, lParam);
}

/** 
 * @brief Initialize dialog.
 */
BOOL ProjectFilePathsDlg::OnInitDialog() 
{
	// Cancel out any invalid paths
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(
			paths_GetLongPath(m_sLeftFile.c_str()).c_str()))
	{
		m_sLeftFile.clear();
	}
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(
			paths_GetLongPath(m_sRightFile.c_str()).c_str()))
	{
		m_sRightFile.clear();
	}
	ODialog::OnInitDialog();
	LanguageSelect.TranslateDialog(m_hWnd);
	UpdateData<Set>();
	return FALSE;
}

/** 
 * @brief Called when Browse-button for left path is selected.
 */
void ProjectFilePathsDlg::OnBnClickedProjLfileBrowse()
{
	String s;
	if (::SelectFileOrFolder(m_hWnd, s, m_sLeftFile.c_str()))
		SetDlgItemText(IDC_PROJ_LFILE_EDIT, s.c_str());
}

/** 
 * @brief Called when Browse-button for right path is selected.
 */
void ProjectFilePathsDlg::OnBnClickedProjRfileBrowse()
{
	String s;
	if (::SelectFileOrFolder(m_hWnd, s, m_sRightFile.c_str()))
		SetDlgItemText(IDC_PROJ_RFILE_EDIT, s.c_str());
}

/** 
 * @brief Called when Selec-button for filter is selected.
 */
void ProjectFilePathsDlg::OnBnClickedProjFilterSelect()
{
	GetMainFrame()->SelectFilter();
	String filterNameOrMask = globalFileFilter.GetFilterNameOrMask();
	SetDlgItemText(IDC_PROJ_FILTER_EDIT, filterNameOrMask.c_str());
}

/** 
 * @brief Callled when Open-button for project file is selected.
 */
void ProjectFilePathsDlg::OnBnClickedProjOpen()
{
	String fileName = AskProjectFileName(TRUE);
	if (fileName.empty())
		return;

	ProjectFile project;

	String sErr;
	if (!project.Read(fileName.c_str(), sErr))
	{
		if (sErr.empty())
			sErr = LanguageSelect.LoadString(IDS_UNK_ERROR_SAVING_PROJECT);
		LanguageSelect.FormatMessage(IDS_ERROR_FILEOPEN,
			fileName.c_str(), sErr.c_str()).MsgBox(MB_ICONSTOP);
	}
	else
	{
		m_sLeftFile = project.GetLeft();
		m_bLeftPathReadOnly = project.GetLeftReadOnly();
		m_sRightFile = project.GetRight();
		m_bRightPathReadOnly = project.GetRightReadOnly();
		m_sFilter = project.GetFilter();
		m_bIncludeSubfolders = project.GetSubfolders();
		UpdateData<Set>();
		LanguageSelect.MsgBox(IDS_PROJFILE_LOAD_SUCCESS, MB_ICONINFORMATION);
	}
}

/** 
 * @brief Called when Save-button for project file is selected.
 */
void ProjectFilePathsDlg::OnBnClickedProjSave()
{
	UpdateData<Get>();

	m_sLeftFile = string_trim_ws(m_sLeftFile);
	m_sRightFile = string_trim_ws(m_sRightFile);
	m_sFilter = string_trim_ws(m_sFilter);

	String fileName = AskProjectFileName(FALSE);
	if (fileName.empty())
		return;

	ProjectFile project;

	if (!m_sLeftFile.empty())
		project.SetLeft(m_sLeftFile, m_bLeftPathReadOnly != BST_UNCHECKED);
	if (!m_sRightFile.empty())
		project.SetRight(m_sRightFile, m_bRightPathReadOnly != BST_UNCHECKED);
	if (!m_sFilter.empty())
		project.SetFilter(m_sFilter);

	project.SetSubfolders(m_bIncludeSubfolders);

	String sErr;
	if (!project.Save(fileName.c_str(), sErr))
	{
		if (sErr.empty())
			sErr = LanguageSelect.LoadString(IDS_UNK_ERROR_SAVING_PROJECT);
		LanguageSelect.FormatMessage(
			IDS_ERROR_FILEOPEN, fileName.c_str(), sErr.c_str()
		).MsgBox(MB_ICONSTOP);
	}
	else
	{
		LanguageSelect.MsgBox(IDS_PROJFILE_SAVE_SUCCESS, MB_ICONINFORMATION);
	}
}

/** 
 * @brief Allow user to select a file to open/save.
 */
String ProjectFilePathsDlg::AskProjectFileName(BOOL bOpen)
{
	// get the default projects path
	String strProjectFileName;
	String strProjectPath = COptionsMgr::Get(OPT_PROJECTS_PATH);
	if (SelectFile(m_hWnd, strProjectFileName, strProjectPath.c_str(), NULL, IDS_PROJECTFILES, bOpen))
	{
		// Add projectfile extension if it is missing
		// So we allow 'filename.otherext' but add extension for 'filename'
		if (*PathFindExtension(strProjectFileName.c_str()) == _T('\0'))
			strProjectFileName += _T(".WinMerge");
		// get the path part from the filename
		strProjectPath = paths_GetParentPath(strProjectFileName.c_str());
		// store this as the new project path
		COptionsMgr::SaveOption(OPT_PROJECTS_PATH, strProjectPath);
	}
	return strProjectFileName;
}
