
int func ();	/*	Declarar uma funcao nao quebra o analisador, mas nao
					eh processado como deveria. */

int main ()
{
	char s[4];
	int a, b;
	float c;
	s[1], a, b, a*b+1.0*2%b;
	c = a;
	s[2], write (1, *s, 4);
}

char func2 ()	/*	Definir uma funcao nao quebra o analisador, mas novamente
					nao eh processado como deveria. */
{
	int func2int = 0;
	char func2str;
	write (1, "Opa\n", 4);
}
