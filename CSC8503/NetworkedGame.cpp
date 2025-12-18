#include "NetworkedGame.h"
#include "PhysicsSystem.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "GameWorld.h"
#include "Window.h"
#include "PlayerObject.h"

#define COLLISION_MSG 30

using namespace NCL;
using namespace CSC8503;

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame(GameWorld& gameWorld, GameTechRendererInterface& renderer, PhysicsSystem& physics) : TutorialGame(gameWorld, renderer, physics)
{
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		networkObjects.push_back(o);
	}
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() {
	//From my understanding, the server shoudln't render the game out.
	//The player should connect as a client even if they are hosting the server.
	//Extra clients should connect as normal.
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);

	thisServer->RegisterPacketHandler(Received_State, this);
	thisServer->RegisterPacketHandler(Player_Connected, this);
	thisServer->RegisterPacketHandler(Player_Disconnected, this);
	thisServer->RegisterPacketHandler(Ack_State, this);


	StartLevel();
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);

	StartLevel();
}

void NetworkedGame::UpdateGame(float dt) {
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		if (thisServer) {
			UpdateAsServer(dt);
		}
		else if (thisClient) {
			UpdateAsClient(dt);
		}
		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}

	if (!thisServer && Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		StartAsServer();
	}
	if (!thisClient && Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		StartAsClient(127,0,0,1);
	}

	// Apply local inputs each frame for client-side prediction (not tied to network tick)
	if (thisClient && world.GetMainPlayer()) {
		char localButtons[8];
		world.GetMainPlayer()->CreateButtonStates(localButtons);

		// record this input with dt and seq
		PendingInput in;
		memcpy(in.buttons, localButtons, 8);
		in.dt = dt;
		in.seq = nextInputSeq++;
		pendingInputs.push_back(in);

		// apply locally
		world.GetMainPlayer()->ApplyButtonStates(localButtons, dt);
	}

	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::ServerProcessReceived(float dt) {
	GamePacket* recievedPacket = nullptr;
	int source = -1;
	while (thisServer->UpdateServer(recievedPacket, source)) {
		this->ReceivePacketWithDT(recievedPacket->type, recievedPacket, source, dt);
		free(recievedPacket);
		recievedPacket = nullptr;
	}
}

void NetworkedGame::ServerSendObjects() {
	int slidingWindow = 5;
	for (auto const& [clientID, ackMap] : clientObjectAcks) {
		for (NetworkObject* o : networkObjects) {
			int objectID = o->GetNetworkID();
			int clientAck = 0;
			auto it = clientObjectAcks[clientID].find(objectID);
			if (it != clientObjectAcks[clientID].end()) clientAck = it->second;

			int objLatest = o->GetLatestStateID();
			bool canDelta = clientAck > 0 && (objLatest - clientAck) <= slidingWindow;

			GamePacket* packet = nullptr;
			if (o->WritePacket(&packet, canDelta, clientAck)) {
				thisServer->SendTargetedPacket(*packet, clientID);
				delete packet;
			}
		}
	}
}

void NetworkedGame::UpdateAsServer(float dt) {
	ServerProcessReceived(dt);
	ServerSendObjects();
	

	/*packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}*/
	//Need to recive packets from clients here too!
}

