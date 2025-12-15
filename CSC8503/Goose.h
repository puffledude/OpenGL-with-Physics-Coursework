#include "EnemyAI.h"
#include "StateTransition.h"

class Goose : public EnemyAI 
{
public:
	Goose(std::vector<NCL::Vector3> waypoints, NCL::CSC8503::NavigationMesh* areaMesh, GameObject* playerObject);

	void Update(float dt) override;
protected:
	void attackPlayer(float dt);
	NCL::CSC8503::State* patrolState;
	NCL::CSC8503::State* attackState;
	NCL::CSC8503::StateTransition* patrolToAttack;
	NCL::CSC8503::StateTransition* attackToPatrol;


};