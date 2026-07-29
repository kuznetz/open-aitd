#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "../../../raylib-cpp/raylib-cpp.h"

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
        float lineHeight = font.baseSize + spacing;
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
        bool lastLine = false;
    };

    struct Page {
        std::vector<Line> lines;
    };

    const raylib::Font& font;
    raylib::Rectangle bounds;
    float spacing;
    std::string rawText;
    std::vector<Token> tokens;
    std::vector<Page> pages;
    int currentPage;

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
                float w = MeasureTextEx(font, word.c_str(), font.baseSize, 1.0f).x;
                tokens.push_back({ Token::WORD, word, w + 3.0f, 0 });
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
            pages.push_back(std::move(page));
        }
    }

    // Builds one page, starting from token idx.
    // Returns true if this is the last page (end of text reached).
    bool buildPage(size_t& idx, Page& page) {
        float maxWidth = bounds.width;
        float lineHeight = font.baseSize + spacing;
        float currentY = 0.0f;
        bool endOfText = false;

        while (currentY + lineHeight <= bounds.height && !endOfText) {
            Line line;
            bool lineFinished = false;
            bool pageBreak = false;
            bool forceNewline = false; // true if the line is terminated due to NEWLINE
            float totalWidth = 0.0f;
            bool centered = false;

            // Collect line
            while (!lineFinished && !pageBreak && !endOfText && idx < tokens.size()) {
                const Token& tok = tokens[idx];

                if (tok.type == Token::COMMAND) {
                    switch (tok.command) {
                        case 'P': { // Page break
                            if (currentY > 0) { // not the first line on page
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
                            float w = MeasureTextEx(font, tabStr.c_str(), font.baseSize, 1.0f).x + 3.0f;
                            if (totalWidth + w <= maxWidth) {
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
                if (totalWidth + tok.width <= maxWidth) {
                    line.words.push_back({ tok.text, tok.width });
                    totalWidth += tok.width;
                    ++idx;
                } else {
                    // Word doesn't fit – line finished, token remains for next line
                    lineFinished = true;
                }
            }

            // End of tokens reached – last page
            if (idx >= tokens.size()) {
                endOfText = true;
                lineFinished = true;
            }

            // Add the line if it has words or a forced newline
            if (!line.words.empty() || forceNewline) {
                line.centered = centered;
                line.lastLine = endOfText; // last line on page (or entire text) – do not justify
                page.lines.push_back(line);
                currentY += lineHeight;
            }

            // If page break occurred – finish page
            if (pageBreak) break;
        }

        // If end of text reached and page is empty – skip it
        if (page.lines.empty() && endOfText) {
            return true;
        }

        return endOfText;
    }

    void drawLine(const Line& line, float y) const {
        if (line.words.empty()) return; // empty line – draw nothing

        float totalWidth = 0.0f;
        for (const auto& w : line.words) totalWidth += w.width;

        float startX;
        if (line.centered) {
            startX = bounds.x + (bounds.width - totalWidth) / 2.0f;
        } else if (!line.lastLine && line.words.size() > 1) {
            // Justify (full width alignment)
            float extraSpace = (bounds.width - totalWidth) / (line.words.size() - 1);
            float x = bounds.x;
            for (size_t i = 0; i < line.words.size(); ++i) {
                const auto& w = line.words[i];
                DrawTextEx(font, w.text.c_str(), {x, y}, font.baseSize, 1.0f, color);
                x += w.width + extraSpace;
            }
            return;
        } else {
            // Left alignment
            startX = bounds.x;
        }

        // Regular drawing (centered or last line)
        float x = startX;
        for (const auto& w : line.words) {
            DrawTextEx(font, w.text.c_str(), {x, y}, font.baseSize, 1.0f, color);
            x += w.width;
        }
    }
};

} // namespace openAITD