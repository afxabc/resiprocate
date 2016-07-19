
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
static const unsigned char PAYLOAD_AUDIO = 8;
static const unsigned int RATE_AUDIO = 8000;
CSipClientRccDummyDlg::CSipClientRccDummyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SIPCLIENTRCCDUMMY_DIALOG, pParent)
	, localNum_(_T("1000"))
	, remoteNum_(_T("5000"))
//	, remoteNum_(_T("9664"))		//����
//	, remoteNum_(_T("9196"))		//echo
	, rccIP_("127.0.0.1")
	, rccPort_(RCC_SERVER_PORT_BASE)
	, rtpAudio_(this, "127.0.0.1", RCC_RTP_PORT_BASE, PAYLOAD_AUDIO, RATE_AUDIO)
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


// CSipClientRccDummyDlg ��Ϣ��������
extern int LoadParamString(const char* profile, char* AppName, char* KeyName, char* KeyVal);
BOOL CSipClientRccDummyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ����Ӷ���ĳ�ʼ������
	static const char* finame = "./config.txt";
	char tmpBuf[1024];
	if (LoadParamString(finame, "RTP", "ADDR", tmpBuf) >= 2)
	{
		rtpAudio_.ip() = tmpBuf;
	}

	if (LoadParamString(finame, "RTP", "PORT", tmpBuf) >= 2)
	{
		unsigned short rtpPortBase = atoi(tmpBuf);
		rtpPortBase = rtpAudio_.tryPort(rtpPortBase);
	}

	if (LoadParamString(finame, "RCC", "ADDR", tmpBuf) >= 2)
		rccIP_ = tmpBuf;
	if (LoadParamString(finame, "RCC", "PORT", tmpBuf) >= 2)
		rccPort_ = atoi(tmpBuf);

	rccAgent_.startAgent(RCC_CLIENT_PORT_BASE, NULL, rccPort_, rccIP_.c_str());
	this->run();

//	audioRead_.enumDevices();

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի���������С����ť������Ҫ����Ĵ���
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

////////////////////////// ͨ�� IRccMessageCallback �̳�

void CSipClientRccDummyDlg::onMessage(RccMessage::MessageType type)
{
	TRACE("onMessage : [%d]\n", (int)type);

	if (type == RccMessage::RCC_CONN)
		onMessageConn();
}

void CSipClientRccDummyDlg::onMessageAcm(RccMessage::MessageType which, unsigned char result)
{
	if (which >= RccMessage::RCC_END)
		return;

	static const char* msgTag[RccMessage::RCC_END] = { "??", "ע��", "ע��", "�������",
		"��Ϣ��Ӧ", "Ӧ��", "�ͷ�", "����", "��չ" };

	if (result == 0)
		showString("%s�ɹ���", msgTag[which]);
	else showString("%sʧ�ܣ�%d������", msgTag[which], result);
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

	rtpAudio_.stop();
	audioWrite_.stop();
	audioRead_.stop();
	audioFile_.stop();

	showString("����ͨ���� (%d)", (int)reason);
	audioCall_ = FALSE;
}

void CSipClientRccDummyDlg::onMessageIam(const char * callNumber, RccRtpDataList & rtpDataList)
{
	USES_CONVERSION;
	numIcome_.Format(_T(" ������룺%s"), A2W(callNumber));
	UpdateData(FALSE);
	this->GetDlgItem(IDC_ACCEPT)->EnableWindow(TRUE);

	showString("������룺%s", callNumber);
	for (unsigned int i = 0; i < rtpDataList.size(); ++i)
	{
		if (PAYLOAD_AUDIO == rtpDataList[i].payload)
		{
			rtpAudio_.setRemote(rtpDataList[i].ip, rtpDataList[i].port, rtpDataList[i].payload, rtpDataList[i].rate);

			showString("  [%d] RTP(%s:%d)", i, rtpDataList[i].ip.c_str(), rtpDataList[i].port);
			showString("  [%d] codec(%d, %d)", i, rtpDataList[i].payload, rtpDataList[i].rate);
		}
	}
}

