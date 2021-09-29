/* COMPARAÇÃO N1 < N2 */

int main ()
{
	int x, y;

	scanf("%d", &x);
	printf("\"n Digite outro valor inteiro para comparação: ");
	scanf("%d", &y);
	
	if (x < y) {
		printf("x = ");
		printf("%d", x); 
		printf(" é menor que y = ");
		printf("%d", y);
	} else {
		printf("x =  ");
		printf("%d", x); 
		printf(" é maior que y = ");
		printf("%d", y);	
	}
}
