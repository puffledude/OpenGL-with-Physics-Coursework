#pragma once
#include "GameObject.h"

class FloatingBox : public NCL::CSC8503::GameObject 
{
public:
	FloatingBox() {};
	//FloatingBox(GameObject* object);

	void SetResetPos();
	~FloatingBox();

	void Update(float dt) override;

	void OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) override;
protected:
	float actualMass = 1.0f;
	Vector3 initialPosition;
	float resetTimer = 0.0f;
	bool collided = false;
};