#include "NetworkBase.h"
#include "./enet/enet.h"
NetworkBase::NetworkBase() {
	netHandle = nullptr;
}

NetworkBase::~NetworkBase() {
	if (netHandle) {
		enet_host_destroy(netHandle);
	}
}

void NetworkBase::Initialise() {
	enet_initialize();
}

void NetworkBase::Destroy() {
	enet_deinitialize();
}

bool NetworkBase::ProcessPacket(GamePacket* packet, int peerID) {
	// Dispatch incoming packet to any registered handlers for its type
	if (!packet) return false;

	PacketHandlerIterator first, last;
	if (!GetPacketHandlers(packet->type, first, last)) {
		return false; // no handlers registered for this packet type
	}

	for (auto it = first; it != last; ++it) {
		PacketReceiver* receiver = it->second;
		if (receiver) {
			receiver->ReceivePacket(packet->type, packet, peerID);
		}
	}

	return true;
}