void NetworkedGame::UpdateAsClient(float dt) {
	
	GamePacket* recievedPacket = nullptr;
	int source = -1;
	while (thisClient->UpdateClient(recievedPacket, source)) {
		// use non-dt path for client processed packets
		this->ReceivePacket(recievedPacket->type, recievedPacket, source);
		free(recievedPacket);
		recievedPacket = nullptr;

	}
	// Send buffered inputs in one packet (up to ClientPacket::inputs capacity)
	if (!pendingInputs.empty()) {
		ClientPacket packet;
		packet.lastID = thisClient->GetLastStateID();
		packet.inputCount = 0;
		Quaternion orientation = world.GetMainPlayer()->GetTransform().GetOrientation();
		packet.orientation = orientation;

		int maxToSend = std::min((int)pendingInputs.size(), (int)std::size(packet.inputs));
		for (int i = 0; i < maxToSend; ++i) {
			packet.inputs[i].seq = pendingInputs[i].seq;
			packet.inputs[i].dt = pendingInputs[i].dt;
			memcpy(packet.inputs[i].buttonstates, pendingInputs[i].buttons, 8);
			packet.inputCount++;
		}

		thisClient->SendPacket(packet);

		// remove sent inputs from buffer
		pendingInputs.erase(pendingInputs.begin(), pendingInputs.begin() + maxToSend);
	}
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		//TODO - you'll need some way of determining
		//when a player has sent the server an acknowledgement
		//and store the lastID somewhere. A map between player
		//and an int could work, or it could be part of a 
		//NetworkPlayer struct. 
		int playerState = 0;
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world.GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

PlayerObject* NetworkedGame::SpawnPlayer() {
	PlayerObject* newPlayer = static_cast<PlayerObject*>(this->AddPlayerToWorld(Vector3(-118.747, 70.8767, 286.553)));
	NetworkObject* netObj = newPlayer->GetNetworkObject();
	if (netObj) {
		networkObjects.push_back(netObj);
	}
	return newPlayer;

}

void NetworkedGame::StartLevel() {
	this->useGravity = true;
	this->physics.UseGravity(true);
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	// forward to DT version with dt = 0
	ReceivePacketWithDT(type, payload, source, 0.0f);
}

void NetworkedGame::SendAck(int objectID, int stateID) {
	ackPacket ack;
	ack.objectID = objectID;
	ack.lastID = stateID;
	if (thisClient) {
		// send ACK reliably so server receives it (ENet reliable flag = 1)
		thisClient->SendPacket(ack, 1);
	}
}

void NetworkedGame::ReceivePacketWithDT(int type, GamePacket* payload, int source, float dt) {
	switch (type) {
	case Player_Connected: {
		// Server-side: create a player for this client, store mapping, init ack map, and send initial full snapshots
		if (thisServer) {
			PlayerObject* newPlayer = SpawnPlayer();
			// store the server-side mapping from client source to player object
			serverPlayers[source] = newPlayer;
			// initialize ack map for this client
			clientObjectAcks.emplace(source, std::unordered_map<int,int>());

			// send full snapshot of all network objects to the new client
			for (NetworkObject* o : networkObjects) {
				GamePacket* packet = nullptr;
				if (o->WritePacket(&packet, false, 0)) { // force full
					thisServer->SendTargetedPacket(*packet, source);
					delete packet;
				}
			}
			// initialize the client's last acknowledged state to 0
			stateIDs[source] = 0;
		}
		else if (thisClient) {
			// If we aren't established, set our id. If we are established, spawn a new player (someone else joined)
			if (payload != nullptr && payload->type == Player_Connected) {
				// interpret payload as PlayerPacket
				PlayerPacket* pp = (PlayerPacket*)payload;
				if (localClientID == -1) {
					localClientID = pp->playerID;
					this->world.SetMainPlayer( this->SpawnPlayer());
					this->world.GetMainCamera().SetPlayer(this->world.GetMainPlayer());
				}
				else {
					this->SpawnPlayer();
				}
				std::cout << "Client: Received assigned playerID = " << localClientID << std::endl;
			}
		}
		break;
	}

	case Player_Disconnected:
		if (thisServer) {
			// Server-side: cleanup player and maps
			auto it = serverPlayers.find(source);
			if (it != serverPlayers.end()) {
				GameObject* playerObj = it->second;
				// TODO: more cleanup if needed (remove from world etc.)
				serverPlayers.erase(it);
			}
			clientObjectAcks.erase(source);
			stateIDs.erase(source);
		}
		else if (thisClient) {
			// client disconnected from server - teardown local client
			delete thisClient;
			thisClient = nullptr;
		}
		break;
	case Received_State: {
		ClientPacket* p = (ClientPacket*)payload;
		stateIDs[source] = p->lastID;  // Updates the last received state ID from this client
		PlayerObject* player = nullptr;
		if (thisServer) {
			auto it = serverPlayers.find(source);
			if (it != serverPlayers.end()) {
				player = dynamic_cast<PlayerObject*>(it->second);
			} else {
				std::cout << "Server: Received input from unknown client " << source << std::endl;
			}
		} else if (thisClient) {
			// On a client, map the incoming source to the local player index if appropriate
			if (localClientID >= 0) {
				// try to find our local player by index 0 or stored mapping
				// fallback to main player
				player = world.GetMainPlayer();
			}
		}

		if (player) {
			// Apply any buffered inputs carried in the packet
			for (int i = 0; i < p->inputCount; ++i) {
				player->ApplyButtonStates(p->inputs[i].buttonstates, p->inputs[i].dt);
			}
			// Always update orientation from incoming packet
			player->GetTransform().SetOrientation(p->orientation);
		}
		UpdateMinimumState();
		break;
	}
	case Delta_State: {
		DeltaPacket* p = (DeltaPacket*)payload;
		for (NetworkObject* o : networkObjects) {
			if (o->GetNetworkID() == p->objectID) {
				o->ReadPacket(*p);
				break;
			}
		}
		this->SendAck(p->objectID, p->fullID);
		/*ClientPacket ackPacket;
		ackPacket.type = Received_State;
		ackPacket.lastID = thisClient->GetLastStateID();
		thisClient->SendPacket(ackPacket);
		break;*/
	}
	case Full_State: {
		FullPacket* p = (FullPacket*)payload;
		for (NetworkObject* o : networkObjects) {
			if (o->GetNetworkID() == p->objectID) {
				o->ReadPacket(*p);
				break;
			}
		}
		thisClient->SetLastStateID(p->fullState.stateID);
		this->SendAck(p->objectID, p->fullState.stateID);
		break;
	}

	case Ack_State:
	{
		if (!payload) break;
		ackPacket* a = (ackPacket*)payload;
		int objectID = a->objectID;
		int stateID  = a->lastID;

		if (thisServer) {
			// ensure map for this client exists
			auto &ackMap = clientObjectAcks[source];
			ackMap[objectID] = stateID;
			std::cout << "Server: Received ACK from client " << source << " for object " << objectID << " state " << stateID << std::endl;
		}
		// clients don't need to process object acks
		break;
	}
	}

	
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisServer->SendGlobalPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisServer->SendGlobalPacket(newPacket);
	}
}