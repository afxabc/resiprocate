
// GUISipClient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CGUISipClientApp: 
// �йش����ʵ�֣������ GUISipClient.cpp
//

class CGUISipClientApp : public CWinApp
{
public:
	CGUISipClientApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CGUISipClientApp theApp;