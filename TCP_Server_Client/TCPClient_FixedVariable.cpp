#include "../Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE 50

int	main(int argc, char *argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
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
		"세번째로 배우는",
		"고정길이+가변길이 데이터 전송 입니다.",
	};
	int len;

	// 서버와 데이터 통신
	for (int i = 0; i < 4; i++)
	{
		// 데이터 입력(시뮬레이션)
		len = (int)strlen(testdata[i]);
		strncpy(buf, testdata[i], len);
		// 데이터 보내기(고정 길이)
		retval = send(sock, (char *)&len, sizeof(int), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break ;
		}
		// 데이터 보내기(가변 길이)
		retval = send(sock, buf, len, 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			break ;
		}
		printf("[TCP 클라이언트] %d 바이트를 보냈습니다.\n", retval);
	}
	// 소켓 닫기
	close(sock);

	return 0;
}