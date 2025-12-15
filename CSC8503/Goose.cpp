#include "Goose.h"
#include "State.h"
#include "PhysicsObject.h"

Goose::Goose(std::vector<NCL::Vector3> waypoints, NCL::CSC8503::NavigationMesh* areaMesh, GameObject* playerObject)
	: EnemyAI(waypoints, areaMesh, playerObject) {
	
	patrolState = new NCL::CSC8503::State([&](float dt)-> void {
		Patrol(dt);
		});

	stateMachine.AddState(patrolState);
	attackState = new NCL::CSC8503::State([&](float dt)-> void {
		attackPlayer(dt);
		std::cout<< "Honk!Honk!I'm attacking the player!" << std::endl;
		});
	stateMachine.AddState(attackState);
	patrolToAttack = new NCL::CSC8503::StateTransition(patrolState, attackState, [&]()-> bool {
		//Check if player is in sight
		return canSeePlayer();
		});
	stateMachine.AddTransition(patrolToAttack);
	attackToPatrol = new NCL::CSC8503::StateTransition(attackState, patrolState, [&]()-> bool {
		//Check if player is no longer in sight
		return !canSeePlayer();
		});
	stateMachine.AddTransition(attackToPatrol);
}

void Goose::Update(float dt) {
	stateMachine.Update(dt);
}

void Goose::attackPlayer(float dt) {
	{
		Vector3 direction = Vector::Normalise(playerObject->GetTransform().GetPosition() - this->GetTransform().GetPosition());
		this->GetPhysicsObject()->AddForce(direction * moveSpeed * dt);
	}
}

