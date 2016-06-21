
// GUISipClientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GUISipClient.h"
#include "GUISipClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#include <rutil/Log.hxx>
// CGUISipClientDlg �Ի���



CGUISipClientDlg::CGUISipClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GUISIPCLIENT_DIALOG, pParent)
	, mLocalUri(_T("sip:1001@10.10.3.202"))
	, mPasswd(_T("1234"))
	, mTargetUri(_T("sip:9664@10.10.3.202"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGUISipClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOCAL_URI, mLocalUri);
	DDX_Text(pDX, IDC_PASSWD, mPasswd);
	DDX_Text(pDX, IDC_TARGET_URI, mTargetUri);
}

BEGIN_MESSAGE_MAP(CGUISipClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REGISTER, &CGUISipClientDlg::OnBnClickedRegister)
	ON_BN_CLICKED(IDC_CALL, &CGUISipClientDlg::OnBnClickedCall)
	ON_BN_CLICKED(IDC_HANGUP, &CGUISipClientDlg::OnBnClickedHangup)
	ON_BN_CLICKED(IDC_ACCEPT, &CGUISipClientDlg::OnBnClickedAccept)
END_MESSAGE_MAP()


// CGUISipClientDlg ��Ϣ�������

BOOL CGUISipClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	resip::Log::initialize("cout", "INFO", "GUISipClient");

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CGUISipClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CGUISipClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGUISipClientDlg::OnBnClickedRegister()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	USES_CONVERSION;
	agent_.start(W2A(mLocalUri), W2A(mPasswd), 7788);
}


void CGUISipClientDlg::OnBnClickedCall()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	USES_CONVERSION;
	agent_.openSession(W2A(mTargetUri));
}


void CGUISipClientDlg::OnBnClickedHangup()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	agent_.closeSession();
}


void CGUISipClientDlg::OnBnClickedAccept()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	agent_.acceptSession();
}
