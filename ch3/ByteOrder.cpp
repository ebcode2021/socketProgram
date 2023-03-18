#include "../Common.h"

int main(int argc, char *argv[])
{
	u_short	x1 = 0x1234;
	u_long	y1 = 0x12345678;
	u_short	x2;
	u_long	y2;

	// host byte -> network byte
	printf("[host byte -> network byte]\n");
	printf("%#x -> %#x\n", x1, x2 = htons(x1));
	printf("%#x -> %#x\n", y1, y2 = htonl(y1));

	// network byte -> host byte
	printf("[network byte -> host byte]\n");
	printf("%#x -> %#x\n", x2 , ntohs(x2));
	printf("%#x -> %#x\n", y2 ,  ntohl(y2));

	// wrong use
	printf("[wrong example]\n");
	printf("%#x -> %#x\n", x1, htonl(x1));

	return 0;
}