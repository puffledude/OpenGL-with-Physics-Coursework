#pragma once
#include "PushdownState.h"
#include "Window.h"
#include "Debug.h"

class FinishedGameState : public NCL::CSC8503::PushdownState 
{
public:
	FinishedGameState() {}
	PushdownResult OnUpdate(float dt, PushdownState** pushFunc) override {
		NCL::Debug::Print("Game Finished!", NCL::Vector2(45, 75), NCL::Vector4(0, 1, 0, 1));
		return NCL::CSC8503::PushdownState::NoChange;
	}


};