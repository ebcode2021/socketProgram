#include "../Common.h"

#define SERVERPORT 9000
#define BUFSIZE 512

// 내부 구현용 함수
// 소켓 수신 버퍼에서 데이터를 한꺼번에 읽어서 내부에 저장해두고 읽기 요청이 있을 때마다 1바이트씩 리턴해주는 사용자함수
int	_recv_ahead(SOCKET s, char *p)
{
	// 소켓 수신 버퍼에서 읽은 데이터를 1바이트씩 리턴하는 데 필요한 핵심 변수들
	// 함수가 리턴해도 값을 계속 유지해야 하므로 static 변수 선언
	static int nbytes = 0;
	static char buf[1024];
	static char *ptr;

	if (nbytes == 0 || nbytes == SOCKET_ERROR) // 읽어들일 데이터가 아직 없거나 모두 소진한 경우
	{
		nbytes = recv(s, buf, sizeof(buf), 0);
		if (nbytes == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (nbytes == 0)
			return 0;
		ptr = buf;
	}
	// 남은 바이트 수 감소
	--nbytes;
	// 포인터 변수 ptr이 가리키는 데이터를 포인터 변수 p가 가리키는 메모리 영역에 복사하고 리턴
	*p = *ptr++;
	return 1;
}

// 사용자 정의 데이터 수신 함수
// '\n'이 나오거나 수신 데이터가 없을 때까지 데이터를 읽어들이는 함수
int	recvline(SOCKET s, char *buf, int maxlen)
{
	int n, nbytes;
	char c, *ptr = buf;

	for (n  = 1; n < maxlen; n++)
	{
		nbytes = _recv_ahead(s, &c);
		if (nbytes == 1)
		{
			*ptr++ = c;
			if (c == '\n')
				break;
		}
		else if (nbytes == 0)
		{
			*ptr = 0;
			return (n - 1);
		}
		else
			return SOCKET_ERROR;
	}
	*ptr = 0;
	return n;
}

int	main(int argc, char *argv[])
{
	int retval;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");
	
	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");
	
	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");
	
	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char buf[BUFSIZE + 1];

	while (1)
	{
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			break ;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속 : IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1)
		{
			// 데이터 받기
			retval = recvline(client_sock, buf, BUFSIZE + 1);
			if (retval == SOCKET_ERROR)
			{
				err_display("recv()");
				break ;
			}
			else if (retval == 0)
				break ;
			// 받은 데이터 출력
			printf("[TCP/%s:%d] %s", addr, ntohs(clientaddr.sin_port), buf);
		}
		close(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));
	}
	close(listen_sock);
	return 0;
}