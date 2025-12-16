#include "EnemyAI.h"
#include "StateTransition.h"
#include <random>

class Goose : public EnemyAI 
{
public:
	Goose(std::vector<NCL::Vector3> waypoints, NCL::CSC8503::NavigationMesh* areaMesh, std::vector<PlayerObject*>& PlayerObjects);

	void Update(float dt) override;

	void OnCollisionBegin(GameObject* otherObject) override;
protected:
	void attackPlayer(float dt);
	int targetPlayer = -1;
	float sinceLastSighting = 0.0f;
	NCL::CSC8503::State* patrolState;
	NCL::CSC8503::State* attackState;
	NCL::CSC8503::StateTransition* patrolToAttack;
	NCL::CSC8503::StateTransition* attackToPatrol;


};