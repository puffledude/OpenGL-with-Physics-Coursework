#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "PlayerObject.h"
#include "SwingBall.h"
#include "FloatingBox.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"

#include "Window.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"

#include "Debug.h"

#include "KeyboardMouseController.h"

#include "GameTechRendererInterface.h"

#include "Ray.h"

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame(GameWorld& inWorld, GameTechRendererInterface& inRenderer, PhysicsSystem& inPhysics)
	:	world(inWorld),
		renderer(inRenderer),
		physics(inPhysics)
{

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	controller = new KeyboardMouseController(*Window::GetWindow()->GetKeyboard(), *Window::GetWindow()->GetMouse());

	world.GetMainCamera().SetController(*controller);

	world.SetSunPosition({ -200.0f, 60.0f, -200.0f });
	world.SetSunColour({ 0.8f, 0.8f, 0.5f });

	controller->MapAxis(0, "Sidestep");
	controller->MapAxis(1, "UpDown");
	controller->MapAxis(2, "Forward");

	controller->MapAxis(3, "XLook");
	controller->MapAxis(4, "YLook");

	cubeMesh	= renderer.LoadMesh("cube.msh");
	sphereMesh	= renderer.LoadMesh("sphere.msh");
	catMesh		= renderer.LoadMesh("ORIGAMI_Chat.msh");
	kittenMesh	= renderer.LoadMesh("Kitten.msh");

	enemyMesh	= renderer.LoadMesh("Keeper.msh");

	bonusMesh	= renderer.LoadMesh("19463_Kitten_Head_v1.msh");
	capsuleMesh = renderer.LoadMesh("capsule.msh");

	defaultTex	= renderer.LoadTexture("Default.png");
	checkerTex	= renderer.LoadTexture("checkerboard.png");
	glassTex	= renderer.LoadTexture("stainedglass.tga");

	checkerMaterial.type		= MaterialType::Opaque;
	checkerMaterial.diffuseTex	= checkerTex;

	glassMaterial.type			= MaterialType::Transparent;
	glassMaterial.diffuseTex	= glassTex;

	InitWorld();
	InitCamera();
	
}

TutorialGame::~TutorialGame()	{
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world.GetMainCamera().UpdateCamera(dt);
	}
	PlayerObject* player = world.GetPlayer();
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics.UseGravity(useGravity);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::P)) {
		world.GetMainCamera().outputPosition();
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::F)) {
		world.GetMainCamera().SetFreeCam();
	}

	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F9)) {
		world.ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F10)) {
		world.ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F7)) {
		world.ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::F8)) {
		world.ShuffleObjects(false);
	}

	if (player) {
		//lockedObject = player;
		if(!world.GetMainCamera().GetFreeCamMode())PlayerMovement();
	}
	else {
		DebugObjectMovement();
	}

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world.Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	//This year we can draw debug textures as well!
	Debug::DrawTex(*defaultTex, Vector2(10, 10), Vector2(5, 5), Debug::WHITE);
	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));
	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	}

	SelectObject();
	MoveSelectedObject();	
	
	world.OperateOnContents(
		[dt](GameObject* o) {
			o->Update(dt);
		}
	);

	if(testStateGameObject)
		testStateGameObject->Update(dt);
}