void CSipClientRccDummyDlg::onMessageAnm(RccRtpDataList & rtpDataList)
{
	showString("�յ�Ӧ��");
	for (unsigned int i = 0; i < rtpDataList.size(); ++i)
	{
		if (PAYLOAD_AUDIO == rtpDataList[i].payload)
		{
			rtpAudio_.setRemote(rtpDataList[i].ip, rtpDataList[i].port, rtpDataList[i].payload, rtpDataList[i].rate);

			showString("  [%d] RTP(%s:%d)", i, rtpDataList[i].ip.c_str(), rtpDataList[i].port);
			showString("  [%d] codec(%d, %d)", i, rtpDataList[i].payload, rtpDataList[i].rate);
		}
	}
}

void CSipClientRccDummyDlg::onInvalidMessage(RccMessage * msg)
{
}

void CSipClientRccDummyDlg::onMessageConn()
{
	this->GetDlgItem(IDC_ACCEPT)->EnableWindow(FALSE);

	rtpAudio_.start(PTIME);
	audioWrite_.start(rtpAudio_.rate(), PTIME);
	if (audioSrc_ == 0)
		audioRead_.start(rtpAudio_.rate(), PTIME);
	else if (audioSrc_ == 1)
		audioFile_.start(rtpAudio_.rate(), PTIME);

	audioCall_ = TRUE;
	showString("ͨ����......");
}

/////////////////////////////////////////////////////////////

void CSipClientRccDummyDlg::OnBnClickedRegister()
{
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageRgst(W2A(localNum_));
}

void CSipClientRccDummyDlg::OnBnClickedInvite()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageIam(W2A(remoteNum_), RccRtpData(rtpAudio_.ip().c_str(), rtpAudio_.port(), rtpAudio_.payload(), rtpAudio_.rate()));
}

void CSipClientRccDummyDlg::OnBnClickedAccept()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	UpdateData();

	USES_CONVERSION;
	rccAgent_.sendMessageAnm(RccRtpData(rtpAudio_.ip().c_str(), rtpAudio_.port(), rtpAudio_.payload(), rtpAudio_.rate()));
}

void CSipClientRccDummyDlg::OnBnClickedClosecall()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	rccAgent_.sendMessageRel();
	audioWrite_.stop();
}


void CSipClientRccDummyDlg::OnCancel()
{
	// TODO: �ڴ�����ר�ô����/����û���
	rtpAudio_.stop();

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
	static Buffer tmpBuf;
	if (audioCall_)
	{
		tmpBuf.erase();
		tmpBuf.pushBack(len/2, true);
		unsigned char* pch = (unsigned char*)tmpBuf.beginRead();
		short* s16 = (short*)data;
		for (int i = 0; i < len / 2; ++i)
			pch[i] = s16_to_alaw(s16[i]);
		
		rtpAudio_.sendData((char*)pch, len/2);
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
		audioWrite_.start(rtpAudio_.r_rate(), PTIME);
		if (audioSrc_ == 0)
			audioRead_.start(rtpAudio_.r_rate(), PTIME);
		else if (audioSrc_ == 1)
			audioFile_.start(rtpAudio_.r_rate(), PTIME);
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
		audioRead_.start(rtpAudio_.r_rate(), PTIME);
	else if (audioSrc_ == 1)
		audioFile_.start(rtpAudio_.r_rate(), PTIME);
}

void CSipClientRccDummyDlg::OnDtmfKey(UINT key_id)
{
	if (!audioCall_)
		return;

	int evt = key_id - IDC_DTMF_0;
	int vol = 10;
	int duration = 480 / 3;

	rtpAudio_.sendDtmfKey(evt, vol, duration);
}

void CSipClientRccDummyDlg::onMediaData(char * data, int len, unsigned char payload)
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
}