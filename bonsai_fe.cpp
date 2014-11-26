#include <cstdlib>
#include <iostream>
#include <mrnet/MRNet.h>
#include <iterator>
#include <string>
#include <sstream>

using namespace MRN;

int main(int argc, char **argv)
{
	Network* net;
	Communicator* comm;
	Stream* stream;
	PacketPtr packet;
	int tag;
	NetworkTopology* topo;
	std::set<NetworkTopology::Node*> nodes;
	NetworkTopology::Node* node;
	std::string listOfHostNames;
	std::stringstream listOfHostNamesStream;
	
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s <topology file> <backend_exe>\n", argv[0]);
		exit(-1);
	}

	tag = FirstApplicationTag;
	net = Network::CreateNetworkFE(argv[1], argv[2], NULL);
	comm = net->get_BroadcastCommunicator();
	stream = net->new_Stream(comm);
	topo = net->get_NetworkTopology();
	topo->get_BackEndNodes(nodes);
	
	std::set<NetworkTopology::Node*>::iterator nodesIterator = nodes.begin();
	//node = *nodesIterator;
	while (nodesIterator != nodes.end()) {
		node = *nodesIterator;
		listOfHostNamesStream << node->get_HostName() + ":";
		listOfHostNamesStream << node->get_Port();
		listOfHostNamesStream << " ";
		//listOfHostNames += node->get_HostName() + ":" + ("%s", node->get_Port()) + " ";
		std::advance(nodesIterator, 1);
	}
	printf("%s", listOfHostNamesStream.str().c_str());
	stream->send(tag, "%s", listOfHostNamesStream.str().c_str());
	/*
	//std::advance(nodesIterator, 0);
	NetworkTopology::Node* node = *nodesIterator;
	
	printf("%s", node->get_HostName().c_str());
	*/
	

	
	// TODO: Send data to backends on command

	delete stream;
	printf("e\n");
	delete net;

	printf("f\n");
	return 0;
}
