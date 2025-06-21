#pragma once
#include "ModuleMain.h"
#include "Menu.h"
#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool LoadTextureFromFile(const char* file_name, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);
void loadCharacterClickButton();
void activateAction(const characterData& charData, CInstance* parentInstance, const std::string& actionName);

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
void AttackDestroyBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);