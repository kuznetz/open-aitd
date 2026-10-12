#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "../../../common/raylib_cpp.hpp"

namespace openAITD {

class VerticalMenuWidget {
public:
    // Элемент меню: текст для отображения и произвольный id,
    // по которому вызывающий код определяет действие.
    struct Item {
        int id = 0;
        std::string text;
    };

    bool active = true;
    raylib::Rectangle bounds;

    VerticalMenuWidget(const raylib::Font& font, raylib::Rectangle bounds, int maxVisible = 5) :
      font(font),
      bounds(bounds),
      maxVisibleItems(maxVisible), 
      selectedIndex(0),
      scrollOffset(0),
      blinkTimer(0.0f) {}

    // Совместимость: элементы без явных id (id = порядковый номер).
    void setItems(const std::vector<std::string>& newItems) {
        std::vector<Item> newMenuItems;
        newMenuItems.reserve(newItems.size());
        for (size_t i = 0; i < newItems.size(); ++i) {
            newMenuItems.push_back({ static_cast<int>(i), newItems[i] });
        }
        setItemsExt(newMenuItems);
    }

    // Устанавливает элементы с явными идентификаторами.
    void setItemsExt(const std::vector<Item>& newItems) {
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

    // id выбранного элемента или -1, если список пуст.
    int getSelectedId() const {
        if (selectedIndex < 0 || selectedIndex >= static_cast<int>(items.size())) return -1;
        return items[selectedIndex].id;
    }

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
            const std::string& text = items[idx].text;
            float textWidth = MeasureTextEx(font, text.c_str(), font.baseSize, 1.0f).x;
            float xPos = bounds.x + (bounds.width - textWidth) * 0.5f;
            DrawTextEx(font, text.c_str(), { xPos, yPos }, font.baseSize, 1.0f, color);
        }

        drawScrollbar(startY, totalHeight);
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
    const raylib::Color scrollbarColor = raylib::WHITE;
    const raylib::Color scrollbarTrackColor = raylib::Color { 0, 0, 0, 80 };
    static constexpr float scrollbarWidth = 6.0f;
    static constexpr float scrollbarMargin = 4.0f;
    std::vector<Item> items;
    int selectedIndex;
    int scrollOffset;
    int maxVisibleItems;

    // Blink state for smooth transition
    float blinkTimer = 0.0f;
    static constexpr float blinkPeriod = 1.0f;

    // Draw a white scrollbar thumb on the right edge when items don't fit on screen.
    void drawScrollbar(float startY, float totalHeight) const {
        const int itemCount = static_cast<int>(items.size());
        if (itemCount <= maxVisibleItems) return;
        if (totalHeight <= 0.0f) return;

        const float x = bounds.x + bounds.width - scrollbarWidth - scrollbarMargin;

        // Track
        DrawRectangle(static_cast<int>(x), static_cast<int>(startY),
                      static_cast<int>(scrollbarWidth), static_cast<int>(totalHeight),
                      scrollbarTrackColor);

        // Thumb size proportional to the visible fraction of the list
        float thumbHeight = totalHeight * static_cast<float>(maxVisibleItems) / static_cast<float>(itemCount);
        if (thumbHeight < scrollbarWidth) thumbHeight = scrollbarWidth;

        const int maxOffset = itemCount - maxVisibleItems;
        const float scrollT = (maxOffset > 0) ? static_cast<float>(scrollOffset) / static_cast<float>(maxOffset) : 0.0f;
        const float thumbY = startY + (totalHeight - thumbHeight) * scrollT;

        DrawRectangle(static_cast<int>(x), static_cast<int>(thumbY),
                      static_cast<int>(scrollbarWidth), static_cast<int>(thumbHeight),
                      scrollbarColor);
    }

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