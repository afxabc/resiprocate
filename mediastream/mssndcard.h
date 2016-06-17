
#ifndef MSSNDCARD_H
#define MSSNDCARD_H

#include <map>
#include "rutil/Data.hxx"

class WincardRead;
class WincardWrite;

class MSSndCard
{
public:
	MSSndCard();
	~MSSndCard();

	int GetAudioCapDevTotal();
	int GetAudioPlayDevTotal();
	void MSSndCardDetect();

	void MSVideoDevDetect();

	void CapCardSetLevel(int cardno, int percent);
	void PlayCardSetLevel(int cardno, int percent);
	void CapCardSetCapture(int cardno);
	void PlayCardSetCapture(int cardno);
	int CapCardGetLevel(int cardnoe);
	int PlayCardGetLevel(int cardnoe);

	void AddCapCardName(int cardno,char* name);
	void AddPlayCardName(int cardno,char* name);
	resip::Data GetCapCardName(int cardno);
	resip::Data GetPlayCardName(int cardno);

	int GetVideoDevTotal();
	resip::Data GetVideoCapName(int cardno);

	void PlayDTMF();
	void process();

	void SetSpkLevel(int level) {spklevel=level;};
	int GetSpkLevel() {return spklevel;};
	void SetSpkMute(bool mute) {spkmute=mute;};
	bool GetSpkMute() {return spkmute;};

	void SetMicLevel(int level) {miclevel=level;};
	int GetMicLevel() {return miclevel;};
	void SetMicMute(bool mute) {micmute=mute;};
	bool GetMicMute() {return micmute;};
	void SetDTMFValue(resip::Data& dt);

	void SetDTMFDevice(int dev) {devid=dev;};

private:
	std::map<int,resip::Data> mCapCards;
	std::map<int,resip::Data> mPlayCards;

	std::map<int,resip::Data> mVideoDevs;

	int spklevel;   
	bool spkmute;

	int miclevel;  
	bool micmute;

	WincardWrite* play;
	int devid;
	resip::Data DTMFValue;
};

#endif
