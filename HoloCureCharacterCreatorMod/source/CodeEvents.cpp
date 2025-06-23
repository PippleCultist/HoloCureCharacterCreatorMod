#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "CommonFunctions.h"
#include "CodeEvents.h"
#include "ScriptFunctions.h"
#include "Menu.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "stb_image/stb_image.h"
#include <random>
#include <queue>

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern YYGML_PushContextStack yyGMLPushContextStack;
extern YYGML_YYSetScriptRef yyGMLYYSetScriptRef;
extern YYGML_PopContextStack yyGMLPopContextStack;
extern RValue lastStructVarGetName;
extern std::string playingCharName;
extern std::unordered_map<std::string, std::vector<buffData>> buffDataListMap;
extern std::unordered_map<std::string, std::vector<actionData>> actionDataListMap;
extern std::unordered_map<std::string, std::vector<projectileData>> projectileDataListMap;
extern int curFrameNum;

extern HWND gameWindow;

// Data
static ID3D11Device*			g_pd3dDevice = nullptr;
static ID3D11DeviceContext*		g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*			g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*	g_mainRenderTargetView = nullptr;

ImGuiIO io;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

extern PFUNC_YYGMLScript origCanSubmitScoreScript;
extern PFUNC_YYGMLScript origExecuteAttackScript;

struct charSpriteData
{
	std::shared_ptr<spriteData> portraitSpritePtr;
	std::shared_ptr<spriteData> largePortraitSpritePtr;
	std::shared_ptr<spriteData> idleSpritePtr;
	std::shared_ptr<spriteData> runSpritePtr;
	std::shared_ptr<spriteData> attackIconPtr;
	std::shared_ptr<spriteData> attackAwakenedIconPtr;
	std::shared_ptr<spriteData> attackAnimationPtr;
	std::shared_ptr<spriteData> specialIconPtr;
	std::shared_ptr<spriteData> specialAnimationPtr;
	std::vector<std::shared_ptr<spriteData>> skillIconPtrList;
	std::unordered_map<std::string, std::shared_ptr<spriteData>> buffIconPtrMap;
	std::unordered_map<std::string, std::shared_ptr<spriteData>> projectileAnimationPtrMap;
};

struct actionQueueData
{
	CInstance* parentInstance;
	std::string triggeredActionName;

	actionQueueData(CInstance* parentInstance, std::string triggeredActionName) : parentInstance(parentInstance), triggeredActionName(triggeredActionName)
	{
	}
};

struct actionQueueDataComp
{
	bool operator()(
		std::pair<int, actionQueueData> &a,
		std::pair<int, actionQueueData> &b
		)
	{
		return a.first > b.first;
	}
};

std::unordered_map<std::string, charSpriteData> charSpriteMap;
std::unordered_map<std::string, characterData> charDataMap;
std::vector<std::string> charNameList;
std::unordered_map<std::string, characterDataStruct> characterDataMap;
std::priority_queue<std::pair<int, actionQueueData>, std::vector<std::pair<int, actionQueueData>>, actionQueueDataComp> actionQueue;

bool hasBackedUpCharacterList = false;
bool isInCharSelectDraw = false;

int rhythmLeftButtonIndex = 100000;
int rhythmRightButtonIndex = 100001;

int charSelectPage = 0;

// Simple helper function to load an image into a DX11 texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
	// Load from disk into a raw RGBA buffer
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create texture
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = image_width;
	desc.Height = image_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D* pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	*out_width = image_width;
	*out_height = image_height;
	stbi_image_free(image_data);

	return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char* file_name, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
	FILE* f = fopen(file_name, "rb");
	if (f == NULL)
		return false;
	fseek(f, 0, SEEK_END);
	size_t file_size = (size_t)ftell(f);
	if (file_size == -1)
		return false;
	fseek(f, 0, SEEK_SET);
	void* file_data = IM_ALLOC(file_size);
	fread(file_data, 1, file_size, f);
	fclose(f);
	bool ret = LoadTextureFromMemory(file_data, file_size, out_srv, out_width, out_height);
	IM_FREE(file_data);
	return ret;
}

void CharacterDataCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	RValue characterData = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterData" });
	RValue characterKeyArr = g_ModuleInterface->CallBuiltin("ds_map_keys_to_array", { characterData });
	int characterKeyArrLen = g_ModuleInterface->CallBuiltin("array_length", { characterKeyArr }).ToInt32();
	for (int i = 0; i < characterKeyArrLen; i++)
	{
		std::string charIdString = characterKeyArr[i].ToString();
		RValue curStruct = g_ModuleInterface->CallBuiltin("ds_map_find_value", { characterData, charIdString.c_str() });
		characterDataStruct tempStruct;
		RValue perks = getInstanceVariable(curStruct, GML_perks);
		if (perks.m_Kind != VALUE_OBJECT)
		{
			continue;
		}

		RValue perksNameArr = g_ModuleInterface->CallBuiltin("variable_struct_get_names", { perks });
		for (int j = 0; j < 3; j++)
		{
			tempStruct.perksStringArr[j] = perksNameArr[j].ToString();
		}

		tempStruct.portraitObjIndex = getInstanceVariable(curStruct, GML_port).ToDouble();
		RValue largePort = getInstanceVariable(curStruct, GML_large_port);
		if (largePort.m_Kind != VALUE_UNDEFINED)
		{
			tempStruct.largePortraitObjIndex = largePort.ToDouble();
		}
		tempStruct.idleObjIndex = getInstanceVariable(curStruct, GML_sprite1).ToDouble();
		tempStruct.runObjIndex = getInstanceVariable(curStruct, GML_sprite2).ToDouble();
		RValue sprite3 = getInstanceVariable(curStruct, GML_sprite3);
		if (sprite3.m_Kind != VALUE_UNDEFINED)
		{
			tempStruct.petObjIndex = sprite3.ToDouble();
		}

		tempStruct.attackID = getInstanceVariable(curStruct, GML_attackID).ToString();
		tempStruct.attackIconObjIndex = getInstanceVariable(curStruct, GML_attackIcon).ToDouble();
		tempStruct.attackNameString = getInstanceVariable(curStruct, GML_attack).ToString();
		tempStruct.attackDesc = getInstanceVariable(curStruct, GML_attackDesc);

		tempStruct.specialIconObjIndex = getInstanceVariable(curStruct, GML_specIcon).ToDouble();
		tempStruct.specialID = getInstanceVariable(curStruct, GML_specID).ToString();
		tempStruct.specialNameString = getInstanceVariable(curStruct, GML_specName).ToString();
		tempStruct.specialDescString = getInstanceVariable(curStruct, GML_specDesc).ToString();
		tempStruct.specialCD = getInstanceVariable(curStruct, GML_specCD).ToDouble();
		
		characterDataMap[charIdString] = tempStruct;
	}
}

void CharSelectCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	charSelectPage = 0;
	RValue characterDataMapRValue = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterData" });
	characterData charData;
	if (!std::filesystem::exists("CharacterCreatorMod"))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the CharacterCreatorMod directory");
		g_ModuleInterface->Print(CM_RED, "Couldn't find the CharacterCreatorMod directory");
		return;
	}

	RValue characterList = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterList" });
	charSpriteMap.clear();
	charDataMap.clear();
	charNameList.clear();

	for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod"))
	{
		auto path = dir.path();
		if (path.filename().string().compare(0, 5, "char_") == 0)
		{
			if (loadCharacterData(path.filename().string(), charData))
			{
				std::string dirStr = "CharacterCreatorMod/" + path.filename().string() + "/";
				charSpriteData curCharSpriteData;
				curCharSpriteData.attackIconPtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackIconFileName, charData.attackIconFileName, 1));
				curCharSpriteData.specialIconPtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.specialIconFileName, charData.specialIconFileName, 1));
				curCharSpriteData.portraitSpritePtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.portraitFileName, charData.portraitFileName, 1));
				curCharSpriteData.largePortraitSpritePtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.largePortraitFileName, charData.largePortraitFileName, 1));
				curCharSpriteData.idleSpritePtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.idleAnimationFileName, charData.idleAnimationFileName, getSpriteNumFrames(charData.idleAnimationFileName)));
				curCharSpriteData.runSpritePtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.runAnimationFileName, charData.runAnimationFileName, getSpriteNumFrames(charData.runAnimationFileName)));
				curCharSpriteData.attackAwakenedIconPtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackAwakenedIconFileName, charData.attackAwakenedIconFileName, 1));
				curCharSpriteData.attackAnimationPtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackAnimationFileName, charData.attackAnimationFileName, getSpriteNumFrames(charData.attackAnimationFileName)));;
				curCharSpriteData.specialAnimationPtr = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.specialAnimationFileName, charData.specialAnimationFileName, getSpriteNumFrames(charData.specialAnimationFileName)));;
				for (int i = 0; i < charData.skillDataList.size(); i++)
				{
					curCharSpriteData.skillIconPtrList.push_back(std::shared_ptr<spriteData>(new spriteData(dirStr + charData.skillDataList[i].skillIconFileName, charData.skillDataList[i].skillIconFileName, 1)));
					double skillIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.skillIconPtrList[i]->spriteRValue}).m_Real;
					double skillIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.skillIconPtrList[i]->spriteRValue }).m_Real;
					g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.skillIconPtrList[i]->spriteRValue, skillIconWidth / 2, skillIconHeight / 2 });
				}

				for (auto& buffData : charData.buffDataList)
				{
					curCharSpriteData.buffIconPtrMap[buffData.buffName] = std::shared_ptr<spriteData>(new spriteData(dirStr + buffData.buffIconFileName, buffData.buffIconFileName, 1));
					double buffIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.buffIconPtrMap[buffData.buffName]->spriteRValue }).m_Real;
					double buffIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.buffIconPtrMap[buffData.buffName]->spriteRValue }).m_Real;
					g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.buffIconPtrMap[buffData.buffName]->spriteRValue, buffIconWidth / 2, buffIconHeight / 2 });
				}

				for (auto& projectileData : charData.projectileDataList)
				{
					curCharSpriteData.projectileAnimationPtrMap[projectileData.projectileName] = std::shared_ptr<spriteData>(new spriteData(dirStr + projectileData.projectileAnimationFileName, projectileData.projectileAnimationFileName, getSpriteNumFrames(projectileData.projectileAnimationFileName)));
					double projectileAnimationWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.projectileAnimationPtrMap[projectileData.projectileName]->spriteRValue }).m_Real;
					double projectileAnimationHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.projectileAnimationPtrMap[projectileData.projectileName]->spriteRValue }).m_Real;
					g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.projectileAnimationPtrMap[projectileData.projectileName]->spriteRValue, projectileAnimationWidth / 2, projectileAnimationHeight / 2 });

				}

				double attackIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.attackIconPtr->spriteRValue }).m_Real;
				double attackIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.attackIconPtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.attackIconPtr->spriteRValue, attackIconWidth / 2, attackIconHeight / 2 });
				
				double attackAwakenedIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.attackAwakenedIconPtr->spriteRValue }).m_Real;
				double attackAwakenedIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.attackAwakenedIconPtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.attackAwakenedIconPtr->spriteRValue, attackAwakenedIconWidth / 2, attackAwakenedIconHeight / 2 });
				
				double specialIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.specialIconPtr->spriteRValue }).m_Real;
				double specialIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.specialIconPtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.specialIconPtr->spriteRValue, specialIconWidth / 2, specialIconHeight / 2 });

				double largePortraitIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.largePortraitSpritePtr->spriteRValue }).m_Real;
				double largePortraitIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.largePortraitSpritePtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.largePortraitSpritePtr->spriteRValue, largePortraitIconWidth / 2, largePortraitIconHeight });

				double idleSpriteIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.idleSpritePtr->spriteRValue }).m_Real;
				double idleSpriteIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.idleSpritePtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.idleSpritePtr->spriteRValue, idleSpriteIconWidth / 2, idleSpriteIconHeight });
				double runSpriteIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.runSpritePtr->spriteRValue }).m_Real;
				double runSpriteIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.runSpritePtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.runSpritePtr->spriteRValue, runSpriteIconWidth / 2, runSpriteIconHeight });

				double specialAnimationIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.specialAnimationPtr->spriteRValue }).m_Real;
				double specialAnimationIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.specialAnimationPtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.specialAnimationPtr->spriteRValue, specialAnimationIconWidth / 2, specialAnimationIconHeight / 2 });
				double attackAnimationIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.attackAnimationPtr->spriteRValue }).m_Real;
				double attackAnimationIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.attackAnimationPtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.attackAnimationPtr->spriteRValue, attackAnimationIconWidth / 2, attackAnimationIconHeight / 2 });

				g_ModuleInterface->CallBuiltin("sprite_set_speed", { curCharSpriteData.idleSpritePtr->spriteRValue, charData.idleAnimationFPS.value, 0 });
				g_ModuleInterface->CallBuiltin("sprite_set_speed", { curCharSpriteData.runSpritePtr->spriteRValue, charData.runAnimationFPS.value, 0 });
				g_ModuleInterface->CallBuiltin("sprite_set_speed", { curCharSpriteData.specialAnimationPtr->spriteRValue, charData.specialAnimationFPS.value, 0 });
				g_ModuleInterface->CallBuiltin("sprite_set_speed", { curCharSpriteData.attackAnimationPtr->spriteRValue, charData.attackAnimationFPS.value, 0 });
				RValue charDataStruct;
				g_RunnerInterface.StructCreate(&charDataStruct);
				g_RunnerInterface.StructAddString(&charDataStruct, "id", charData.charName.c_str());
				g_RunnerInterface.StructAddString(&charDataStruct, "charName", charData.charName.c_str());
				if (charData.isUsingInGamePortraitSprite)
				{
					g_RunnerInterface.StructAddDouble(&charDataStruct, "port", characterDataMap[charData.inGamePortraitSpriteChar].portraitObjIndex);
				}
				else
				{
					g_RunnerInterface.StructAddRValue(&charDataStruct, "port", &curCharSpriteData.portraitSpritePtr->spriteRValue);
				}

				if (charData.isUsingInGameLargePortraitSprite)
				{
					g_RunnerInterface.StructAddDouble(&charDataStruct, "large_port", characterDataMap[charData.inGameLargePortraitSpriteChar].largePortraitObjIndex);
				}
				else
				{
					g_RunnerInterface.StructAddRValue(&charDataStruct, "large_port", &curCharSpriteData.largePortraitSpritePtr->spriteRValue);
				}
				
				if (charData.isUsingInGameIdleSprite)
				{
					g_RunnerInterface.StructAddDouble(&charDataStruct, "sprite1", characterDataMap[charData.inGameIdleSpriteChar].idleObjIndex);
				}
				else
				{
					g_RunnerInterface.StructAddRValue(&charDataStruct, "sprite1", &curCharSpriteData.idleSpritePtr->spriteRValue);
				}
				
				if (charData.isUsingInGameRunSprite)
				{
					g_RunnerInterface.StructAddDouble(&charDataStruct, "sprite2", characterDataMap[charData.inGameRunSpriteChar].runObjIndex);
				}
				else
				{
					g_RunnerInterface.StructAddRValue(&charDataStruct, "sprite2", &curCharSpriteData.runSpritePtr->spriteRValue);
				}
				
				g_RunnerInterface.StructAddDouble(&charDataStruct, "HP", charData.hp.value);
				g_RunnerInterface.StructAddDouble(&charDataStruct, "ATK", charData.atk.value);
				g_RunnerInterface.StructAddDouble(&charDataStruct, "SPD", charData.spd.value);
				g_RunnerInterface.StructAddDouble(&charDataStruct, "crit", charData.crit.value);

				if (charData.isUsingInGameMainWeapon)
				{
					g_RunnerInterface.StructAddString(&charDataStruct, "attackID", characterDataMap[charData.inGameMainWeaponChar].attackID.c_str());
					g_RunnerInterface.StructAddDouble(&charDataStruct, "attackIcon", characterDataMap[charData.inGameMainWeaponChar].attackIconObjIndex);
					g_RunnerInterface.StructAddString(&charDataStruct, "attack", characterDataMap[charData.inGameMainWeaponChar].attackNameString.c_str());
					g_RunnerInterface.StructAddRValue(&charDataStruct, "attackDesc", &characterDataMap[charData.inGameMainWeaponChar].attackDesc);
				}
				else
				{
					g_RunnerInterface.StructAddString(&charDataStruct, "attackID", charData.attackName.c_str());
					g_RunnerInterface.StructAddRValue(&charDataStruct, "attackIcon", &curCharSpriteData.attackIconPtr->spriteRValue);
					g_RunnerInterface.StructAddString(&charDataStruct, "attack", charData.attackName.c_str());
					RValue attackDescArr = g_ModuleInterface->CallBuiltin("array_create", { 1 });
					if (!charData.weaponLevelDataList.empty())
					{
						attackDescArr[0] = charData.weaponLevelDataList[0].attackDescription.c_str();
					}
					g_RunnerInterface.StructAddRValue(&charDataStruct, "attackDesc", &attackDescArr);
				}

				if (charData.isUsingInGameSpecial)
				{
					g_RunnerInterface.StructAddString(&charDataStruct, "specName", characterDataMap[charData.inGameSpecialChar].specialNameString.c_str());
					g_RunnerInterface.StructAddString(&charDataStruct, "specID", characterDataMap[charData.inGameSpecialChar].specialID.c_str());
					g_RunnerInterface.StructAddDouble(&charDataStruct, "specIcon", characterDataMap[charData.inGameSpecialChar].specialIconObjIndex);
					g_RunnerInterface.StructAddString(&charDataStruct, "specDesc", characterDataMap[charData.inGameSpecialChar].specialDescString.c_str());
					g_RunnerInterface.StructAddDouble(&charDataStruct, "specCD", characterDataMap[charData.inGameSpecialChar].specialCD);
				}
				else
				{
					g_RunnerInterface.StructAddString(&charDataStruct, "specName", charData.specialName.c_str());
					g_RunnerInterface.StructAddString(&charDataStruct, "specID", charData.specialName.c_str());
					g_RunnerInterface.StructAddRValue(&charDataStruct, "specIcon", &curCharSpriteData.specialIconPtr->spriteRValue);
					g_RunnerInterface.StructAddString(&charDataStruct, "specDesc", charData.specialDescription.c_str());
					g_RunnerInterface.StructAddDouble(&charDataStruct, "specCD", charData.specialCooldown.value);
				}

				RValue perksStruct;
				g_RunnerInterface.StructCreate(&perksStruct);
				if (charData.skillDataList.size() >= 3)
				{
					for (int i = 0; i < charData.skillDataList.size(); i++)
					{
						if (charData.skillDataList[i].isUsingInGameSkill)
						{
							g_RunnerInterface.StructAddDouble(&perksStruct, charData.skillDataList[i].inGameSkillName.c_str(), 0);
						}
						else
						{
							g_RunnerInterface.StructAddDouble(&perksStruct, charData.skillDataList[i].skillName.c_str(), 0);
						}
					}
				}
				g_RunnerInterface.StructAddRValue(&charDataStruct, "perks", &perksStruct);

				g_RunnerInterface.StructAddDouble(&charDataStruct, "sizeGrade", charData.sizeGrade.value);

				g_ModuleInterface->CallBuiltin("ds_map_set", { characterDataMapRValue, charData.charName.c_str(), charDataStruct});
				charNameList.push_back(charData.charName);
				charSpriteMap[charData.charName] = curCharSpriteData;
				charDataMap[charData.charName] = charData;
			}
		}
	}

	CInstance* Self = std::get<0>(Args);
	RValue charListByGen = getInstanceVariable(Self, GML_charListByGen);
	RValue characterInfo = getInstanceVariable(Self, GML_characterInfo);
	g_ModuleInterface->CallBuiltin("variable_global_set", { "characterInfoCopy", characterInfo });
	g_ModuleInterface->CallBuiltin("variable_global_set", { "charListByGenCopy", charListByGen });
}

void CharSelectDrawBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
}

void CharSelectDrawAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	RValue charSelected = g_ModuleInterface->CallBuiltin("variable_global_get", { "charSelected" });
	if (static_cast<int>(lround(charSelected.m_Real)) != -1 || charDataMap.empty())
	{
		return;
	}
	int boxWidth = 42;
	int boxHeight = 37;
	g_ModuleInterface->CallBuiltin("draw_rectangle_color", { 20, 166, 20 + boxWidth, 166 + boxHeight, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, true });
	g_ModuleInterface->CallBuiltin("draw_sprite", { sprRhythmButtonsIndex, 2, 20 + 21, 166 + 18 });

	g_ModuleInterface->CallBuiltin("draw_rectangle_color", { 578, 166, 578 + boxWidth, 166 + boxHeight, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, true });
	g_ModuleInterface->CallBuiltin("draw_sprite", { sprRhythmButtonsIndex, 3, 578 + 21, 166 + 18 });
	
	bool hasClicked = false;
	RValue mouseX;
	RValue mouseY;
	g_ModuleInterface->GetBuiltin("mouse_x", nullptr, NULL_INDEX, mouseX);
	g_ModuleInterface->GetBuiltin("mouse_y", nullptr, NULL_INDEX, mouseY);
	if (mouseX.m_Real >= 20 && mouseX.m_Real <= 20 + boxWidth && mouseY.m_Real >= 166 && mouseY.m_Real <= 166 + boxHeight)
	{
		if (g_ModuleInterface->CallBuiltin("mouse_check_button_pressed", { 1 }).ToBoolean())
		{
			if (charSelectPage > 0)
			{
				charSelectPage--;
			}
			else
			{
				charSelectPage = static_cast<int>(charDataMap.size() - 1) / 40 + 1;
			}
			hasClicked = true;
		}
	}
	if (mouseX.m_Real >= 578 && mouseX.m_Real <= 578 + boxWidth && mouseY.m_Real >= 166 && mouseY.m_Real <= 166 + boxHeight)
	{
		if (g_ModuleInterface->CallBuiltin("mouse_check_button_pressed", { 1 }).ToBoolean())
		{
			if (charSelectPage < (charDataMap.size() - 1) / 40 + 1)
			{
				charSelectPage++;
			}
			else
			{
				charSelectPage = 0;
			}
			hasClicked = true;
		}
	}
	if (hasClicked)
	{
		CInstance* Self = std::get<0>(Args);
		if (charSelectPage == 0)
		{
			RValue charListByGen = g_ModuleInterface->CallBuiltin("variable_global_get", { "charListByGenCopy" });
			RValue characterInfo = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterInfoCopy" });
			setInstanceVariable(Self, GML_charListByGen, charListByGen);
			setInstanceVariable(Self, GML_characterInfo, characterInfo);
		}
		else
		{
			RValue characterDataMap = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterData" });
			RValue charInfoArr = g_ModuleInterface->CallBuiltin("array_create", { 0 });
			RValue charListByGenArr = g_ModuleInterface->CallBuiltin("array_create", { 4 });
			for (int i = 0; i < 4; i++)
			{
				charListByGenArr[i] = g_ModuleInterface->CallBuiltin("array_create", { 0 });
			}
			for (int i = 0; i < 40 && i + (charSelectPage - 1) * 40 < charNameList.size(); i++)
			{
				std::string curCharName = charNameList[i + (charSelectPage - 1) * 40];
				RValue curCharData = g_ModuleInterface->CallBuiltin("ds_map_find_value", { characterDataMap, curCharName.c_str()});
				g_ModuleInterface->CallBuiltin("array_push", { charListByGenArr[i / 10], curCharData });
				g_ModuleInterface->CallBuiltin("array_push", { charInfoArr, curCharData });

				RValue characterFollowings = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterFollowings" });
				RValue tempArr = g_ModuleInterface->CallBuiltin("array_create", { 2 });
				tempArr[0] = curCharName.c_str();
				tempArr[1] = 0;
				g_ModuleInterface->CallBuiltin("array_push", { characterFollowings, tempArr });
			}
			setInstanceVariable(Self, GML_charListByGen, charListByGenArr);
			setInstanceVariable(Self, GML_characterInfo, charInfoArr);
		}
		setInstanceVariable(Self, GML_selectingGen, 0);
		setInstanceVariable(Self, GML_selectingChar, 0);
		setInstanceVariable(Self, GML_selectedCharacter, 0);
	}
}

