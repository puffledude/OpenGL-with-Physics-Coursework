#include "EnemyAI.h"
#include "State.h"
#include "StateTransition.h"
#include "NavigationPath.h"

EnemyAI::EnemyAI(std::vector<NCL::Vector3>* waypoints, NCL::CSC8503::NavigationGrid* areaMesh):patrolWaypoints(waypoints),
navMesh(areaMesh){
	
	NCL::CSC8503::State* patrolState = new NCL::CSC8503::State([&](float dt)-> void {
		Patrol(dt);
		});
	stateMachine.AddState(patrolState);
	NCL::CSC8503::State* attackState = new NCL::CSC8503::State([&](float dt)-> void {
		AttackPlayer(dt);});
	stateMachine.AddState(attackState);
	//Will need to raycast from enemy to player to see if they are in sight.
	//If true in attack state, else patrol state.
	NCL::CSC8503::StateTransition* patrolToAttack = new NCL::CSC8503::StateTransition(patrolState, attackState, [&]()-> bool {
		//Check if player is in sight
		return false; //placeholder
		});
	stateMachine.AddTransition(patrolToAttack);
	NCL::CSC8503::StateTransition* attackToPatrol = new NCL::CSC8503::StateTransition(attackState, patrolState, [&]()-> bool {
		//Check if player is no longer in sight
		return false; //placeholder
		});
	stateMachine.AddTransition(attackToPatrol);
	//Need two states, one for patrol and one for attack.
	//The patrol state is active when the enemy is not aware of the player.
	//In this, move from waypoint to waypoint. Using the navmesh to find the best route.
}
void EnemyAI::Update(float dt) {
	stateMachine.Update(dt);
}

void EnemyAI::Patrol(float dt) {
	if (patrolWaypoints->empty()) {
		return;
	}
	std::vector<NCL::Vector3> waypoints = *patrolWaypoints;
	if (Vector::Length(waypoints[currentTargetIndex] - this->GetTransform().GetPosition()) < 0.5) {
		currentTargetIndex += 1;
		if (currentTargetIndex >= waypoints.size()) { currentTargetIndex = 0; }
	}
	NCL::CSC8503::NavigationPath path;
	navMesh->FindPath(this->GetTransform().GetPosition(), waypoints[currentTargetIndex], path);
	

	//Move towards the current target waypoint.
	//Implement string pulling to move towards the waypoint using the navemesh

}

void EnemyAI::AttackPlayer(float dt) {
	//Chase the player.
}