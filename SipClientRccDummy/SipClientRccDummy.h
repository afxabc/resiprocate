
// SipClientRccDummy.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSipClientRccDummyApp: 
// �йش����ʵ�֣������ SipClientRccDummy.cpp
//

class CSipClientRccDummyApp : public CWinApp
{
public:
	CSipClientRccDummyApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSipClientRccDummyApp theApp;