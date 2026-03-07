#include <game.h>
#include <calls.h>

void BSTimer::Update(UInt32 auiTime) {

	ThisCall(0x86C280, this, auiTime);
}

bool BGSSaveLoadGame::IsLoading() const {
	return (uiGlobalFlags & 2) != 0;
}

bool StartMenu::GetSettingsChanged() const {
	return (uiFlags & 2) != 0;
}

void StartMenu::SaveSettings() {
	CdeclCall(0x6806F0);
}

bool IsInPauseFade() {
	return *reinterpret_cast<bool*>(0x107A0F5);
}

float GetGlobalTimeMultiplier()
{
	return *reinterpret_cast<float*>(0xF7544C);
}

bool IsLoadingNewGame() {
	return *reinterpret_cast<bool*>(0x1075A07);
}