//Probabbly redesign to be more puzzly.
//E.G 3 items for three different locations. 
void TutorialGame::LoadLevel() {
	AddCubeToWorld(Vector3(-14.00, 6.09, 35.26) * 8.0f, Vector3(14.47, 5.32, 9.42) * 4.0f, 0);
	AddCubeToWorld(Vector3(-14.00, 6.09, 35.26) * 8.0f, Vector3(14.47, 5.32, 9.42) * 4.0f, 0);
	AddOBBCubeToWorld(Vector3(-13.89, -2.53, -0.19) * 8.0f, Vector3(29.45, 0.27, 8.20) * 4.0f, Quaternion(0.00000, 0.70711, 0.00000, -0.70711), 0);
	AddCubeToWorld(Vector3(-6.38, -2.54, -18.84) * 8.0f, Vector3(23.35, 0.28, 7.81) * 4.0f, 0);
	AddOBBCubeToWorld(Vector3(-13.98, 2.16, 22.93) * 8.0f, Vector3(14.45, 1.70, 20.18) * 4.0f, Quaternion(0.30071, 0.00000, 0.00000, -0.95372), 0);
	AddCubeToWorld(Vector3(21.86, -2.90, -20.90) * 8.0f, Vector3(33.17, 0.76, 3.69) * 4.0f, 0);
	AddCubeToWorld(Vector3(46.90, 0.11, -20.76) * 8.0f, Vector3(2.14, 0.37, 2.57) * 4.0f, 0);
	AddCubeToWorld(Vector3(49.74, 1.01, -20.74) * 8.0f, Vector3(2.14, 0.37, 2.57) * 4.0f, 0);
	AddCubeToWorld(Vector3(44.47, -0.77, -20.76) * 8.0f, Vector3(2.14, 0.37, 2.57) * 4.0f, 0);
	AddCubeToWorld(Vector3(100.68, -1.32, -25.72) * 8.0f, Vector3(4.24, 3.12, 4.26) * 4.0f, 0);
	AddCubeToWorld(Vector3(125.03, -0.86, 39.41) * 8.0f, Vector3(4.77, 4.10, 3.36) * 4.0f, 0);
	AddOBBCubeToWorld(Vector3(82.36, 0.93, 7.54) * 8.0f, Vector3(7.45, 51.33, 1.34) * 4.0f, Quaternion(0.50000, -0.50000, -0.50000, 0.50000), 0);
	AddCubeToWorld(Vector3(105.72, -1.63, -18.22) * 8.0f, Vector3(4.76, 2.51, 4.35) * 4.0f, 0);
	AddCubeToWorld(Vector3(92.24, -0.60, -5.38) * 8.0f, Vector3(4.14, 3.81, 2.09) * 4.0f, 0);
	AddCubeToWorld(Vector3(98.51, -1.11, -14.20) * 8.0f, Vector3(2.67, 3.36, 2.68) * 4.0f, 0);
	AddCubeToWorld(Vector3(78.67, 0.85, -20.76) * 8.0f, Vector3(8.89, 0.37, 2.57) * 4.0f, 0);
	AddCubeToWorld(Vector3(109.08, -0.96, -31.40) * 8.0f, Vector3(53.87, 3.61, 0.49) * 4.0f, 0);
	AddCubeToWorld(Vector3(83.95, 0.74, -18.35) * 8.0f, Vector3(4.54, 7.00, 0.44) * 4.0f, 0);
	AddCubeToWorld(Vector3(83.61, 0.70, -23.43) * 8.0f, Vector3(4.54, 7.00, 0.44) * 4.0f, 0);
	AddCubeToWorld(Vector3(65.78, 1.59, 38.72) * 8.0f, Vector3(5.31, 3.19, 5.06) * 4.0f, 0);
	AddCubeToWorld(Vector3(41.50, -1.94, -20.88) * 8.0f, Vector3(2.90, 0.26, 3.65) * 4.0f, 0);
	AddOBBCubeToWorld(Vector3(109.07, -3.74, 7.99) * 8.0f, Vector3(1.95, 53.96, 79.31) * 4.0f, Quaternion(0.00000, 0.00000, 0.70711, -0.70711), 0);
	AddOBBCubeToWorld(Vector3(82.55, -0.99, -27.42) * 8.0f, Vector3(3.52, 7.47, 1.12) * 4.0f, Quaternion(0.50000, -0.50000, -0.50000, 0.50000), 0);
	AddCubeToWorld(Vector3(136.57, -1.10, 8.58) * 8.0f, Vector3(0.79, 4.08, 80.56) * 4.0f, 0);
	AddCubeToWorld(Vector3(109.68, -0.16, 3.97) * 8.0f, Vector3(32.26, 5.22, 4.86) * 4.0f, 0);
	AddCubeToWorld(Vector3(108.75, -1.19, 48.11) * 8.0f, Vector3(54.99, 4.19, 0.69) * 4.0f, 0);
	AddCubeToWorld(Vector3(96.14, -1.41, 32.21) * 8.0f, Vector3(8.22, 2.72, 5.04) * 4.0f, 0);
	AddCubeToWorld(Vector3(106.80, -1.20, 19.48) * 8.0f, Vector3(5.49, 3.13, 6.39) * 4.0f, 0);
	AddCubeToWorld(Vector3(104.97, -0.15, 42.44) * 8.0f, Vector3(7.33, 5.23, 6.04) * 4.0f, 0);
	AddOBBCubeToWorld(Vector3(88.84, 0.27, 16.89) * 8.0f, Vector3(5.82, 11.63, 11.03) * 4.0f, Quaternion(0.00000, 0.00000, 0.70711, -0.70711), 0);
	AddCubeToWorld(Vector3(132.34, -0.99, -15.16) * 8.0f, Vector3(7.27, 3.55, 18.68) * 4.0f, 0);
	AddCubeToWorld(Vector3(115.53, -0.43, -9.81) * 8.0f, Vector3(8.70, 4.67, 4.85) * 4.0f, 0);
	AddCubeToWorld(Vector3(126.05, -0.61, 23.45) * 8.0f, Vector3(8.13, 4.31, 5.81) * 4.0f, 0);
}

