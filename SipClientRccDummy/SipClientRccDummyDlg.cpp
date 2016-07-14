
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
CSipClientRccDummyDlg::CSipClientRccDummyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIPCLIENTRCCDUMMY_DIALOG, pParent)
	, localNum_(_T("1000"))
	, remoteNum_(_T("5000"))
//	, remoteNum_(_T("9664"))		//音乐
//	, remoteNum_(_T("9196"))		//echo
//	, rtpIP_("10.10.3.100")
	, rccPort_(RCC_SERVER_PORT_BASE)
	, rtpPort_(RCC_RTP_PORT_BASE)
//	, rtpPayload_(0)		//"ULaw"
	, rtpPayload_(8)		//"ALaw"
	, rtpRate_(8000)
	, audioRead_(this)
	, audioFile_(this)
	, audioTest_(FALSE)
	, audioCall_(FALSE)
	, audioSrc_(0)
	, numIcome_(_T(""))
{
	memset(rtpIP_, 0, RccMessage::IP_STR_SIZE);
	memset(rccIP_, 0, RccMessage::IP_STR_SIZE);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSipClientRccDummyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOCAL_NUM, localNum_);
	DDX_Text(pDX, IDC_REMOTE_NUM, remoteNum_);
	DDX_Control(pDX, IDC_MSG_LIST, msgList_);
	DDX_Check(pDX, IDC_AUDIO_TEST, audioTest_);
	DDX_CBIndex(pDX, IDC_AUDIO_SRC, audioSrc_);
	DDX_Text(pDX, IDC_INCOME_NUM, numIcome_);
}

BEGIN_MESSAGE_MAP(CSipClientRccDummyDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_REGISTER, &CSipClientRccDummyDlg::OnBnClickedRegister)
	ON_BN_CLICKED(IDC_INVITE, &CSipClientRccDummyDlg::OnBnClickedInvite)
	ON_BN_CLICKED(IDC_ACCEPT, &CSipClientRccDummyDlg::OnBnClickedAccept)
	ON_BN_CLICKED(IDC_CLOSECALL, &CSipClientRccDummyDlg::OnBnClickedClosecall)
	ON_BN_CLICKED(IDC_AUDIO_TEST, &CSipClientRccDummyDlg::OnBnClickedAudioTest)
	ON_CBN_SELCHANGE(IDC_AUDIO_SRC, &CSipClientRccDummyDlg::OnCbnSelchangeAudioSrc)
	ON_COMMAND_RANGE(IDC_DTMF_0, IDC_DTMF_SHARP, &CSipClientRccDummyDlg::OnDtmfKey)
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

	// TODO: 在此添加额外的初始化代码
	static const char* finame = "./config.txt";
	char tmpBuf[1024];
	if (LoadParamString(finame, "RTP", "ADDR", rtpIP_) < 2)
		strcpy(rtpIP_, "10.10.3.100");
	if (LoadParamString(finame, "RTP", "PORT", tmpBuf) >= 2)
		rtpPort_ = atoi(tmpBuf);
	if (LoadParamString(finame, "RCC", "ADDR", rccIP_) < 2)
		strcpy(rccIP_, "10.10.3.100");
	if (LoadParamString(finame, "RCC", "PORT", tmpBuf) >= 2)
		rccPort_ = atoi(tmpBuf);

	int ret = 0;
	while ((ret = rtpSession_.Create(rtpPort_)) < 0)
	{
		TRACE("tpSession_.Create(%d) return %d !!!\n", rtpPort_, ret);
		rtpPort_ += 2;
	}
	rccAgent_.startAgent(RCC_CLIENT_PORT_BASE, NULL, rccPort_, rccIP_);
	this->run();

//	audioRead_.enumDevices();

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
	int sz = rccAgent_.getMessage((RccMessage*)buf, 2048);
	if (sz > 0)
	{
		queue_.putBack(resip::Data(buf, sz));
		this->PostMessage(WM_COMMAND, IDOK);
	}

	return (sz > 0);
}

