void main()
{
	float x, y, res;
	res = 0.0;
	
	scanf("%f", &x);
	printf("\"nAgora, digite outro número(float): ");
	scanf("%f", &y);
	res = x * y;
	printf("\"nO resultado para a multiplicação é: ");
	printf("%f", res);
}