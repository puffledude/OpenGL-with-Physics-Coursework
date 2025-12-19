#pragma once
#include "GameObject.h"
#include <string>

class GlassObject : public NCL::CSC8503::GameObject
{
public:
	/// <summary>
	/// Creates a glass object with a specified force resistance
	/// </summary>
	/// <param name="forceResistance">How much force the glass can resist without breaking</param>
	GlassObject(float forceResistance) : NCL::CSC8503::GameObject("Glass"),
		forceResistance(forceResistance)
	{
	}
	~GlassObject()
	{
	}

	//void OnCollisionBegin(NCL::CSC8503::GameObject* otherObject) override;

	float GetForceResistance() const {
		return forceResistance;
	}

	bool GetSmashed() const {
		return smashed;
	}

	void SetSmashed(bool state) {
		smashed = state;
	}
	Vector3 GetResetPosition() const {
		return resetPosition;
	}
	void SetResetPosition(const Vector3& pos) {
		resetPosition = pos;
	}

protected:
	Vector3 resetPosition;
	float forceResistance;
	bool smashed = false;

};