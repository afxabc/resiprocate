
// SipClientRccDummyDlg.h : 头文件
//

#pragma once

#include "queue.h"
#include "AudioWrite.h"

#include "jrtplib\rtpsession.h"
#include "jrtplib\rtppacket.h"
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

	virtual BOOL OnInitDialog();

// 实现
protected:

	// 生成的消息映射函数
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRegister();
	afx_msg void OnBnClickedInvite();
	afx_msg void OnBnClickedAccept();
	afx_msg void OnBnClickedClosecall();

	// 通过 ThreadIf 继承
	virtual void thread() override;
	virtual void OnOK();

	bool checkForRcc();

protected:
	HICON m_hIcon;
	CString localNum_;
	CString remoteNum_;
	CListBox msgList_;

	RccUserAgent rccAgent_;
	typedef Queue<resip::Data> QUEUE;
	QUEUE queue_;

	RTPSession rtpSession_;

	const char* rtpIP_;
	unsigned short rtpPort_;
	unsigned char rtpPayload_;
	unsigned int rtpRate_;

	UInt32 remoteRtpIP_;
	unsigned short remoteRtpPort_;
	unsigned char remoteRtpPayload_;
	unsigned int remoteRtpRate_;

	AudioWrite audioWrite_;
};
