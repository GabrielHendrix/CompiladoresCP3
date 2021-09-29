/* OPERADOR LÓGICO OR ATRAVÉS DE UM LAÇO WHILE */

int main ()
{
	int i, y;
	i = 10;
	y = 2;
	while (i>=1 || y<20)
	{
		i = i - 1;
		printf("%d", i);
		printf("<- i");
		y = y + 2;
		printf("\"n");
		printf("y ->");
		printf("%d", y);
		printf("  \"n");
	}
}