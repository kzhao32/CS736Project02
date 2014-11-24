#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void main() {
	//char str[13] = "1415927476669";
	//long long int ret;
	//ret = strtoll(str, NULL, 10);
	//printf("The number(unsigned long integer) is %lld\n", ret);
	
	//string fileName = "output.txt";
	FILE*  child = popen("java -jar PrintSampleStream.jar output.txt 100", "r");
	pclose(child);
	printf("Hello World, done with collecting data\n");

	FILE * fd = fopen("output.txt", "r");
	int timeLength = 13 + 1; //"1415927476669".length() + strlen("\0");
	int screenNameLength = 15 + 1;
	int textLength = 140 + 1;
	int numberOfFollowersLength = 9 + 1;
	int totalTweetLength = timeLength + screenNameLength + textLength + numberOfFollowersLength + strlen("\n");
	char time[timeLength];
	char screenName[screenNameLength];
	char text[textLength];
	char numberOfFollowers[numberOfFollowersLength];
	char newLine[strlen("\n")];
	int i;
	for (i = 0; i < 3; i++) {
		 //fseek(fd, i * totalTweetLength, SEEK_SET);
		 fread(time, timeLength, 1, fd);
		 long long int longTweetTime = strtoll(time, NULL, 10);
		 fread(screenName, screenNameLength, 1, fd);
		 //fseek(fd, i * totalTweetLength + timeLength + screenNameLength + 1, SEEK_SET);
		 fread(text, textLength, 1, fd);
		 fread(numberOfFollowers, numberOfFollowersLength, 1, fd);
		 fread(newLine, strlen("\n"), 1, fd);
		 printf("%s\n", time);
		 printf("%lld\n", longTweetTime);
		 printf("%s\n", screenName);
		 printf("%s\n", text);
	}
	/*
	
	while (true) {

		if (!fread(tweetTime, tweetTimeLength, 1, fd)) {
			break;
		};
		printf("%s\n", tweetTime);
		if (!fread(screenName, screenNameLength, 1, fd)) {
			break;
		};
		printf("%s\n", screenName);
		if (!fread(tweetText, tweetTextLength, 1, fd)) {
			break;
		};
		printf("%s\n", tweetText);
		if (!fread(tweetTime, 1, 1, fd)) {
			break;
		};
		
		fread(tweetTime, tweetTimeLength, 1, fd);
		printf("%s\n", tweetTime);
		fread(screenName, screenNameLength, 1, fd);
		printf("%s\n", screenName);
		fread(tweetText, tweetTextLength, 1, fd);
		printf("%s\n", tweetText);
		if (feof(fd)) {
			break;
		}
	}
	//printf("%s\n", buf);
/*
FILE * fd = fopen("file.txt", "w");
int x = 5;
fwrite(&x, sizeof(int), 1, fd);
printf("%li\n", x);
    fclose(fd);
	
fd = fopen("file.txt", "r");
int y;
//fwrite(&x, sizeof(__int64), 1, fd);
fread(&y, sizeof(int), 1, fd);  
printf("%li\n", y);
 
        //fprintf(fd, "%li\n", 245749955);
 
    fclose(fd);
	
	FILE *fp;
   char str[] = "This is tutorialspoint.com";

   fp = fopen( "file.txt" , "w" );
   long x = 123
   fwrite(str , 1 , sizeof(str) , fp );
   fwrite(x , 1 , sizeof(long) , fp );

   fclose(fp);
  
   return(0);

	printf("Hello World");
	FILE *fp;
	fp = fopen("output2.txt", "rw");
	long x;
    fread(&x, sizeof(int), 2, fp); 
	printf("%d", x);
	*/
}