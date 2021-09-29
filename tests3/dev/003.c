/* SUBTRAÇÃO (FLOAT) */

void main()
{
	float x, y, res;
	res = 0.0;
	
	scanf("%f", &x);
	printf("\"nAgora, digite o subtraendo(float): ");
	scanf("%f", &y);
	res = x - y;
	printf("\"nO resultado para a subtração é: ");
	printf("%f", res);
}