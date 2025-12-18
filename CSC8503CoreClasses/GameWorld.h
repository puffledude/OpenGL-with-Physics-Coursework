#pragma once
#include "./Camera.h"
#include "../CSC8503/PlayerObject.h"
#include "../CSC8503/PlayerCamera.h"
#include "../CSC8503/GlassObject.h"
#include "../CSC8503/Goose.h"
#include "NavigationMesh.h"
#include <string>

namespace NCL {
		namespace Maths {
			class Ray;
			struct RayCollision;
		}
		class Camera;
		class PerspectiveCamera;

	namespace CSC8503 {
		class GameObject;
		class Constraint;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld	
		{
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c, bool andDelete = false);

			PlayerCamera& GetMainCamera()  
			{
				return mainCamera;
			}

			void ShuffleConstraints(bool state) 
			{
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) 
			{
				shuffleObjects = state;
			}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false, GameObject* ignore = nullptr) const;

			virtual void UpdateWorld(float dt);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;

			void GetConstraintIterators(
				std::vector<Constraint*>::const_iterator& first,
				std::vector<Constraint*>::const_iterator& last) const;

			int GetWorldStateID() const 
			{
				return worldStateCounter;
			}

			void SetSunPosition(const Vector3& pos) 
			{
				sunPosition = pos;
			}

			Vector3 GetSunPosition() const
			{
				return sunPosition;
			}

			void SetSunColour(const Vector3& col)
			{
				sunColour = col;
			}

			Vector3 GetSunColour() const
			{
				return sunColour;
			}

			PlayerObject* GetMainPlayer() const 
				{
					return playerObject;
				}

			void AddPlayer(PlayerObject* p) 
				{
					playerObjects.push_back(p);
			}

			// Return reference to internal player list so callers can avoid copies
			std::vector<PlayerObject*>& GetAllPlayers() {
				return playerObjects;
			}

			void SetMainPlayer(PlayerObject* p) 
				{
					playerObject = p;
			}
			PlayerObject* GetPlayerFromArray(int i =0) const 
				{
					return playerObjects[i];
			}


			/*void SetPlayer(PlayerObject* p, int i =0) 
				{
					playerObjects[i];
			}*/

			GlassObject* GetGlassObject() const 
			{
				return glassObject;
			}
			void SetGlassObject(GlassObject* g) 
			{
				glassObject = g;
			}

			NavigationMesh* GetNavigationMesh() const 
			{
				return navigationMesh;
			}

			Goose* GetGoose() const 
			{
				return gooseEnemy;
			}
			void SetGoose(Goose* g) 
			{
				gooseEnemy = g;
			}
			
		protected:
			std::vector<GameObject*> gameObjects;
			std::vector<PlayerObject*>playerObjects;
			std::vector<Constraint*> constraints;

			PlayerCamera mainCamera;
			PlayerObject* playerObject;
			GlassObject* glassObject;
			Goose* gooseEnemy;
			NavigationMesh* navigationMesh = new NavigationMesh("Level.navmesh");


			bool	shuffleConstraints;
			bool	shuffleObjects;
			int		worldIDCounter;
			int		worldStateCounter;

			Vector3 sunPosition;
			Vector3 sunColour;
		};
	}
}

