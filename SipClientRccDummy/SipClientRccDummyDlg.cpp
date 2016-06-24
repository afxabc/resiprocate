
// SipClientRccDummyDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SipClientRccDummy.h"
#include "SipClientRccDummyDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSipClientRccDummyDlg �Ի���



CSipClientRccDummyDlg::CSipClientRccDummyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIPCLIENTRCCDUMMY_DIALOG, pParent)
	, localNum_(_T("1001"))
	, remoteNum_(_T("1000"))
	, remoteMsg_(_T(""))
	, rtpIP_("10.10.3.100")
	, rtpPort_(12345)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSipClientRccDummyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOCAL_NUM, localNum_);
	DDX_Text(pDX, IDC_REMOTE_NUM, remoteNum_);
	DDX_Text(pDX, IDC_REMOTE_MSG, remoteMsg_);
}

BEGIN_MESSAGE_MAP(CSipClientRccDummyDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REGISTER, &CSipClientRccDummyDlg::OnBnClickedRegister)
	ON_BN_CLICKED(IDC_INVITE, &CSipClientRccDummyDlg::OnBnClickedInvite)
	ON_BN_CLICKED(IDC_ACCEPT, &CSipClientRccDummyDlg::OnBnClickedAccept)
	ON_BN_CLICKED(IDC_CLOSECALL, &CSipClientRccDummyDlg::OnBnClickedClosecall)
END_MESSAGE_MAP()


// CSipClientRccDummyDlg ��Ϣ�������

BOOL CSipClientRccDummyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	rccAgent_.startAgent(21358, NULL, DUMMY_RCC_PORT, "10.10.3.100");
	this->run();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSipClientRccDummyDlg::OnPaint()
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
HCURSOR CSipClientRccDummyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSipClientRccDummyDlg::OnDestroy()
{
	OnBnClickedClosecall();
	rccAgent_.stopAgent();
	this->shutdown();
	this->join();
	CDialogEx::OnDestroy();
}

void CSipClientRccDummyDlg::thread()
{
	static char buf[1024];

	while (rccAgent_.isValid())
	{
		int sz = rccAgent_.getMessage((RccMessage*)buf, 1024);
		if (sz <= 0)
		{
			Sleep(100);
			continue;
		}

		queue_.putBack(resip::Data(buf, sz));
		this->PostMessage(WM_COMMAND, IDOK);
	}
}

void CSipClientRccDummyDlg::OnOK()
{
	resip::Data data;
	while (queue_.getFront(data))
	{
		RccMessage* msg = (RccMessage*)data.begin();
		switch (msg->mType)
		{
		case RccMessage::CALL_ANSWER:
		{
			struct in_addr addr;
			memcpy(&addr, &msg->mRtpIP, 4);
			CStringA str;
			str.Format("Ӧ��%s RTP=%s:%d(%d)", msg->mCallNum, inet_ntoa(addr), msg->mRtpPort, msg->mRtpPayload);

			USES_CONVERSION;
			remoteMsg_ = A2W(str);

			UpdateData(FALSE);

			break;
		}
		case RccMessage::CALL_RING:
		{
			struct in_addr addr;
			memcpy(&addr, &msg->mRtpIP, 4);
			CStringA str;
			str.Format("���壺%s RTP=%s:%d(%d)", msg->mCallNum, inet_ntoa(addr), msg->mRtpPort, msg->mRtpPayload);

			USES_CONVERSION;
			remoteMsg_ = A2W(str);

			UpdateData(FALSE);

			break;
		}

		}
	}
}

void CSipClientRccDummyDlg::OnBnClickedRegister()
{
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessage(RccMessage::CALL_REGISTER, W2A(localNum_));
}

void CSipClientRccDummyDlg::OnBnClickedInvite()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessage(RccMessage::CALL_INVITE, W2A(remoteNum_), rtpIP_, rtpPort_, 0);
}

void CSipClientRccDummyDlg::OnBnClickedAccept()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessage(RccMessage::CALL_ACCEPT, W2A(localNum_), rtpIP_, rtpPort_, 0);
}

void CSipClientRccDummyDlg::OnBnClickedClosecall()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	rccAgent_.sendMessage(RccMessage::CALL_CLOSE);
}