void TutorialGame::LoadDynamic() {
	Vector3 swingBallPositions[] = {
		Vector3(-110.593, 35.5, -60.8067),
		Vector3(-109.784, 35.5, -4.3544),
		Vector3(-112.006, 35.5, 45.8838)
	};
	//Vector3 leftside = Vector3(-137.364, -17.9602, 21.0267);
	//Vector3 rightside = Vector3(-79.292, -12.1535, 14.8348);
	//Vector3 direction = Vector::Normalise(rightside - leftside);
	//std::cout << "Direction for first bit: " << direction << std::endl;
	Vector3 direction = Vector3(0.989485f, 0.09894f, -0.105503f);
	for(Vector3 pos : swingBallPositions)
	{
		AddSwingBallToWorld(pos,40.0f, 15.0f, 0.01, direction, 4000.0f);
		direction = -direction;
	}

	Vector3 floatingBoxPositions[] = {
	Vector3(426.586, 6.6364, -153.317),
	Vector3(468.895, 6.6364, -207.777),
	Vector3(530.329, 6.6364, -185.664)
	};
	for (Vector3 pos : floatingBoxPositions) {
		AddFloatingBoxToWorld(pos, Vector3(2.0f, 2.0f, 2.0f));
	}

	AddPlayerToWorld(Vector3(-118.747, 70.8767, 286.553));
	
}


