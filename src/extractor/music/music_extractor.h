#pragma once
#include <string>

namespace AITDExtractor {
  void renderMusic(const void* musicData, const std::string outFilename, const bool ogg = true, const float quality = 0.8f);
}