#include "GameObject.h"
#include "Camera.h"

using namespace NCL::Maths;

class PlayerObject : public NCL::CSC8503::GameObject
{
public:
	PlayerObject(NCL::Camera* cam, Vector3 offset) : GameObject("PlayerObject")
	{
		playerCam = cam;
		this->offset = offset;
	}

	~PlayerObject()
	{
	}

	void Update(float dt) override;



	NCL::Camera* GetPlayerCam() const {
		return playerCam;
	}

	void SetPlayerCam(NCL::Camera* cam) {
		playerCam = cam;
	}

protected:
	Vector3 offset;
	NCL::Camera* playerCam;

};