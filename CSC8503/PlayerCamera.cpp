#include "PlayerCamera.h"
#include "./Matrix.h"
#include "./Vector.h"
#include "./GameObject.h"
#include "./Transform.h"


using namespace NCL;

PlayerCamera::PlayerCamera(PlayerObject* player)
		:playerObject(player)
{
}

Vector3 PlayerCamera::DirectionToPlayer() {
	if (!playerObject) {
		return Vector3(0, 0, 0);
	}
	return playerObject->GetTransform().GetPosition() - GetPosition();
}


void PlayerCamera::UpdateCamera(float dt) {
	if (!playerObject) {
		return;
	}
	if (!activeController) {
		return;
	}
	if (freeCam) {
		PerspectiveCamera::UpdateCamera(dt);
		return;
	}
	/*Vector3 direction = DirectionToPlayer();
	direction = Vector::Normalise(direction);*/
	//Set the camera position a fixed distance behind the player
	const float distanceFromPlayer = 15.0f;
	Vector3 currentPos = this->GetPosition();
	

	orbitPitch += activeController->GetNamedAxis("YLook");
	orbitYaw -= activeController->GetNamedAxis("XLook");
	
	

	//Bounds check the pitch, to be between straight up and straight down ;)
	orbitPitch = std::min(orbitPitch, 90.0f);
	orbitPitch = std::max(orbitPitch, -90.0f);

	if (orbitYaw < 0) {
		orbitYaw += 360.0f;
	}
	if (orbitYaw > 360.0f) {
		orbitYaw -= 360.0f;
	}
	
	Vector3 target = playerObject->GetTransform().GetPosition();
	
	//Using Rotation matraces to calculate offset
	//Rotate left-right (yaw) around y axis first, then up-down (pitch) around x axis

	Matrix3 yrotation = Matrix::RotationMatrix3x3(orbitYaw, Vector3(0, 1, 0));
	Matrix3 xrotation = Matrix::RotationMatrix3x3(orbitPitch, Vector3(1, 0, 0));
	Matrix3 combinedRotation = yrotation * xrotation;

	Vector3 offset = combinedRotation* Vector3(0,0, -distanceFromPlayer);
	//offset.x = distanceFromPlayer * cos(pitch) * sin(yaw);
	//offset.y = distanceFromPlayer * sin(pitch);
	//offset.z = distanceFromPlayer * cos(pitch) * cos(yaw);

	SetPosition(target + offset);

	SetYaw(orbitYaw +180);
	SetPitch(-orbitPitch);


	//Vector3 desiredPosition = playerObject->GetTransform()->GetPosition() - direction * distanceFromPlayer;
	//SetPosition(desiredPosition);
	////Calculate pitch and yaw to look at the player
	//Vector3 toPlayer = playerObject->GetPosition() - GetPosition();
	//toPlayer = Vector::Normalise(toPlayer);
	//float pitchAngle = asinf(toPlayer.y) * (180.0f / 3.14159f);
	//float yawAngle = atan2f(toPlayer.x, toPlayer.z) * (180.0f / 3.14159f);
	//SetPitch(pitchAngle);
	//SetYaw(yawAngle);
}