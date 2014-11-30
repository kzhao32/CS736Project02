#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>		//for std::vector
#include <string>		//for std::string

#define timeLength (13 + 1) //"1415927476669".length() + strlen("\0");
#define screenNameLength (15 + 1)
#define textLength (140 + 1)
#define numberOfFollowersLength (9 + 1)
#define totalTweetLength (timeLength + screenNameLength + textLength + numberOfFollowersLength + strlen("\n")) //182

int main(int argc, char **argv) {
	if(argc != 2){
		fprintf(stderr, "Usage: %s <twitter_input>\n", argv[0]);
		exit(-1);
	}
	
	long long int followerCount = 0;
	FILE * fd = fopen(argv[1], "r");
	char screenName[screenNameLength];
	char text[textLength];
	char numberOfFollowers[numberOfFollowersLength];
	
	std::vector<std::string> keywords;
	keywords.push_back("for");
	keywords.push_back("to");
	// following line of code is for be
	//std::cout << "keywords length = " << keywords.size() << std::endl;
	//std::cout << "keywords[0] = " << keywords[0] << std::endl;
	int i;
	for (i = 0; ; ++i) {
		fseek(fd, i * totalTweetLength + timeLength, SEEK_SET);
		fread(screenName, screenNameLength, 1, fd);
		fseek(fd, i * totalTweetLength + timeLength + screenNameLength, SEEK_SET);
		fread(text, textLength, 1, fd);
		fseek(fd, i * totalTweetLength + timeLength + screenNameLength + textLength, SEEK_SET);
		fread(numberOfFollowers, numberOfFollowersLength, 1, fd);
		if (feof(fd)) {
			break;
		}
		std::string textString(text);
		for (int j = 0; j < keywords.size(); j++) {
			if (textString.find(keywords[j]) != std::string::npos) {
				printf("found at i = %d; screenName = %s; text = %s; followCount = %lld\n", i, screenName, text, strtoll(numberOfFollowers, NULL, 10));
				followerCount += strtoll(numberOfFollowers, NULL, 10);
				break;
			}
		}
		//printf("i = %d; screenName = %s; followCount = %lld\n", i, screenName, strtoll(numberOfFollowers, NULL, 10));
		//printf("followerCount = %lld\n", strtoll(numberOfFollowers, NULL, 10));
	}
	printf("followerCount = %lld\n", followerCount);
}