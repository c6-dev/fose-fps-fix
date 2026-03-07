#include <PluginAPI.h>
#include <fps.h>
extern "C" {
	__declspec(dllexport) bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info) {
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "HighFpsFix";
		info->version = 120;
		return true;
	}

	__declspec(dllexport) bool FOSEPlugin_Load(FOSEInterface* fose) {
		if (!fose->isEditor) {
			WritePatches();
		}
		return true;
	}
}
