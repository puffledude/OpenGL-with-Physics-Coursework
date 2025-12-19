#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"

// PlayerObject is declared in the global namespace, forward-declare it here
class PlayerObject;

namespace NCL::CSC8503 {
	class GameServer;
	class GameClient;
	class NetworkPlayer;
	class NetworkObject;

	class NetworkedGame : public TutorialGame, public PacketReceiver 
	{
	public:
		NetworkedGame(GameWorld& gameWorld, GameTechRendererInterface& renderer, PhysicsSystem& physics, bool isClient=false);
		~NetworkedGame();

		void StartAsServer();
		void StartAsClient(char a, char b, char c, char d);

		void UpdateGame(float dt) override;

		::PlayerObject* SpawnPlayer();

		void StartLevel();

		// PacketReceiver override
		void ReceivePacket(int type, GamePacket* payload, int source) override;

		void SendAck(int objectID, int stateID);

		void ServerProcessReceived(float dt);
		void ReceivePacketWithDT(int type, GamePacket* payload, int source, float dt);
		void ServerSendObjects();

		void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

	protected:
		void UpdateAsServer(float dt);
		void UpdateAsClient(float dt);

		void BroadcastSnapshot(bool deltaFrame);
		void UpdateMinimumState();
		std::map<int, int> stateIDs;

		std::unordered_map<int, std::unordered_map<int, int>> clientObjectAcks;

		GameServer* thisServer;
		GameClient* thisClient;
		float timeToNextPacket;
		int packetsToSnapshot;

		std::vector<NetworkObject*> networkObjects;

		std::map<int, GameObject*> serverPlayers;
		std::map <int, int> confirmedStates;
		PlayerObject* localPlayer;
		int localClientID = -1;
		bool localClientPending = false; // true when StartAsServer created a local client but server hasn't linked it yet

		// Input buffering for client-side prediction / reconciliation
		struct PendingInput {
			char buttons[8];
			float dt;
			int seq;
		};

		std::vector<PendingInput> pendingInputs; // unacked inputs
		std::vector<PendingInput> unackedInputs; // inputs sent to server but awaiting server snapshot for reconciliation
		int nextInputSeq = 0;
		bool isClient = false;
	};
}