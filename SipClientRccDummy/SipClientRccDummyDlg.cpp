
// SipClientRccDummyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SipClientRccDummy.h"
#include "SipClientRccDummyDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSipClientRccDummyDlg 对话框



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


// CSipClientRccDummyDlg 消息处理程序

BOOL CSipClientRccDummyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	rccAgent_.startAgent(21358, NULL, DUMMY_RCC_PORT, "10.10.3.100");
	this->run();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSipClientRccDummyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
			str.Format("应答：%s RTP=%s:%d(%d)", msg->mCallNum, inet_ntoa(addr), msg->mRtpPort, msg->mRtpPayload);

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
			str.Format("振铃：%s RTP=%s:%d(%d)", msg->mCallNum, inet_ntoa(addr), msg->mRtpPort, msg->mRtpPayload);

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
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessage(RccMessage::CALL_INVITE, W2A(remoteNum_), rtpIP_, rtpPort_, 0);
}

void CSipClientRccDummyDlg::OnBnClickedAccept()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessage(RccMessage::CALL_ACCEPT, W2A(localNum_), rtpIP_, rtpPort_, 0);
}

void CSipClientRccDummyDlg::OnBnClickedClosecall()
{
	// TODO: 在此添加控件通知处理程序代码
	rccAgent_.sendMessage(RccMessage::CALL_CLOSE);
}
