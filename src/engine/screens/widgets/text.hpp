#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "../../../common/raylib_cpp.hpp"

namespace openAITD {

using namespace raylib;

class TextWidget {
public:
    TextWidget(const raylib::Font& font, raylib::Rectangle bounds, float spacing = 4.0f)
        : font(font), bounds(bounds), spacing(spacing), currentPage(0) {}

    void setBounds(raylib::Rectangle new_bounds) {
        bounds = new_bounds;
    }

    void setText(const std::string& text) {
        rawText = text;
        tokenize();
        buildPages();
        currentPage = 0;
    }

    void nextPage() {
        if (currentPage < (int)pages.size() - 1) ++currentPage;
    }
    void prevPage() {
        if (currentPage > 0) --currentPage;
    }

    int getCurrentPage() const { return currentPage; }
    int getPageCount() const { return (int)pages.size(); }
    bool hasNext() const { return currentPage < (int)pages.size() - 1; }
    bool hasPrev() const { return currentPage > 0; }

    void draw() const {
        if (pages.empty() || currentPage >= pages.size()) return;
        const Page& page = pages[currentPage];
        // Line spacing is computed per page so the lines fill bounds.height evenly.
        float lineHeight = page.lineHeight > 0.0f ? page.lineHeight : (font.baseSize + spacing);
        float y = bounds.y;
        for (const Line& line : page.lines) {
            drawLine(line, y);
            y += lineHeight;
        }
    }

    raylib::Color color = raylib::BLACK;

private:
    struct Token {
        enum Type { WORD, COMMAND, NEWLINE };
        Type type;
        std::string text;
        float width;
        char command;     // for COMMAND – 'P', 'C', 'T', 'G'
    };

    struct Word {
        std::string text;
        float width;
    };

    struct Line {
        std::vector<Word> words;
        bool centered = false;
    };

    struct Page {
        std::vector<Line> lines;
        float lineHeight = 0.0f; // computed after the page is built: bounds.height / lines.size()
    };

    const raylib::Font& font;
    raylib::Rectangle bounds;
    float spacing;
    std::string rawText;
    std::vector<Token> tokens;
    std::vector<Page> pages;
    int currentPage;

    // Inter-word gap limits, expressed as coefficients of the natural space
    // width (the width of the ' ' glyph in the current font).
    static constexpr float MIN_SPACE = 0.5f;
    static constexpr float MAX_SPACE = 8.0f;

    void tokenize() {
        tokens.clear();
        const char* p = rawText.c_str();
        while (*p) {
            // Handle command
            if (*p == '#') {
                ++p;
                if (*p) {
                    char cmd = *p++;
                    tokens.push_back({ Token::COMMAND, std::string(1, cmd), 0.0f, cmd });
                }
                continue;
            }

            // Skip spaces and tabs (word separators)
            if (*p == ' ' || *p == '\t') {
                ++p;
                continue;
            }

            // Handle newline
            if (*p == '\n' || *p == '\r') {
                if (*p == '\r' && *(p + 1) == '\n') {
                    p += 2; // skip \r\n
                } else {
                    ++p;
                }
                tokens.push_back({ Token::NEWLINE, "", 0.0f, 0 });
                continue;
            }

            // Read a word until a delimiter or command
            const char* start = p;
            while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '#') {
                ++p;
            }
            if (p > start) {
                std::string word(start, p - start);
                // Store the raw glyph width; inter-word gaps are applied separately.
                float w = MeasureTextEx(font, word.c_str(), font.baseSize, 1.0f).x;
                tokens.push_back({ Token::WORD, word, w, 0 });
            }
        }
    }

    void buildPages() {
        pages.clear();
        if (tokens.empty()) return;

        size_t idx = 0;
        bool lastPage = false;
        while (!lastPage && idx < tokens.size()) {
            Page page;
            lastPage = buildPage(idx, page);
            // A trailing '#P' leaves the last page empty – do not add it.
            if (page.lines.empty() && lastPage) break;
            // Interline spacing depends on how many lines the page actually has.
            // It may shrink to fit the lines, but must never exceed the reference
            // spacing (font.baseSize + spacing).
            float reference = font.baseSize + spacing;
            size_t lineCount = page.lines.size();
            float computed = lineCount > 0 ? bounds.height / (float)lineCount : reference;
            page.lineHeight = std::min(computed, reference);
            pages.push_back(std::move(page));
        }
    }

    // Width of the natural space glyph (' ') in the current font.
    float naturalSpace() const {
        float w = MeasureTextEx(font, " ", font.baseSize, 1.0f).x;
        return w > 0.0f ? w : 1.0f;
    }

