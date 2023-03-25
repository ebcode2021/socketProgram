#include "Poll.hpp"

int	main(int argc, char *argv[])
{
	int	retval;
	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");
	
	// bind()
	struct sockaddr_in	serveraddr;
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
	
	// 넌블로킹 소켓으로 전환
	int flags = fcntl(listen_sock, F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(listen_sock, F_SETFL, flags);

	// 소켓 이벤트 등록(1)
	Pollfds[0].fd = listen_sock;
	Pollfds[0].events = POLLIN; /*POLLRDNORM도 같음*/
	++nTotalSockets;

	// 데이터 통신에 사용할 변수
	int nready;
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;

	printf("??\n");
	while (1)
	{
		// 소켓 이벤트 등록(2)
		// 소켓 정보 구조체를 참조해 모든 소켓에 대해 이벤트를 등록한다.
		// 받은 데이터가 보낸 데이터보다 많으면 POLLOUT 이벤트를, 그렇지 않으면 POLLIN 이벤트를 등록한다.
		// 이는 에코 서버 특성상 받은 데이터를 그대로 보내줘야 하기 때문이다.
		for (int i = 1; i < nTotalSockets; i++)
		{
			if (SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes)
				Pollfds[i].events = POLLOUT; /* POLLWRNORM도 같음 */
			else
				Pollfds[i].events = POLLIN; /* POLLRDNORM도 같음 */
		}
		// poll()
		// 타임아웃에 -1을 사용하므로 조건을 만족할 때까지 무한히 대기
		nready = poll(Pollfds, nTotalSockets, -1);
		if (nready == SOCKET_ERROR)
			err_quit("poll()");
		
		// 소켓 이벤트 검사(1) : 클라이언트 접속 수용
		if (Pollfds[0].revents & POLLIN)
		{
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET)
			{
				err_display("accept()");
				break ;
			}
			else
			{
				// 넌블로킹 소켓으로 전환
				int flags = fcntl(client_sock, F_GETFL);
				flags |= O_NONBLOCK;
				fcntl(client_sock, F_SETFL, flags);
				// 클라이언트 정보 출력
				char addr[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
				printf("\n[TCP 서버] 클라이언트 접속 : IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));
				// 소켓 정보 추가 : 실패 시 소켓 닫음
				if (!AddSocketInfo(client_sock))
					close(client_sock);
			}
			if (--nready <= 0)
				continue;
		}
		// 소켓 이벤트 검사(2) : 데이터 통신
		// poll() 함수는 조건을 만족하는 소켓의 개수를 리턴하지만 구체적으로 어떤 소켓인지는 알려주지 않는다.
		// 따라서 응용 프로그램이 관리하는 모든 소켓에 대해 이벤트가 발생했는지 일일히 확인해야 한다.
		// 연결 대기 소켓은 이미 확인했으므로 인덱스 1부터 시작한다.
		for (int i = 1; i < nTotalSockets; i++)
		{
			SOCKETINFO *ptr = SocketInfoArray[i];
			SOCKET sock = Pollfds[i].fd;
			if (Pollfds[i].revents & POLLIN) // 데이터 수신이 가능한지
			{
				// 데이터 받기
				retval = recv(sock, ptr->buf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("recv()");
					RemoveSocketInfo(i);
				}
				else if (retval == 0)
					RemoveSocketInfo(i);
				else
				{
					ptr->recvbytes = retval;
					// 클라이언트 정보 얻기
					addrlen = sizeof(clientaddr);
					getpeername(sock, (struct sockaddr *)&clientaddr, &addrlen);
					// 받은 데이터 출력
					ptr->buf[ptr->recvbytes] = '\0';
					char addr[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
					printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), ptr->buf);
				}
			}
			if (Pollfds[i].revents & POLLOUT) // 데이터 송신이 가능한지
			{
				// 데이터 보내기
				retval = send(sock, ptr->buf + ptr->sendbytes, ptr->recvbytes - ptr->sendbytes, 0);
				if (retval == SOCKET_ERROR)
				{
					err_display("send()");
					RemoveSocketInfo(i);
				}
				else
				{
					ptr->sendbytes += retval;
					if (ptr->recvbytes == ptr->sendbytes)
						ptr->recvbytes = ptr->sendbytes = 0;
				}
			}
		} /* end of for */
	} /* end of while */

	// 소켓 닫기
	close(listen_sock);
	return 0;
}