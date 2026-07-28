#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "../../../raylib-cpp/raylib-cpp.h"

namespace openAITD {

class VerticalMenuWidget {
public:
    bool active = true;
    raylib::Rectangle bounds;

    VerticalMenuWidget(const raylib::Font& font, raylib::Rectangle bounds, int maxVisible = 5) :
      font(font),
      bounds(bounds),
      maxVisibleItems(maxVisible), 
      selectedIndex(0),
      scrollOffset(0),
      blinkTimer(0.0f) {}

    void setItems(const std::vector<std::string>& newItems) {
        items = newItems;
        if (items.size() < selectedIndex) {
            selectedIndex = 0;
            scrollOffset = 0;
        }

        clampSelection();
        resetBlink();
    }

    void setSelectedIndex(int idx) {
        if (idx >= 0 && idx < static_cast<int>(items.size())) {
            selectedIndex = idx;
            updateScroll();
            resetBlink();
        }
    }

    int getSelectedIndex() const { return selectedIndex; }

    void moveUp() {
        if (selectedIndex > 0) {
            --selectedIndex;
            updateScroll();
            resetBlink();
        }
    }

    void moveDown() {
        if (selectedIndex < static_cast<int>(items.size()) - 1) {
            ++selectedIndex;
            updateScroll();
            resetBlink();
        }
    }

    void draw() const {
        if (items.empty()) return;

        const float lineHeight = font.baseSize;
        int visibleCount = std::min(maxVisibleItems, static_cast<int>(items.size()) - scrollOffset);
        if (visibleCount <= 0) return;

        float totalHeight = visibleCount * lineHeight;
        float startY = bounds.y + (bounds.height - totalHeight) * 0.5f;

        float t = 0.0f;
        if (active && !items.empty()) {
            float phase = 2.0f * PI * blinkTimer / blinkPeriod;
            t = (1.0f - cosf(phase)) * 0.5f; // from 0 to 1 and back
        }

        for (int i = 0; i < visibleCount; ++i) {
            int idx = scrollOffset + i;
            const bool selected = (idx == selectedIndex);
            raylib::Color color;
            if (!active) {
                color = inactiveColor;
            } else if (selected) {
                color = lerpColor(selectedColor, selectedColor2, t);
            } else {
                color = normalColor;
            }

            float yPos = startY + i * lineHeight;
            const std::string& text = items[idx];
            float textWidth = MeasureTextEx(font, text.c_str(), font.baseSize, 1.0f).x;
            float xPos = bounds.x + (bounds.width - textWidth) * 0.5f;
            DrawTextEx(font, text.c_str(), { xPos, yPos }, font.baseSize, 1.0f, color);
        }
    }

    void process(float timeDelta) {
        if (!active) return;
        blinkTimer += timeDelta;
        if (blinkTimer > blinkPeriod * 2) blinkTimer -= blinkPeriod * 2;
    }

private:
    const raylib::Font& font;
    const raylib::Color inactiveColor = raylib::GRAY;
    const raylib::Color normalColor = raylib::WHITE;
    const raylib::Color selectedColor = raylib::GOLD;
    const raylib::Color selectedColor2 = raylib::YELLOW;
    std::vector<std::string> items;
    int selectedIndex;
    int scrollOffset;
    int maxVisibleItems;

    // Blink state for smooth transition
    float blinkTimer = 0.0f;
    static constexpr float blinkPeriod = 2.0f;

    void clampSelection() {
        if (selectedIndex >= static_cast<int>(items.size()))
            selectedIndex = std::max(0, static_cast<int>(items.size()) - 1);
    }

    void updateScroll() {
        if (selectedIndex < scrollOffset)
            scrollOffset = selectedIndex;
        else if (selectedIndex >= scrollOffset + maxVisibleItems)
            scrollOffset = selectedIndex - maxVisibleItems + 1;

        scrollOffset = std::max(0, scrollOffset);
        int maxOffset = std::max(0, static_cast<int>(items.size()) - maxVisibleItems);
        if (scrollOffset > maxOffset) scrollOffset = maxOffset;
    }

    void resetBlink() {
        blinkTimer = 0.0f;
    }

    static raylib::Color lerpColor(const raylib::Color& c1, const raylib::Color& c2, float t) {
        return raylib::Color {
            (uint8_t)(c1.r + (c2.r - c1.r) * t),
            (uint8_t)(c1.g + (c2.g - c1.g) * t),
            (uint8_t)(c1.b + (c2.b - c1.b) * t),
            (uint8_t)(c1.a + (c2.a - c1.a) * t)
        };
    }
};

}