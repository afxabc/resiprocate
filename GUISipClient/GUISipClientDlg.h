
// GUISipClientDlg.h : ͷ�ļ�
//

#pragma once

#include "basicClientUserAgent.hxx"

// CGUISipClientDlg �Ի���
class CGUISipClientDlg : public CDialogEx
{
// ����
public:
	CGUISipClientDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GUISIPCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
