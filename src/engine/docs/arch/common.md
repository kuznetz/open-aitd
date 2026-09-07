# Architectural Documentation: `src/common`

## `src/common/euler_angles.hpp`
**Role:**  
Provides a high-level abstraction for 3D rotational data, extending the base vector type to handle Euler angle normalization and comparison across the engine.

**Components:**
*   **`EulerAngles`**: Represents a 3D rotation using Euler angles by inheriting from the engine's base `Vector3` type. It standardizes how rotations are normalized and compared, ensuring consistent angular mathematics throughout the application.

---

## `src/common/metrics.hpp`
**Role:**  
Defines global transformation matrices and scaling constants used to convert between internal engine coordinates and external model or room spaces.

**Components:**
*   **`Vector3i`**: A lightweight integer-based 3D vector structure used for discrete coordinate representations and fixed-point rotation values.
*   **`Metrics`**: A utility class holding static transformation matrices and scaling factors that standardize coordinate and rotation conversions between different subsystems.

---

## `src/common/name_collection.hpp`
**Role:**  
Manages the loading and retrieval of human-readable names mapped to integer identifiers from JSON configuration files, decoupling raw IDs from display logic.

**Components:**
*   **`NameCollection`**: Handles the parsing of JSON files into an internal mapping of integer IDs to string names. It provides a consistent interface for name resolution and fallback naming conventions for unmapped identifiers.

---

## `src/common/name_decoders.hpp`
**Role:**  
Aggregates multiple `NameCollection` instances to provide a unified interface for decoding various game entity types, such as variables, lifespans, models, objects, and console variables.

**Components:**
*   **`NameDecoders`**: Orchestrates the initialization and loading of distinct name collections for different engine subsystems. It centralizes asset and entity naming resolution, allowing other modules to query readable names without managing multiple file loads.

---

## `src/common/raylib_cpp.hpp`
**Role:**  
Acts as a C++ namespace wrapper for the Raylib graphics library, integrating core graphics, math, and camera headers while providing custom matrix operations tailored to the engine's needs.

**Components:**
*   **`raylib` (Namespace)**: Encapsulates all Raylib dependencies (`raylib.h`, `raymath.h`, etc.) and introduces custom static matrix transformation utilities. It serves as the foundational bridge between the C-based graphics library and the C++ engine architecture.

---

## Module Relationships & Dependencies

### Internal Dependencies (`src/common`)
*   **`raylib_cpp.hpp`** is the foundational dependency for the math modules. It exposes the `raylib` namespace and core types like `Vector3` and `Matrix`.
*   **`euler_angles.hpp`** depends directly on `raylib_cpp.hpp` to extend the `Vector3` type into a domain-specific rotation class.
*   **`metrics.hpp`** depends on both `raylib_cpp.hpp` (for matrix operations) and `euler_angles.hpp` (for rotation conversions), acting as a bridge between raw math and engine-specific coordinate systems.
*   **`name_decoders.hpp`** depends on `name_collection.hpp`, instantiating multiple collections to manage different categories of game entities.

### External Dependencies
*   **Graphics & Math Stack**: `raylib_cpp.hpp` interfaces with the external **Raylib** library. `euler_angles.hpp` and `metrics.hpp` propagate these types upward to rendering, physics, and animation modules that require standardized 3D transformations.
*   **Configuration & I/O**: `name_collection.hpp` relies on the **nlohmann/json** library and standard C++ I/O streams. It decouples the engine's core logic from file parsing by providing a clean API for external subsystems (like UI, debuggers, or asset loaders) to resolve integer IDs into human-readable strings.
*   **Game Logic Integration**: `name_decoders.hpp` is typically consumed by higher-level game state managers, script interpreters, or debugging tools that need to translate raw engine data (e.g., model IDs, variable codes) into meaningful context without hardcoding names.