void CSipClientRccDummyDlg::thread()
{
	fd_set fdset;
	struct timeval rttprev = { 0,0 }, rtt, tv;
	SOCKET sock1, sock2;
	rtpSession_.GetRTPSocket(&sock1);
	rtpSession_.GetRTCPSocket(&sock2);

	int tick;
	static const int PCM_BUF_SIZE = 2048;
	short pcmBuf[PCM_BUF_SIZE];

	while (!isShutdown())
	{
		if (!checkForRcc() && !audioWrite_.isStart())
			Sleep(20);

		if (audioWrite_.isStart())
		{
			
			tv.tv_sec = 0;
			tv.tv_usec = PTIME*1000;
			FD_ZERO(&fdset);
			FD_SET(sock1, &fdset);
			FD_SET(sock2, &fdset);
			FD_SET(0, &fdset); // check for keypress

			select(FD_SETSIZE, &fdset, NULL, NULL, &tv);
			if (!FD_ISSET(0, &fdset))
				continue;
			
			rtpSession_.PollData();
			// check incoming packets
			if (rtpSession_.GotoFirstSourceWithData())
			{
				do
				{
					RTPSourceData *srcdat;
					srcdat = rtpSession_.GetCurrentSourceInfo();
					rtt = srcdat->INF_GetRoundTripTime();
					RTPPacket *pack;
					while ((pack = rtpSession_.GetNextPacket()) != NULL)
					{
						// You can examine the data here
						char* data = (char*)pack->GetPayload();
						int len = pack->GetPayloadLength();
						/*
						static int tick0 = ::GetTickCount();
						tick = ::GetTickCount(); 
						TRACE("[%d] RTPPacket len=%d\n", tick-tick0, len);
						tick0 = tick;
						*/
						while (pack->GetPayloadType() == rtpPayload_ && len > 0)	//is alaw?
						{
							int sz = (len > PCM_BUF_SIZE) ? PCM_BUF_SIZE : len;
							for (int j = 0; j<sz; j++)
								pcmBuf[j] = alaw_to_s16(data[j]);
							audioWrite_.inputPcm((char*)pcmBuf, sz*2);
							len -= sz;
							data += sz;
						}

						// we don't longer need the packet, so
						// we'll delete it
						delete pack;
					}
				} while (rtpSession_.GotoNextSourceWithData());
			}

		}
	}
}

void CSipClientRccDummyDlg::printRccAck(bool ok, RccMessage::MessageType which, CStringA& str)
{
	switch (which)
	{
	case RccMessage::CALL_REGISTER:
		str = "注册";
		break;
	case RccMessage::CALL_UNREGISTER:
		str = "注销";
		break;
	case RccMessage::CALL_INVITE:
		str = "呼叫";
		break;
	default:
		str = "执行";
	}
	str += (ok) ? "成功" : "失败";
}

void CSipClientRccDummyDlg::OnOK()
{
	resip::Data data;
	CStringA str;
	while (queue_.getFront(data))
	{
		str = "";
		RccMessage* msg = (RccMessage*)data.begin();
		switch (msg->mType)
		{
		case RccMessage::CALL_ACCEPT:
			str.Format("应答：RTP(%s:%d)", msg->rccAccept.mRtpIP, msg->rccAccept.mRtpPort);
			{
				USES_CONVERSION;
				msgList_.AddString(A2W(str));
			}
			str.Format("应答：codec(%d, %d)", msg->rccAccept.mRtpPayload, msg->rccAccept.mRtpRate);

			remoteRtpIP_ = msg->rccAccept.mRtpIP;
			remoteRtpPort_ = msg->rccAccept.mRtpPort;
			remoteRtpPayload_ = msg->rccAccept.mRtpPayload;
			remoteRtpRate_ = msg->rccAccept.mRtpRate;
			break;
		case RccMessage::CALL_INVITE:
			{
				USES_CONVERSION;
				numIcome_.Format(_T(" 来电号码：%s"), A2W(msg->rccInvite.mCallNum));
				UpdateData(FALSE);
				this->GetDlgItem(IDC_ACCEPT)->EnableWindow(TRUE);
			}

			str.Format("来电：RTP(%s:%d)", msg->rccInvite.mRtpIP, msg->rccInvite.mRtpPort);
			{
				USES_CONVERSION;
				msgList_.AddString(A2W(str));
			}
			str.Format("来电：codec(%d, %d)", msg->rccInvite.mRtpPayload, msg->rccInvite.mRtpRate);
			
			remoteRtpIP_ = msg->rccAccept.mRtpIP;
			remoteRtpPort_ = msg->rccAccept.mRtpPort;
			remoteRtpPayload_ = msg->rccAccept.mRtpPayload;
			remoteRtpRate_ = msg->rccAccept.mRtpRate;
			break;
		case RccMessage::CALL_CONNECTED:
			this->GetDlgItem(IDC_ACCEPT)->EnableWindow(FALSE);
			str = "通话中......";
			audioCall_ = TRUE;
			rtpSession_.ClearDestinations();
			rtpSession_.AddDestination(ntohl(inet_addr(remoteRtpIP_.begin())), remoteRtpPort_);
			rtpSession_.SetDefaultPayloadType(remoteRtpPayload_);
			rtpSession_.SetTimestampUnit(1.0 / remoteRtpRate_);
			rtpSession_.SetDefaultTimeStampIncrement(PTIME*remoteRtpRate_/1000);
			rtpSession_.SetDefaultMark(false);
			audioWrite_.start(remoteRtpRate_, PTIME);
			if (audioSrc_ == 0)
				audioRead_.start(remoteRtpRate_, PTIME);
			else if (audioSrc_ == 1)
				audioFile_.start(remoteRtpRate_, PTIME);
			break;
		case RccMessage::CALL_CLOSE:
			numIcome_ = _T("");
			UpdateData(FALSE);
			this->GetDlgItem(IDC_ACCEPT)->EnableWindow(FALSE);
			audioWrite_.stop();
			audioRead_.stop();
			audioFile_.stop();
			str.Format("结束通话。 (%d)", msg->rccClose.mError);
			rtpSession_.ClearDestinations();
			audioCall_ = FALSE;
			break;
		case RccMessage::CALL_RESULT:
			printRccAck(msg->rccResult.ok, (RccMessage::MessageType)msg->rccResult.which, str);
			break;
		case RccMessage::CALL_TRYING:
			str = "呼叫中......";
			break;

		}

		if (str.GetLength() > 2)
		{
			USES_CONVERSION;
			msgList_.AddString(A2W(str));
			msgList_.SetCurSel(msgList_.GetCount() - 1);
		}
		
	}

}

