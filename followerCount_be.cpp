#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <mrnet/MRNet.h>
#include "followerCount_header.h"

#define timeLength (13 + 1) //"1415927476669".length() + strlen("\0");
#define screenNameLength (15 + 1)
#define textLength (140 + 1)
#define numberOfFollowersLength (9 + 1)
#define totalTweetLength (timeLength + screenNameLength + textLength + numberOfFollowersLength + strlen("\n")) //182

using namespace MRN;

int main(int argc, char **argv)
{
	Stream* stream = NULL;
	PacketPtr packet;
	int pid;
	int tag;
	int rc;

	for(int i = 0; i < argc; i++)
	{
		std::cout << "ARGV[" << i << "]: " << argv[i] << std::endl;
	}

	Network* net = Network::CreateNetworkBE(argc, argv);
	/*
	pid = fork();
	
	if(pid == 0)
	{
		// TODO: Configure filler using information from MRNet
		execlp("filler", "filler" , NULL);
	}
	*/

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
		switch(tag) {
			case PROT_STARTPROC: {
				int followerCount = 0;
				FILE * fd = fopen("/tmp/bonsai.dat", "r");
				char screenName[screenNameLength];
				char numberOfFollowers[numberOfFollowersLength];
				for (int i = 0; ; ++i) {
					fseek(fd, i * totalTweetLength + timeLength, SEEK_SET);
					fread(screenName, screenNameLength, 1, fd);
					fseek(fd, i * totalTweetLength + timeLength + screenNameLength + textLength, SEEK_SET);
					fread(numberOfFollowers, numberOfFollowersLength, 1, fd);
					if (feof(fd)) {
						break;
					}
					followerCount += strtoll(numberOfFollowers, NULL, 10);
				}
				fclose(fd);
				fprintf(stderr, "Number of users found: %d\n", followerCount);
				if( stream->send(tag, "%d", followerCount) == -1 ) {
					fprintf( stderr, "BE: stream::send(%%d) failure in PROT_SUM\n" );
					tag = PROT_EXIT;
					break;
				}
				if( stream->flush() == -1 ) {
					fprintf( stderr, "BE: stream::flush() failure in PROT_SUM\n" );
					break;
				}
				}
				break;
			case PROT_EXIT:
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

	// TODO: Find a better way to terminate filler
	//kill(pid, SIGTERM);

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
