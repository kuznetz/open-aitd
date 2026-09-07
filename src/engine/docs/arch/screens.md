# Architectural Documentation: `src/engine/screens`

## `src/engine/screens/book_screen.h`
**Role:**  
Renders an in-game book-reading overlay, displaying paginated text from `BookData` with background theming based on book type.

**Components:**
*   **`BookType`**: A structure defining the visual layout (background picture ID and text bounds) for a specific book category.
*   **`bookTypes`**: A compile-time array of `BookType` entries for `READ_MESSAGE`, `READ_BOOK`, and `READ_CARNET`, providing layout constants for each book variant.
*   **`BookScreen`**: Manages the book-reading UI state, wrapping a `TextWidget` for paginated text display. It synchronizes with `World::bookData` to reload content when the active text changes and handles keyboard navigation (pages, exit).

---

## `src/engine/screens/char_select_screen.h`
**Role:**  
Presents a character model selection interface, allowing the player to toggle between alternative character models before gameplay begins.

**Components:**
*   **`CharSelectScreen`**: Displays two side-by-side character model previews with selection highlighting. It updates `World::altModels` and `GameObject::altModels` upon confirmation, ensuring the chosen model variant propagates to all game objects.

---

## `src/engine/screens/console_screen.h`
**Role:**  
Provides a debug console for runtime inspection and manipulation of game state, enabling developers to query objects, modify variables, and navigate the world.

**Components:**
*   **`ConsoleScreen`**: A command-line interface that parses text input into tokens and dispatches to internal commands (`J` for jump, `T` for take, `O` for object info, `V` for variables, `S` for screen shake, `L` for lighting). It renders a scrollable history buffer and an input line directly via Raylib primitives.

---

## `src/engine/screens/found_screen.h`
**Role:**  
Displays a transitional "new item found" overlay, rendering a rotating 3D preview of the discovered item and allowing the player to accept or discard it.

**Components:**
*   **`FoundScreen`**: Captures the found item ID, renders its 3D model in a dedicated render texture with continuous rotation, and presents "Leave"/"Take" choices. On submission, it either adds the item to the inventory or schedules a timeout to discard it.

---

## `src/engine/screens/inventory_screen.h`
**Role:**  
Manages the player's inventory UI, providing item browsing, 3D model preview, and action selection with animated transitions.

**Components:**
*   **`InventoryScreen`**: Composes two `VerticalMenuWidget` instances for item listing and action selection. It renders a rotating 3D preview of the selected item in a viewport, animates menu appearance/disappearance, and writes the chosen item and action back to `World::curInvGObject` and `World::curInvAction`.

---

## `src/engine/screens/menu_screen.h`
**Role:**  
Serves as the central pause/main menu hub, delegating to sub-screens for saves, options, and game flow control.

**Components:**
*   **`MenuScreenResult`**: An enumeration of possible menu outcomes (`resume`, `newGame`, `saveGame`, `loadGame`, `exit`), consumed by the game loop to transition state.
*   **`MenuScreenState`**: An enumeration tracking the active menu sub-view (`main`, `save`, `load`, `options`).
*   **`MenuScreen`**: Orchestrates the main menu lifecycle, composing `VerticalMenuWidget`, `SavesScreen`, and `OptionsScreen` as sub-views. It routes keyboard input based on the current state and exposes `MenuScreenResult` and `saveSlot` for the caller to act upon.

---

## `src/engine/screens/options_screen.h`
**Role:**  
Provides a configuration interface for display settings, persisting changes to the engine's `Config`.

**Components:**
*   **`ScreenRes`**: A lightweight structure holding width and height for a supported resolution.
*   **`resolutions`**: A global array of `ScreenRes` defining the available resolution presets.
*   **`OptionsScreen`**: Renders a `VerticalMenuWidget` listing current settings (fullscreen, resolution, 3D scale, FPS display). It maintains a `newConfig` copy of `Config`, applies changes immediately via `saveConfig()`, and reloads the menu labels to reflect updates.

