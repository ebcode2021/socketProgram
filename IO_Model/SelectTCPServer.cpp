#include "Select.hpp"

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

	// 데이터 통신에 사용할 변수
	fd_set rset, wset;
	int nready;
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	socklen_t addrlen;

	while (1)
	{
		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(listen_sock, &rset);
		for (int i = 0; i < nTotalSockets; i++)
		{
			if (SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes)
				FD_SET(SocketInfoArray[i]->sock, &wset);
			else
				FD_SET(SocketInfoArray[i]->sock, &rset);
		}
		// select()
		nready = select(GetMaxFDPlus1(listen_sock), &rset, &wset, NULL, NULL);
		if (nready == SOCKET_ERROR)
			err_quit("select()");
		
		// 소켓 셋 검사(1) : 클라이언트 접속 수용
		if (FD_ISSET(listen_sock, &rset))
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
		// 소켓 셋 검사(2) : 데이터 통신
		for (int i = 0; i < nTotalSockets; i++)
		{
			SOCKETINFO *ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset))
			{
				// 데이터 받기
				retval = recv(ptr->sock, ptr->buf, BUFSIZE, 0);
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
					getpeername(ptr->sock, (struct sockaddr *)&clientaddr, &addrlen);
					// 받은 데이터 출력
					ptr->buf[ptr->recvbytes] = '\0';
					char addr[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
					printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), ptr->buf);
				}
			}
			if (FD_ISSET(ptr->sock, &wset))
			{
				// 데이터 보내기
				retval = send(ptr->sock, ptr->buf + ptr->sendbytes, ptr->recvbytes - ptr->sendbytes, 0);
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