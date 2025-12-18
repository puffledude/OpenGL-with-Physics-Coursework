#include "NetworkedGame.h"
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

	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::ServerProcessReceived(float dt) {
	GamePacket recievedPacket;
	int source = -1;
	while (thisServer->UpdateServer(recievedPacket, source)) {
		this->ReceivePacketWithDT(recievedPacket.type,&recievedPacket, source, dt);
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
	
	GamePacket recievedPacket;
	int source = -1;
	while (thisClient->UpdateClient(recievedPacket, source)) {
		// use non-dt path for client processed packets
		this->ReceivePacket(recievedPacket.type,&recievedPacket, source);
	}
	ClientPacket newPacket;
	newPacket.type = Received_State;
	world.GetPlayer()->CreateButtonStates(newPacket.buttonstates);
	Quaternion orientation = world.GetPlayer()->GetTransform().GetOrientation();
	newPacket.orientation = orientation;

	newPacket.lastID = thisClient->GetLastStateID();

	thisClient->SendPacket(newPacket);
	

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
	useGravity = true;
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	// forward to DT version with dt = 0
	ReceivePacketWithDT(type, payload, source, 0.0f);
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
			// client-side behaviour: create a local player object to represent self
			if (payload != nullptr && payload->type == Player_Connected) {
				// interpret payload as PlayerPacket
				PlayerPacket* pp = (PlayerPacket*)payload;
				localClientID = pp->playerID;
				std::cout << "Client: Received assigned playerID = " << localClientID << std::endl;
			}
			this->SpawnPlayer();
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
		if (p->buttonstates != nullptr) {
			PlayerObject* player = world.GetPlayerFromArray(source);
			player->ApplyButtonStates(p->buttonstates, dt);
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
		ClientPacket ackPacket;
		ackPacket.type = Received_State;
		ackPacket.lastID = thisClient->GetLastStateID();
		thisClient->SendPacket(ackPacket);
		break;
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