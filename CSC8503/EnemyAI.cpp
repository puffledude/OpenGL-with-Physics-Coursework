#include "EnemyAI.h"
#include "State.h"
#include "StateTransition.h"
#include "NavigationPath.h"
#include "PhysicsObject.h"


EnemyAI::EnemyAI(std::vector<NCL::Vector3> waypoints, NCL::CSC8503::NavigationMesh* areaMesh):patrolWaypoints(waypoints),
navMesh(areaMesh){
	
	//NCL::CSC8503::State* patrolState = new NCL::CSC8503::State([&](float dt)-> void {
	//	Patrol(dt);
	//	});
	//stateMachine.AddState(patrolState);
	//NCL::CSC8503::State* attackState = new NCL::CSC8503::State([&](float dt)-> void {
	//	AttackPlayer(dt);});
	//stateMachine.AddState(attackState);
	////Will need to raycast from enemy to player to see if they are in sight.
	////If true in attack state, else patrol state.
	//NCL::CSC8503::StateTransition* patrolToAttack = new NCL::CSC8503::StateTransition(patrolState, attackState, [&]()-> bool {
	//	//Check if player is in sight
	//	return false; //placeholder
	//	});
	//stateMachine.AddTransition(patrolToAttack);
	//NCL::CSC8503::StateTransition* attackToPatrol = new NCL::CSC8503::StateTransition(attackState, patrolState, [&]()-> bool {
	//	//Check if player is no longer in sight
	//	return false; //placeholder
	//	});
	//stateMachine.AddTransition(attackToPatrol);
	//Need two states, one for patrol and one for attack.
	//The patrol state is active when the enemy is not aware of the player.
	//In this, move from waypoint to waypoint. Using the navmesh to find the best route.
}
void EnemyAI::Update(float dt) {
	stateMachine.Update(dt);
}

void EnemyAI::Patrol(float dt) {
	if (patrolWaypoints.empty()) {
		return;
	}
	/*std::vector<NCL::Vector3> waypoints = *patrolWaypoints;
	if (Vector::Length(waypoints[currentTargetIndex] - this->GetTransform().GetPosition()) < 0.5) {
		currentTargetIndex += 1;
		if (currentTargetIndex >= waypoints.size()) { currentTargetIndex = 0; }
	}*/
	//Need ifs on if close to main waypoint. Another if on subwaypoint and another for building path.
	Vector2 flatSubPos = Vector2(nextSubPos.x, nextSubPos.z);
	Vector2 flatPos = Vector2(this->GetTransform().GetPosition().x, this->GetTransform().GetPosition().z);
	if(Vector::Length(Vector2(patrolWaypoints[targetWaypointIndex].x , patrolWaypoints[targetWaypointIndex].z) - flatPos) < 4.0f){
		//Reached main waypoint, go to next one.
		targetWaypointIndex += 1;
		if (targetWaypointIndex >= patrolWaypoints.size()) {
			targetWaypointIndex = 0;
		}
		//Clear current path so a new one is built.
		if (currentPath) {
			delete currentPath;
			currentPath = nullptr;
		}
	}

	if (!currentPath) {
		currentPath = new NCL::CSC8503::NavigationPath();
		if (navMesh->FindPath(this->GetTransform().GetPosition(), patrolWaypoints[targetWaypointIndex], *currentPath))
			currentPath->PopWaypoint(nextSubPos);
	}


	if (currentPath) {
		//Have a path, check if close to next sub waypoint.
		Vector2 flatSubPos = Vector2(nextSubPos.x, nextSubPos.z);

		if (Vector::Length(flatPos - flatSubPos) < 1.5f) {
			//Reached sub waypoint, get next one.
			if (!currentPath->PopWaypoint(nextSubPos)) {
				//No more waypoints, reached destination.
				delete currentPath;
				currentPath = nullptr;
			}
		}
	}

	if (!currentPath) {
		currentPath = new NCL::CSC8503::NavigationPath();
		if (navMesh->FindPath(this->GetTransform().GetPosition(), patrolWaypoints[targetWaypointIndex], *currentPath)) currentPath->PopWaypoint(nextSubPos);
	}
	std::cout << "Testing: Next Sub Pos: " << nextSubPos.x << "," << nextSubPos.y << "," << nextSubPos.z << "\n";
	std::cout << "Enemy Pos: " << this->GetTransform().GetPosition().x << "," << this->GetTransform().GetPosition().y << "," << this->GetTransform().GetPosition().z << "\n";
	std::cout << "Distance between: " << Vector::Length(nextSubPos - this->GetTransform().GetPosition()) << "\n";
	Vector3 dir = Vector::Normalise(nextSubPos - this->GetTransform().GetPosition());
	this->GetPhysicsObject()->AddForce(dir * moveSpeed* dt);
	



	//Move towards the current target waypoint.
	//Implement string pulling to move towards the waypoint using the navemesh

}