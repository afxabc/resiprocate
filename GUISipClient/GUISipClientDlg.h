
// GUISipClientDlg.h : 头文件
//

#pragma once

#include "basicClientUserAgent.hxx"

// CGUISipClientDlg 对话框
class CGUISipClientDlg : public CDialogEx
{
// 构造
public:
	CGUISipClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GUISIPCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	resip::BasicClientUserAgent agent_;
	RccUserAgent rccAgent_;

public:
	CString mSipHost;
	CString mPasswd;
	afx_msg void OnBnClickedRegister();
	afx_msg void OnBnClickedCall();
	CString mTargetUri;
	afx_msg void OnBnClickedHangup();
	afx_msg void OnBnClickedAccept();
	afx_msg void OnBnClickedAgentStart();
	afx_msg void OnBnClickedAgentStop();
	CString mLocalUri;
};