void loadCharacterClickButton()
{
	if (g_pd3dDevice == nullptr)
	{
		// Create application window
		//ImGui_ImplWin32_EnableDpiAwareness();
		WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Character Mod Menu", nullptr };
		::RegisterClassExW(&wc);
		HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Character Mod Menu", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, nullptr, nullptr, wc.hInstance, nullptr);
		// Initialize Direct3D
		if (!CreateDeviceD3D(hwnd))
		{
			CleanupDeviceD3D();
			::UnregisterClassW(wc.lpszClassName, wc.hInstance);
			return;
		}

		// Show the window
		::ShowWindow(hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(hwnd);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		ImGui::StyleColorsDark();
		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
	}
}

void TitleScreenDrawAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (g_pd3dDevice == nullptr)
	{
		return;
	}

	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{

			}
		}

		// Handle window being minimized or screen locked
		if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
		{
			::Sleep(10);
			return;
		}
		g_SwapChainOccluded = false;

		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		handleImGUI();

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present
		HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
		//HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
		g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
	}
}

void setJSONNumberToStruct(RValue& outputStruct, VariableNames varName, JSONInt& inputJSONInt)
{
	if (inputJSONInt.isDefined)
	{
		setInstanceVariable(outputStruct, varName, inputJSONInt.value);
	}
}

void setJSONNumberToStruct(RValue& outputStruct, VariableNames varName, JSONDouble& inputJSONDouble)
{
	if (inputJSONDouble.isDefined)
	{
		setInstanceVariable(outputStruct, varName, inputJSONDouble.value);
	}
}

void AttackControllerCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	// TODO: Should probably append a string to the attack, special, and char name to make sure they're unique
	CInstance* Self = std::get<0>(Args);
	RValue attackIndexMap = getInstanceVariable(Self, GML_attackIndex);
	RValue attack = g_ModuleInterface->CallBuiltin("ds_map_find_value", { attackIndexMap, "CircleHitbox" });

	for (auto& charDataPair : charDataMap)
	{
		auto& charData = charDataPair.second;
		auto& curCharSprite = charSpriteMap[charDataPair.first];
		if (!charData.isUsingInGameMainWeapon)
		{
			RValue newAttack = g_ModuleInterface->CallBuiltin("variable_clone", { attack });
			setInstanceVariable(newAttack, GML_attackID, charData.attackName.c_str());
			RValue config = getInstanceVariable(newAttack, GML_config);
			setInstanceVariable(config, GML_sprite_index, curCharSprite.attackAnimationPtr->spriteRValue);
			setJSONNumberToStruct(config, GML_attackTime, charData.weaponLevelDataList[0].attackTime);
			setJSONNumberToStruct(config, GML_duration, charData.weaponLevelDataList[0].duration);
			setJSONNumberToStruct(config, GML_damage, charData.weaponLevelDataList[0].damage);
			setJSONNumberToStruct(config, GML_hitLimit, charData.weaponLevelDataList[0].hitLimit);
			setJSONNumberToStruct(config, GML_speed, charData.weaponLevelDataList[0].speed);
			setJSONNumberToStruct(config, GML_hitCD, charData.weaponLevelDataList[0].hitCD);
			setJSONNumberToStruct(config, GML_attackCount, charData.weaponLevelDataList[0].attackCount);
			setJSONNumberToStruct(config, GML_attackDelay, charData.weaponLevelDataList[0].attackDelay);
			setJSONNumberToStruct(config, GML_range, charData.weaponLevelDataList[0].range);
			setInstanceVariable(config, GML_attackID, charData.attackName.c_str());
			setInstanceVariable(config, GML_optionID, charData.attackName.c_str());
			setInstanceVariable(config, GML_onCreate, RValue());
			setInstanceVariable(config, GML_customDrawScriptBelow, RValue());
			setInstanceVariable(config, GML_collides, true);
			setInstanceVariable(config, GML_isMain, true);
			setInstanceVariable(config, GML_maxLevel, 7);
			setInstanceVariable(config, GML_weaponType, charData.mainWeaponWeaponType.c_str());
			setInstanceVariable(config, GML_optionIcon, curCharSprite.attackIconPtr->spriteRValue);
			if (charData.mainWeaponWeaponType.compare("Melee") == 0)
			{
				setInstanceVariable(config, GML_stayOnCreator, true);
				setInstanceVariable(config, GML_isMelee, true);
				setInstanceVariable(config, GML_faceCreatorDirection, true);
			}
			else if (charData.mainWeaponWeaponType.compare("Multishot") == 0)
			{
				setInstanceVariable(config, GML_faceCreatorDirection, true);
			}
			else if (charData.mainWeaponWeaponType.compare("Ranged") == 0)
			{
				setInstanceVariable(config, GML_faceCreatorDirection, true);
			}
			RValue levels = g_ModuleInterface->CallBuiltin("array_create", { 6 });
			for (int j = 1; j < 7; j++)
			{
				RValue curStruct;
				g_RunnerInterface.StructCreate(&curStruct);
				RValue tempConfig;
				g_RunnerInterface.StructCreate(&tempConfig);
				setInstanceVariable(tempConfig, GML_optionName, std::format("{} LV {}", charData.attackName, j + 1).c_str());
				setInstanceVariable(tempConfig, GML_optionDescription, charData.weaponLevelDataList[j].attackDescription.c_str());
				setJSONNumberToStruct(tempConfig, GML_attackTime, charData.weaponLevelDataList[j].attackTime);
				setJSONNumberToStruct(tempConfig, GML_duration, charData.weaponLevelDataList[j].duration);
				setJSONNumberToStruct(tempConfig, GML_damage, charData.weaponLevelDataList[j].damage);
				setJSONNumberToStruct(tempConfig, GML_hitLimit, charData.weaponLevelDataList[j].hitLimit);
				setJSONNumberToStruct(tempConfig, GML_speed, charData.weaponLevelDataList[j].speed);
				setJSONNumberToStruct(tempConfig, GML_hitCD, charData.weaponLevelDataList[j].hitCD);
				setJSONNumberToStruct(tempConfig, GML_attackCount, charData.weaponLevelDataList[j].attackCount);
				setJSONNumberToStruct(tempConfig, GML_attackDelay, charData.weaponLevelDataList[j].attackDelay);
				setJSONNumberToStruct(tempConfig, GML_range, charData.weaponLevelDataList[j].range);
				if (j == 6)
				{
					setInstanceVariable(tempConfig, GML_optionIcon, curCharSprite.attackAwakenedIconPtr->spriteRValue);
				}
				setInstanceVariable(curStruct, GML_config, tempConfig);
				levels[j - 1] = curStruct;
			}
			setInstanceVariable(config, GML_levels, levels);

			g_ModuleInterface->CallBuiltin("ds_map_set", { attackIndexMap, charData.attackName.c_str(), newAttack });
		}

		if (!charData.isUsingInGameSpecial)
		{
			RValue newSpecial = g_ModuleInterface->CallBuiltin("variable_clone", { attack });
			setInstanceVariable(newSpecial, GML_attackID, charData.specialName.c_str());
			RValue specialConfig = getInstanceVariable(newSpecial, GML_config);
			setInstanceVariable(specialConfig, GML_attackID, charData.specialName.c_str());
			setInstanceVariable(specialConfig, GML_optionID, charData.specialName.c_str());
			setInstanceVariable(specialConfig, GML_sprite_index, curCharSprite.specialAnimationPtr->spriteRValue);
			setInstanceVariable(specialConfig, GML_duration, charData.specialDuration.value);
			setInstanceVariable(specialConfig, GML_damage, charData.specialDamage.value);
			setInstanceVariable(specialConfig, GML_collides, true);
			setInstanceVariable(specialConfig, GML_isMain, true);
			setInstanceVariable(specialConfig, GML_onCreate, RValue());
			setInstanceVariable(specialConfig, GML_customDrawScriptBelow, RValue());

			g_ModuleInterface->CallBuiltin("ds_map_set", { attackIndexMap, charData.specialName.c_str(), newSpecial });
		}

		for (auto& projectileData : charData.projectileDataList)
		{
			RValue newAttack = g_ModuleInterface->CallBuiltin("variable_clone", { attack });
			RValue config = getInstanceVariable(newAttack, GML_config);
			setInstanceVariable(config, GML_sprite_index, curCharSprite.projectileAnimationPtrMap[projectileData.projectileName]->spriteRValue);
			setJSONNumberToStruct(config, GML_duration, projectileData.projectileDuration);
			setJSONNumberToStruct(config, GML_damage, projectileData.projectileDamage);
			setJSONNumberToStruct(config, GML_hitLimit, projectileData.projectileHitLimit);
			setJSONNumberToStruct(config, GML_speed, projectileData.projectileSpeed);
			setJSONNumberToStruct(config, GML_hitCD, projectileData.projectileHitCD);
			setJSONNumberToStruct(config, GML_range, projectileData.projectileHitRange);
			setInstanceVariable(config, GML_attackID, projectileData.projectileName.c_str());
			setInstanceVariable(config, GML_optionID, projectileData.projectileName.c_str());
			setInstanceVariable(config, GML_onCreate, RValue());
			setInstanceVariable(config, GML_customDrawScriptBelow, RValue());
			setInstanceVariable(config, GML_collides, true);
			setInstanceVariable(config, GML_isMain, false);
			// TODO: Add weapon type to the projectile
			/*
			setInstanceVariable(config, GML_weaponType, charData.mainWeaponWeaponType.c_str());
			*/
			g_ModuleInterface->CallBuiltin("ds_map_set", { attackIndexMap, projectileData.projectileName.c_str(), newAttack });
		}
	}
}

RValue& buffApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue buffConfig = *Args[1];
	RValue buffName = getInstanceVariable(buffConfig, GML_buffName);
	RValue playerCharacter = *Args[0];
	RValue stacks = getInstanceVariable(buffConfig, GML_stacks);

	auto& curBuffDataList = buffDataListMap[buffName.ToString()];
	for (auto& buffData : curBuffDataList)
	{
		if (buffData.levels[0].attackIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_ATK, getInstanceVariable(playerCharacter, GML_ATK).m_Real + buffData.levels[0].attackIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].critIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_crit, getInstanceVariable(playerCharacter, GML_crit).m_Real + buffData.levels[0].critIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].hasteIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_haste, getInstanceVariable(playerCharacter, GML_haste).m_Real + buffData.levels[0].hasteIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].speedIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_SPD, getInstanceVariable(playerCharacter, GML_SPD).m_Real + buffData.levels[0].speedIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].DRMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_DR, getInstanceVariable(playerCharacter, GML_DR).m_Real * buffData.levels[0].DRMultiplier.value * stacks.m_Real);
		}
		if (buffData.levels[0].healMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_healMultiplier, getInstanceVariable(playerCharacter, GML_healMultiplier).m_Real + buffData.levels[0].healMultiplier.value * stacks.m_Real);
		}
		if (buffData.levels[0].food.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_food, getInstanceVariable(playerCharacter, GML_food).m_Real + buffData.levels[0].food.value * stacks.m_Real);
		}
		if (buffData.levels[0].weaponSize.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_weaponSize, getInstanceVariable(playerCharacter, GML_weaponSize).m_Real + buffData.levels[0].weaponSize.value * stacks.m_Real);
		}
		if (buffData.levels[0].pickupRange.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_pickupRange, getInstanceVariable(playerCharacter, GML_pickupRange).m_Real + buffData.levels[0].pickupRange.value * stacks.m_Real);
		}
		if (buffData.levels[0].critMod.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_CritMod, getInstanceVariable(playerCharacter, GML_CritMod).m_Real + buffData.levels[0].critMod.value * stacks.m_Real);
		}
		if (buffData.levels[0].bonusProjectile.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_bonusProjectiles, getInstanceVariable(playerCharacter, GML_bonusProjectiles).m_Real + buffData.levels[0].bonusProjectile.value * stacks.m_Real);
		}
	}

	return ReturnValue;
}

