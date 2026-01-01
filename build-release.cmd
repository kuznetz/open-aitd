mkdir build-release
cmake -S . -B build-release -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release --target ALL_BUILD --parallel
cmake --install build-release --prefix "./dist"