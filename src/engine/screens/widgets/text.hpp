#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "../../../raylib-cpp/raylib-cpp.h"

class TextWidget {
public:
    raylib::Rectangle bounds;
    raylib::Color color = raylib::WHITE;    

    TextWidget(const raylib::Font& font, raylib::Rectangle bounds, float spacing = 2.0f)
      : font(font), bounds(bounds), spacing(spacing), scrollOffset(0) {}

    void setLines(const std::vector<std::string>& lines) {
        wrappedLines = lines;
        clampScroll();
    }

    void setText(const std::string& text) {
        wrappedLines.clear();
        size_t start = 0, end;
        while ((end = text.find('\n', start)) != std::string::npos) {
            wrappedLines.push_back(text.substr(start, end - start));
            start = end + 1;
        }
        if (start < text.length()) {
            wrappedLines.push_back(text.substr(start));
        }
        clampScroll();
    }

    void scrollUp() {
        if (scrollOffset > 0) { --scrollOffset; }
    }
    void scrollDown() {
        int maxOffset = std::max(0, static_cast<int>(wrappedLines.size()) - getVisibleLines());
        if (scrollOffset < maxOffset) { ++scrollOffset; }
    }

    void setScrollOffset(int offset) {
        scrollOffset = std::max(0, offset);
        clampScroll();
    }

    int getScrollOffset() const { return scrollOffset; }
    int getTotalLines() const { return static_cast<int>(wrappedLines.size()); }
    int getVisibleLines() const {
        return static_cast<int>(bounds.height / (font.baseSize + spacing));
    }

    void draw() const {
        if (wrappedLines.empty()) return;
        int visible = getVisibleLines();
        if (visible <= 0) return;
        float lineHeight = font.baseSize + spacing;
        float startY = bounds.y;
        for (int i = 0; i < visible && (scrollOffset + i) < (int)wrappedLines.size(); ++i) {
            int idx = scrollOffset + i;
            const std::string& line = wrappedLines[idx];
            float yPos = startY + i * lineHeight;
            DrawTextEx(font, line.c_str(), { bounds.x, yPos }, font.baseSize, 1.0f, color);
        }
    }

    void process(float timeDelta) {}

private:
    const raylib::Font& font;
    std::vector<std::string> wrappedLines;
    int scrollOffset;
    float spacing;

    void clampScroll() {
        int maxOffset = std::max(0, static_cast<int>(wrappedLines.size()) - getVisibleLines());
        if (scrollOffset > maxOffset) scrollOffset = maxOffset;
        if (scrollOffset < 0) scrollOffset = 0;
    }
};