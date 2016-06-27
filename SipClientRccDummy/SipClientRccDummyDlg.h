
// SipClientRccDummyDlg.h : 头文件
//

#pragma once

#include "queue.h"
#include "SipClientUdp\RccUserAgent.h"
#include "rutil\ThreadIf.hxx"
#include "afxwin.h"

// CSipClientRccDummyDlg 对话框
class CSipClientRccDummyDlg : public CDialogEx, resip::ThreadIf
{
// 构造
public:
	CSipClientRccDummyDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIPCLIENTRCCDUMMY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	RccUserAgent rccAgent_;
	typedef Queue<resip::Data> QUEUE;
	QUEUE queue_;
	const char* rtpIP_;
	unsigned short rtpPort_;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();

	// 通过 ThreadIf 继承
	virtual void thread() override;
	virtual void OnOK();
	afx_msg void OnBnClickedRegister();
	afx_msg void OnBnClickedInvite();
	afx_msg void OnBnClickedAccept();
	afx_msg void OnBnClickedClosecall();
	CString localNum_;
	CString remoteNum_;
	CListBox msgList_;
};
