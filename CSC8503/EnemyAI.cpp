#include "EnemyAI.h"
#include "PlayerObject.h"
#include "State.h"
#include "StateTransition.h"
#include "NavigationPath.h"
#include "PhysicsObject.h"
#include "Ray.h"
#include "CollisionDetection.h"


EnemyAI::EnemyAI(std::vector<NCL::Vector3> waypoints, NCL::CSC8503::NavigationMesh* areaMesh, std::vector<PlayerObject*>& playerObjects):patrolWaypoints(waypoints),
navMesh(areaMesh), players(playerObjects){}
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
	Vector2 flatPos = Vector2(this->GetTransform().GetPosition().x, this->GetTransform().GetPosition().z);
	if(Vector::Length(Vector2(patrolWaypoints[targetWaypointIndex].x , patrolWaypoints[targetWaypointIndex].z) - flatPos) < 5.0f){
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

		if (Vector::Length(flatPos - flatSubPos) < 3.0f) {
			//Reached sub waypoint, get next one.
			if (!currentPath->PopWaypoint(nextSubPos)) {
				//No more waypoints, reached destination.
				delete currentPath;
				currentPath = nullptr;
			}
		}
	}

	
	/*std::cout << "Testing: Next Sub Pos: " << nextSubPos.x << "," << nextSubPos.y << "," << nextSubPos.z << "\n";
	std::cout << "Enemy Pos: " << this->GetTransform().GetPosition().x << "," << this->GetTransform().GetPosition().y << "," << this->GetTransform().GetPosition().z << "\n";
	std::cout << "Distance between: " << Vector::Length(nextSubPos - this->GetTransform().GetPosition()) << "\n";*/
	Vector3 dir = Vector::Normalise(nextSubPos - this->GetTransform().GetPosition());

	Vector3 forward = Vector3(-dir.x, 0, -dir.z);
	Vector3 up = Vector3(0, 1, 0);
	Vector3 right = Vector::Cross(up, forward);
	
	Matrix4 temp = Matrix::View(this->GetTransform().GetPosition() + forward, this->GetTransform().GetPosition(), up);  //Make a view matrix looking in the move direction
	Matrix4 modelMat = Matrix::Inverse(temp);  //Make the model matrix in model space
	Quaternion q(modelMat);   //Extract the rotation
	this->GetTransform().SetOrientation(q.Normalised());

	this->GetPhysicsObject()->AddForce(dir * moveSpeed* dt);

}

int EnemyAI::canSeePlayer() {
	for (int i = 0; i < players.size(); i++) {
		PlayerObject* playerObject = players[i];
		Vector3 vecDist = playerObject->GetTransform().GetPosition() - this->GetTransform().GetPosition();
		float dist = Vector::Length(vecDist);
		if (dist > sightDistance) {
			continue;
		}
		Vector3 dirToPlayer = Vector::Normalise(vecDist);
		Quaternion orientation = this->GetTransform().GetOrientation();
		NCL::Vector3 eforward = orientation * Vector3(0, 0, -1); //Enemy forward vector
		float cosT = Vector::Dot(eforward, dirToPlayer);
		if (cosT < cosf(NCL::Maths::DegreesToRadians(fovAngle / 2.0f))) {
			continue; //Player not within fov.
		}

		Ray rayToPlayer(this->GetTransform().GetPosition(), dirToPlayer);
		RayCollision rayColl;
		if (NCL::CollisionDetection::RayIntersection(rayToPlayer, *playerObject, rayColl)) {
			return i;
		}
	}
	//Raycast from enemy to player, if hits player return index, else -1.
	return -1;
}