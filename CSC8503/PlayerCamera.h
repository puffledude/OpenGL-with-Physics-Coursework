#include "Camera.h"
#include "PlayerObject.h"

class PlayerCamera : public NCL::PerspectiveCamera
{
public:

	PlayerCamera(PlayerObject* player = nullptr);

	void UpdateCamera(float dt) override;

	PlayerObject* GetPlayer()const {
		return playerObject;
	}
	void SetPlayer(PlayerObject* player) {
		playerObject = player;
	}

	NCL::Vector3 DirectionToPlayer();

protected:
	PlayerObject* playerObject;
	
	float orbitPitch = 0.0f;
	float orbitYaw = 0.0f;

};