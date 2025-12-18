#include "GameMainMenu.h"
#include "PushdownState.h"
#include "TutorialGame.h"
#include "NetworkedGame.h"
#include "InGame.h"
#include "InGameServer.h"
#include "InGameClient.h"


NCL::CSC8503::PushdownState::PushdownResult GameMainMenu::OnUpdate(float dt, NCL::CSC8503::PushdownState** pushFunc){
	NCL::Debug::Print("Glass Run", NCL::Vector2(50, 40));
	NCL::Debug::Print("Press ENTER to Start", NCL::Vector2(50, 50));
	NCL::Debug::Print("Press H to Host a server", NCL::Vector2(50, 60));
	NCL::Debug::Print("Press J to Join a server", NCL::Vector2(50, 70));
	if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
		return NCL::CSC8503::PushdownState::Pop;
	}
	if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::RETURN)) {
		NCL::CSC8503::TutorialGame* tutorialGame = new TutorialGame(*gameWorld, *renderer, *physics);
		*pushFunc = new InGame(gameWorld, physics, renderer, tutorialGame, menuWidth, menuHeight);
		return NCL::CSC8503::PushdownState::Push;
	}
	if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::H)) {
		NCL::CSC8503::NetworkedGame* networkedGame = new NCL::CSC8503::NetworkedGame(*gameWorld, *renderer, *physics);
		networkedGame->StartAsServer();
		*pushFunc = new InGameServer(gameWorld, physics, renderer, networkedGame, menuWidth, menuHeight);
		return NCL::CSC8503::PushdownState::Push;
	}
	if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::J)) {
		NCL::CSC8503::NetworkedGame* networkedGame = new NCL::CSC8503::NetworkedGame(*gameWorld, *renderer, *physics);
		networkedGame->StartAsClient(127, 0, 0, 1);
		*pushFunc = new InGameClient(gameWorld, physics, renderer, networkedGame, menuWidth, menuHeight);
		return NCL::CSC8503::PushdownState::Push;
	}


	return NCL::CSC8503::PushdownState::NoChange;
}