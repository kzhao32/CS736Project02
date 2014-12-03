/*
 * Copyright Kai Zhao
 * Borrowed code from Benjamin Welton's MRNet Twitter username search tutorial
 * Borrowed code from Dorian C. Arnold, Philip C. Roth, and Barton P. Miller's
 *	MRNet IntegerAddition example
 *
 * This starts the front end of search:
 * 	Distributes keywords to all backends
 *	Times when all BEs are done with search
 */

#include <cstdlib>
#include <iostream>
#include <mrnet/MRNet.h>
#include <iterator>
#include <string>
#include <sstream>
#include <time.h>
#include "search_header.h"

using namespace MRN;

long int timediff(struct timespec start, struct timespec end)
{
	struct timespec diff;
	// take the difference
	if(end.tv_nsec < start.tv_nsec) { // if carry over
		diff.tv_sec = end.tv_sec-start.tv_sec-1;
		diff.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	}
	else {
		diff.tv_sec = end.tv_sec-start.tv_sec;
		diff.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return (1000000000*diff.tv_sec)+diff.tv_nsec;
}

bool saw_failure = false;
void Failure_Callback( Event* evt, void* )
{
    if( (evt->get_Class() == Event::TOPOLOGY_EVENT) &&
        (evt->get_Type() == TopologyEvent::TOPOL_REMOVE_NODE) )
        saw_failure = true;
}

int main(int argc, char **argv)
{
	// main variables
	long etime;
	struct timespec start;
	struct timespec stop;
	clock_gettime(CLOCK_MONOTONIC, &start);
	long searchCount = 0;
	int tag, retval;
	PacketPtr packet;

	if(argc < 4)
	{	// if no keywords are provided, then it will process all the tweets
		fprintf(stderr, "Usage: %s <topology file> <backend_exe> <so_file> [<keyword1> <keyword2> <keyword3> ...]\n", argv[0]);
		exit(-1);
	}

	// extract user input and initialize variables
	const char * topology_file = argv[1];
	const char * backend_exe = argv[2];
	const char * so_file = argv[3];
	int keywordsLength = 0;
	for(int i = 4; i < argc; i++) {
		keywordsLength += strlen(argv[i]) + 1;
	}
	// build string of keyword
	//	String keyword = keyword1 + " " + keyword2 + " " + ...;
	char * keywords = (char*) malloc(keywordsLength);
	for(int i = 4; i < argc; i++) {
		strcat(keywords, argv[i]);
		strcat(keywords, " ");
	}
	std::cout << "keywords to be sent are: " << keywords << std::endl;
	saw_failure = false;

	// create backend
	Network * net = Network::CreateNetworkFE( topology_file, backend_exe, NULL );
	if( net->has_Error() ) {
		net->perror("Network creation failed");
		exit(-1);
	}
	if( ! net->set_FailureRecovery(false) ) {
		fprintf( stdout, "Failed to disable failure recovery\n" );
		delete net;
		return -1;
	}
	bool cbrett = net->register_EventCallback( Event::TOPOLOGY_EVENT,
                                                   TopologyEvent::TOPOL_REMOVE_NODE,
                                                   Failure_Callback, NULL );
	if( !cbrett ) {
		fprintf( stdout, "Failed to register callback for node failure topology event\n" );
		delete net;
		return -1;
	}
	// load filter
	int filter_id = net->load_FilterFunc( so_file, "Search" );
	if( filter_id == -1 ){
		fprintf( stderr, "Network::load_FilterFunc() failure\n" );
		delete net;
		return -1;
	}
	Communicator * comm_BC = net->get_BroadcastCommunicator( );
	Stream * stream = net->new_Stream( comm_BC, filter_id, SFILTER_WAITFORALL );
	tag = PROT_STARTPROC;
	// send keywords to backend, split by spaces
	if( stream->send( tag, "%s", keywords) == -1 ){
		fprintf( stderr, "stream::send() failure\n" );
		return -1;
        }
	if( stream->flush( ) == -1 ){
		fprintf( stderr, "stream::flush() failure\n" );
		return -1;
	}

	// wait for response
	retval = stream->recv(&tag, packet);
	if( retval == 0 ) {
		//shouldn't be 0, either error or block for data, unless a failure occured
		fprintf( stderr, "stream::recv() returned zero\n" );
		if( saw_failure ) {
			return -1;
		}
	}
	if( retval == -1 ) {
		//recv error
		fprintf( stderr, "stream::recv() unexpected failure\n" );
		if( saw_failure ) {
			return -1;
		}
	}
	
	// extract searchCount from response
	if( packet->unpack( "%ld", &searchCount ) == -1 ){
		fprintf( stderr, "stream::unpack() failure\n" );
		return -1;
	}
	printf("searchCount = %ld\n", searchCount);
	
	// exit backend, clean up, and shutdown
	if( saw_failure ) {
            fprintf( stderr, "FE: a network process has failed, killing network\n" );
            delete net;
        }
	delete stream;
	Stream * ctl_stream = net->new_Stream( comm_BC, TFILTER_MAX, SFILTER_WAITFORALL );
	if(ctl_stream->send(PROT_EXIT, "") == -1){
		fprintf( stderr, "stream::send(exit) failure\n" );
		return -1;
	}
	if(ctl_stream->flush() == -1){
		fprintf( stderr, "stream::flush() failure\n" );
		return -1;
	}
	retval = ctl_stream->recv(&tag, packet);
	if( retval == -1){
		fprintf( stderr, "stream::recv() failure\n" );
		return -1;
	}
	delete ctl_stream;
	if( tag == PROT_EXIT ) {
		delete net;
	}
	clock_gettime(CLOCK_MONOTONIC, &stop);
	etime = timediff(start, stop);
	printf("etime = %ld nanoseconds\n", etime);
	return 0;
}