void CSipClientRccDummyDlg::OnBnClickedRegister()
{
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageRegister(W2A(localNum_));
}

void CSipClientRccDummyDlg::OnBnClickedInvite()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageInvite(W2A(remoteNum_), rtpIP_, rtpPort_, rtpPayload_, rtpRate_);
}

void CSipClientRccDummyDlg::OnBnClickedAccept()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageAccept(rtpIP_, rtpPort_, rtpPayload_, rtpRate_);
}

void CSipClientRccDummyDlg::OnBnClickedClosecall()
{
	// TODO: 在此添加控件通知处理程序代码
	rccAgent_.sendMessageClose();
	audioWrite_.stop();
}


void CSipClientRccDummyDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类
	OnBnClickedClosecall();
	rccAgent_.sendMessage(RccMessage::CALL_UNREGISTER);
	Sleep(500);
	rccAgent_.stopAgent();
	this->shutdown();
	this->join();
	rtpSession_.Destroy();
	__super::OnCancel();
}

void CSipClientRccDummyDlg::outputPcm(char * data, int len)
{
	static Buffer tmpBuf;
	if (audioCall_)
	{
		tmpBuf.erase();
		tmpBuf.pushBack(len/2, true);
		unsigned char* pch = (unsigned char*)tmpBuf.beginRead();
		short* s16 = (short*)data;
		for (int i = 0; i < len / 2; ++i)
			pch[i] = s16_to_alaw(s16[i]);
		rtpSession_.SendPacket(pch, len/2);
	}
	else if (audioTest_)
		audioWrite_.inputPcm(data, len);
}

void CSipClientRccDummyDlg::OnBnClickedAudioTest()
{
	if (audioCall_)
		return;

	UpdateData();
	if (audioTest_)
	{
		audioWrite_.start(rtpRate_, PTIME);
		if (audioSrc_ == 0)
			audioRead_.start(rtpRate_, PTIME);
		else if (audioSrc_ == 1)
			audioFile_.start(rtpRate_, PTIME);
	}
	else
	{
		//rtpSession_.ClearDestinations();
		audioWrite_.stop();
		audioRead_.stop();
		audioFile_.stop();
	}
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
		audioRead_.start(rtpRate_, PTIME);
	else if (audioSrc_ == 1)
		audioFile_.start(rtpRate_, PTIME);
}


typedef struct _telephone_event
{
#ifdef RTP_BIG_ENDIAN
	uint32_t evt : 8;
	uint32_t E : 1;
	uint32_t R : 1;
	uint32_t volume : 6;
	uint32_t duration : 16;
#else
	unsigned __int32 evt : 8;
	unsigned __int32 volume : 6;
	unsigned __int32 R : 1;
	unsigned __int32 E : 1;
	unsigned __int32 duration : 16;
#endif
}telephone_event_t;

#define TELEPHONE_EVENT 101
void CSipClientRccDummyDlg::OnDtmfKey(UINT key_id)
{
	if (!audioCall_)
		return;

	int evt = key_id - IDC_DTMF_0;
	int vol = 10;
	int duration = 480 / 3;

	telephone_event_t event_hdr;
	event_hdr.evt = evt;
	event_hdr.R = 0;
	event_hdr.E = 0;
	event_hdr.volume = vol;
	event_hdr.duration = duration;
	rtpSession_.SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, true, (unsigned long)0);

	event_hdr.duration += duration;
	rtpSession_.SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, false, (unsigned long)0);

	event_hdr.duration += duration;
	event_hdr.E = 1;
	rtpSession_.SendPacket((void *)(&event_hdr), sizeof(event_hdr), (unsigned char)TELEPHONE_EVENT, false, (unsigned long)480);
}
