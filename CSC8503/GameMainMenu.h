#pragma once
#include "PushdownState.h"
#include "Vector.h"
#include "Debug.h"
#include "Window.h"

// Forward declarations for engine types in their namespaces
namespace NCL {
    namespace CSC8503 {
        class GameWorld;
        class PhysicsSystem;
        class GameTechRenderer;
        class TutorialGame;
    }
}

class InGame; // kept in global namespace

class GameMainMenu : public NCL::CSC8503::PushdownState
{
public:
    GameMainMenu(NCL::CSC8503::GameWorld* world, NCL::CSC8503::PhysicsSystem* physics, NCL::CSC8503::GameTechRenderer* renderer, float Width, float Height) :
        gameWorld(world), physics(physics), renderer(renderer),
        menuWidth(Width), menuHeight(Height) {
    }

    NCL::CSC8503::PushdownState::PushdownResult OnUpdate(float dt, NCL::CSC8503::PushdownState** pushFunc) override;
protected:
    NCL::CSC8503::GameWorld* gameWorld;
    NCL::CSC8503::PhysicsSystem* physics;
    NCL::CSC8503::GameTechRenderer* renderer;
    float menuWidth;
    float menuHeight;
};