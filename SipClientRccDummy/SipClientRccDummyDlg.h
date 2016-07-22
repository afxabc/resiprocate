
// SipClientRccDummyDlg.h : ͷ�ļ�
//

#pragma once

#include "queue.h"
#include "AudioRead.h"
#include "AudioFile.h"
#include "AudioWrite.h"
#include "RTPMedia.h"
#include "VideoCapDesktop.h"
#include "VideoDraw.h"

#include "jrtplib\rtpsession.h"
#include "jrtplib\rtppacket.h"
#include "SipClientUdp\RccUserAgent.h"
#include "rutil\ThreadIf.hxx"

#include "afxwin.h"

// CSipClientRccDummyDlg �Ի���
class CSipClientRccDummyDlg : public CDialogEx, 
								resip::ThreadIf, 
								IAudioReadCallback, 
								IRccMessageCallback,
								IRTPMediaCallback,
								IVideoEncodecCallback
{
// ����
public:
	CSipClientRccDummyDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIPCLIENTRCCDUMMY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	virtual BOOL OnInitDialog();

// ʵ��
protected:

	// ���ɵ���Ϣӳ�亯��
	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRegister();
	afx_msg void OnBnClickedInvite();
	afx_msg void OnBnClickedAccept();
	afx_msg void OnBnClickedClosecall();
	afx_msg void OnBnClickedAudioTest();
	afx_msg void OnCbnSelchangeAudioSrc();

	afx_msg void OnDtmfKey(UINT key_id);

	// ͨ�� ThreadIf �̳�
	virtual void thread() override;
	virtual void OnOK();
	virtual void OnCancel();

	bool checkForRcc();
	void showString(LPCSTR pszFormat, ...);

	// ͨ�� IAudioReadCallback �̳�
	virtual void outputPcm(char * data, int len) override;

	// ͨ�� IRccMessageCallback �̳�
	virtual void onMessage(RccMessage::MessageType type) override;
	virtual void onMessageAcm(RccMessage::MessageType which, unsigned char result) override;
	virtual void onMessageRgst(const char * callNumber) override;
	virtual void onMessageUrgst(const char * callNumber) override;
	virtual void onMessageRel(unsigned char reason) override;
	virtual void onMessageIam(const char * callNumber, RccRtpDataList & rtpDataList) override;
	virtual void onMessageAnm(RccRtpDataList & rtpDataList) override;
	virtual void onInvalidMessage(RccMessage * msg) override;
	void onMessageConn();

protected:
	HICON m_hIcon;
	CString localNum_;
	CString remoteNum_;
	CListBox msgList_;

	std::string rccIP_;
	unsigned short rccPort_;
	RccUserAgent rccAgent_;
	typedef Queue<resip::Data> QUEUE;
	QUEUE queue_;

	RTPMedia rtpAudio_;
	RTPMedia rtpVideo_;

	VideoCapDesktop videoCap_; 
	VideoDraw	videoDraw_;

	AudioWrite audioWrite_;
	AudioRead audioRead_;
	AudioFile audioFile_;
	BOOL audioTest_;
	BOOL audioCall_;
	int audioSrc_;
	CString numIcome_;
	BOOL videoEnable_;

	CStatic drawWnd_;
	int winMin_;
	int winMax_;

	// ͨ�� IRTPMediaCallback �̳�
	virtual void onMediaData(char * data, int len, unsigned char payload) override;

	// ͨ�� IVideoEncodecCallback �̳�
	virtual void onVideoEncodeFin(char * data, int len, unsigned char pt, bool mark, unsigned long tm) override;

public:
	afx_msg void OnBnClickedVideoEnable();
};
