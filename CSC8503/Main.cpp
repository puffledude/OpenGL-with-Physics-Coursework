#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"
#include "TestPacketReceiver.h"
#include "ParallelBehaviour.h"

#include "PushdownMachine.h"
#include "introScreen.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"
#include "Inverter.h"

#include "PhysicsSystem.h"

#ifdef USEOPENGL
#include "GameTechRenderer.h"
#define CAN_COMPILE
#endif
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#define CAN_COMPILE
#endif

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>


std::vector<Vector3> testNodes;
void TestPathfinding() {

	
	NavigationGrid grid("TestGrid1.txt");

	NavigationPath outPath;

	Vector3 startPos(80, 0, 10);
	Vector3 endPos(80, 0, 80);

	bool found = grid.FindPath(startPos, endPos, outPath);
	Vector3 pos;
	while (outPath.PopWaypoint(pos))
	{
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}


void TestStateMachine() {

	StateMachine* testMachine = new StateMachine();
	int data = 0;


	State* A = new State([&](float dt)->void {
		std::cout << "I'm in state A " << std::endl;
		data++;
		});
	State* B = new State([&](float dt)->void {
		std::cout << "I'm in state B " << std::endl;
		data--;
		});


	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool {
		return data > 10;
		});
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool {
		return data < 0;
		});

	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);

	for (int i = 0; i < 100; ++i) {
		testMachine->Update(1.0f);
	}

}

void TestBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;
	BehaviourAction* findKey = new BehaviourAction("Find Key", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Finding Key...\n";
			behaviourTimer = rand() & 100;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			behaviourTimer -= dt;
			if (behaviourTimer <= 0) {
				std::cout << "Found Key!\n";
				return Success;
			}
		}
		return state;
		});

	BehaviourAction* goToRoom = new BehaviourAction("Go to Room", [&](float dt, BehaviourState state)->BehaviourState {

		if (state == Initialise) {
			std::cout << "Going to Room...\n";
			state = Ongoing;
		}
		else if (state == Ongoing) {
			distanceToTarget -= dt * 10;
			if (distanceToTarget <= 0) {
				std::cout << "Reached Room!\n";
				return Success;
			}
		}
		return state;
		});

	BehaviourAction* openDoor = new BehaviourAction("Open Door", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Opening Door...\n";
			return Success;
		}
		return state;
		});

	BehaviourAction* lookForTreasure = new BehaviourAction("Look for Treasure", 
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for Treasure...\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "Found Treasure!\n";
					return Success;
				}
				std::cout << "Couldn't find Treasure.\n";
				return Failure;

			}
		return state;
		});

	BehaviourAction* lookForItems = new BehaviourAction("Look for Items",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for Items...\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "Found Items!\n";
					return Success;
				}
				std::cout << "Couldn't find Items.\n";
				return Failure;
			}
			return state;
		});


	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");

	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	/*BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);*/

	ParallelBehaviour* selection = new ParallelBehaviour("Loot Parallel");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	Inverter* inverter = new Inverter("Inverter on Items");
	inverter->SetChild(selection);

	BehaviourSequence* root = new BehaviourSequence("Root Sequence");
	root->AddChild(sequence);
	root->AddChild(inverter);


	for (int i = 0; i < 5; ++i) {
		std::cout << "---- Behaviour Tree Iteration " << i << " ----\n";
		root->Reset();
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		while (state == Ongoing) {
			state = root->Execute(1.0f);
		}
		if (state == Success) {
			std::cout << "Behaviour Tree Succeeded!\n";
		}
		else {
			std::cout << "Behaviour Tree Failed!\n";
		}
	}
	std::cout << "All done" << std::endl;
}

void TestPushDownAutomata(Window* w) {
	PushdownMachine machine(new IntroScreen(w));

	while (w->UpdateWindow()) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (!machine.Update(dt)) {
			return;
		}
	}

}


void TestNetworking() {
	NetworkBase::Initialise();
	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");

	int port = NetworkBase::GetDefaultPort();
	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	for (int i = 0; i < 100; ++i) {
		StringPacket serverPacket = StringPacket("Hello from server! Packet #" + std::to_string(i));
		server->SendGlobalPacket(serverPacket);

		StringPacket clientPacket = StringPacket("Hello from client! Packet #" + std::to_string(i));
		client->SendPacket(clientPacket);
		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	}
	NetworkBase::Destroy();
}

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	WindowInitialisation initInfo;
	initInfo.width		= 1280;
	initInfo.height		= 720;
	initInfo.windowTitle = "CSC8503 Game technology!";

	Window*w = Window::CreateGameWindow(initInfo);

	if (!w->HasInitialised()) {
		return -1;
	}	

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	GameWorld* world = new GameWorld();
	PhysicsSystem* physics = new PhysicsSystem(*world);

#ifdef USEVULKAN
	GameTechVulkanRenderer* renderer = new GameTechVulkanRenderer(*world);
#elif USEOPENGL
	GameTechRenderer* renderer = new GameTechRenderer(*world);
#endif

	TutorialGame* g = new TutorialGame(*world, *renderer, *physics);
	TestBehaviourTree();
	TestPathfinding();
	//TestPushDownAutomata(w);
	TestNetworking();
	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
		DisplayPathfinding();
		world->UpdateWorld(dt);
		physics->Update(dt);
		renderer->Update(dt);	
		renderer->Render();
		
		
		Debug::UpdateRenderables(dt);
	}
	Window::DestroyGameWindow();
}