    // Builds one page, starting from token idx.
    // A page is terminated strictly by the '#P' tag (or by the end of text) –
    // vertical overflow is no longer used to split pages.
    // Returns true if this is the last page (end of text reached).
    bool buildPage(size_t& idx, Page& page) {
        float maxWidth = bounds.width;
        float baseSpace = naturalSpace();
        float minSpace = MIN_SPACE * baseSpace;
        bool endOfText = false;
        bool pageBreak = false;

        // A word fits while the resulting inter-word gap stays >= MIN_SPACE;
        // otherwise the word is wrapped to the next line.
        auto fitsOnLine = [&](float sum, size_t count, float w) -> bool {
            size_t newCount = count + 1;
            float newSum = sum + w;
            if (newCount <= 1) return newSum <= maxWidth;
            return (maxWidth - newSum) / (float)(newCount - 1) >= minSpace;
        };

        while (!endOfText && !pageBreak) {
            Line line;
            bool lineFinished = false;
            bool forceNewline = false; // true if the line is terminated due to NEWLINE
            float totalWidth = 0.0f;
            bool centered = false;
            size_t lineStartIdx = idx;

            // Collect line
            while (!lineFinished && !pageBreak && !endOfText && idx < tokens.size()) {
                const Token& tok = tokens[idx];

                if (tok.type == Token::COMMAND) {
                    switch (tok.command) {
                        case 'P': { // Page break
                            if (!page.lines.empty()) { // not the first line on page
                                pageBreak = true;
                                lineFinished = true;
                            } else {
                                // #P at the beginning of a page – ignore
                                ++idx;
                            }
                            break;
                        }
                        case 'C': { // Center line
                            centered = true;
                            ++idx;
                            break;
                        }
                        case 'T': { // Tab – insert two spaces
                            std::string tabStr = "  ";
                            float w = MeasureTextEx(font, tabStr.c_str(), font.baseSize, 1.0f).x;
                            if (fitsOnLine(totalWidth, line.words.size(), w)) {
                                line.words.push_back({ tabStr, w });
                                totalWidth += w;
                            } else {
                                // Does not fit – wrap to next line
                                lineFinished = true;
                            }
                            ++idx; // #T always consumed
                            break;
                        }
                        case 'G': { // Ignored (not implemented)
                            ++idx;
                            break;
                        }
                        default:
                            ++idx;
                            break;
                    }
                    continue;
                }

                if (tok.type == Token::NEWLINE) {
                    // Force line break
                    forceNewline = true;
                    lineFinished = true;
                    ++idx;
                    break;
                }

                // Normal word
                if (fitsOnLine(totalWidth, line.words.size(), tok.width)) {
                    line.words.push_back({ tok.text, tok.width });
                    totalWidth += tok.width;
                    ++idx;
                } else {
                    // Word doesn't fit – line finished, token remains for next line
                    lineFinished = true;
                }
            }

            // Guarantee progress: a single word wider than the page never advances
            // idx on its own, which would otherwise cause an infinite loop.
            if (!pageBreak && !endOfText && idx == lineStartIdx
                && idx < tokens.size() && tokens[idx].type == Token::WORD) {
                line.words.push_back({ tokens[idx].text, tokens[idx].width });
                ++idx;
            }

            // Add the line if it has words or a forced newline
            if (!line.words.empty() || forceNewline) {
                line.centered = centered;
                page.lines.push_back(line);
            }

            // Only a page break ends the page; otherwise continue until tokens run out.
            if (pageBreak) break;
            if (idx >= tokens.size()) endOfText = true;
        }

        // If end of text reached and page is empty – skip it
        if (page.lines.empty() && endOfText) {
            return true;
        }

        return endOfText;
    }

    void drawLine(const Line& line, float y) const {
        if (line.words.empty()) return; // empty line – draw nothing

        float baseSpace = naturalSpace();
        float minSpace = MIN_SPACE * baseSpace;
        float maxSpace = MAX_SPACE * baseSpace;

        size_t count = line.words.size();
        float totalWidth = 0.0f;
        for (const auto& w : line.words) totalWidth += w.width;

        // Centered lines ignore the gap rule and use the natural spacing.
        if (line.centered) {
            float full = totalWidth + (count > 1 ? (count - 1) * baseSpace : 0.0f);
            float x = bounds.x + (bounds.width - full) / 2.0f;
            for (const auto& w : line.words) {
                DrawTextEx(font, w.text.c_str(), {x, y}, font.baseSize, 1.0f, color);
                x += w.width + baseSpace;
            }
            return;
        }

        // Gap between words:
        //   space < MIN_SPACE -> cramped, clamp up (wrapping is done while building);
        //   space > MAX_SPACE -> too stretched, fall back to the natural space;
        //   otherwise         -> use the computed gap (justify).
        float gap = baseSpace;
        if (count > 1) {
            float space = (bounds.width - totalWidth) / (float)(count - 1);
            if (space > maxSpace) {
                gap = baseSpace;
            } else if (space < minSpace) {
                gap = minSpace;
            } else {
                gap = space;
            }
        }

        float x = bounds.x;
        for (const auto& w : line.words) {
            DrawTextEx(font, w.text.c_str(), {x, y}, font.baseSize, 1.0f, color);
            x += w.width + gap;
        }
    }
};

} // namespace openAITD