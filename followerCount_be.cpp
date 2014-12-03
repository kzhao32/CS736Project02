/*
 * Copyright Kai Zhao
 * Borrowed code from Benjamin Welton's MRNet Twitter username search tutorial
 * Borrowed code from Dorian C. Arnold, Philip C. Roth, and Barton P. Miller's
 *	MRNet IntegerAddition example
 *
 * This starts the back end of follower count:
 * 	Reads /tmp/bonsai.dat
 *	Aggregate followerCount from each tweet
 */

#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <mrnet/MRNet.h>
#include "followerCount_header.h"

#define timeMillisecLength (13 + 1) //"1415927476669".length() + strlen("\0") == 13 + 1;
#define timeDateLength (28 + 1) // "Mon Dec 01 14:48:17 CST 2014".length() == 28
#define screenNameLength (15 + 1)
#define textLength (140 + 1)
#define numberOfFollowersLength (9 + 1)
#define totalTweetLength (timeMillisecLength + timeDateLength + screenNameLength + textLength + numberOfFollowersLength + strlen("\n")) //182

using namespace MRN;

int main(int argc, char **argv)
{
	// main variables
	Stream* stream = NULL;
	PacketPtr packet;
	int tag;
	int rc;

	Network* net = Network::CreateNetworkBE(argc, argv);

	// listen to FE
	do {
		rc = net->recv(&tag, packet, &stream);
		if( rc == -1 ) {
		    fprintf( stderr, "BE: Network::recv() failure\n" );
		    break;
		}
		else if( rc == 0 ) {
		    // a stream was closed
		    continue;
		}
		switch(tag) { // switch to execute or exit
			case PROT_STARTPROC: {
				long followerCount = 0;
				// unpack keywords
				char * keyword;
				packet->unpack("%s", &keyword);
				std::cout << "keywords unpacked are: " << keyword << std::endl;
				char * keywordTokens = strtok(keyword, " ");
				std::vector<std::string> keywords;
				while(keywordTokens) {
					keywords.push_back(keywordTokens);
					keywordTokens = strtok(NULL, " ");
				}
				// open data file
				FILE * fd = fopen("/tmp/bonsai.dat", "r");
				char text[textLength];
				char numberOfFollowers[numberOfFollowersLength];
				if (fd) {	// while file is good
					for (int i = 0; ; ++i) {
						// read tweet and read follower_count
						fseek(fd, i * totalTweetLength + timeMillisecLength + timeDateLength + screenNameLength, SEEK_SET);
						fread(text, textLength, 1, fd);
						fseek(fd, i * totalTweetLength + timeMillisecLength + timeDateLength + screenNameLength + textLength, SEEK_SET);
						fread(numberOfFollowers, numberOfFollowersLength, 1, fd);
						if (feof(fd)) {
							break;
						}
						std::string textString(text);
						// if no keywords, then sum all follower_count
						if (keywords.size() == 0) {
							followerCount += strtol(numberOfFollowers, NULL, 10);
						}
						// else only sum follower_count that contains tweet
						for (unsigned int j = 0; j < keywords.size(); ++j) {
							if (textString.find(keywords[j]) != std::string::npos) {
								followerCount += strtol(numberOfFollowers, NULL, 10);
								break;
							}
						}
						//fprintf(stderr, "Number of l followers found: %d\n", followerCount);
					}
					fclose(fd);
				} else {
					std::cout << "error opening file: /tmp/bonsai.dat" << std::endl;
				}
				// send aggregate number of follower_count to FE
				fprintf(stderr, "Number of followers found: %ld\n", followerCount);
				if( stream->send(tag, "%ld", followerCount) == -1 ) {
					fprintf( stderr, "BE: stream::send(%%ld) failure in PROT_STARTPROC\n" );
					tag = PROT_EXIT;
					break;
				}
				if( stream->flush() == -1 ) {
					fprintf( stderr, "BE: stream::flush() failure in PROT_STARTPROC\n" );
					break;
				}
				}
				break;
			case PROT_EXIT:
				// send reply and break loop if exit
				if( stream->send(tag, "%d", 0) == -1 ) {
					fprintf( stderr, "BE: stream::send(%%s) failure in PROT_EXIT\n" );
				}
				if( stream->flush() == -1 ) {
					fprintf( stderr, "BE: stream::flush() failure in PROT_EXIT\n" );
				}
				break;
			default:
				fprintf( stderr, "BE: Unknown Protocol: %d\n", tag );
				tag = PROT_EXIT;
				break;
		}
		fflush(stderr);
	} while (tag != PROT_EXIT);

	// clean up and exit backend
	if (stream != NULL) {
		while(!stream->is_Closed()) {
			sleep(1);
		}
		delete stream;
	}
	net->waitfor_ShutDown();
	delete net;
	return 0;
}
