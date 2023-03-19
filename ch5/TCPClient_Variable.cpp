#include "../Common.h"

char *SERVERIP = (char *)"127.0.0.1";

#define SERVERPORT	9000
#define BUFSIZE		50

int	main(int argc, char *argv[])
{
	int	retval;

	if (argc > 1)
		SERVERIP = argv[1];
	
	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		err_quit("socket()");
	
	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");
	
	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	const char *testdata[] = {
		"안녕하세요",
		"반가워요",
		"이거는 가변길이 데이터 전송 연습",
		"이에요",
	};
	int	len;

	// 서버와 데이터 통신
	for (int i = 0; i < 4; i++)
	{
		// 데이터 입력(시뮬레이션)
		len = (int)strlen(testdata[i]);
		strncpy(buf, testdata[i], len);
		buf[len++] = '\n';
		// 데이터 보내기(len 크기만큼 데이터를 보냄)
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break ;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
	}
	close(sock);
	return 0;
}