---

## `src/engine/screens/picture_screen.h`
**Role:**  
Renders a temporary full-screen picture overlay with an automatic timeout, used for cutscenes or narrative images.

**Components:**
*   **`PictureScreen`**: Draws a picture from `Resources::backgrounds` keyed by `World::picture.id`. It accumulates elapsed time and clears the picture ID when the configured delay expires, causing the overlay to disappear.

---

## `src/engine/screens/saves_screen.h`
**Role:**  
Handles save/load slot selection with screenshot previews, interfacing with the `SaveHelper` for persistence operations.

**Components:**
*   **`SavesScreen::Mode`**: An enumeration distinguishing between `Save` and `Load` modes, which affects the menu items and slot selection logic.
*   **`SavesScreen`**: Displays a `VerticalMenuWidget` of save slots alongside a screenshot preview. It delegates slot listing, deletion, and path resolution to `SaveHelper`, and exposes `isComplete()` and `getSelectedSlot()` for the parent `MenuScreen` to consume the result.

---

## `src/engine/screens/widgets/button.hpp`
**Role:**  
Provides a simple, state-aware text button widget with animated selection highlighting.

**Components:**
*   **`ButtonWidget`**: Renders a single text label within a rectangular bounds, cycling between normal, inactive, and selected colors. The selected state uses a cosine-based blink animation for visual feedback.

---

## `src/engine/screens/widgets/text.hpp`
**Role:**  
Implements a paginated, formatted text display widget with support for page breaks, centering, tabs, and justification.

**Components:**
*   **`TextWidget`**: Parses raw text into tokens (words, commands, newlines), lays them out into justified or centered lines, and paginates based on bounds. It supports `#P` (page break), `#C` (center), `#T` (tab), and `#G` (ignored) directives, and provides navigation (`nextPage`/`prevPage`) for multi-page content.

---

## `src/engine/screens/widgets/vertical_menu.hpp`
**Role:**  
Implements a scrollable vertical list widget with animated selection highlighting, used as the primary navigation primitive across all screens.

**Components:**
*   **`VerticalMenuWidget`**: Manages a list of string items with a single selection index, automatic scroll offset calculation, and a cosine-based blink animation for the selected item. It supports activation/deactivation states and exposes `moveUp`/`moveDown` for keyboard navigation.

---

## Module Relationships & Dependencies

### Internal Dependencies (`src/engine/screens`)
*   **Widget Layer**: `button.hpp`, `text.hpp`, and `vertical_menu.hpp` form the foundational UI widget layer. They depend only on `raylib_cpp.hpp` and have no internal screen dependencies.
*   **Screen Composition**: `MenuScreen` composes `SavesScreen` and `OptionsScreen` as sub-views, delegating their input processing and rendering. `InventoryScreen` and `MenuScreen` both use `VerticalMenuWidget`. `BookScreen` uses `TextWidget`.
*   **Shared Patterns**: All screen classes follow a consistent interface: a constructor accepting `World*` or `World&`, a `process(float)` method for per-frame updates, a `render()` method for drawing, and keyboard input handling.

### External Dependencies
*   **`src/engine/world`**: Every screen depends on `World` for game state access (inventory, objects, variables, book data, picture state) and on `SaveHelper` (used by `SavesScreen`).
*   **`src/engine/resources`**: All screens depend on `Resources` for textures, fonts, models, backgrounds, configuration, and text localization.
*   **`src/common`**: All screens include `raylib_cpp.hpp` for rendering primitives, math types (`Rectangle`, `Vector2`, `Camera3D`), and the `raylib` namespace.
*   **Screen Lifecycle**: Screens are not managed by a formal stack or registry within this directory. The game loop (outside this module) instantiates, drives `process()`/`render()`, and destroys screens based on game state transitions.