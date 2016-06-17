#include "rutil/Data.hxx"

class StreamParam
{
public:
	StreamParam();
	~StreamParam();

	void setLocalPort(const int port) {localport=port;};
	void setRemotePort(const int port) {remoteport=port;};
	void setRemoteRtcpport(const int port) {remotertcpport=port;};
	void setPayType(const int type) {pt=type;};
	void setRelaySessionId(const resip::Data id) {relay_session_id=id;};
	void setNatPort(const int port) {natd_port=port;};
	void setRemoteAddr(const resip::Data addr) {remoteaddr=addr;};
	void setNatAddr(const resip::Data addr) {natd_addr=addr;};

	const int getLocalPort() {return localport;};
	const int getRemotePort() {return remoteport;};
	const int getRemoteRtcpport() {return remotertcpport;};
	const int getPayType() {return pt;};
	const resip::Data& getRelaySessionId() {return relay_session_id;};
	const int getNatPort() {return natd_port;};
	const resip::Data& getRemoteAddr() {return remoteaddr;};
	const resip::Data& getNatAddr() {return natd_addr;};

private:
	int initialized;
	int line;
	int localport;
	int remoteport;
	int remotertcpport;
	int pt;
	resip::Data relay_session_id;
	int natd_port;
	resip::Data remoteaddr;
	resip::Data natd_addr;
};