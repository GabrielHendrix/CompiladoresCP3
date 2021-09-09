#include  <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[])
{
	char var[] = "asas safsaf %dsahgsagfdhgf";
	char string[strlen(var)];
	char *s;
	memcpy(string, var, strlen(var));
	if(strstr(var, "%d") != NULL)
	{
		s =  strtok(var, "%");

		printf("%s\n", s);
		s = strtok(NULL, "%");
		char aux[strlen(s) - 1];
		for (int c = 1; s[c] != '\0'; c++)
			aux[c-1] = s[c];
		printf("%s", aux);
	}

	
	// printf("%s\n",  strstr(string, "%d"));
	// if(strstr(var, "%d") != NULL)
	// {
	// 	return printf("%s", "YES\n");
	// }
	// return printf("%s", "NO\n");


	// while(string != NULL)
	// {
	// 	printf("%s\n", string);
		
	// 	if((string = strtok(NULL, "%d")) != NULL)
	// 		cont++;
	// }
	// printf("%d", cont);


	// string = strtok(var, "%d");


	// while (string != NULL)
	// {
	// 	printf("%s", string);
	// 	string = strtok(NULL, "%d");
	// }
	// while (1)
	// {
	// 	// read (2, var, 13);
	// 	// write (1, var, 13);
	// 	//memset(var, '\0', strlen(var));
	// }
}
