/*
 * Copyright Kai Zhao
 * Borrowed code from Benjamin Welton's MRNet Twitter username search tutorial
 * Borrowed code from Dorian C. Arnold, Philip C. Roth, and Barton P. Miller's
 *	MRNet IntegerAddition example
 *
 * This is the search filter:
 * 	does nothing useful
 */

/****************************************************************************
 * Copyright © 2003-2012 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include <vector>

#include "mrnet/Packet.h"
#include "mrnet/NetworkTopology.h"

using namespace MRN;

extern "C" {
	const char * Search_format_string = "";
	void Search( std::vector< PacketPtr >& packets_in,
			 std::vector< PacketPtr >& packets_out,
			 std::vector< PacketPtr >& /* packets_out_reverse */,
			 void ** /* client data */,
			 PacketPtr& /* params */,
			 const TopologyLocalInfo& )
	{
		PacketPtr new_packet ( new Packet(packets_in[0]->get_StreamId(),
					      packets_in[0]->get_Tag(), "" ) );
		packets_out.push_back( new_packet );
	}
} /* extern "C" */
