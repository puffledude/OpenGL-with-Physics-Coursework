#pragma once
#include "GameObject.h"
#include "Vector.h"
#include "PositionConstraint.h"

class SwingBall : public NCL::CSC8503::GameObject 
{
public:
	/// <summary>
	/// Constructor for  a swinging ball attached to a fixed point by a constraint.
	/// </summary>
	/// <param name="position">Position of the constraint the swinging ball is attached to.</param>
	/// <param name="length">Distance from swinging ball to the constraint.</param>
	/// <param name="radius">Radius of the swining ball</param>
	/// <param name="InverseMass">Inverse mass of the swinging ball</param>
	/// <param name="pushForce">Force applied to the ball when at its lowest point</param>
	SwingBall(GameObject* sphere, GameObject* anchor, float distance,NCL::Vector3 direction, float pushforce) {
		this->anchor = anchor;
		this->ball = sphere;
		constraint = new NCL::CSC8503::PositionConstraint(anchor, sphere, distance);
		this->direction = Vector::Normalise(direction);
		this->pushForce = pushforce;
	}

	~SwingBall() {
		delete constraint;
	}

	void Update(float dt) override;

	NCL::CSC8503::Constraint* GetConstraint() const {
		return constraint;
	}

protected:
	GameObject* ball;
	GameObject* anchor;
	NCL::CSC8503::PositionConstraint* constraint;
	NCL::Vector3 direction;
	float pushForce;
	bool flipped = false;
};
