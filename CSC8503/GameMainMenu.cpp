#include "GameMainMenu.h"
#include "PushdownState.h"
#include "TutorialGame.h"
#include "InGame.h"


NCL::CSC8503::PushdownState::PushdownResult GameMainMenu::OnUpdate(float dt, NCL::CSC8503::PushdownState** pushFunc){
	NCL::Debug::Print("Glass Run", NCL::Vector2(50, 40));
	NCL::Debug::Print("Press ENTER to Start", NCL::Vector2(50, 50));
	if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
		return NCL::CSC8503::PushdownState::Pop;
	}
	if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::RETURN)) {
		NCL::CSC8503::TutorialGame* tutorialGame = new TutorialGame(*gameWorld, *renderer, *physics);
		*pushFunc = new InGame(gameWorld, physics, renderer, tutorialGame, menuWidth, menuHeight);
		return NCL::CSC8503::PushdownState::Push;
	}

	return NCL::CSC8503::PushdownState::NoChange;
}