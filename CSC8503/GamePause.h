#pragma once
#include "pushdownState.h"
#include "Window.h"
#include "Debug.h"

class MainMenu;

class GamePause : public NCL::CSC8503::PushdownState 
{
public:
	GamePause() {
	}
	PushdownResult OnUpdate(float dt, PushdownState** pushFunc) override {
		NCL::Debug::Print("Game Paused", NCL::Vector2(45, 75), NCL::Vector4(1, 0, 0, 1));
		NCL::Debug::Print("Press U to Unpause", NCL::Vector2(45, 85), NCL::Vector4(1, 0, 0, 1));
		if (NCL::Window::GetKeyboard()->KeyDown(NCL::KeyCodes::U)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}


};
