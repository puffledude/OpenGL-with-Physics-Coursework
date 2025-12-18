#include "GameServer.h"
#include "GameWorld.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)	{
	port		= onPort;
	clientMax	= maxClients;
	clientCount = 0;
	netHandle	= nullptr;
	Initialise();
}

GameServer::~GameServer()	{
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() {

	ENetAddress address;

	address.host = ENET_HOST_ANY;
	address.port = port;
	netHandle = enet_host_create(&address, clientMax, 1, 0, 0);

	if (!netHandle) {
		std::cout << __FUNCTION__ << ": An error occurred while trying to create an ENet server host on port " << port << "!\n";
		return false;
	}
	return true;
}

bool GameServer::SendGlobalPacket(int msgID) {
	GamePacket packet;
	packet.type = msgID;
	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) {
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;

}

bool GameServer::SendTargetedPacket(GamePacket& packet, int targetID) {
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	ENetPeer* targetPeer = nullptr;
	for (int i = 0; i < netHandle->peerCount; ++i) {
		ENetPeer* p = &netHandle->peers[i];
		if (p->incomingPeerID == targetID) {
			targetPeer = p;
			break;
		}
	}
	if (!targetPeer) {
		return false;
	}
	enet_peer_send(targetPeer, 0, dataPacket);
	return true;
}

void GameServer::UpdateServer() {
	if (!netHandle) {
		return;
	}
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0)
	{
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;

		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
			std::cout<<"Server: New client connected from "
				<< (p->address.host & 0xFF) << "."
				<< ((p->address.host >> 8) & 0xFF) << "."
				<< ((p->address.host >> 16) & 0xFF) << "."
				<< ((p->address.host >> 24) & 0xFF)
				<< ":" << p->address.port << std::endl;

			// Assign a new client id (use incomingPeerID as unique handle)
			++clientCount;
			PlayerPacket pkt;
			pkt.playerID = peer; // inform client of their peer id

			// send targeted packet to this peer
			ENetPacket* dataPacket = enet_packet_create(&pkt, pkt.GetTotalSize(), 0);
			enet_peer_send(p, 0, dataPacket);

			// notify any registered handlers for Player_Connected
			PacketHandlerIterator first, last;
			if (GetPacketHandlers(Player_Connected, first, last)) {
				for (auto it = first; it != last; ++it) {
					it->second->ReceivePacket(Player_Connected, &pkt, peer);
				}
			}
		}

		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
			std::cout << "Server: Client disconnected." << std::endl;

			// notify handlers for Player_Disconnected
			PacketHandlerIterator first, last;
			if (GetPacketHandlers(Player_Disconnected, first, last)) {
				for (auto it = first; it != last; ++it) {
					it->second->ReceivePacket(Player_Disconnected, nullptr, peer);
				}
			}
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet, peer);
		}
	}
}

bool GameServer::UpdateServer(GamePacket& receivedPacket, int& source) {
	if (!netHandle) {
		return false;
	}
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0)
	{
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;

		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT) {
			std::cout << "Server: New client connected from "
				<< (p->address.host & 0xFF) << "."
				<< ((p->address.host >> 8) & 0xFF) << "."
				<< ((p->address.host >> 16) & 0xFF) << "."
				<< ((p->address.host >> 24) & 0xFF)
				<< ":" << p->address.port << std::endl;

			++clientCount;
			PlayerPacket pkt;
			pkt.playerID = peer;

			// send targeted packet to this peer
			ENetPacket* dataPacket = enet_packet_create(&pkt, pkt.GetTotalSize(), 0);
			enet_peer_send(p, 0, dataPacket);

			// notify handlers
			PacketHandlerIterator first, last;
			if (GetPacketHandlers(Player_Connected, first, last)) {
				for (auto it = first; it != last; ++it) {
					it->second->ReceivePacket(Player_Connected, &pkt, peer);
				}
			}
		}

		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT) {
			std::cout << "Server: Client disconnected." << std::endl;
			PacketHandlerIterator first, last;
			if (GetPacketHandlers(Player_Disconnected, first, last)) {
				for (auto it = first; it != last; ++it) {
					it->second->ReceivePacket(Player_Disconnected, nullptr, peer);
				}
			}
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) {
			GamePacket* packet = (GamePacket*)event.packet->data;
			receivedPacket	= *packet;
			source = peer;
			return true;
		}
	}
	return false;
}

void GameServer::SetGameWorld(GameWorld &g) {
	gameWorld = &g;
}