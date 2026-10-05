#pragma once

#include <array>
#include <vector>
#include <cmath>
#include "../../common/raylib_cpp.hpp"

namespace openAITD {

enum class InputAction {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Sprint,
    Interact,
    OpenInventory,
    OpenMenu,
    ToggleFreelook,
    TogglePause,
    ToggleConsole,
    SpeedHack,
    UiUp,
    UiDown,
    UiLeft,
    UiRight,
    UiSelect,
    UiBack,
    Count   // всегда последним
};

struct InputBinding {
    enum class Type { Keyboard, GamepadButton, GamepadAxis };

    Type type;
    int id;
    int gamepadIndex = 0;
    float threshold = 0.5f;
    bool inverted = false;

    static InputBinding fromKey(int key) {
        return {Type::Keyboard, key, 0, 0.5f, false};
    }
    static InputBinding fromGamepadButton(int button, int gamepad = 0) {
        return {Type::GamepadButton, button, gamepad, 0.5f, false};
    }
    static InputBinding fromGamepadAxis(int axis, int gamepad = 0, float threshold = 0.5f, bool inverted = false) {
        return {Type::GamepadAxis, axis, gamepad, threshold, inverted};
    }
};

class InputManager {
public:
    InputManager() = default;
    ~InputManager() = default;

    void init();

    bool isActionDown(InputAction action) const;
    bool isActionPressed(InputAction action) const;
    bool isActionReleased(InputAction action) const;

    bool isInteracting() const;
    bool isSprinting() const;

    raylib::Vector2 GetMoveVector() const;

    void bindAction(InputAction action, const InputBinding& binding);
    void bindAction(InputAction action, const std::vector<InputBinding>& bindings);
    std::vector<InputBinding> getBindings(InputAction action) const;

    void update();

private:
    static constexpr size_t ACTION_COUNT = static_cast<size_t>(InputAction::Count);
    std::array<std::vector<InputBinding>, ACTION_COUNT> bindings;
    std::array<bool, ACTION_COUNT> currentState{};
    std::array<bool, ACTION_COUNT> previousState{};

