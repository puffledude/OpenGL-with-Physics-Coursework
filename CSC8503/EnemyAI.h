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
	EnemyAI(std::vector<NCL::Vector3>* waypoints, NCL::CSC8503::NavigationMesh* areaMesh);
	void Update(float dt) override;


protected:
	NCL::CSC8503::StateMachine stateMachine;
	void Patrol(float dt);
	virtual void AttackPlayer(float dt);
	std::vector<NCL::Vector3>* patrolWaypoints;
	int currentTargetIndex =0;
	NCL::CSC8503::NavigationMesh* navMesh = nullptr;
};