#include<stdio.h>
#include<string.h>
#include <stdlib.h>

main() {
	//char str[13] = "1415927476669";
	//long long int ret;
	//ret = strtoll(str, NULL, 10);
	//printf("The number(unsigned long integer) is %lld\n", ret);
	

	FILE*  child = popen("java -jar PrintSampleStream.jar out.txt 100", "r");
	pclose(child);
	printf("Hello World");

	FILE * fd = fopen("out.txt", "r");
	int tweetTimeLength = 13; //"1415927476669".length();
	int screenNameLength = 15;
	int tweetTextLength = 140;
	int totalTweetLength = tweetTimeLength + screenNameLength + tweetTextLength + strlen("\n");
	char tweetTime[tweetTimeLength];
	char screenName[screenNameLength];
	char tweetText[tweetTextLength];
	for (int i = 0; i < 100; i++) {
		 fseek(fd, i * totalTweetLength, SEEK_SET);
		 fread(tweetTime, tweetTimeLength, 1, fd);
		 long long int longTweetTime = strtoll(tweetTime, NULL, 10);
		 fread(screenName, screenNameLength, 1, fd);
		 fread(tweetText, tweetTextLength, 1, fd);
		 printf("%s\n", tweetTime);
		 printf("%lld\n", longTweetTime);
		 printf("%s\n", screenName);
		 printf("%s\n", tweetText);
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