    void updateActionStates();
    bool checkBinding(const InputBinding& binding) const;
    float getAxisValue(const InputBinding& binding) const;
};

inline void InputManager::init() {
    bindAction(InputAction::MoveForward,   InputBinding::fromKey(KEY_UP));
    bindAction(InputAction::MoveBackward,  InputBinding::fromKey(KEY_DOWN));
    bindAction(InputAction::MoveLeft,      InputBinding::fromKey(KEY_LEFT));
    bindAction(InputAction::MoveRight,     InputBinding::fromKey(KEY_RIGHT));
    bindAction(InputAction::Sprint,        InputBinding::fromKey(KEY_LEFT_SHIFT));
    bindAction(InputAction::Interact,      InputBinding::fromKey(KEY_SPACE));
    bindAction(InputAction::OpenInventory, InputBinding::fromKey(KEY_ENTER));
    bindAction(InputAction::OpenMenu,      InputBinding::fromKey(KEY_ESCAPE));
    bindAction(InputAction::ToggleFreelook, InputBinding::fromKey(KEY_O));
    bindAction(InputAction::TogglePause,    InputBinding::fromKey(KEY_P));
    bindAction(InputAction::ToggleConsole,  InputBinding::fromKey(KEY_GRAVE));
    bindAction(InputAction::SpeedHack,      InputBinding::fromKey(KEY_I));
    bindAction(InputAction::UiUp,     InputBinding::fromKey(KEY_UP));
    bindAction(InputAction::UiDown,   InputBinding::fromKey(KEY_DOWN));
    bindAction(InputAction::UiLeft,   InputBinding::fromKey(KEY_LEFT));
    bindAction(InputAction::UiRight,  InputBinding::fromKey(KEY_RIGHT));
    bindAction(InputAction::UiSelect, InputBinding::fromKey(KEY_ENTER));
    bindAction(InputAction::UiBack,   InputBinding::fromKey(KEY_ESCAPE));

    // bindAction(InputAction::MoveForward, InputBinding::fromGamepadAxis(GAMEPAD_AXIS_LEFT_Y, 0, 0.2f, true));
}

inline void InputManager::bindAction(InputAction action, const InputBinding& binding) {
    bindings[static_cast<size_t>(action)].push_back(binding);
}

inline void InputManager::bindAction(InputAction action, const std::vector<InputBinding>& bindingsList) {
    bindings[static_cast<size_t>(action)] = bindingsList;
}

inline std::vector<InputBinding> InputManager::getBindings(InputAction action) const {
    return bindings[static_cast<size_t>(action)];
}

inline bool InputManager::checkBinding(const InputBinding& binding) const {
    switch (binding.type) {
        case InputBinding::Type::Keyboard:
            return raylib::IsKeyDown(binding.id);
        case InputBinding::Type::GamepadButton:
            return raylib::IsGamepadButtonDown(binding.gamepadIndex, binding.id);
        case InputBinding::Type::GamepadAxis: {
            float val = raylib::GetGamepadAxisMovement(binding.gamepadIndex, binding.id);
            if (binding.inverted) val = -val;
            return std::abs(val) > binding.threshold;
        }
    }
    return false;
}

inline float InputManager::getAxisValue(const InputBinding& binding) const {
    if (binding.type != InputBinding::Type::GamepadAxis) return 0.0f;
    float val = raylib::GetGamepadAxisMovement(binding.gamepadIndex, binding.id);
    if (binding.inverted) val = -val;
    return val;
}

inline void InputManager::update() {
    previousState = currentState;
    currentState.fill(false);
    updateActionStates();
}

inline void InputManager::updateActionStates() {
    for (size_t i = 0; i < ACTION_COUNT; ++i) {
        bool any = false;
        for (const auto& binding : bindings[i]) {
            if (checkBinding(binding)) {
                any = true;
                break;
            }
        }
        currentState[i] = any;
    }
}

inline bool InputManager::isActionDown(InputAction action) const {
    return currentState[static_cast<size_t>(action)];
}

inline bool InputManager::isActionPressed(InputAction action) const {
    size_t idx = static_cast<size_t>(action);
    return currentState[idx] && !previousState[idx];
}

inline bool InputManager::isActionReleased(InputAction action) const {
    size_t idx = static_cast<size_t>(action);
    return !currentState[idx] && previousState[idx];
}

inline bool InputManager::isInteracting() const {
    return isActionDown(InputAction::Interact);
}

inline bool InputManager::isSprinting() const {
    return isActionDown(InputAction::Sprint);
}

inline raylib::Vector2 InputManager::GetMoveVector() const {
    float x = 0.0f, y = 0.0f;

    auto processAction = [&](InputAction action, float dx, float dy) {
        const auto& vec = bindings[static_cast<size_t>(action)];
        for (const auto& binding : vec) {
            switch (binding.type) {
                case InputBinding::Type::Keyboard:
                    if (raylib::IsKeyDown(binding.id)) {
                        x += dx;
                        y += dy;
                    }
                    break;
                case InputBinding::Type::GamepadAxis: {
                    float val = getAxisValue(binding);
                    x += val * dx;
                    y += val * dy;
                    break;
                }
                case InputBinding::Type::GamepadButton:
                    if (raylib::IsGamepadButtonDown(binding.gamepadIndex, binding.id)) {
                        x += dx;
                        y += dy;
                    }
                    break;
            }
        }
    };

    processAction(InputAction::MoveLeft,   -1.0f,  0.0f);
    processAction(InputAction::MoveRight,   1.0f,  0.0f);
    processAction(InputAction::MoveForward, 0.0f,  1.0f);
    processAction(InputAction::MoveBackward,0.0f, -1.0f);

    float len = std::sqrt(x*x + y*y);
    if (len > 1.0f) {
        x /= len;
        y /= len;
    }
    return raylib::Vector2{x, y};
}

} // namespace openAITD