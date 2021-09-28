void main()
{
	int x, y, res;
	res = 0;
	
	scanf("%d", &x);
	printf("\"nAgora, digite outro número: ");
	scanf("%d", &y);
	res = x * y;
	printf("\"nO resultado para a multiplicação é: ");
	printf("%d", res);
}