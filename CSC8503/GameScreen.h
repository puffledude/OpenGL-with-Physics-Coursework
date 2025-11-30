#pragma once
#include <iostream>
#include "PauseScreen.h"


class GameScreen: public NCL::CSC8503::PushdownState {
public:

	GameScreen(NCL::Window* window) :window(window) {};
	
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		pauseReminder -= dt;
		if (pauseReminder < 0) {
			std::cout << "Coins Mined:" << coinsMined << std::endl;
			std::cout << "Press P to pause the game, or F1 to return to main menu" << std::endl;
			pauseReminder += 1.0f ;
		}
		if (window->GetKeyboard()->KeyPressed(NCL::KeyCodes::P)) {
			*newState = new PauseScreen(window);
			std::cout << "Pausing game" << std::endl;
			return Push;
		}
		if (window->GetKeyboard()->KeyPressed(NCL::KeyCodes::F1)) {
			std::cout << "Exiting to main menu" << std::endl;
			return Pop;
		}

		if (rand()%7 ==0) {
			coinsMined++;
		}


		return NoChange;
	}
	void OnAwake() override {
		std::cout << "Preparing to mine coins" << std::endl;
	}


protected:
	NCL::Window* window;
	int coinsMined = 0;
	int pauseReminder = 1;

};