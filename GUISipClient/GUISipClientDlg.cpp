
// GUISipClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GUISipClient.h"
#include "GUISipClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#include <rutil/Log.hxx>
// CGUISipClientDlg 对话框



CGUISipClientDlg::CGUISipClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GUISIPCLIENT_DIALOG, pParent)
	, mSipHost(_T("10.10.3.202"))
	, mPasswd(_T("1234"))
	, mTargetUri(_T("9664"))
	, mLocalUri(_T("1001"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGUISipClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SIP_HOST, mSipHost);
	DDX_Text(pDX, IDC_PASSWD, mPasswd);
	DDX_Text(pDX, IDC_TARGET_URI, mTargetUri);
	DDX_Text(pDX, IDC_LOCAL_URI, mLocalUri);
}

BEGIN_MESSAGE_MAP(CGUISipClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REGISTER, &CGUISipClientDlg::OnBnClickedRegister)
	ON_BN_CLICKED(IDC_CALL, &CGUISipClientDlg::OnBnClickedCall)
	ON_BN_CLICKED(IDC_HANGUP, &CGUISipClientDlg::OnBnClickedHangup)
	ON_BN_CLICKED(IDC_ACCEPT, &CGUISipClientDlg::OnBnClickedAccept)
	ON_BN_CLICKED(IDC_AGENT_START, &CGUISipClientDlg::OnBnClickedAgentStart)
	ON_BN_CLICKED(IDC_AGENT_STOP, &CGUISipClientDlg::OnBnClickedAgentStop)
END_MESSAGE_MAP()


// CGUISipClientDlg 消息处理程序

BOOL CGUISipClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	resip::Log::initialize("cout", "INFO", "GUISipClient");
	resip::initNetwork();
	OnBnClickedAgentStart();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGUISipClientDlg::OnPaint()
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
HCURSOR CGUISipClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGUISipClientDlg::OnBnClickedAgentStart()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	USES_CONVERSION;
	agent_.start(W2A(mSipHost), W2A(mPasswd), 7788);
	rccAgent_.startAgent(7799, NULL, 7788, "10.10.3.100");
}


void CGUISipClientDlg::OnBnClickedAgentStop()
{
	// TODO: 在此添加控件通知处理程序代码
	agent_.stop();
	rccAgent_.stopAgent();
}

void CGUISipClientDlg::OnBnClickedRegister()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessage(RccMessage::CALL_REGISTER, W2A(mLocalUri));
}

void CGUISipClientDlg::OnBnClickedCall()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessage(RccMessage::CALL_INVITE, W2A(mTargetUri), "10.10.8.100", 12345, 0);
}

void CGUISipClientDlg::OnBnClickedHangup()
{
	// TODO: 在此添加控件通知处理程序代码
	rccAgent_.sendMessage(RccMessage::CALL_CLOSE);
}

void CGUISipClientDlg::OnBnClickedAccept()
{
	// TODO: 在此添加控件通知处理程序代码
	rccAgent_.sendMessage(RccMessage::CALL_ACCEPT);
}