RValue& buffRemove(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue buffConfig = *Args[1];
	RValue buffName = getInstanceVariable(buffConfig, GML_buffName);
	RValue playerCharacter = *Args[0];
	RValue stacks = getInstanceVariable(buffConfig, GML_stacks);

	auto& curBuffDataList = buffDataListMap[buffName.ToString()];
	for (auto& buffData : curBuffDataList)
	{
		if (buffData.levels[0].attackIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_ATK, getInstanceVariable(playerCharacter, GML_ATK).m_Real - buffData.levels[0].attackIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].critIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_crit, getInstanceVariable(playerCharacter, GML_crit).m_Real - buffData.levels[0].critIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].hasteIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_haste, getInstanceVariable(playerCharacter, GML_haste).m_Real - buffData.levels[0].hasteIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].speedIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_SPD, getInstanceVariable(playerCharacter, GML_SPD).m_Real - buffData.levels[0].speedIncrement.value * stacks.m_Real);
		}
		if (buffData.levels[0].DRMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_DR, getInstanceVariable(playerCharacter, GML_DR).m_Real / (buffData.levels[0].DRMultiplier.value * stacks.m_Real));
		}
		if (buffData.levels[0].healMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_healMultiplier, getInstanceVariable(playerCharacter, GML_healMultiplier).m_Real - buffData.levels[0].healMultiplier.value * stacks.m_Real);
		}
		if (buffData.levels[0].food.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_food, getInstanceVariable(playerCharacter, GML_food).m_Real - buffData.levels[0].food.value * stacks.m_Real);
		}
		if (buffData.levels[0].weaponSize.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_weaponSize, getInstanceVariable(playerCharacter, GML_weaponSize).m_Real - buffData.levels[0].weaponSize.value * stacks.m_Real);
		}
		if (buffData.levels[0].pickupRange.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_pickupRange, getInstanceVariable(playerCharacter, GML_pickupRange).m_Real - buffData.levels[0].pickupRange.value * stacks.m_Real);
		}
		if (buffData.levels[0].critMod.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_CritMod, getInstanceVariable(playerCharacter, GML_CritMod).m_Real - buffData.levels[0].critMod.value * stacks.m_Real);
		}
		if (buffData.levels[0].bonusProjectile.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_bonusProjectiles, getInstanceVariable(playerCharacter, GML_bonusProjectiles).m_Real - buffData.levels[0].bonusProjectile.value * stacks.m_Real);
		}
	}
	return ReturnValue;
}

RValue& onDebuff(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	return ReturnValue;
}

RValue& onAttackCreate(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	return ReturnValue;
}

RValue& onCriticalHit(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	// TODO: Maybe make it so that it can target the attack instead of the player?
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	ReturnValue = *Args[3];
	return ReturnValue;
}

RValue& onHeal(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[1];
	auto& charData = charDataMap[playingCharName];
	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	ReturnValue = *Args[0];
	return ReturnValue;
}

RValue& onKill(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	return ReturnValue;
}

RValue& onTakeDamage(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[3];
	auto& charData = charDataMap[playingCharName];
	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	ReturnValue = *Args[0];
	return ReturnValue;
}

RValue& onDodge(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[3];
	auto& charData = charDataMap[playingCharName];
	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	ReturnValue = *Args[0];
	return ReturnValue;
}

void AttackControllerOther11After(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	RValue buffsMap = getInstanceVariable(Self, GML_Buffs);
	for (auto& charDataPair : charDataMap)
	{
		for (auto& buffData : charDataPair.second.buffDataList)
		{
			RValue buffStruct;
			g_RunnerInterface.StructCreate(&buffStruct);
			setInstanceVariable(buffStruct, GML_timer, buffData.levels[0].timer.value);
			RValue retVal;
			yyGMLPushContextStack(Self);
			yyGMLYYSetScriptRef(&retVal, buffApply, Self);
			yyGMLPopContextStack(1);
			setInstanceVariable(buffStruct, GML_Apply, retVal);
			yyGMLPushContextStack(Self);
			yyGMLYYSetScriptRef(&retVal, buffRemove, Self);
			yyGMLPopContextStack(1);
			setInstanceVariable(buffStruct, GML_Callback, retVal);
			setInstanceVariable(buffStruct, GML_buffIcon, charSpriteMap[charDataPair.first].buffIconPtrMap[buffData.buffName]->spriteRValue);
			g_ModuleInterface->CallBuiltin("ds_map_set", { buffsMap, buffData.buffName.c_str(), buffStruct});
		}
	}
}

int curSkillIndex = 0;
int curSkillLevel = 0;

