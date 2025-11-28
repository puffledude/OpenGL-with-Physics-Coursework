#pragma once
#include "Decorators.h"
#include <string>

class Inverter : public DecoratorNode {
public:

	Inverter(const std::string& nodeName) : DecoratorNode(nodeName) {}
	~Inverter() {}
	BehaviourState Execute(float dt) override {
		BehaviourState childState = childNode->Execute(dt);
		switch (childState) {
		case Success:
			currentState = Failure;
			break;
		case Failure:
			currentState = Success;
			break;
		case Ongoing:
			currentState = Ongoing;
			break;
		}
		return currentState;
	}


};
