#include <WinSock.h>

struct ListAddress
{
	static ListAddress Array[8];

	u_short Port;
	u_long  Ip;
};

class Nethack
{
public:
	static bool PortHack;

	static
	int WINAPI SendTo(
		int sockfd,
		char* buf,
		size_t len,
		int flags,
		sockaddr_in* dest_addr,
		int addrlen
	);

	static
	int WINAPI RecvFrom(
		int sockfd,
		char* buf,
		size_t len,
		int flags,
		sockaddr_in* src_addr,
		int* addrlen
	);
};

class Tunnel
{
public:
	static u_short Id;
	static u_long  Ip;
	static u_short Port;

	static
	int WINAPI SendTo(
		int sockfd,
		char* buf,
		size_t len,
		int flags,
		sockaddr_in* dest_addr,
		int addrlen
	);

	static
	int WINAPI RecvFrom(
		int sockfd,
		char* buf,
		size_t len,
		int flags,
		sockaddr_in* src_addr,
		int* addrlen
	);
};