RValue& skillApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = getInstanceVariable(Self, GML_playerCharacter);
	RValue attackController = g_ModuleInterface->CallBuiltin("instance_find", { objAttackControllerIndex, 0 });
	RValue buffsMap = getInstanceVariable(attackController, GML_Buffs);
	auto& charData = charDataMap[playingCharName];
	auto& curSkillLevelData = charData.skillDataList[curSkillIndex].skillLevelDataList[curSkillLevel];
	if (curSkillLevelData.attackIncrement.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_ATK, getInstanceVariable(playerCharacter, GML_ATK).m_Real + curSkillLevelData.attackIncrement.value);
	}
	if (curSkillLevelData.critIncrement.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_crit, getInstanceVariable(playerCharacter, GML_crit).m_Real + curSkillLevelData.critIncrement.value);
	}
	if (curSkillLevelData.hasteIncrement.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_haste, getInstanceVariable(playerCharacter, GML_haste).m_Real + curSkillLevelData.hasteIncrement.value);
	}
	if (curSkillLevelData.speedIncrement.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_SPD, getInstanceVariable(playerCharacter, GML_SPD).m_Real + curSkillLevelData.speedIncrement.value);
	}
	if (curSkillLevelData.DRMultiplier.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_DR, getInstanceVariable(playerCharacter, GML_DR).m_Real * curSkillLevelData.DRMultiplier.value);
	}
	if (curSkillLevelData.healMultiplier.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_healMultiplier, getInstanceVariable(playerCharacter, GML_healMultiplier).m_Real + curSkillLevelData.healMultiplier.value);
	}
	if (curSkillLevelData.food.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_food, getInstanceVariable(playerCharacter, GML_food).m_Real + curSkillLevelData.food.value);
	}
	if (curSkillLevelData.weaponSize.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_weaponSize, getInstanceVariable(playerCharacter, GML_weaponSize).m_Real + curSkillLevelData.weaponSize.value);
	}
	if (curSkillLevelData.pickupRange.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_pickupRange, getInstanceVariable(playerCharacter, GML_pickupRange).m_Real + curSkillLevelData.pickupRange.value);
	}
	if (curSkillLevelData.critMod.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_CritMod, getInstanceVariable(playerCharacter, GML_CritMod).m_Real + curSkillLevelData.critMod.value);
	}
	if (curSkillLevelData.bonusProjectile.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_bonusProjectiles, getInstanceVariable(playerCharacter, GML_bonusProjectiles).m_Real + curSkillLevelData.bonusProjectile.value);
	}

	for (auto& skillTriggerData : curSkillLevelData.skillTriggerList)
	{
		RValue onTriggerStruct = g_ModuleInterface->CallBuiltin("variable_instance_get", { playerCharacter, skillTriggerTypeMap[skillTriggerData.skillTriggerType].c_str() });
		RValue retVal;
		yyGMLPushContextStack(Self);

		auto& actionName = skillTriggerData.triggeredActionName;
		switch (skillTriggerData.skillTriggerType)
		{
			case skillTriggerType_OnDebuff:
			{
				yyGMLYYSetScriptRef(&retVal, onDebuff, Self);
				break;
			}
			case skillTriggerType_OnAttackCreate:
			{
				yyGMLYYSetScriptRef(&retVal, onAttackCreate, Self);
				break;
			}
			case skillTriggerType_OnCriticalHit:
			{
				yyGMLYYSetScriptRef(&retVal, onCriticalHit, Self);
				break;
			}
			case skillTriggerType_OnHeal:
			{
				yyGMLYYSetScriptRef(&retVal, onHeal, Self);
				break;
			}
			case skillTriggerType_OnKill:
			{
				yyGMLYYSetScriptRef(&retVal, onKill, Self);
				break;
			}
			case skillTriggerType_OnTakeDamage:
			{
				yyGMLYYSetScriptRef(&retVal, onTakeDamage, Self);
				break;
			}
			case skillTriggerType_OnDodge:
			{
				yyGMLYYSetScriptRef(&retVal, onDodge, Self);
				break;
			}
			default:
			{
				g_ModuleInterface->Print(CM_RED, "Invalid on trigger type for %s", skillTriggerData.skillTriggerName);
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Invalid on trigger type for %s", skillTriggerData.skillTriggerName);
			}
		}

		yyGMLPopContextStack(1);
		g_ModuleInterface->CallBuiltin("variable_instance_set", { onTriggerStruct, skillTriggerData.triggeredActionName.c_str(), retVal });
	}
	return ReturnValue;
}

RValue& skillOneLevelOneApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 0;
	curSkillLevel = 0;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillOneLevelTwoApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 0;
	curSkillLevel = 1;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillOneLevelThreeApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 0;
	curSkillLevel = 2;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillTwoLevelOneApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 1;
	curSkillLevel = 0;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillTwoLevelTwoApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 1;
	curSkillLevel = 1;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillTwoLevelThreeApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 1;
	curSkillLevel = 2;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillThreeLevelOneApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 2;
	curSkillLevel = 0;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillThreeLevelTwoApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 2;
	curSkillLevel = 1;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

RValue& skillThreeLevelThreeApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	curSkillIndex = 2;
	curSkillLevel = 2;
	return skillApply(Self, Other, ReturnValue, numArgs, Args);
}

std::vector<PFUNC_YYGMLScript> onApplyList { skillOneLevelOneApply, skillOneLevelTwoApply, skillOneLevelThreeApply, skillTwoLevelOneApply, skillTwoLevelTwoApply, skillTwoLevelThreeApply, skillThreeLevelOneApply, skillThreeLevelTwoApply, skillThreeLevelThreeApply };

void PlayerManagerOther22After(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	RValue perksMap = getInstanceVariable(Self, GML_PERKS);
	RValue textContainer = g_ModuleInterface->CallBuiltin("variable_global_get", { "TextContainer" });

	RValue FPSMastery = g_ModuleInterface->CallBuiltin("ds_map_find_value", { perksMap, "FPSMastery" });
	for (auto& charDataPair : charDataMap)
	{
		auto& charData = charDataPair.second;
		for (int j = 0; j < 3; j++)
		{
			RValue newPerk = g_ModuleInterface->CallBuiltin("variable_clone", { FPSMastery });
			auto& skillData = charData.skillDataList[j];
			
			setInstanceVariable(newPerk, GML_id, skillData.skillName.c_str());
			setInstanceVariable(newPerk, GML_optionIcon, charSpriteMap[charDataPair.first].skillIconPtrList[j]->spriteRValue);
			setInstanceVariable(newPerk, GML_name, skillData.skillName.c_str());
			setInstanceVariable(newPerk, GML_optionName, skillData.skillName.c_str());
			setInstanceVariable(newPerk, GML_optionDescription, skillData.skillLevelDataList[0].skillDescription.c_str());
			setInstanceVariable(newPerk, GML_optionID, skillData.skillName.c_str());
			
			RValue descriptionArr = g_ModuleInterface->CallBuiltin("array_create", { 3 });
			RValue skillOnApply = g_ModuleInterface->CallBuiltin("array_create", { 3 });
			for (int level = 0; level < 3; level++)
			{
				RValue retVal;
				yyGMLPushContextStack(Self);
				yyGMLYYSetScriptRef(&retVal, onApplyList[j * 3 + level], Self);
				yyGMLPopContextStack(1);
				skillOnApply[level] = retVal;
				descriptionArr[level] = skillData.skillLevelDataList[level].skillDescription.c_str();
			}
			setInstanceVariable(newPerk, GML_OnApply, skillOnApply);

			RValue descContainer;
			g_RunnerInterface.StructCreate(&descContainer);
			setInstanceVariable(descContainer, GML_selectedLanguage, descriptionArr);
			g_ModuleInterface->CallBuiltin("variable_instance_set", { textContainer, std::string_view(skillData.skillName + "Description"), descContainer });

			g_ModuleInterface->CallBuiltin("ds_map_set", { perksMap, skillData.skillName.c_str(), newPerk});
		}
	}
}

