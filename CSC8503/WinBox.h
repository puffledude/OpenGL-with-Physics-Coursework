#include "GameObject.h"
#include "PlayerObject.h"
#include <string>

class WinBox : public NCL::CSC8503::GameObject {
public:
	WinBox() :
		GameObject("Winbox") {};
	~WinBox();

	void OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) override {
		if (otherObject->GetName() == "PlayerObject") {
			// Trigger win condition
			PlayerObject* player = static_cast<PlayerObject*>(otherObject);
			player->setHasWon(true);
			//std::cout << "Player has reached the WinBox! You win!" << std::endl;
			// Additional logic to end the game or transition to a win state can be added here
		}
	}

};
