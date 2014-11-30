#include <cstdlib>
#include <iostream>
#include <mrnet/MRNet.h>
#include <iterator>
#include <string>
#include <sstream>
#include "followerCount_header.h"

using namespace MRN;

bool saw_failure = false;
void Failure_Callback( Event* evt, void* )
{
    if( (evt->get_Class() == Event::TOPOLOGY_EVENT) &&
        (evt->get_Type() == TopologyEvent::TOPOL_REMOVE_NODE) )
        saw_failure = true;
}

int main(int argc, char **argv)
{
	int followerCount = 0;
	int tag, retval;
	PacketPtr packet;
	
	if(argc < 4)
	{
		fprintf(stderr, "Usage: %s <topology file> <backend_exe> <so_file> [<keyword1> <keyword2> <keyword3> ...]\n", argv[0]);
		exit(-1);
	}
	
	const char * topology_file = argv[1];
	const char * backend_exe = argv[2];
	const char * so_file = argv[3];
	int keywordsLength = 0;
	for(int i = 4; i < argc; i++) {
		std::cout << "argv[i].length is " << strlen(argv[i]) << std::endl;
		keywordsLength += strlen(argv[i]) + 1;
	}
	char * keywords = (char*) malloc(keywordsLength); 
	for(int i = 4; i < argc; i++) {
		strcat(keywords, argv[i]);
		strcat(keywords, " ");
	}
	std::cout << "keywords to be sent are " << keywords << std::endl;
	saw_failure = false;
	
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
	int filter_id = net->load_FilterFunc( so_file, "FollowersAdd" );
	if( filter_id == -1 ){
		fprintf( stderr, "Network::load_FilterFunc() failure\n" );
		delete net;
		return -1;
	}
	Communicator * comm_BC = net->get_BroadcastCommunicator( );
	Stream * stream = net->new_Stream( comm_BC, filter_id, SFILTER_WAITFORALL );
	tag = PROT_STARTPROC;
	if( stream->send( tag, "%s", keywords) == -1 ){
		fprintf( stderr, "stream::send() failure\n" );
		return -1;
        }
	if( stream->flush( ) == -1 ){
		fprintf( stderr, "stream::flush() failure\n" );
		return -1;
	}
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
	if( packet->unpack( "%d", &followerCount ) == -1 ){
		fprintf( stderr, "stream::unpack() failure\n" );
		return -1;
	}
	printf("followerCount = %d\n", followerCount);
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
		//recv error
		fprintf( stderr, "stream::recv() failure\n" );
		return -1;
	}
	delete ctl_stream;
	if( tag == PROT_EXIT ) {
		delete net;
	}
	return 0;
}
