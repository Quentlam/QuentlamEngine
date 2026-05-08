#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

namespace Quentlam {

    // ---------------------------------------------------------
    // Event Graph System (Zero-Code Visual Scripting)
    // ---------------------------------------------------------

    enum class PinType { Exec, Bool, Int, Float, String, Vector2, EntityID };

    struct NodePin {
        std::string Name;
        PinType Type;
        bool IsInput;
        std::string ConnectedToNodeID;
        std::string ConnectedToPinName;
        
        // Default values for disconnected pins
        std::string DefaultValueStr;
        float DefaultValueFloat = 0.0f;
    };

    class EventNode {
    public:
        std::string NodeID;
        std::string Title;
        std::vector<NodePin> Inputs;
        std::vector<NodePin> Outputs;

        virtual void Execute() = 0;
        virtual ~EventNode() = default;
    };

    // Example: On Input Event
    class InputEventNode : public EventNode {
    public:
        InputEventNode() {
            Title = "On Key Pressed";
            Outputs.push_back({"Exec", PinType::Exec, false, "", ""});
            Outputs.push_back({"KeyCode", PinType::Int, false, "", ""});
        }
        void Execute() override {
            // Trigger connected exec nodes
        }
    };

    // Example: Play Audio Node
    class PlayAudioNode : public EventNode {
    public:
        PlayAudioNode() {
            Title = "Play Audio";
            Inputs.push_back({"Exec", PinType::Exec, true, "", ""});
            Inputs.push_back({"SoundFile", PinType::String, true, "", ""});
            Outputs.push_back({"Exec", PinType::Exec, false, "", ""});
        }
        void Execute() override {
            // Read SoundFile pin, play audio, trigger next exec
        }
    };

    // Example: Add Score Node
    class AddScoreNode : public EventNode {
    public:
        AddScoreNode() {
            Title = "Add Score";
            Inputs.push_back({"Exec", PinType::Exec, true, "", ""});
            Inputs.push_back({"Amount", PinType::Int, true, "", "", "1"});
            Outputs.push_back({"Exec", PinType::Exec, false, "", ""});
        }
        void Execute() override {
            // Update ScoreSystem component
        }
    };


    // ---------------------------------------------------------
    // Script Hook System (Lua/TypeScript)
    // ---------------------------------------------------------
    
    struct NativeScriptComponent {
        std::string ScriptPath;
        
        // Runtime
        void* Instance = nullptr;

        // Function hooks
        std::function<void()> InstantiateFunction;
        std::function<void()> DestroyFunction;
        std::function<void(float)> OnUpdateFunction;

        NativeScriptComponent() = default;
        NativeScriptComponent(const NativeScriptComponent&) = default;
        
        void Bind(const std::string& path) {
            ScriptPath = path;
            // Load Lua/TS script, map lifecycle hooks
        }
    };

}
