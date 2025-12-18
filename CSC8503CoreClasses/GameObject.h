#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "Texture.h"
#include"RenderObject.h"

//using std::vector;

namespace NCL::CSC8503 {
	//class NetworkObject;
	class RenderObject;
	class PhysicsObject;
	class NetworkObject;

	class GameObject	{
	public:
		GameObject(const std::string& name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) 
		{
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const 
		{
			return boundingVolume;
		}

		bool IsActive() const 
		{
			return isActive;
		}

		Transform& GetTransform() 
		{
			return transform;
		}

		RenderObject* GetRenderObject() const 
		{
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const 
		{
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const 
		{
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) 
		{
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) 
		{
			physicsObject = newObject;
		}

		const std::string& GetName() const 
		{
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		virtual void Update(float dt) 
		{

		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) 
		{
			worldID = newID;
		}

		int		GetWorldID() const 
		{
			return worldID;
		}

		NCL::Rendering::Texture* GetTexture(bool diffuse) {
			if (!renderObject) {
				return nullptr;
			}
			if (diffuse) {
				return renderObject->GetMaterial().diffuseTex;
			}
			else {
				return renderObject->GetMaterial().bumpTex;
			}
		}

		bool IsTrigger() const {
			return isTrigger;
		}
		void SetTrigger(bool state) {
			isTrigger = state;
		}
		void SetTransform(const Transform& newTransform)
		{
			transform = newTransform;
		}
		void createNetworkObject();
		void createNetworkObject(int id); // create with explicit network id

	protected:
		
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject* 		physicsObject;
		RenderObject* 		renderObject;
		NetworkObject* 		networkObject;

		bool				isActive;
		bool				isTrigger;
		int					worldID;
		std::string			name;

		Vector3				broadphaseAABB;
	};
}

