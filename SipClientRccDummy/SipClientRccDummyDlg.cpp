
// SipClientRccDummyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SipClientRccDummy.h"
#include "SipClientRccDummyDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//////////////////////////////////////

int WINAPI val_seg(int val)
{
	int r = 0;
	val >>= 7;
	if (val & 0xf0) {
		val >>= 4;
		r += 4;
	}
	if (val & 0x0c) {
		val >>= 2;
		r += 2;
	}
	if (val & 0x02)
		r += 1;
	return r;
}

unsigned char WINAPI s16_to_alaw(int pcm_val)
{
	int		mask;
	int		seg;
	unsigned char	aval;

	if (pcm_val >= 0) {
		mask = 0xD5;
	}
	else {
		mask = 0x55;
		pcm_val = -pcm_val;
		if (pcm_val > 0x7fff)
			pcm_val = 0x7fff;
	}

	if (pcm_val < 256)
		aval = pcm_val >> 4;
	else {
		/* Convert the scaled magnitude to segment number. */
		seg = val_seg(pcm_val);
		aval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0x0f);
	}
	return aval ^ mask;
}

/*
* alaw_to_s16() - Convert an A-law value to 16-bit linear PCM
*
*/
int WINAPI alaw_to_s16(unsigned char a_val)
{
	int		t;
	int		seg;

	a_val ^= 0x55;
	t = a_val & 0x7f;
	if (t < 16)
		t = (t << 4) + 8;
	else {
		seg = (t >> 4) & 0x07;
		t = ((t & 0x0f) << 4) + 0x108;
		t <<= seg - 1;
	}
	return ((a_val & 0x80) ? t : -t);
}

/////////////////////////////////////
// CSipClientRccDummyDlg 对话框

/*
const Codec Codec::ULaw_8000("PCMU", 0, 8000);
const Codec Codec::GSM_8000("GSM", 3, 8000);
const Codec Codec::G723_8000("G723", 4, 8000);
const Codec Codec::ALaw_8000("PCMA", 8, 8000);
const Codec Codec::G722_8000("G722", 9, 8000);
*/

static const int PTIME = 20;
static const unsigned char PAYLOAD_AUDIO = 8;
static const unsigned int RATE_AUDIO = 8000;
static const unsigned int RATE_VIDEO = 90000;
static const unsigned int VIDEO_WIDTH = 480;
static const unsigned int VIDEO_HEIGHT = 360;
static const unsigned int VIDEO_FPS = 10;

CSipClientRccDummyDlg::CSipClientRccDummyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIPCLIENTRCCDUMMY_DIALOG, pParent)
	, localNum_(_T("1000"))
//	, remoteNum_(_T("5000"))
//	, remoteNum_(_T("9664"))		//音乐
	, remoteNum_(_T("9196"))		//echo
	, rccIP_("127.0.0.1")
	, rccPort_(RCC_SERVER_PORT_BASE)
	, rtpAudio_(this, "127.0.0.1", RCC_RTP_PORT_BASE, PAYLOAD_AUDIO, RATE_AUDIO)
	, rtpVideo_(this, "127.0.0.1", RCC_RTP_PORT_BASE+2, RTP_PAYLOAD_H264, RATE_VIDEO)
	, videoCap_(this)
	, audioRead_(this)
	, audioFile_(this)
	, mediaTest_(FALSE)
	, mediaCall_(FALSE)
	, audioSrc_(0)
	, numIcome_(_T(""))
	, videoEnable_(FALSE)
	, sendAudio_(0)
	, sendVideo_(0)
	, msgTxt_(_T("可可、向日葵、菠萝、马铃薯、木薯、巴西橡胶树、烟草、金鸡纳树、玉米、番茄、巴拉圭茶、辣椒"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSipClientRccDummyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOCAL_NUM, localNum_);
	DDX_Text(pDX, IDC_REMOTE_NUM, remoteNum_);
	DDX_Control(pDX, IDC_MSG_LIST, msgList_);
	DDX_Check(pDX, IDC_AUDIO_TEST, mediaTest_);
	DDX_CBIndex(pDX, IDC_AUDIO_SRC, audioSrc_);
	DDX_Text(pDX, IDC_INCOME_NUM, numIcome_);
	DDX_Check(pDX, IDC_VIDEO_ENABLE, videoEnable_);
	DDX_Control(pDX, IDC_DRAW_BK, drawWnd_);
	DDX_Text(pDX, IDC_MESSAGE_TXT, msgTxt_);
}

