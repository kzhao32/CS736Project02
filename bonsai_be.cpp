#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <mrnet/MRNet.h>

using namespace MRN;

int main(int argc, char **argv)
{
	Network* net;
	Stream* stream;
	PacketPtr packet;
	int pid;
	int tag;

	for(int i = 0; i < argc; i++)
	{
		std::cout << "ARGV[" << i << "]: " << argv[i] << std::endl;
	}

	net = Network::CreateNetworkBE(argc, argv);
	pid = fork();
	
	if(pid == 0)
	{
		// TODO: Configure filler using information from MRNet
		execlp("filler", "filler" , NULL);
	}

	// TODO: Wait for and process data

	net->recv(&tag, packet, &stream);

	// TODO: Find a better way to terminate filler
	kill(pid, SIGTERM);

	if(stream)
	{
		while(!stream->is_Closed())
		{
			sleep(1);
		}

		delete stream;
	}

	net->waitfor_ShutDown();

	delete net;

	return 0;
}
