#pragma once
#include "BehaviourNode.h"
#include <string>

class DecoratorNode : public BehaviourNode {
public:
	DecoratorNode(const std::string& nodeName) : BehaviourNode(nodeName) {}
	~DecoratorNode() {}

	void SetChild(BehaviourNode* child) {
		childNode = child;
	}

	virtual BehaviourState Execute(float dt) override = 0;


protected:
	BehaviourNode* childNode;

};