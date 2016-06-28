
// SipClientRccDummyDlg.h : ͷ�ļ�
//

#pragma once

#include "queue.h"
#include "AudioWrite.h"

#include "jrtplib\rtpsession.h"
#include "jrtplib\rtppacket.h"
#include "SipClientUdp\RccUserAgent.h"
#include "rutil\ThreadIf.hxx"

#include "afxwin.h"

// CSipClientRccDummyDlg �Ի���
class CSipClientRccDummyDlg : public CDialogEx, resip::ThreadIf
{
// ����
public:
	CSipClientRccDummyDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIPCLIENTRCCDUMMY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	virtual BOOL OnInitDialog();

// ʵ��
protected:

	// ���ɵ���Ϣӳ�亯��
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRegister();
	afx_msg void OnBnClickedInvite();
	afx_msg void OnBnClickedAccept();
	afx_msg void OnBnClickedClosecall();

	// ͨ�� ThreadIf �̳�
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
