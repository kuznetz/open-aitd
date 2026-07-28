#pragma once
#include <string>
#include "../../../raylib-cpp/raylib-cpp.h"

namespace openAITD {

class ButtonWidget {
public:
    bool active = true;
    bool selected = false;
    raylib::Rectangle bounds;

    ButtonWidget(const raylib::Font& font, raylib::Rectangle bounds, const std::string& text)
        : font(font)
        , bounds(bounds)
        , text(text)
    {
    }

    void draw() const {
        if (text.empty()) return;

        raylib::Color color;
        if (!active) {
            color = inactiveColor;
        } else if (selected) {
            float t = (1.0f - cosf(2.0f * PI * blinkTimer / blinkPeriod)) * 0.5f;
            color = lerpColor(selectedColor, selectedColor2, t);
        } else {
            color = normalColor;
        }

        float fontSize = font.baseSize;
        float spacing = 1.0f;
        raylib::Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, spacing);
        float x = bounds.x + (bounds.width - textSize.x) * 0.5f;
        float y = bounds.y + (bounds.height - textSize.y) * 0.5f;

        DrawTextEx(font, text.c_str(), { x, y }, fontSize, spacing, color);
    }

    void process(float timeDelta) {
        if (!active) return;
        blinkTimer += timeDelta;
        if (blinkTimer > blinkPeriod * 2) blinkTimer -= blinkPeriod * 2;
    }

private:
    const raylib::Font& font;
    std::string text;

    raylib::Color normalColor   = raylib::WHITE;
    raylib::Color inactiveColor = raylib::GRAY;
    raylib::Color selectedColor = raylib::GOLD;
    raylib::Color selectedColor2 = raylib::YELLOW;

    mutable float blinkTimer = 0.0f;
    static constexpr float blinkPeriod = 2.0f;

    void resetBlink() {
        blinkTimer = 0.0f;
    }

    static raylib::Color lerpColor(const raylib::Color& c1, const raylib::Color& c2, float t) {
        return raylib::Color{
            static_cast<uint8_t>(c1.r + (c2.r - c1.r) * t),
            static_cast<uint8_t>(c1.g + (c2.g - c1.g) * t),
            static_cast<uint8_t>(c1.b + (c2.b - c1.b) * t),
            static_cast<uint8_t>(c1.a + (c2.a - c1.a) * t)
        };
    }
};

} // namespace openAITD