BEGIN_MESSAGE_MAP(CSipClientRccDummyDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REGISTER, &CSipClientRccDummyDlg::OnBnClickedRegister)
	ON_BN_CLICKED(IDC_INVITE, &CSipClientRccDummyDlg::OnBnClickedInvite)
	ON_BN_CLICKED(IDC_ACCEPT, &CSipClientRccDummyDlg::OnBnClickedAccept)
	ON_BN_CLICKED(IDC_CLOSECALL, &CSipClientRccDummyDlg::OnBnClickedClosecall)
	ON_BN_CLICKED(IDC_AUDIO_TEST, &CSipClientRccDummyDlg::OnBnClickedMediaTest)
	ON_CBN_SELCHANGE(IDC_AUDIO_SRC, &CSipClientRccDummyDlg::OnCbnSelchangeAudioSrc)
	ON_COMMAND_RANGE(IDC_DTMF_0, IDC_DTMF_SHARP, &CSipClientRccDummyDlg::OnDtmfKey)
	ON_BN_CLICKED(IDC_VIDEO_ENABLE, &CSipClientRccDummyDlg::OnBnClickedVideoEnable)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_MESSAGE_SEND, &CSipClientRccDummyDlg::OnBnClickedMessageSend)
END_MESSAGE_MAP()


// CSipClientRccDummyDlg 消息处理程序
extern int LoadParamString(const char* profile, char* AppName, char* KeyName, char* KeyVal);
BOOL CSipClientRccDummyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	GetWindowText(strTitle_);

	// TODO: 在此添加额外的初始化代码
	static const char* finame = "./config.txt";
	char tmpBuf[1024];
	if (LoadParamString(finame, "RTP", "ADDR", tmpBuf) >= 2)
	{
		rtpAudio_.ip() = tmpBuf;
		rtpVideo_.ip() = tmpBuf;
	}

	if (LoadParamString(finame, "RTP", "PORT", tmpBuf) >= 2)
	{
		unsigned short rtpPortBase = atoi(tmpBuf);
		rtpAudio_.port() = rtpPortBase;
		rtpVideo_.port() = rtpPortBase+2;
	}

	if (LoadParamString(finame, "RCC", "ADDR", tmpBuf) >= 2)
		rccIP_ = tmpBuf;
	if (LoadParamString(finame, "RCC", "PORT", tmpBuf) >= 2)
		rccPort_ = atoi(tmpBuf);

	rccAgent_.startAgent(RCC_CLIENT_PORT_BASE, NULL, rccPort_, rccIP_.c_str());
	this->run();

	RECT r;
	drawWnd_.GetWindowRect(&r);
	RECT R;
	this->GetWindowRect(&R);
	winMin_ = r.left - R.left;
	winMax_ = R.right - R.left;

	OnBnClickedVideoEnable();

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
	CDialogEx::OnDestroy();
}

bool CSipClientRccDummyDlg::checkForRcc()
{
	if (!rccAgent_.isValid())
		return false;

	static char buf[2048];
	int sz = rccAgent_.getMessage(buf, 2048);
	if (sz > 0)
	{
		queue_.putBack(resip::Data(buf, sz));
		this->PostMessage(WM_COMMAND, IDOK);
	}

	return (sz > 0);
}

void CSipClientRccDummyDlg::thread()
{
	mShutdown = false;
	while (!isShutdown())
	{
		if (!checkForRcc() && !audioWrite_.isStart())
			Sleep(20);

	}
}

void CSipClientRccDummyDlg::showString(LPCSTR pszFormat, ...)
{
	CStringA str;
	va_list argList;
	va_start(argList, pszFormat);
	str.FormatV(pszFormat, argList);
	va_end(argList);

	USES_CONVERSION;
	msgList_.AddString(A2W(str));
	msgList_.SetCurSel(msgList_.GetCount() - 1);
}

