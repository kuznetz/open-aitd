#pragma once
#include <map>
#include <string>
#include <memory>

namespace AITDExtractor {
    using namespace std;

    class IAITDExtractor {
    public:
        virtual ~IAITDExtractor() = default;
        virtual void processStages() = 0;
        virtual void processModels() = 0;
        virtual void processScripts(bool floppy) = 0;
        virtual void processTracks() = 0;
        virtual void processSounds() = 0;
        virtual void processAdlibMusic() = 0;
        virtual void processTexts() = 0;
        virtual void processPictures() = 0;
        virtual void extractAllData(bool floppy) = 0;
   };

   std::unique_ptr<IAITDExtractor> createAITDExtractor();
}