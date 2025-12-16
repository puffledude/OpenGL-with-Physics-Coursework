#pragma once
#include "GamePause.h"
#include "pushdownState.h"
#include "TutorialGame.h"
#include "GameTechRenderer.h"
#include "PhysicsSystem.h"



//class GameWorld;
//class PhysicsSystem;
//class GameTechRenderer;

class InGame : public NCL::CSC8503::PushdownState 
{
	public:
	InGame(NCL::CSC8503::GameWorld* world, NCL::CSC8503::PhysicsSystem* physics, NCL::CSC8503::GameTechRenderer* renderer,
		NCL::CSC8503::TutorialGame* tutorialgame, float width, float height) :
		gameWorld(world), physics(physics), renderer(renderer), tutorialGame(tutorialgame), width(width), height(height) {
	}
	PushdownResult OnUpdate(float dt, PushdownState** pushFunc) override {
		if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::P)) {
			*pushFunc = new GamePause();
			return PushdownResult::Push;
		}
		tutorialGame->UpdateGame(dt);
		gameWorld->UpdateWorld(dt);
		physics->Update(dt);
		//renderer->Update(dt);

		return PushdownResult::NoChange;
	}

	protected:
		float width;
		float height;
		NCL::CSC8503::GameWorld* gameWorld;
		NCL::CSC8503::PhysicsSystem* physics;
		NCL::CSC8503::GameTechRenderer* renderer;
		NCL::CSC8503::TutorialGame* tutorialGame;


};