void CSipClientRccDummyDlg::OnOK()
{
	resip::Data data;
	while (queue_.getFront(data))
	{
		rccAgent_.dispatchMessage(data.begin(), this);
	}

}

////////////////////////// 通过 IRccMessageCallback 继承

void CSipClientRccDummyDlg::onMessage(RccMessage::MessageType type)
{
	if (type == RccMessage::RCC_CONN)
	{
		onMessageConn();
	}
	else TRACE("onMessage : [%d]\n", (int)type);
}

void CSipClientRccDummyDlg::onMessageAcm(RccMessage::MessageType which, unsigned char result)
{
	if (which >= RccMessage::RCC_END)
		return;

	static const char* msgTag[RccMessage::RCC_END] = { "??", "注册", "注销", "发起呼叫",
		"消息响应", "应答", "释放", "连接", "扩展", "发送消息" };

	if (result == 0)
		showString("%s成功。", msgTag[which]);
	else showString("%s失败（%d）！！", msgTag[which], result);
}

void CSipClientRccDummyDlg::onMessageRgst(const char * callNumber)
{
}

void CSipClientRccDummyDlg::onMessageUrgst(const char * callNumber)
{
}

void CSipClientRccDummyDlg::onMessageRel(unsigned char reason)
{
	numIcome_ = _T("");
	UpdateData(FALSE);
	this->GetDlgItem(IDC_ACCEPT)->EnableWindow(FALSE);

	mediaStop();

	showString("结束通话。 (%d)", (int)reason);
	mediaCall_ = FALSE;
}

void CSipClientRccDummyDlg::onMessageIam(const char * callNumber, RccRtpDataList & rtpDataList)
{
	USES_CONVERSION;
	numIcome_.Format(_T(" 来电号码：%s"), A2W(callNumber));
	UpdateData(FALSE);
	this->GetDlgItem(IDC_ACCEPT)->EnableWindow(TRUE);

	showString("来电号码：%s", callNumber);
	for (unsigned int i = 0; i < rtpDataList.size(); ++i)
	{
		if (rtpAudio_.payload() == rtpDataList[i].payload)
		{
			rtpAudio_.setRemote(rtpDataList[i].ip, rtpDataList[i].port, rtpDataList[i].payload, rtpDataList[i].rate);

			showString("  [%d] RTP(%s:%d)", i, rtpDataList[i].ip.c_str(), rtpDataList[i].port);
			showString("  [%d] codec(%d, %d)", i, rtpDataList[i].payload, rtpDataList[i].rate);
		}
		else if (rtpVideo_.payload() == rtpDataList[i].payload)
		{
			rtpVideo_.setRemote(rtpDataList[i].ip, rtpDataList[i].port, rtpDataList[i].payload, rtpDataList[i].rate);

			showString("  [%d] RTP(%s:%d)", i, rtpDataList[i].ip.c_str(), rtpDataList[i].port);
			showString("  [%d] codec(%d, %d)", i, rtpDataList[i].payload, rtpDataList[i].rate);
		}
	}
}

void CSipClientRccDummyDlg::onMessageAnm(RccRtpDataList & rtpDataList)
{
	showString("收到应答：");
	for (unsigned int i = 0; i < rtpDataList.size(); ++i)
	{
		if (rtpAudio_.payload() == rtpDataList[i].payload)
		{
			rtpAudio_.setRemote(rtpDataList[i].ip, rtpDataList[i].port, rtpDataList[i].payload, rtpDataList[i].rate);

			showString("  [%d] RTP(%s:%d)", i, rtpDataList[i].ip.c_str(), rtpDataList[i].port);
			showString("  [%d] codec(%d, %d)", i, rtpDataList[i].payload, rtpDataList[i].rate);
		}
		else if (rtpVideo_.payload() == rtpDataList[i].payload)
		{
			rtpVideo_.setRemote(rtpDataList[i].ip, rtpDataList[i].port, rtpDataList[i].payload, rtpDataList[i].rate);
			showString("  [%d] RTP(%s:%d)", i, rtpDataList[i].ip.c_str(), rtpDataList[i].port);
			showString("  [%d] codec(%d, %d)", i, rtpDataList[i].payload, rtpDataList[i].rate);
		}
	}
}

