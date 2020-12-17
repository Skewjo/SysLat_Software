// PreferencesDlg.cpp : implementation file
//
#pragma once

#include "resource.h"
#include "StdAfx.h"
#include "SysLat_Software.h"
#include "PreferencesDlg.h"
#include "afxdialogex.h"


// PreferencesDlg dialog

IMPLEMENT_DYNAMIC(PreferencesDlg, CDialogEx)

PreferencesDlg::PreferencesDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PREFERENCES_PROPPAGE, pParent)
{

}

PreferencesDlg::~PreferencesDlg()
{
}

void PreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(PreferencesDlg, CDialogEx)
END_MESSAGE_MAP()


// PreferencesDlg message handlers
