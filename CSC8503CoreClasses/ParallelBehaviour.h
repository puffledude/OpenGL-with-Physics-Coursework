#pragma once
#include <string>
#include "BehaviourNodeWithChildren.h"

class ParallelBehaviour : public BehaviourNodeWithChildren {
public:

	ParallelBehaviour(const std::string& nodeName) : BehaviourNodeWithChildren(nodeName) {}
	~ParallelBehaviour() {}
	BehaviourState Execute(float dt) override {
		int successCount = 0;
		bool anyRunning = false;
		for (auto& i : childNodes) {

			BehaviourState nodeState = i->Execute(dt);
			switch (nodeState) {
			case Success:
				successCount++;
				break;
			case Ongoing:
				anyRunning = true;
				break;
			case Failure:
				currentState = Failure;
			}


		}
		if (successCount >0 ) {
			currentState = Success;
		}
		else if (anyRunning) {
			currentState = Ongoing;
		}
		else {
			currentState = Failure;
		}
		return currentState;
	}
};