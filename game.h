#pragma once
#include <types.h>
#include <cmath>
class BSTimer {
public:
	UInt8		ucDisableCounter;
	float		fClamp;
	float		fClampRemainder;
	float		fDelta;
	UInt32		uiLastTime;
	UInt32		uiFirstTime;
	bool		bIsChangeTimeMultSlowly;
	bool		bSameFrameDelta;

	void Update(UInt32 auiTime);
};

static_assert(sizeof(BSTimer) == 0x1C);

class BGSSaveLoadGame {
public:
	char		padding[0x244];
	UInt32		uiGlobalFlags;
	UInt8		ucCurrentMinorVersion;

	static BGSSaveLoadGame* GetSingleton() {
		return *reinterpret_cast<BGSSaveLoadGame**>(0x1079858);
	}
	bool IsLoading() const;
};

static_assert(sizeof(BGSSaveLoadGame) == 0x24C);

class StartMenu {
public:
	char		padding[0x1A0];
	UInt32	uiFlags;

	static StartMenu* GetSingleton() {
		return *reinterpret_cast<StartMenu**>(0x10770AC);
	}

	bool GetSettingsChanged() const;

	static void SaveSettings();
};

static_assert(offsetof(StartMenu, uiFlags) == 0x1A0);


bool IsInPauseFade();

float GetGlobalTimeMultiplier();

bool IsLoadingNewGame();