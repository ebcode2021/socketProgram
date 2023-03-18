#include "../Common.h"

#define TESTNAME "www.google.com"

// 도메인 이름 -> IPv4 주소
bool GetIPAddr(const char *name, struct in_addr *addr)
{
	struct hostent *ptr = gethostbyname(name);
	if (ptr == NULL) // 오류가 발생하면 오류를 출력하고 false 리턴
	{
		err_display("gethostbyname()");
		return false;
	}
	if (ptr->h_addrtype != AF_INET)// AF_INET 주소 체계가 아니면 false 리턴
		return false;
	memcpy(addr, ptr->h_addr, ptr->h_length); // IP 주소를 in_addr 구조체에 복사하고 true 리턴
	return true;
}

// IPV4 주소 -> 도메인 이름
bool GetDomainName(struct in_addr addr, char *name, int namelen)
{
	struct hostent *ptr = gethostbyaddr((const char *)&addr, sizeof(addr), AF_INET);
	if (ptr == NULL)
	{
		err_display("gethostbyaddr()");
		return false;
	}
	if (ptr->h_addrtype != AF_INET)
		return false;
	// 도메인 이름을 사용자가 전달할 버퍼에 복사하고 true 리턴
	// 문자열 복사 시 strcpy() 함수보다는 strncpy() 함수를 사용하는 것이 좀 더 안전
	strncpy(name, ptr->h_name, namelen); 
	return true;
}

int	main(int argc, char *argv[])
{
	printf("도메인 이름(변환 전) : %s\n", TESTNAME);

	// 도메인 이름 -> IP 주소
	struct in_addr addr;
	if (GetIPAddr(TESTNAME, &addr))
	{
		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &addr, str, sizeof(str));
		printf("IP 주소(변환 후) : %s\n", str);

		// IP 주소 -> 도메인 이름
		char name[256];
		if (GetDomainName(addr, name, sizeof(name)))
			printf("도메인 이름(다시 변환 후) : %s\n", name);
	}
	return 0;
}