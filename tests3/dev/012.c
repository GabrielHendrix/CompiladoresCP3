/* OPERADOR LÓGICO AND E LAÇO WHILE */

int main ()
{
	int i, y;
	i = 1;
	y = 2;
	while (i==1 && 2==y)
	{
		i = 4;
		printf(" %d", i);
		i = 8;
		printf("\"n ");
		printf("%d", i);
		i = i + 8;	
	}
	printf("\"n");
	printf("%d ", i);
}