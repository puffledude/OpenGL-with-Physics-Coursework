#pragma once
#include "GameObject.h"

class FloatingBox : public NCL::CSC8503::GameObject 
{
public:
	FloatingBox() {};
	//FloatingBox(GameObject* object);

	~FloatingBox();

	void Update(float dt) override;

	void OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) override;
protected:
	//GameObject* box;
	bool collided = false;
};