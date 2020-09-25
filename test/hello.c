#include <stdio.h>
extern int add_func(int a, int b);
int main()
{
	int a = add_func(3, 4);
	printf("ret = %d\n", a);
	printf("hello woorld!\n");
	return 0;
}

