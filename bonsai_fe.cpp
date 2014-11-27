#include <cstdlib>
#include <iostream>
#include <mrnet/MRNet.h>

using namespace MRN;

int main(int argc, char **argv)
{
	Network* net;
	Communicator* comm;
	Stream* stream;
	PacketPtr packet;
	int tag;

	if(argc != 3)
	{
		std::cerr	<< "Usage: "
				<< argv[0]
				<< " <topology file> <backend_exe>"
				<< std::endl;

		return -1;
	}

	tag = FirstApplicationTag;
	net = Network::CreateNetworkFE(argv[1], argv[2], NULL);
	comm = net->get_BroadcastCommunicator();
	stream = net->new_Stream(comm);

	// TODO: Send data to backends on command

	delete stream;
	delete net;

	return 0;
}
