#include "GameClient.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
}

GameClient::~GameClient()	{
	enet_host_destroy(netHandle);
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
	ENetAddress address;
	address.port = portNum;
	address.host = (d << 24) | (c << 16) | (b << 8) | a;
	netPeer = enet_host_connect(netHandle, &address, 2, 0);
	return netPeer != nullptr;

}

void GameClient::UpdateClient() {

	if (netHandle == nullptr) {
		return;
	}
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			std::cout << "Client connected to server!" << std::endl;
		}

		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			std::cout << "Packet Recieved from server." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet);
		}
		else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
			std::cout << "Client Disconnected from server." << std::endl;
		}
		enet_packet_destroy(event.packet);

	}

}

bool GameClient::UpdateClient(GamePacket& recivedPacket, int& source) {
	if (netHandle == nullptr) {
		return false;
	}
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			std::cout << "Client connected to server!" << std::endl;
		}

		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			std::cout << "Packet Recieved from server." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			recivedPacket = *packet;
			// use incomingPeerID to match server's peer numbering
			source = event.peer->incomingPeerID;
			std::cout << "Client: packet from server with peer incomingPeerID=" << source << std::endl;
			return true;
		}
		else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
			std::cout << "Client Disconnected from server." << std::endl;
		}
		enet_packet_destroy(event.packet);

	}
	return false;
}

void GameClient::SendPacket(GamePacket&  payload, int flags) {

	ENetPacket* packet = enet_packet_create(&payload, payload.GetTotalSize(), flags);
	enet_peer_send(netPeer, 0, packet);

}
