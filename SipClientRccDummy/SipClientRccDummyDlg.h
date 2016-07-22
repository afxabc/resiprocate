
// SipClientRccDummyDlg.h : 头文件
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

// CSipClientRccDummyDlg 对话框
class CSipClientRccDummyDlg : public CDialogEx, 
								resip::ThreadIf, 
								IAudioReadCallback, 
								IRccMessageCallback,
								IRTPMediaCallback,
								IVideoEncodecCallback
{
// 构造
public:
	CSipClientRccDummyDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIPCLIENTRCCDUMMY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	virtual BOOL OnInitDialog();

// 实现
protected:

	// 生成的消息映射函数
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

	// 通过 ThreadIf 继承
	virtual void thread() override;
	virtual void OnOK();
	virtual void OnCancel();

	bool checkForRcc();
	void showString(LPCSTR pszFormat, ...);

	// 通过 IAudioReadCallback 继承
	virtual void outputPcm(char * data, int len) override;

	// 通过 IRccMessageCallback 继承
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

	// 通过 IRTPMediaCallback 继承
	virtual void onMediaData(char * data, int len, unsigned char payload) override;

	// 通过 IVideoEncodecCallback 继承
	virtual void onVideoEncodeFin(char * data, int len, unsigned char pt, bool mark, unsigned long tm) override;

public:
	afx_msg void OnBnClickedVideoEnable();
};
