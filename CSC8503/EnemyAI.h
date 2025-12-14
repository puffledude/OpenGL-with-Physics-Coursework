#pragma once
#include "GameObject.h"
#include "Vector.h"
#include "StateMachine.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include<vector>

class EnemyAI : public NCL::CSC8503::GameObject 
{
public:
	//Could use pushdown Automata here?
	//Patrol until player seen, then chase player.
	//Once player is no longer visable, return to patrol.
	EnemyAI(std::vector<NCL::Vector3> waypoints, NCL::CSC8503::NavigationMesh* areaMesh);
	void Update(float dt) override;

	float getMoveSpeed() const {
		return moveSpeed;
	}
	void setMoveSpeed(float speed) {
		moveSpeed = speed;
	}

protected:
	NCL::CSC8503::StateMachine stateMachine{};
	void Patrol(float dt);
	bool canSeePlayer();
	std::vector<Vector3> patrolWaypoints;
	int targetWaypointIndex = 0;
	Vector3 targetWaypoint;
	NCL::CSC8503::NavigationPath* currentPath = nullptr;
	Vector3 nextSubPos;
	NCL::CSC8503::NavigationMesh* navMesh = nullptr;
	float moveSpeed = 10.0f;
};