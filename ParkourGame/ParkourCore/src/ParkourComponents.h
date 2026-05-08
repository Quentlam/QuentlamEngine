#pragma once

#include <glm/glm.hpp>
#include <string>

// Instead of #include "ParticleSystem.h" which causes build errors,
// we just declare simple particle data structs directly here so it works 
// within the ECS framework.

namespace Quentlam {

    struct SimpleParticle
    {
        bool Active = false;
        glm::vec2 Position;
        float Rotation;
        glm::vec2 Velocity;
        glm::vec4 ColorBegin;
        glm::vec4 ColorEnd;
        float SizeBegin;
        float SizeEnd;
        float LifeTime;
        float LifeRemaining;
    };

    struct SimpleParticleProps
    {
        glm::vec2 Velocity = { 0.0f, 0.0f };
        glm::vec2 Position = { 0.0f, 0.0f };
        glm::vec2 VelocityVariation = { 2.0f, 1.0f };
        glm::vec4 ColorBegin = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 ColorEnd = { 1.0f, 1.0f, 1.0f, 1.0f };
        float SizeVariation = 0.0f;
        float SizeBegin = 0.5f;
        float SizeEnd = 0.0f;
        float LifeTime = 1.0f;
    };

    struct SimpleParticleSystemComponent
    {
        std::vector<SimpleParticle> ParticlePool;
        uint32_t PoolIndex = 0;

        SimpleParticleSystemComponent()
        {
            ParticlePool.resize(100);
        }
    };

    // 1. PlayerController Component
    struct PlayerControllerComponent {
        glm::vec2 StartPosition = {0.0f, 0.0f};
        glm::vec2 StartVelocity = {5.0f, 0.0f};
        float EnginePower = 1.0f;
        float MaxVelocityY = 20.0f;
        float RotationSpeedSlow = 240.0f; // Increased base slow rotation
        float RotationSpeedFast = 600.0f; // Increased base fast rotation

        // Used for runtime tracking
        float VisualRotation = 0.0f;
        
        SimpleParticleProps EngineParticle;

        PlayerControllerComponent()
        {
            EngineParticle.ColorBegin = { 254 / 255.0f, 212 / 255.0f, 123 / 255.0f, 1.0f };
            EngineParticle.ColorEnd = { 254 / 255.0f, 109 / 255.0f, 41 / 255.0f, 1.0f };
            EngineParticle.SizeBegin = 0.5f;
            EngineParticle.SizeVariation = 0.3f;
            EngineParticle.SizeEnd = 0.0f;
            EngineParticle.LifeTime = 0.5f;
            EngineParticle.Velocity = { 0.0f, 0.0f };
            EngineParticle.VelocityVariation = { 2.0f, 1.0f };
            EngineParticle.Position = { 0.0f, 0.0f };
        }
        PlayerControllerComponent(const PlayerControllerComponent&) = default;
    };

    // 2. ObstacleSpawner Component
    struct ObstacleSpawnerComponent {
        int PoolSize = 20;
        float StartXOffset = 20.0f;
        float SpacingX = 10.0f;
        float MoveSpeedX = -5.0f;
        float MinGap = 8.0f;
        float MaxGap = 14.0f;
        float CenterVariation = 6.0f;

        // Used for runtime tracking
        float CurrentXTarget = 0.0f;

        ObstacleSpawnerComponent() = default;
        ObstacleSpawnerComponent(const ObstacleSpawnerComponent&) = default;
    };

    // 3. PhysicsMaterial Component
    struct PhysicsMaterialComponent {
        float Friction = 0.3f;
        float Restitution = 0.0f;
        float GravityScale = 4.0f;
        float LinearDamping = 0.1f;
        float AngularDamping = 0.3f;
        float Mass = 1.0f;

        PhysicsMaterialComponent() = default;
        PhysicsMaterialComponent(const PhysicsMaterialComponent&) = default;
    };

    // 4. ScoreSystem Component
    struct ScoreSystemComponent {
        int PointsPerObstacle = 1;
        float ScoreMultiplier = 1.0f;
        int CurrentScore = 0;
        int HighScore = 0;

        ScoreSystemComponent() = default;
        ScoreSystemComponent(const ScoreSystemComponent&) = default;
    };

    // 5. AudioBank Component
    struct AudioBankComponent {
        std::string JumpSound = "assets/audio/jump.wav";
        std::string ScoreSound = "assets/audio/score.wav";
        std::string GameOverSound = "assets/audio/gameover.wav";
        float SpatialAttenuation = 1.0f; // 3D spatial attenuation factor

        AudioBankComponent() = default;
        AudioBankComponent(const AudioBankComponent&) = default;
    };

    // 6. UI Flow Component
    struct UIFlowComponent {
        enum class State { MainMenu, Playing, Paused, GameOver };
        State CurrentState = State::Playing; // Change default to Playing

        std::string StartScreenWidget = "UI_Start";
        std::string PauseScreenWidget = "UI_Pause";
        std::string GameOverWidget = "UI_GameOver";
        std::string HUDWidget = "UI_HUD";

        UIFlowComponent() = default;
        UIFlowComponent(const UIFlowComponent&) = default;
    };
}
