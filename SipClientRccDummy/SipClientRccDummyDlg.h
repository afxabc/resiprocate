
// SipClientRccDummyDlg.h : ͷ�ļ�
//

#pragma once

#include "queue.h"
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


// ʵ��
protected:
	HICON m_hIcon;
	RccUserAgent rccAgent_;
	typedef Queue<resip::Data> QUEUE;
	QUEUE queue_;
	const char* rtpIP_;
	unsigned short rtpPort_;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();

	// ͨ�� ThreadIf �̳�
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
