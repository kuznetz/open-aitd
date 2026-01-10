#pragma once
#include <string>
namespace AITDExtractor {
	void processStages();
	void processModels();
	void processScripts(bool floppy);
	void processSounds();
	void processTracks();
	void processAdlibMusic();
	void extractAllData(bool floppy);
}
