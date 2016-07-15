
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

static const int PTIME = 20;
CSipClientRccDummyDlg::CSipClientRccDummyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIPCLIENTRCCDUMMY_DIALOG, pParent)
	, localNum_(_T("1000"))
	, remoteNum_(_T("5000"))
//	, remoteNum_(_T("9664"))		//����
//	, remoteNum_(_T("9196"))		//echo
	, rccIP_("127.0.0.1")
	, rccPort_(RCC_SERVER_PORT_BASE)
	, rtpIP_("127.0.0.1")
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


// CSipClientRccDummyDlg ��Ϣ�������
extern int LoadParamString(const char* profile, char* AppName, char* KeyName, char* KeyVal);
BOOL CSipClientRccDummyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	static const char* finame = "./config.txt";
	char tmpBuf[1024];
	if (LoadParamString(finame, "RTP", "ADDR", tmpBuf) >= 2)
		rtpIP_ = tmpBuf;
	if (LoadParamString(finame, "RTP", "PORT", tmpBuf) >= 2)
		rtpPort_ = atoi(tmpBuf);
	if (LoadParamString(finame, "RCC", "ADDR", tmpBuf) >= 2)
		rccIP_ = tmpBuf;
	if (LoadParamString(finame, "RCC", "PORT", tmpBuf) >= 2)
		rccPort_ = atoi(tmpBuf);

	int ret = 0;
	while ((ret = rtpSession_.Create(rtpPort_)) < 0)
	{
		TRACE("tpSession_.Create(%d) return %d !!!\n", rtpPort_, ret);
		rtpPort_ += 2;
	}
	rccAgent_.startAgent(RCC_CLIENT_PORT_BASE, NULL, rccPort_, rccIP_.c_str());
	this->run();

//	audioRead_.enumDevices();

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
			
			resip::Lock lock(rtpMutex_);

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
		str = "ע��";
		break;
	case RccMessage::CALL_UNREGISTER:
		str = "ע��";
		break;
	case RccMessage::CALL_INVITE:
		str = "����";
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
			{
				struct in_addr addr;
				addr.S_un.S_addr = htonl(msg->rccAccept.mRtpIP);
				remoteRtpIP_ = inet_ntoa(addr);
				remoteRtpPort_ = msg->rccAccept.mRtpPort;
				remoteRtpPayload_ = msg->rccAccept.mRtpPayload;
				remoteRtpRate_ = msg->rccAccept.mRtpRate;

				USES_CONVERSION;
				str.Format("Ӧ��RTP(%s:%d)", remoteRtpIP_.c_str(), remoteRtpPort_);
				msgList_.AddString(A2W(str));
				str.Format("Ӧ��codec(%d, %d)", remoteRtpPayload_, remoteRtpRate_);
			}
			break;
		case RccMessage::CALL_INVITE:
			{
				USES_CONVERSION;
				numIcome_.Format(_T(" ������룺%s"), A2W(msg->rccInvite.mCallNum));
				UpdateData(FALSE);
				this->GetDlgItem(IDC_ACCEPT)->EnableWindow(TRUE);

				struct in_addr addr;
				addr.S_un.S_addr = htonl(msg->rccInvite.mRtpIP);
				remoteRtpIP_ = inet_ntoa(addr);
				remoteRtpPort_ = msg->rccInvite.mRtpPort;
				remoteRtpPayload_ = msg->rccInvite.mRtpPayload;
				remoteRtpRate_ = msg->rccInvite.mRtpRate;

				str.Format("���磺RTP(%s:%d)", remoteRtpIP_.c_str(), remoteRtpPort_);
				msgList_.AddString(A2W(str));
				str.Format("���磺codec(%d, %d)", remoteRtpPayload_, remoteRtpRate_);
			}
			break;
		case RccMessage::CALL_CONNECTED:
			this->GetDlgItem(IDC_ACCEPT)->EnableWindow(FALSE);
			str = "ͨ����......";
			audioCall_ = TRUE;
			{
				resip::Lock lock(rtpMutex_);
				rtpSession_.ClearDestinations();
			}
			rtpSession_.AddDestination(ntohl(inet_addr(remoteRtpIP_.c_str())), remoteRtpPort_);
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
			str.Format("����ͨ���� (%d)", msg->rccClose.mReason);
			{
				resip::Lock lock(rtpMutex_);
				rtpSession_.ClearDestinations();
			}
			audioCall_ = FALSE;
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
	rccAgent_.sendMessageInvite(W2A(remoteNum_), rtpIP_.c_str(), rtpPort_, rtpPayload_, rtpRate_);
}

void CSipClientRccDummyDlg::OnBnClickedAccept()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageAccept(rtpIP_.c_str(), rtpPort_, rtpPayload_, rtpRate_);
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
		
		resip::Lock lock(rtpMutex_);
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
