
// SipClientRccDummyDlg.cpp : ʵ���ļ�
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
// CSipClientRccDummyDlg �Ի���

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
	, remoteNum_(_T("1000"))
//	, remoteNum_(_T("9664"))
	, rtpIP_("10.10.3.100")
	, rtpPort_(24680)
//	, rtpPayload_(0)		//"ULaw"
	, rtpPayload_(8)		//"ALaw"
	, rtpRate_(8000)
	, audioRead_(this)
	, audioTest_(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSipClientRccDummyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOCAL_NUM, localNum_);
	DDX_Text(pDX, IDC_REMOTE_NUM, remoteNum_);
	DDX_Control(pDX, IDC_MSG_LIST, msgList_);
	DDX_Check(pDX, IDC_AUDIO_TEST, audioTest_);
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
	int ret = rtpSession_.Create(rtpPort_);
	rccAgent_.startAgent(21358, NULL, DUMMY_RCC_PORT, "10.10.3.100");
	this->run();

	audioRead_.enumDevices();

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
	fd_set fdset;
	struct timeval rttprev = { 0,0 }, rtt, tv;
	SOCKET sock1, sock2;
	rtpSession_.GetRTPSocket(&sock1);
	rtpSession_.GetRTCPSocket(&sock2);

	while (!isShutdown())
	{
		if (!checkForRcc() && !audioWrite_.isStart())
			Sleep(20);

		if (audioWrite_.isStart())
		{
			tv.tv_sec = 0;
			tv.tv_usec = 10000;
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

						if (pack->GetPayloadType() == rtpPayload_ && len > 0)	//is alaw?
						{
							
							//tmpBuf.pushBack(data, len, true);
							TRACE("RTPPacket len=%d\n", len);
							tmpBuf.pushBack(len*2, true);
							short* s16 = (short*)tmpBuf.beginRead();
							for (int j = 0; j<len; j++)
								s16[j] = alaw_to_s16(data[j]);
							
							audioWrite_.inputPcm(tmpBuf.beginRead(), tmpBuf.readableBytes());
							tmpBuf.erase();
						}
						else
						{
							int a = 1;
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
		str = "ע��";
		break;
	case RccMessage::CALL_UNREGISTER:
		str = "ע��";
		break;
	default:
		str = "ִ��";
	}
	str += (ok) ? "�ɹ�" : "ʧ��";
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
			str.Format("Ӧ��RTP(%s:%d), codec(%d, %d)", msg->rccAccept.mRtpIP, msg->rccAccept.mRtpPort, msg->rccAccept.mRtpPayload, msg->rccAccept.mRtpRate);
			remoteRtpIP_ = msg->rccAccept.mRtpIP;
			remoteRtpPort_ = msg->rccAccept.mRtpPort;
			remoteRtpPayload_ = msg->rccAccept.mRtpPayload;
			remoteRtpRate_ = msg->rccAccept.mRtpRate;
			break;
		case RccMessage::CALL_INVITE:
			str.Format("���磺%s, RTP(%s:%d), codec(%d, %d)", msg->rccInvite.mCallNum, msg->rccInvite.mRtpIP, msg->rccInvite.mRtpPort, msg->rccInvite.mRtpPayload, msg->rccInvite.mRtpRate);
			remoteRtpIP_ = msg->rccAccept.mRtpIP;
			remoteRtpPort_ = msg->rccAccept.mRtpPort;
			remoteRtpPayload_ = msg->rccAccept.mRtpPayload;
			remoteRtpRate_ = msg->rccAccept.mRtpRate;
			break;
		case RccMessage::CALL_CONNECTED:
			str = "ͨ����......";
			rtpSession_.AddDestination(ntohl(inet_addr(remoteRtpIP_.begin())), remoteRtpPort_);
			rtpSession_.SetTimestampUnit(1.0 / remoteRtpRate_);
			audioWrite_.start(remoteRtpRate_, &rtpSession_);
			break;
		case RccMessage::CALL_CLOSE:
			audioWrite_.stop();
			str.Format("����ͨ���� (%d)", msg->rccClose.mError);
			rtpSession_.ClearDestinations();
			break;
		case RccMessage::CALL_RESULT:
			printRccAck(msg->rccResult.ok, (RccMessage::MessageType)msg->rccResult.which, str);
			break;
		case RccMessage::CALL_TRYING:
			str = "������......";
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageInvite(W2A(remoteNum_), rtpIP_, rtpPort_, rtpPayload_, rtpRate_);
}

void CSipClientRccDummyDlg::OnBnClickedAccept()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageAccept(rtpIP_, rtpPort_, rtpPayload_, rtpRate_);
}

void CSipClientRccDummyDlg::OnBnClickedClosecall()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	rccAgent_.sendMessageClose();
	audioWrite_.stop();
}


void CSipClientRccDummyDlg::OnCancel()
{
	// TODO: �ڴ����ר�ô����/����û���
	rccAgent_.sendMessage(RccMessage::CALL_UNREGISTER);
	Sleep(500);
	__super::OnCancel();
}

void CSipClientRccDummyDlg::outputPcm(const char * data, int len)
{
	if (audioTest_ && audioWrite_.isStart())
		audioWrite_.inputPcm(data, len);
}

void CSipClientRccDummyDlg::OnBnClickedAudioTest()
{
	UpdateData();
	if (audioTest_)
	{
		audioWrite_.start(rtpRate_, &rtpSession_);
		audioRead_.start(rtpRate_);
	}
	else
	{
		audioWrite_.stop();
		audioRead_.stop();
	}
}