void CSipClientRccDummyDlg::onMessageTxt(const char * callNumber, const char * txt, unsigned short len)
{
	showString("来自%s的消息[%d]：", callNumber, len);
	showString("  %s", txt);
}

void CSipClientRccDummyDlg::onInvalidMessage(RccMessage * msg)
{
}

void CSipClientRccDummyDlg::onMessageConn()
{
	this->GetDlgItem(IDC_ACCEPT)->EnableWindow(FALSE);
	mediaCall_ = TRUE;

	mediaStart();

	showString("通话中......");
}

/////////////////////////////////////////////////////////////

void CSipClientRccDummyDlg::mediaStart()
{
	sendAudio_ = 0;
	rtpAudio_.start(PTIME);
	audioWrite_.start(rtpAudio_.rate(), PTIME);
	if (audioSrc_ == 0)
		audioRead_.start(rtpAudio_.rate(), PTIME);
	else if (audioSrc_ == 1)
		audioFile_.start(rtpAudio_.rate(), PTIME);

	sendVideo_ = 0;
	if (videoEnable_)
	{
		rtpVideo_.start(0, VIDEO_FPS);
		videoCap_.start(VIDEO_WIDTH, VIDEO_HEIGHT, rtpVideo_.rate(), VIDEO_FPS);
		videoDraw_.start(&drawWnd_, rtpVideo_.r_rate());
	}
	
	GetDlgItem(IDC_VIDEO_ENABLE)->EnableWindow(FALSE);

	SetTimer(1, TIMER_SPAN*1000, NULL);
}

void CSipClientRccDummyDlg::mediaStop()
{
	KillTimer(1);

	rtpAudio_.stop();
	audioWrite_.stop();
	audioRead_.stop();
	audioFile_.stop();

	rtpVideo_.stop();
	videoCap_.stop();
	videoDraw_.stop();
	drawWnd_.Invalidate();

	GetDlgItem(IDC_VIDEO_ENABLE)->EnableWindow(TRUE);

	SetWindowText(strTitle_);
}

void CSipClientRccDummyDlg::OnBnClickedRegister()
{
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageRgst(W2A(localNum_));
}

void CSipClientRccDummyDlg::OnBnClickedInvite()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	RccRtpDataList rccRtpDataList;
	rccRtpDataList.push_back(RccRtpData(rtpAudio_.ip().c_str(), rtpAudio_.port(), rtpAudio_.payload(), rtpAudio_.rate()));
	if (videoEnable_)
		rccRtpDataList.push_back(RccRtpData(rtpVideo_.ip().c_str(), rtpVideo_.port(), rtpVideo_.payload(), rtpVideo_.rate()));
	
	USES_CONVERSION;
	rccAgent_.sendMessageIam(W2A(remoteNum_), rccRtpDataList);
}

void CSipClientRccDummyDlg::OnBnClickedAccept()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	RccRtpDataList rccRtpDataList;
	rccRtpDataList.push_back(RccRtpData(rtpAudio_.ip().c_str(), rtpAudio_.port(), rtpAudio_.payload(), rtpAudio_.rate()));
	if (videoEnable_ && rtpVideo_.r_port() != 0)
		rccRtpDataList.push_back(RccRtpData(rtpVideo_.ip().c_str(), rtpVideo_.port(), rtpVideo_.payload(), rtpVideo_.rate()));
	
	USES_CONVERSION;
	rccAgent_.sendMessageAnm(rccRtpDataList);
}

void CSipClientRccDummyDlg::OnBnClickedMessageSend()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	USES_CONVERSION;
	rccAgent_.sendMessageTxt(W2A(remoteNum_), W2A(msgTxt_), strlen(W2A(msgTxt_)));
}

void CSipClientRccDummyDlg::OnBnClickedClosecall()
{
	// TODO: 在此添加控件通知处理程序代码
	rccAgent_.sendMessageRel();
	mediaStop();
}

void CSipClientRccDummyDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	
	OnBnClickedClosecall();
	USES_CONVERSION;
	rccAgent_.sendMessageUrgst(W2A(localNum_));

	Sleep(500);
	rccAgent_.stopAgent();
	this->shutdown();
	this->join();
	__super::OnCancel();
}

