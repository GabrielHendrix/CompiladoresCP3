/* CALCULADORA COM OPERAÇÕES SIMPLES PARA NÚMEROS INTEIROS */

int main ()
{
	int n1, n2, res, option, control;
	control = 1;
	
	while(control == 1) 
	{
		scanf("%d", &n1);
		printf("\"n Agora, insira outro valor inteiro: ");
		scanf("%d", &n2);
		printf("\"n Selecione dentre as opções:");
		printf("\"n 0 - Soma | 1 - Subtração | 2 - Divisão | 3 - Multiplicação \"n");
		scanf("%d", &option);
		printf("\"n O Resultado de ");
		printf("%d", n1);
		if (option == 0) 
		{
			res = n1 + n2;
			printf(" + ");
		} 
		if (option == 1)
		{
			res = n1 - n2;
			printf(" - ");
		}
		if (option == 2)
		{
			res = n1 / n2;
			printf(" / ");
		}
		if (option == 3)
		{
			res = n1 * n2;
			printf(" * ");
		}
		printf("%d", n2);
		printf(" é: ");
		printf("%d", res);
		printf("\"n\"n Deseja realizar outra operação? (1 - sim | 0 - não) ");
		scanf("%d", &control);
		if (control == 1) 
		{
			printf("\"n Insira um valor inteiro: ");
		}
	}
}