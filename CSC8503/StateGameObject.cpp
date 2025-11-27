#include "StateGameObject.h"
#include "StateTransition.h"
#include "StateMachine.h"
#include "State.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)->void {
		this->MoveLeft(dt);
		});

	State* stateB = new State([&](float dt)->void {
		this->MoveRight(dt);
		});

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&](void)->bool 
		{
		return this->counter > 2.0f;
		}));

	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&](void)->bool
		{
			return this->counter <= 0.0f;
		}));

}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->SetLinearVelocity(Vector3(-10.0f, 0.0f, 0.0f));
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->SetLinearVelocity(Vector3(10.0f, 0.0f, 0.0f));
	counter -= dt;
}
