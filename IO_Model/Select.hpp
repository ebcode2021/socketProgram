#include "../Common.h"

#define	SERVERPORT	9000
#define BUFSIZE		512

// 소켓 정보 저장을 위한 구조체와 변수
struct	SOCKETINFO
{
	SOCKET	sock;
	char	buf[BUFSIZE + 1];
	int		recvbytes;
	int		sendbytes;
};

int	nTotalSockets = 0;
SOCKETINFO	*SocketInfoArray[FD_SETSIZE];


/* 소켓 정보 관리 함수 */
// 소켓 정보 추가
bool	AddSocketInfo(SOCKET sock)
{
	if (nTotalSockets >= FD_SETSIZE)
	{
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return false;
	}
	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		printf("[오류] 메모리가 부족합니다!\n");
		return false;
	}
	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;
	return true;
}
// 소켓 정보 삭제
void	RemoveSocketInfo(int nIndex)
{
	SOCKETINFO *ptr = SocketInfoArray[nIndex];

	// 클라이언트 정보 얻기
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (struct sockaddr *)&clientaddr, &addrlen);

	// 클라이언트 정보 출력
	char addr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("[TCP 서버] 클라이언트 종료 : IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));

	// 소켓 닫기
	close(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];
	--nTotalSockets;
}

// {최대 소켓 번호 + 1} 얻는 함수
int		GetMaxFDPlus1(SOCKET s)
{
	int	maxfd = s;

	for (int i = 0; i < nTotalSockets; i++)
	{
		if (SocketInfoArray[i]->sock > maxfd)
			maxfd = SocketInfoArray[i]->sock;
	}
	return maxfd + 1;
}