void CSipClientRccDummyDlg::outputPcm(char * data, int len)
{
	if (!mediaCall_ && !mediaTest_)
		return;

	static Buffer tmpBuf;
	
	tmpBuf.erase();
	tmpBuf.pushBack(len/2, true);
	unsigned char* pch = (unsigned char*)tmpBuf.beginRead();
	short* s16 = (short*)data;
	for (int i = 0; i < len / 2; ++i)
		pch[i] = s16_to_alaw(s16[i]);
	
	sendAudio_ += len / 2;
	rtpAudio_.sendData((char*)pch, len/2);
}

void CSipClientRccDummyDlg::OnBnClickedMediaTest()
{
	if (mediaCall_)
		return;

	UpdateData();
	if (mediaTest_)
	{
		rtpAudio_.setRemoteSelf();
		rtpVideo_.setRemoteSelf();
		mediaStart();
	}
	else mediaStop();
}

void CSipClientRccDummyDlg::OnCbnSelchangeAudioSrc()
{
	int src = audioSrc_;
	UpdateData();
	if (src == audioSrc_)
		return;

	audioRead_.stop();
	audioFile_.stop();
	if (audioSrc_ == 0)
		audioRead_.start(rtpAudio_.r_rate(), PTIME);
	else if (audioSrc_ == 1)
		audioFile_.start(rtpAudio_.r_rate(), PTIME);
}

void CSipClientRccDummyDlg::OnDtmfKey(UINT key_id)
{
	if (!mediaCall_)
		return;

	int evt = key_id - IDC_DTMF_0;
	int vol = 10;
	int duration = 480 / 3;

	rtpAudio_.sendDtmfKey(evt, vol, duration);
}

void CSipClientRccDummyDlg::onMediaData(char * data, int len, unsigned char payload, unsigned short seq)
{
	if (rtpAudio_.r_payload() == payload)
	{
		static const int PCM_BUF_SIZE = 2048;
		short pcmBuf[PCM_BUF_SIZE];

		while (len > 0)
		{
			int sz = (len > PCM_BUF_SIZE) ? PCM_BUF_SIZE : len;
			for (int j = 0; j < sz; j++)
				pcmBuf[j] = alaw_to_s16(data[j]);

			audioWrite_.inputPcm((char*)pcmBuf, sz * 2);
			len -= sz;
			data += sz;
		}
	}
	else if (rtpVideo_.r_payload() == payload)
	{
		static unsigned short seqBak = 0;
		if (seq - seqBak > 1)
		{
			TRACE("======== 包号：%d  %d\r\n", seq, (seq - seqBak));
		}
		seqBak = seq;

		videoDraw_.decodeAndDraw(data, len);
	}
	else
	{
		TRACE("????????????????????? %d\n", payload);
	}
}

void CSipClientRccDummyDlg::onMediaError(int status)
{
	TRACE("!!!!!!!!!!!!!!!!! MediaError %d\n", status);
}

void CSipClientRccDummyDlg::onVideoEncodeFin(char * data, int len, unsigned char pt, bool mark, unsigned long tm)
{
	if (!mediaCall_ && !mediaTest_)
		return;

	sendVideo_ += len;
//	if (mediaCall_)
		rtpVideo_.sendData(data, len, pt, true, tm);
//	else onMediaData(data, len, pt, 0);
}

void CSipClientRccDummyDlg::OnBnClickedVideoEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	RECT r;
	this->GetWindowRect(&r);
	if (videoEnable_)
		r.right = r.left + winMax_;
	else r.right = r.left + winMin_;
	this->MoveWindow(&r);
}


void CSipClientRccDummyDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	__super::OnTimer(nIDEvent);

	float av = (float)sendAudio_ / TIMER_SPAN/1000;
	sendAudio_ = 0;
	float vv = (float)sendVideo_ / TIMER_SPAN/1000;
	sendVideo_ = 0;

	CString str;
	str.Format(_T("%s A=%.2fK V=%.2fK"), strTitle_, av, vv);
	SetWindowText(str);
}

