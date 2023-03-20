#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// 포인터보다 큰 값을 전달할 Point3D 구조체
struct Point3D
{
	int x, y, z;
};

// 스레드 함수 리턴은 스레드를 종료하는 방법 중 하나
void	*MyThread(void *arg)
{
	sleep (1);
	Point3D *pt = (Point3D *)arg;
	printf("Running MyThread() %lu : %d, %d, %d\n", pthread_self(), pt->x, pt->y, pt->z);
	return NULL;
}

int	main(int argc, char *argv[])
{
	int	retval;

	// 첫 번째 스레드 생성
	pthread_t tid1;
	Point3D pt1 = {10, 20, 30};
	retval = pthread_create(&tid1, NULL, MyThread, &pt1);
	if (retval != 0)
		return 1;
	
	// 두 번째 스레드 생성
	pthread_t tid2;
	Point3D pt2 = {40, 50, 60};
	retval = pthread_create(&tid2, NULL, MyThread, &pt2);
	if (retval != 0)
		return 1;
	
	printf("Running main() %lu\n", pthread_self());
	sleep (2);
	return 0;
}