#pragma once
#include "GameScreen.h"

class IntroScreen : public NCL::CSC8503::PushdownState 
{
public:

	IntroScreen(NCL::Window* window) :window(window) {};

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (window->GetKeyboard()->KeyPressed(NCL::KeyCodes::SPACE)) {
			*newState = new GameScreen(window);
			std::cout << "Starting game!" << std::endl;

			return Push;
		}
		if(window->GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
			std::cout << "Exiting game!" << std::endl;
			return Pop;
		}

		return NoChange;
	}


	void OnAwake() override {
		std::cout << "Welcome to the Mining Game!" << std::endl;
		std::cout << "Press SPACE to start, or ESCAPE to quit." << std::endl;
	}

protected:
	NCL::Window* window;

};