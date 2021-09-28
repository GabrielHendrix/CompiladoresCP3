void main()
{
	int x, y, res;
	res = 0;
	
	scanf("%d", &x);
	printf("\"nAgora, digite outro número inteiro: ");
	scanf("%d", &y);
	res = x % y;
	printf("\"nO resto da divisão é: ");
	printf("%d", res);
}