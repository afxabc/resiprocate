
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

CSipClientRccDummyDlg::CSipClientRccDummyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIPCLIENTRCCDUMMY_DIALOG, pParent)
	, localNum_(_T("1001"))
//	, remoteNum_(_T("1000"))
	, remoteNum_(_T("9664"))
	, rtpIP_("10.10.3.100")
	, rtpPort_(24680)
//	, rtpPayload_(0)		//"ULaw"
	, rtpPayload_(8)		//"ALaw"
	, rtpRate_(8000)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSipClientRccDummyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOCAL_NUM, localNum_);
	DDX_Text(pDX, IDC_REMOTE_NUM, remoteNum_);
	DDX_Control(pDX, IDC_MSG_LIST, msgList_);
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
	int ret = rtpSession_.Create(rtpPort_);
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
	rtpSession_.Destroy();
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
	Buffer tmpBuf;
	while (!isShutdown())
	{
		if (!checkForRcc() && !audioWrite_.isStart())
			Sleep(100);

		if (audioWrite_.isStart())
		{
			rtpSession_.PollData();

			// check incoming packets
			if (rtpSession_.GotoFirstSourceWithData())
			{
				do
				{
					RTPPacket *pack;
					while ((pack = rtpSession_.GetNextPacket()) != NULL)
					{
						// You can examine the data here
						char* data = (char*)pack->GetPayload();
						int len = pack->GetPayloadLength();

						if (pack->GetPayloadType() == rtpPayload_ && len > 0)	//is alaw?
						{
							tmpBuf.pushBack(len*2, true);
							short* s16 = (short*)tmpBuf.beginRead();
							for (int j = 0; j<len; j++)
								s16[j] = alaw_to_s16(data[j]);
							audioWrite_.inputPcm(tmpBuf.beginRead(), tmpBuf.readableBytes());
							tmpBuf.erase();
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
		{
			struct in_addr addr;
			memcpy(&addr, &msg->rccAccept.mRtpIP, 4);
			str.Format("应答：RTP(%s:%d), codec(%d, %d)", inet_ntoa(addr), msg->rccAccept.mRtpPort, msg->rccAccept.mRtpPayload, msg->rccAccept.mRtpRate);
			remoteRtpIP_ = msg->rccAccept.mRtpIP;
			remoteRtpPort_ = msg->rccAccept.mRtpPort;
			remoteRtpPayload_ = msg->rccAccept.mRtpPayload;
			remoteRtpRate_ = msg->rccAccept.mRtpRate;
			break;
		}
		case RccMessage::CALL_INVITE:
		{
			struct in_addr addr;
			memcpy(&addr, &msg->rccInvite.mRtpIP, 4);
			str.Format("来电：%s, RTP(%s:%d), codec(%d, %d)", msg->rccInvite.mCallNum, inet_ntoa(addr), msg->rccInvite.mRtpPort, msg->rccInvite.mRtpPayload, msg->rccInvite.mRtpRate);
			remoteRtpIP_ = msg->rccAccept.mRtpIP;
			remoteRtpPort_ = msg->rccAccept.mRtpPort;
			remoteRtpPayload_ = msg->rccAccept.mRtpPayload;
			remoteRtpRate_ = msg->rccAccept.mRtpRate;
			break;
		}
		case RccMessage::CALL_CONNECTED:
			str = "通话中......";
			rtpSession_.AddDestination(ntohl(remoteRtpIP_), remoteRtpPort_);
			audioWrite_.start(remoteRtpRate_);
			break;
		case RccMessage::CALL_CLOSE:
			audioWrite_.stop();
			str.Format("结束通话。 (%d)", msg->rccClose.mError);
			rtpSession_.ClearDestinations();
			break;
		case RccMessage::CALL_REG_OK:
			str = "注册成功。";
			break;
		case RccMessage::CALL_REG_FAILED:
			str = "注册失败！";
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