void activateAction(const characterData& charData, CInstance* parentInstance, const std::string& actionName)
{
	RValue playerManager = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerManagerIndex, 0 });
	RValue playerCharacter = getInstanceVariable(playerManager, GML_playerCharacter);
	RValue attackController = g_ModuleInterface->CallBuiltin("instance_find", { objAttackControllerIndex, 0 });
	CInstance* attackControllerInstance = attackController.ToInstance();

	auto& curActionDataList = actionDataListMap[actionName];
	for (auto& actionData : curActionDataList)
	{
		std::random_device rd;
		std::default_random_engine generator(rd());
		std::uniform_real_distribution<double> distribution(0, 100);
		double randNum = distribution(generator);
		if (randNum >= actionData.probability)
		{
			continue;
		}
		if (actionData.actionType == actionType_SpawnProjectile)
		{
			auto& actionProjectileData = actionData.actionProjectileData;
			RValue overrideConfig;
			g_RunnerInterface.StructCreate(&overrideConfig);
			if (actionProjectileData.isAbsoluteSpawnDir)
			{
				g_RunnerInterface.StructAddDouble(&overrideConfig, "direction", actionProjectileData.spawnDir.value);
			}
			else
			{
				RValue direction = getInstanceVariable(parentInstance, GML_direction);
				g_RunnerInterface.StructAddDouble(&overrideConfig, "direction", actionProjectileData.spawnDir.value + direction.ToDouble());
			}
			RValue xPos = getInstanceVariable(parentInstance, GML_x);
			RValue yPos = getInstanceVariable(parentInstance, GML_y);
			g_RunnerInterface.StructAddDouble(&overrideConfig, "x", xPos.ToDouble() + actionProjectileData.relativeSpawnPosX.value);
			g_RunnerInterface.StructAddDouble(&overrideConfig, "y", yPos.ToDouble() + actionProjectileData.relativeSpawnPosY.value);

			RValue** args = new RValue*[3];
			args[0] = new RValue(actionProjectileData.projectileDataName.c_str());
			args[1] = new RValue(playerCharacter);
			args[2] = new RValue(overrideConfig);
			RValue result;
			origExecuteAttackScript(attackControllerInstance, attackControllerInstance, result, 3, args);
		}
		else if (actionData.actionType == actionType_ApplyBuff)
		{
			auto& actionBuffData = actionData.actionBuffData;

			RValue buffsMap = getInstanceVariable(attackController, GML_Buffs);
			RValue buffsMapData = g_ModuleInterface->CallBuiltin("ds_map_find_value", { buffsMap, actionBuffData.buffName.c_str() });
			auto& curBuffDataList = buffDataListMap[actionBuffData.buffName];
			for (auto& buffData : curBuffDataList)
			{
				RValue buffConfig;
				g_RunnerInterface.StructCreate(&buffConfig);
				setInstanceVariable(buffConfig, GML_reapply, true);
				setInstanceVariable(buffConfig, GML_stacks, 1.0);
				setInstanceVariable(buffConfig, GML_maxStacks, buffData.levels[0].maxStacks.value);
				setInstanceVariable(buffConfig, GML_buffName, buffData.buffName.c_str());
				setInstanceVariable(buffConfig, GML_buffIcon, getInstanceVariable(buffsMapData, GML_buffIcon));
				// TODO: Should probably replace this with something more efficient
				RValue ApplyBuffMethod = getInstanceVariable(attackController, GML_ApplyBuff);
				RValue ApplyBuffArr = g_ModuleInterface->CallBuiltin("array_create", { 4 });
				ApplyBuffArr[0] = playerCharacter;
				ApplyBuffArr[1] = buffData.buffName.c_str();
				ApplyBuffArr[2] = buffsMapData;
				ApplyBuffArr[3] = buffConfig;
				g_ModuleInterface->CallBuiltin("method_call", { ApplyBuffMethod, ApplyBuffArr });
			}
		}
		else if (actionData.actionType == actionType_SetProjectileStats)
		{
			auto& actionSetProjectileStatsData = actionData.actionSetProjectileStatsData;
			RValue xPos = getInstanceVariable(parentInstance, GML_x);
			RValue yPos = getInstanceVariable(parentInstance, GML_y);
			setInstanceVariable(parentInstance, GML_x, xPos.ToDouble() + actionSetProjectileStatsData.relativePosX.value);
			setInstanceVariable(parentInstance, GML_y, yPos.ToDouble() + actionSetProjectileStatsData.relativePosY.value);
			if (actionSetProjectileStatsData.speed.isDefined)
			{
				setInstanceVariable(parentInstance, GML_speed, actionSetProjectileStatsData.speed.value);
			}
		}

		for (auto& nextActionData : actionData.nextActionList)
		{
			if (nextActionData.actionFrameDelay <= 0)
			{
				activateAction(charData, parentInstance, nextActionData.triggeredActionName);
			}
			else
			{
				actionQueue.push(std::make_pair<int, actionQueueData>(curFrameNum + nextActionData.actionFrameDelay, actionQueueData(parentInstance, nextActionData.triggeredActionName)));
			}
		}
	}
}

void checkProjectileActionList(const characterData& charData, CInstance* parentInstance, const std::vector<projectileActionData>& projectileActionList, projectileActionTriggerTypeEnum onTriggerType)
{
	for (auto& projectileAction : projectileActionList)
	{
		if (projectileAction.projectileActionTriggerType == onTriggerType)
		{
			activateAction(charData, parentInstance, projectileAction.triggeredActionName);
		}
	}
}

void checkOnTrigger(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args, CInstance* parentInstance, projectileActionTriggerTypeEnum onTriggerType)
{
	CInstance* Self = std::get<0>(Args);

	RValue attackID = getInstanceVariable(Self, GML_attackID);

	RValue playerManager = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerManagerIndex, 0 });
	RValue playerCharacter = getInstanceVariable(playerManager, GML_playerCharacter);
	auto& charData = charDataMap[playingCharName];
	const char* strAttackID = attackID.ToCString();

	// TODO: Probably want to avoid string comparison for the attack name

	if (charData.attackName.compare(strAttackID) == 0)
	{
		RValue weapons = getInstanceVariable(playerManager, GML_weapons);
		RValue mainWeapon = g_ModuleInterface->CallBuiltin("variable_instance_get", { weapons, strAttackID });
		RValue level = getInstanceVariable(mainWeapon, GML_level);
		auto& weaponLevelData = charData.weaponLevelDataList[level.ToInt32() - 1];
		checkProjectileActionList(charData, parentInstance, weaponLevelData.projectileActionList, onTriggerType);
	}

	if (charData.specialName.compare(strAttackID) == 0)
	{
		checkProjectileActionList(charData, parentInstance, charData.specialProjectileActionList, onTriggerType);
	}

	auto& curProjectileDataList = projectileDataListMap[strAttackID];
	for (auto& projectileData : curProjectileDataList)
	{
		checkProjectileActionList(charData, parentInstance, projectileData.projectileActionList, onTriggerType);
	}
}

void AttackCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	checkOnTrigger(Args, Self, projectileActionTriggerType_OnCreate);
}

void AttackDestroyBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	checkOnTrigger(Args, Self, projectileActionTriggerType_OnDestroy);
}

void PlayerStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	curFrameNum++;
	auto& charData = charDataMap[playingCharName];
	while (!actionQueue.empty() && curFrameNum >= actionQueue.top().first)
	{
		auto& curActionQueueData = actionQueue.top().second;
		actionQueue.pop();
		// TODO: Maybe should cache the instance exist function
		if (g_ModuleInterface->CallBuiltin("instance_exists", { curActionQueueData.parentInstance }).ToBoolean())
		{
			activateAction(charData, curActionQueueData.parentInstance, curActionQueueData.triggeredActionName);
		}
	}
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}