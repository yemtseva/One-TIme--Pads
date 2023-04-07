#include <stdlib.h>
#include <stdio.h>
#include <time.h>

char* radomizeText(int textLength){
	char* randomText = malloc(textLength);
	int i;
	for(i = 0; i < textLength; i++){
		int randomChar = rand()%27 + 65;
		if(randomChar == 91)
			randomText[i] = ' ';
		else
			randomText[i] = randomChar;
	}
	return randomText;

}

int main(int argc, char* argv[]){
	srand(time(0));
	int textLength = atoi(argv[1]);
	char* randomText = radomizeText(textLength+1);
	fprintf(stdout, "%s", randomText); 

	return 0;
}