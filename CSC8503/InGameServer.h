#pragma once
#include "GamePause.h"
#include "pushdownState.h"
#include "TutorialGame.h"
#include "GameTechRenderer.h"
#include "PhysicsSystem.h"

class InGameServer : public NCL::CSC8503::PushdownState
{
public:
	InGameServer(NCL::CSC8503::GameWorld* world, NCL::CSC8503::PhysicsSystem* physics, NCL::CSC8503::GameTechRenderer* renderer,
		NCL::CSC8503::NetworkedGame* networkedgame, float width, float height) :
		gameWorld(world), physics(physics), renderer(renderer), networkedGame(networkedgame), width(width), height(height) {
	}
	~InGameServer() {
		//networkedGame->
	}

	PushdownResult OnUpdate(float dt, PushdownState** pushFunc) override {
		if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::P)) {
			*pushFunc = new GamePause();
			return PushdownResult::Push;
		}
		networkedGame->UpdateGame(dt);
		gameWorld->UpdateWorld(dt);
		physics->Update(dt);
		
		if (networkedGame->IsGameFinished()) {
			*pushFunc = new FinishedGameState();
			return PushdownResult::Push;
		}
		return PushdownResult::NoChange;
	}


protected:
	float width;
	float height;
	NCL::CSC8503::GameWorld* gameWorld;
	NCL::CSC8503::PhysicsSystem* physics;
	NCL::CSC8503::GameTechRenderer* renderer;
	NCL::CSC8503::NetworkedGame* networkedGame;
};