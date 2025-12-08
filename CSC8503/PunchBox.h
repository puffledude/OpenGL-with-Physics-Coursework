#pragma once
#include "GameObject.h"
#include "Vector.h"

class PunchBox : public NCL::CSC8503::GameObject
{
public:
	/// <summary>
	/// Constructor for a PunchBox
	/// </summary>
	/// <param name="direction">Direction punched towards</param>
	/// <param name="force">How much force is applied each frame</param>
	/// <param name="distance">How far the punch box extends</param>
	PunchBox(NCL::Vector3 direction, float force, float distance) {
		punchDirection = Vector::Normalise(direction);
		punchForce = force;
		punchDistance = distance;
		intialPosition = this->GetTransform().GetPosition();

		//Need to figure out how to avoid falling due to gravity
	}
	PunchBox() {};

	~PunchBox() = default;

	void Update(float dt) override;

	NCL::Vector3 GetPunchDirection() const {
		return punchDirection;
	}
	void SetPunchDirection(const NCL::Vector3& direction) {
		punchDirection = Vector::Normalise(direction);
	}

	NCL::Vector3 GetIntialPosition() const {
		return intialPosition;
	}
	void SetIntialPosition(const NCL::Vector3& position) {
		intialPosition = position;
	}

	void OnCollisionEnd(NCL::CSC8503::GameObject* otherObject) override;

	float GetPunchForce() const {
		return punchForce;
	}
	void SetPunchForce(float force) {
		punchForce = force;
	}
	float GetPunchDistance() const {
		return punchDistance;
	}
	void SetPunchDistance(float distance) {
		punchDistance = distance;
	}

	
protected:
	NCL::Vector3 punchDirection;
	NCL::Vector3 intialPosition;
	float punchForce;
	float punchDistance;
	bool flipped = false;
};