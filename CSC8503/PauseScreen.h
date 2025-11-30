#pragma once
#include <iostream>
#include"PushdownState.h"
#include "Window.h"

using namespace NCL::CSC8503;

class PauseScreen : public PushdownState {
public:
	PauseScreen(NCL::Window* window) :window(window) {};

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (window->GetKeyboard()->KeyPressed(NCL::KeyCodes::U)) {
			return Pop;
		}

		return NoChange;
	}

	void OnAwake() override {
		std::cout << "Press U to unpause the game" << std::endl;
	}
protected:
	NCL::Window* window;
};


