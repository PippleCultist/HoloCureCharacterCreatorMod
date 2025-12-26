#pragma once
#include "ModuleMain.h"
#include "Menu.h"
#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool LoadTextureFromFile(const char* file_name, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
void loadCharacterClickButton();

struct pinVariableData
{
	// Variable to check if the pin data has ever been cached
	bool isCached = false;
	// TODO: Should add an int option
	// TODO: For some reason, putting an RValue here breaks the mod loading? Investigate later
	// Result storage for number
	double pinNumberResult = 0;
	// Result storage for instance
	CInstance* pinInstanceResult;
	// Result storage for projectile data
	projectileData pinProjDataResult;
	// Result storage for boolean data
	bool pinBoolDataResult = false;
	// Result storage for string data
	std::string pinStringResult;
	RValue stringGMLHash;
	bool isStringGMLHashSet = false;
	// Result storage for int data
	int pinIntegerResult = 0;
	// Result storage for RValue data
	RValue pinRValueResult;
	// Result storage for buff data
	buffData pinBuffDataResult;
	// Result storage for DS List data
	RValue pinDSListResult;
	// Result storage for DS Map data
	RValue pinDSMapResult;
	// Result storage for Array data
	RValue pinArrayResult;
	// Result storage for sound data
	soundData pinSoundDataResult;
};

struct characterDataStruct
{
	double portraitObjIndex;
	double largePortraitObjIndex;
	double idleObjIndex;
	double runObjIndex;
	double petObjIndex;
	
	std::string attackID;
	double attackIconObjIndex;
	std::string attackNameString;
	RValue attackDesc;

	double specialIconObjIndex;
	std::string specialID;
	std::string specialNameString;
	std::string specialDescString;
	double specialCD;

	std::string perksStringArr[3];
};

void CharacterDataCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void CharSelectCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void CharSelectDrawBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void CharSelectDrawAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void TitleScreenDrawAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void AttackControllerCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void AttackControllerOther11After(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void PlayerManagerOther22After(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void AttackCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void AttackStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void AttackDestroyBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void PlayerStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void PlayerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void BaseMobStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);