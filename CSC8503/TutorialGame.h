#pragma once
#include "RenderObject.h"
#include "StateGameObject.h"
namespace NCL {
	class Controller;

	namespace Rendering {
		class Mesh;
		class Texture;
		class Shader;
	}
	namespace CSC8503 {
		class GameTechRendererInterface;
		class PhysicsSystem;
		class GameWorld;
		class GameObject;

		class TutorialGame {
		public:
			TutorialGame(GameWorld& gameWorld, GameTechRendererInterface& renderer, PhysicsSystem& physics);
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitCamera();

			void InitWorld();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on).
			*/
			void InitGameExamples();

			void CreateSphereGrid(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void CreatedMixedGrid(int numRows, int numCols, float rowSpacing, float colSpacing);
			void CreateAABBGrid(int numRows, int numCols, float rowSpacing, float colSpacing, const NCL::Maths::Vector3& cubeDims);

			/// <summary>
			/// Loads static Level elements (the floor)
			/// </summary>
			void LoadLevel();

			/// <summary>
			/// Loads dynamic Level elements (enemies, bonuses, swinging balls)
			/// </summary>
			void LoadDynamic();
			void BridgeConstraintTest();

			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void PlayerMovement();
			float jumpCooldown = 0.0f;

			GameObject* AddFloorToWorld(const NCL::Maths::Vector3& position);
			GameObject* AddSphereToWorld(const NCL::Maths::Vector3& position, float radius, float inverseMass = 10.0f, bool hollow=false);
			GameObject* AddCubeToWorld(const NCL::Maths::Vector3& position, NCL::Maths::Vector3 dimensions, float inverseMass = 10.0f, bool heavy=false);
			GameObject* AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, Quaternion rotation, float inverseMass);

			GameObject* AddPlayerToWorld(const NCL::Maths::Vector3& position);
			GameObject* AddEnemyToWorld(const NCL::Maths::Vector3& position);
			GameObject* AddBonusToWorld(const NCL::Maths::Vector3& position);

			//Added
			GameObject* AddSwingBallToWorld(const NCL::Maths::Vector3& position, float distance, float radius, float inverseMass, NCL::Maths::Vector3 direction, float pushForce);
			GameObject* AddFloatingBoxToWorld(const NCL::Maths::Vector3& position, NCL::Maths::Vector3 dimensions);

			GameWorld& world;
			GameTechRendererInterface& renderer;
			PhysicsSystem& physics;
			Controller* controller;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			Rendering::Mesh* capsuleMesh	= nullptr;
			Rendering::Mesh* cubeMesh		= nullptr;
			Rendering::Mesh* sphereMesh		= nullptr;

			Rendering::Texture* defaultTex  = nullptr;
			Rendering::Texture* checkerTex	= nullptr;
			Rendering::Texture* glassTex	= nullptr;

			//Coursework Meshes
			Rendering::Mesh* catMesh	= nullptr;
			Rendering::Mesh* kittenMesh = nullptr;
			Rendering::Mesh* enemyMesh	= nullptr;
			Rendering::Mesh* bonusMesh	= nullptr;

			GameTechMaterial checkerMaterial;
			GameTechMaterial glassMaterial;
			GameTechMaterial notexMaterial;

			//Coursework Additional functionality	
			GameObject* lockedObject = nullptr;
			NCL::Maths::Vector3 lockedOffset = NCL::Maths::Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;

			StateGameObject* AddStateObjectToWorld(const NCL::Maths::Vector3& position);
			StateGameObject* testStateGameObject;
		};
	}
}