void TutorialGame::InitCamera() {
	world.GetMainCamera().SetNearPlane(0.1f);
	world.GetMainCamera().SetFarPlane(2000.0f);
	world.GetMainCamera().SetPitch(-15.0f);
	world.GetMainCamera().SetYaw(315.0f);
	world.GetMainCamera().SetPosition(Vector3(-60, 40, 60));
	world.GetMainCamera().SetPlayer(world.GetPlayer());
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world.ClearAndErase();
	physics.Clear();
	//Need to spawn player at -135.822, 121.127, 340.848

	//CreatedMixedGrid(15, 15, 3.5f, 3.5f);

	//InitGameExamples();

	//AddFloorToWorld(Vector3(0, -20, 0));

	//BridgeConstraintTest();
	
	LoadLevel();
	LoadDynamic();

	testStateGameObject = AddStateObjectToWorld(Vector3(0, 10, 0));
}
/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume(volume);
	floor->GetTransform()
		.SetScale(floorSize * 2.0f)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(floor->GetTransform(), cubeMesh, checkerMaterial));
	floor->SetPhysicsObject(new PhysicsObject(floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, bool hollow) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume(volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(sphere->GetTransform(), sphereMesh, checkerMaterial));
	sphere->SetPhysicsObject(new PhysicsObject(sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	if (hollow) {
		sphere->GetPhysicsObject()->InitHollowSphereInertia();
	}
	else {
		sphere->GetPhysicsObject()->InitSphereInertia();
	}
	world.AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, bool heavy) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume(volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, checkerMaterial));
	cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));
	if (heavy) {
		inverseMass /= 10.0f;
	}

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, Quaternion rotation, float inverseMass) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume(volume);

	cube->GetTransform().SetPosition(position)
		.SetScale(dimensions * 2.0f)
		.SetOrientation(rotation);
	
	cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, checkerMaterial));
	cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 5.0f;
	float inverseMass	= 0.5f;

	PlayerObject* character = new PlayerObject();
	SphereVolume* volume  = new SphereVolume(0.8f);

	character->SetBoundingVolume(volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(character->GetTransform(), catMesh, notexMaterial));
	character->SetPhysicsObject(new PhysicsObject(character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(character);
	world.SetPlayer(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume(volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(character->GetTransform(), enemyMesh, notexMaterial));
	character->SetPhysicsObject(new PhysicsObject(character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume(volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(apple->GetTransform(), bonusMesh, glassMaterial));
	apple->SetPhysicsObject(new PhysicsObject(apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(apple);

	return apple;
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::CreateSphereGrid(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::CreatedMixedGrid(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				if (rand() % 4 == 0) {
					AddCubeToWorld(position, cubeDims, 1.0f, true);
				}
				else {
					AddCubeToWorld(position, cubeDims);
				}
			}
			else {
				if (rand() % 3 == 0) {
					AddSphereToWorld(position, sphereRadius, 5.0f, true);
				}
				else {
					AddSphereToWorld(position, sphereRadius, 5.0f);
				}
			}
		}
	}
}

void TutorialGame::CreateAABBGrid(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		Debug::Print("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::Left)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(world.GetMainCamera());

			RayCollision closestCollision;
			if (world.Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;

				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
		if (Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}
		}
	}
	else {
		Debug::Print("Press Q to change to select mode!", Vector2(5, 85));
	}
	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::Right)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(world.GetMainCamera());

		RayCollision closestCollision;
		if (world.Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view = world.GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = Matrix::Inverse(view);

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);

	if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
		selectionObject->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
		selectionObject->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::NEXT)) {
		selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void TutorialGame::PlayerMovement() {
	Matrix4 view = world.GetMainCamera().BuildViewMatrix();
	Matrix4 camWorld = Matrix::Inverse(view);
	PlayerObject* player = world.GetPlayer();
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis = Vector::Normalise(fwdAxis);
	Vector3 lookDir = fwdAxis;
	fwdAxis *= 10.0f;
	rightAxis *= 10.0f;

	if (Window::GetKeyboard()->KeyDown(KeyCodes::W)) {
		Vector3 playerPos = player->GetTransform().GetPosition();
		Matrix4 temp = Matrix::View(playerPos + lookDir, playerPos, Vector3(0, 1, 0));  //Make a view matrix looking in the move direction
		Matrix4 modelMat = Matrix::Inverse(temp);  //Make the model matrix in model space
		Quaternion q(modelMat);   //Extract the rotation
		player->GetTransform().SetOrientation(q.Normalised());
		player->GetPhysicsObject()->AddForce(fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::S)) {
		player->GetPhysicsObject()->AddForce(-fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyCodes::A)) {
		player->GetPhysicsObject()->AddForce(-rightAxis);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::D)) {
		player->GetPhysicsObject()->AddForce(rightAxis);
	}
	if (Window::GetKeyboard()->KeyDown(KeyCodes::SPACE) && player->CanJump()) {
		float inverseMass = player->GetPhysicsObject()->GetInverseMass();
		player->GetPhysicsObject()->AddForce(Vector3(0, 1750, 0));
		player->SetJumpCooldown(0.5f);
	}
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyCodes::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::BridgeConstraintTest() {

	Vector3 cubeSize = Vector3(6, 6, 6);

	float invCubeMass = 5;
	int numLinks = 10;
	float maxDistance = 30;
	float cubeDistance = 15;

	Vector3 startPos = Vector3(0, 135, 0);

	GameObject* start = AddCubeToWorld(startPos, cubeSize, 0.0f); //immovable start anchor
	GameObject* end = AddCubeToWorld(startPos+ Vector3((numLinks+2)*cubeDistance,0, 0), cubeSize, 0.0f) ;

	GameObject* previous = start;


	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);

		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world.AddConstraint(constraint);
		previous = block;
	}

	PositionConstraint* endConstraint = new PositionConstraint(previous, end, maxDistance);
	world.AddConstraint(endConstraint);


}


StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume(volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(apple->GetTransform(), bonusMesh, glassMaterial));
	apple->SetPhysicsObject(new PhysicsObject(apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world.AddGameObject(apple);

	return apple;
}

/// <summary>
/// Adds a swinging ball to the world at a given position, distance from pivot, radius, inverse mass, direction and push force
/// </summary>
/// <param name="position">The position the anchor will be spawned</param>
/// <param name="distance">The distance between the ball and the anchor</param>
/// <param name="radius">Radius of the ball</param>
/// <param name="inverseMass">Mass of the ball</param>
/// <param name="direction">Direction the ball will be pushed</param>
/// <param name="pushForce">The strength of the push force given to ball</param>
/// <returns></returns>
GameObject* TutorialGame::AddSwingBallToWorld(const Vector3& position, float distance, float radius, float inverseMass, Vector3 direction, float pushForce) 
{
	GameObject* anchor = AddCubeToWorld(position, Vector3(2, 2, 2), 0.0f); //Anchor cube

	GameObject* ball = AddSphereToWorld(Vector3(position.x, position.y - distance, position.z), radius, inverseMass);

	SwingBall* construct = new SwingBall(ball, anchor, distance, direction, pushForce);
	world.AddGameObject(construct);
	world.AddConstraint(construct->GetConstraint());
	return construct;
}

GameObject* TutorialGame::AddFloatingBoxToWorld(const Vector3& position, Vector3 dimensions) 
{
	FloatingBox* cube = new FloatingBox();

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume(volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2.0f);

	cube->SetRenderObject(new RenderObject(cube->GetTransform(), cubeMesh, checkerMaterial));
	cube->SetPhysicsObject(new PhysicsObject(cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(0);
	cube->GetPhysicsObject()->InitCubeInertia();

	world.AddGameObject(cube);

	return cube;

	//Could just steal the code from the add cube.
	/*GameObject* box = AddCubeToWorld(position, dimensions, 0.0f);
	FloatingBox* construct = new FloatingBox(box);

	world.AddGameObject(construct);
	return construct;*/
}