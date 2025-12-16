#include "Goose.h"
#include "State.h"
#include "PhysicsObject.h"


Goose::Goose(std::vector<NCL::Vector3> waypoints, NCL::CSC8503::NavigationMesh* areaMesh, std::vector<PlayerObject*>& PlayerObjects)
	: EnemyAI(waypoints, areaMesh, PlayerObjects) {
	
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
		int player = -1;
		player = canSeePlayer();
		if (player != -1) {
			targetPlayer = player;
			sinceLastSighting = 5.0f;
			return true;
		}
		return false;
		});
	stateMachine.AddTransition(patrolToAttack);

	attackToPatrol = new NCL::CSC8503::StateTransition(attackState, patrolState, [&]()-> bool {
		//Check if player is no longer in sight
		std::cout << "Checking if I can still see the player..." << std::endl;
		if (canSeePlayer() != targetPlayer) {
			sinceLastSighting -= 0.01; //Can't use dt here so approximating.
			std::cout << "I lost sight of the player!" << std::endl;
			std::cout << "sinceLastSighting: " << sinceLastSighting << std::endl;
		}
		return sinceLastSighting <= 0.0f;
		});
	stateMachine.AddTransition(attackToPatrol);
}

void Goose::Update(float dt) {
	stateMachine.Update(dt);
}

void Goose::attackPlayer(float dt) {
	{
		PlayerObject* playerObject = nullptr;
		if (targetPlayer >= 0 && targetPlayer < players.size()) {
			playerObject = players[targetPlayer];
		}
		
		Vector3 direction = Vector::Normalise(playerObject->GetTransform().GetPosition() - this->GetTransform().GetPosition());
		this->GetPhysicsObject()->AddForce(direction * moveSpeed * dt);
	}
}

void Goose::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName() == "PlayerObject") {
		Vector3 direction = Vector::Normalise((this->GetTransform().GetOrientation() * Vector3(0, 0, -1) + Vector3(0,50,0)));
		otherObject->GetPhysicsObject()->AddForce(direction * 1000000.0f);
	}
}

