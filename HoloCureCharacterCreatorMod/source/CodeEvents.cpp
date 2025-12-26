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
#include <numbers>

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern YYGML_PushContextStack yyGMLPushContextStack;
extern YYGML_YYSetScriptRef yyGMLYYSetScriptRef;
extern YYGML_PopContextStack yyGMLPopContextStack;
//extern RValue lastStructVarGetName;
extern std::string playingCharName;
extern std::unordered_map<std::string, std::vector<buffData>> buffDataListMap;
//extern std::unordered_map<std::string, std::vector<actionData>> actionDataListMap;
extern std::unordered_map<std::string, std::vector<projectileDataWrapper>> projectileDataListMap;
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
	CInstance* parentInstancePtr;
	nodeEditorNode* curNodePtr;
	std::vector<nodeEditorNode*> loopBlockStack;

	actionQueueData(CInstance* parentInstancePtr, nodeEditorNode* curNodePtr, std::vector<nodeEditorNode*> loopBlockStack) : parentInstancePtr(parentInstancePtr), curNodePtr(curNodePtr), loopBlockStack(loopBlockStack)
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
std::random_device rd;
std::default_random_engine randomGenerator(rd());

RValue tempResult;
RValue RValueInputArgs[3];

bool hasBackedUpCharacterList = false;
bool isInCharSelectDraw = false;

int rhythmLeftButtonIndex = 100000;
int rhythmRightButtonIndex = 100001;
int curLogLevel = 0;

int charSelectPage = 0;

void checkOnTrigger(CInstance* parentInstance, nodeEditorNodeTypeEnum triggerType);

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
		DbgPrintEx(LOG_SEVERITY_ERROR, "Couldn't find the CharacterCreatorMod directory");
		return;
	}

	RValue characterList = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterList" });
	charSpriteMap.clear();
	charDataMap.clear();
	charNameList.clear();
	curLogLevel = 0;

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
					curCharSpriteData.skillIconPtrList.push_back(std::shared_ptr<spriteData>(new spriteData(dirStr + charData.skillDataList[i].data.skillIconFileName, charData.skillDataList[i].data.skillIconFileName, 1)));
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
					curCharSpriteData.projectileAnimationPtrMap[projectileData.data.projectileName] = std::shared_ptr<spriteData>(new spriteData(dirStr + projectileData.data.projectileAnimationFileName, projectileData.data.projectileAnimationFileName, getSpriteNumFrames(projectileData.data.projectileAnimationFileName)));
					double projectileAnimationWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.projectileAnimationPtrMap[projectileData.data.projectileName]->spriteRValue }).m_Real;
					double projectileAnimationHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.projectileAnimationPtrMap[projectileData.data.projectileName]->spriteRValue }).m_Real;
					g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.projectileAnimationPtrMap[projectileData.data.projectileName]->spriteRValue, projectileAnimationWidth / 2, projectileAnimationHeight / 2 });
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
						if (charData.skillDataList[i].data.isUsingInGameSkill)
						{
							g_RunnerInterface.StructAddDouble(&perksStruct, charData.skillDataList[i].data.inGameSkillName.c_str(), 0);
						}
						else
						{
							g_RunnerInterface.StructAddDouble(&perksStruct, charData.skillDataList[i].data.skillName.c_str(), 0);
						}
					}
				}
				g_RunnerInterface.StructAddRValue(&charDataStruct, "perks", &perksStruct);

				g_RunnerInterface.StructAddDouble(&charDataStruct, "sizeGrade", charData.sizeGrade.value);

				g_ModuleInterface->CallBuiltin("ds_map_set", { characterDataMapRValue, charData.charName.c_str(), charDataStruct});
				charNameList.push_back(charData.charName);
				charSpriteMap[charData.charName] = curCharSpriteData;
				charDataMap[charData.charName] = charData;
				// Seems like this needs to be after it's set in the map in order to avoid the audio being destroyed
				for (auto& soundData : charDataMap[charData.charName].soundDataList)
				{
					soundData.soundRValue.soundIndex = g_ModuleInterface->CallBuiltin("audio_create_stream", { (dirStr + soundData.soundRValue.soundFile).c_str() });
				}
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
			setInstanceVariable(config, GML_collides, charData.attackCollides);
			setInstanceVariable(config, GML_isMain, true);
			setInstanceVariable(config, GML_applyWeaponSize, true);
			setInstanceVariable(config, GML_maxLevel, 7);
			setInstanceVariable(config, GML_weaponType, charData.mainWeaponWeaponType.c_str());
			setInstanceVariable(config, GML_optionType, "Weapon");
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
			setInstanceVariable(config, GML_sprite_index, curCharSprite.projectileAnimationPtrMap[projectileData.data.projectileName]->spriteRValue);
			setJSONNumberToStruct(config, GML_duration, projectileData.data.projectileDuration);
			setJSONNumberToStruct(config, GML_damage, projectileData.data.projectileDamage);
			setJSONNumberToStruct(config, GML_hitLimit, projectileData.data.projectileHitLimit);
			setJSONNumberToStruct(config, GML_speed, projectileData.data.projectileSpeed);
			setJSONNumberToStruct(config, GML_hitCD, projectileData.data.projectileHitCD);
			setJSONNumberToStruct(config, GML_range, projectileData.data.projectileHitRange);
			setInstanceVariable(config, GML_attackID, projectileData.data.projectileName.c_str());
			setInstanceVariable(config, GML_optionID, projectileData.data.projectileName.c_str());
			setInstanceVariable(config, GML_onCreate, RValue());
			setInstanceVariable(config, GML_customDrawScriptBelow, RValue());
			setInstanceVariable(config, GML_collides, true);
			setInstanceVariable(config, GML_isMain, false);
			// TODO: Add weapon type to the projectile
			/*
			setInstanceVariable(config, GML_weaponType, charData.mainWeaponWeaponType.c_str());
			*/
			g_ModuleInterface->CallBuiltin("ds_map_set", { attackIndexMap, projectileData.data.projectileName.c_str(), newAttack });
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
		if (buffData.data.attackIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_ATK, getInstanceVariable(playerCharacter, GML_ATK).ToDouble() + buffData.data.attackIncrement.value / 100.0 * stacks.ToInt32());
		}
		if (buffData.data.critIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_crit, getInstanceVariable(playerCharacter, GML_crit).ToDouble() + buffData.data.critIncrement.value * stacks.ToInt32());
		}
		if (buffData.data.hasteIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_haste, getInstanceVariable(playerCharacter, GML_haste).ToDouble() + buffData.data.hasteIncrement.value * stacks.ToInt32());
		}
		if (buffData.data.speedIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_SPD, getInstanceVariable(playerCharacter, GML_SPD).ToDouble() + buffData.data.speedIncrement.value / 100.0 * stacks.ToInt32());
		}
		if (buffData.data.DRMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_DR, getInstanceVariable(playerCharacter, GML_DR).ToDouble() * buffData.data.DRMultiplier.value * stacks.ToInt32());
		}
		if (buffData.data.healMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_healMultiplier, getInstanceVariable(playerCharacter, GML_healMultiplier).ToDouble() + buffData.data.healMultiplier.value * stacks.ToInt32());
		}
		if (buffData.data.food.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_food, getInstanceVariable(playerCharacter, GML_food).ToDouble() + buffData.data.food.value * stacks.ToInt32());
		}
		if (buffData.data.weaponSize.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_weaponSize, getInstanceVariable(playerCharacter, GML_weaponSize).ToDouble() + buffData.data.weaponSize.value * stacks.ToInt32());
		}
		if (buffData.data.pickupRange.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_pickupRange, getInstanceVariable(playerCharacter, GML_pickupRange).ToDouble() + buffData.data.pickupRange.value * stacks.ToInt32());
		}
		if (buffData.data.critMod.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_CritMod, getInstanceVariable(playerCharacter, GML_CritMod).ToDouble() + buffData.data.critMod.value * stacks.ToInt32());
		}
		if (buffData.data.bonusProjectile.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_bonusProjectiles, getInstanceVariable(playerCharacter, GML_bonusProjectiles).ToDouble() + buffData.data.bonusProjectile.value * stacks.ToInt32());
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
		if (buffData.data.attackIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_ATK, getInstanceVariable(playerCharacter, GML_ATK).ToDouble() - buffData.data.attackIncrement.value / 100.0 * stacks.ToInt32());
		}
		if (buffData.data.critIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_crit, getInstanceVariable(playerCharacter, GML_crit).ToDouble() - buffData.data.critIncrement.value * stacks.ToInt32());
		}
		if (buffData.data.hasteIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_haste, getInstanceVariable(playerCharacter, GML_haste).ToDouble() - buffData.data.hasteIncrement.value * stacks.ToInt32());
		}
		if (buffData.data.speedIncrement.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_SPD, getInstanceVariable(playerCharacter, GML_SPD).ToDouble() - buffData.data.speedIncrement.value / 100.0 * stacks.ToInt32());
		}
		if (buffData.data.DRMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_DR, getInstanceVariable(playerCharacter, GML_DR).ToDouble() / (buffData.data.DRMultiplier.value * stacks.ToInt32()));
		}
		if (buffData.data.healMultiplier.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_healMultiplier, getInstanceVariable(playerCharacter, GML_healMultiplier).ToDouble() - buffData.data.healMultiplier.value * stacks.ToInt32());
		}
		if (buffData.data.food.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_food, getInstanceVariable(playerCharacter, GML_food).ToDouble() - buffData.data.food.value * stacks.ToInt32());
		}
		if (buffData.data.weaponSize.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_weaponSize, getInstanceVariable(playerCharacter, GML_weaponSize).ToDouble() - buffData.data.weaponSize.value * stacks.ToInt32());
		}
		if (buffData.data.pickupRange.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_pickupRange, getInstanceVariable(playerCharacter, GML_pickupRange).ToDouble() - buffData.data.pickupRange.value * stacks.ToInt32());
		}
		if (buffData.data.critMod.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_CritMod, getInstanceVariable(playerCharacter, GML_CritMod).ToDouble() - buffData.data.critMod.value * stacks.ToInt32());
		}
		if (buffData.data.bonusProjectile.isDefined)
		{
			setInstanceVariable(playerCharacter, GML_bonusProjectiles, getInstanceVariable(playerCharacter, GML_bonusProjectiles).ToDouble() - buffData.data.bonusProjectile.value * stacks.ToInt32());
		}
	}
	return ReturnValue;
}

RValue& onDebuff(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
//	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	return ReturnValue;
}

RValue& onAttackCreate(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
//	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	return ReturnValue;
}

RValue& onCriticalHit(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	// TODO: Maybe make it so that it can target the attack instead of the player?
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
//	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	ReturnValue = *Args[3];
	return ReturnValue;
}

RValue& onHeal(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[1];
	auto& charData = charDataMap[playingCharName];
//	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	ReturnValue = *Args[0];
	return ReturnValue;
}

RValue& onKill(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	auto& charData = charDataMap[playingCharName];
//	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	return ReturnValue;
}

RValue& onTakeDamage(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[3];
	auto& charData = charDataMap[playingCharName];
//	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
	ReturnValue = *Args[0];
	return ReturnValue;
}

RValue& onDodge(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[3];
	auto& charData = charDataMap[playingCharName];
//	activateAction(charData, playerCharacter.ToInstance(), lastStructVarGetName.ToString());
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
			setInstanceVariable(buffStruct, GML_timer, buffData.data.timer.value);
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
	auto& curSkillLevelData = charData.skillDataList[curSkillIndex].data.skillLevelDataList[curSkillLevel];
	if (curSkillLevelData.attackIncrement.isDefined)
	{
		setInstanceVariable(playerCharacter, GML_ATK, getInstanceVariable(playerCharacter, GML_ATK).m_Real + curSkillLevelData.attackIncrement.value / 100.0);
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
		setInstanceVariable(playerCharacter, GML_SPD, getInstanceVariable(playerCharacter, GML_SPD).m_Real + curSkillLevelData.speedIncrement.value / 100.0);
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

	checkOnTrigger(Self, nodeEditorNodeType_OnSkillApply);

	// TODO: Get this to trigger from the node editor instead
	/*
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
	*/
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
			
			setInstanceVariable(newPerk, GML_id, skillData.data.skillName.c_str());
			setInstanceVariable(newPerk, GML_optionIcon, charSpriteMap[charDataPair.first].skillIconPtrList[j]->spriteRValue);
			setInstanceVariable(newPerk, GML_name, skillData.data.skillName.c_str());
			setInstanceVariable(newPerk, GML_optionName, skillData.data.skillName.c_str());
			setInstanceVariable(newPerk, GML_optionDescription, skillData.data.skillLevelDataList[0].skillDescription.c_str());
			setInstanceVariable(newPerk, GML_optionID, skillData.data.skillName.c_str());
			
			RValue descriptionArr = g_ModuleInterface->CallBuiltin("array_create", { 3 });
			RValue skillOnApply = g_ModuleInterface->CallBuiltin("array_create", { 3 });
			for (int level = 0; level < 3; level++)
			{
				RValue retVal;
				yyGMLPushContextStack(Self);
				yyGMLYYSetScriptRef(&retVal, onApplyList[j * 3 + level], Self);
				yyGMLPopContextStack(1);
				skillOnApply[level] = retVal;
				descriptionArr[level] = skillData.data.skillLevelDataList[level].skillDescription.c_str();
			}
			setInstanceVariable(newPerk, GML_OnApply, skillOnApply);

			RValue descContainer;
			g_RunnerInterface.StructCreate(&descContainer);
			setInstanceVariable(descContainer, GML_selectedLanguage, descriptionArr);
			g_ModuleInterface->CallBuiltin("variable_instance_set", { textContainer, std::string_view(skillData.data.skillName + "Description"), descContainer });

			g_ModuleInterface->CallBuiltin("ds_map_set", { perksMap, skillData.data.skillName.c_str(), newPerk});
		}
	}
}

nodeEditorNodePin* getConnectedOutputPinForInputPin(nodeEditorNode& curNode, nodeEditorNodePin& inputPin)
{
	auto& curLink = inputPin.nodeLinks[0];
	auto& outputPin = (curLink.startPinID == inputPin.pinID) ? curLink.endPinPtr : curLink.startPinPtr;
	return outputPin;
}

void copyPinValue(nodeEditorNodePin* inputPin, nodeEditorNodePin& outputPin)
{
	if (inputPin->pinType == nodeEditorPinType_Integer)
	{
		outputPin.data->pinIntegerResult = inputPin->data->pinIntegerResult;
	}
	else if (inputPin->pinType == nodeEditorPinType_Number)
	{
		outputPin.data->pinNumberResult = inputPin->data->pinNumberResult;
	}
	else if (inputPin->pinType == nodeEditorPinType_String)
	{
		outputPin.data->pinStringResult = inputPin->data->pinStringResult;
	}
	else if (inputPin->pinType == nodeEditorPinType_RValue)
	{
		outputPin.data->pinRValueResult = inputPin->data->pinRValueResult;
	}
	else if (inputPin->pinType == nodeEditorPinType_Instance)
	{
		outputPin.data->pinInstanceResult = inputPin->data->pinInstanceResult;
	}
	else if (inputPin->pinType == nodeEditorPinType_DSList)
	{
		outputPin.data->pinDSListResult = inputPin->data->pinDSListResult;
	}
	else if (inputPin->pinType == nodeEditorPinType_DSMap)
	{
		outputPin.data->pinDSMapResult = inputPin->data->pinDSMapResult;
	}
	else if (inputPin->pinType == nodeEditorPinType_Array)
	{
		outputPin.data->pinArrayResult = inputPin->data->pinArrayResult;
	}
}

void processAction(nodeEditorNode& curNode, CInstance* parentInstance, bool isCodeFlow)
{
	// TODO: Make delay a no-op since it'll be done in processCodeActionFlow
	switch (curNode.nodeType)
	{
		case nodeEditorNodeType_OnCreateProjectile:
		case nodeEditorNodeType_OnDestroyProjectile:
		case nodeEditorNodeType_Delay:
		case nodeEditorNodeType_MergeCodeFlow:
		case nodeEditorNodeType_OnFrameStep:
		case nodeEditorNodeType_OnSkillApply:
		case nodeEditorNodeType_MergeFlush:
		{
			// no-op
			return;
		}
		case nodeEditorNodeType_SpawnProjectile:
		{
			if (!isCodeFlow)
			{
				return;
			}
			RValue playerManager = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerManagerIndex, 0 });
			RValue playerCharacter = getInstanceVariable(playerManager, GML_playerCharacter);
			RValue attackController = g_ModuleInterface->CallBuiltin("instance_find", { objAttackControllerIndex, 0 });
			CInstance* attackControllerInstance = attackController.ToInstance();

			auto outputProjectilePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& outputProjectileNode = *outputProjectilePin->parentNodePtr;
			processAction(outputProjectileNode, parentInstance, false);

			auto outputDirectionPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
			auto& outputDirectionNode = *outputDirectionPin->parentNodePtr;
			processAction(outputDirectionNode, parentInstance, false);

			auto outputXPosPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[3]);
			auto& outputXPosNode = *outputXPosPin->parentNodePtr;
			processAction(outputXPosNode, parentInstance, false);

			auto outputYPosPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[4]);
			auto& outputYPosNode = *outputYPosPin->parentNodePtr;
			processAction(outputYPosNode, parentInstance, false);

			// TODO: Do more error checking later
			
			auto& actionProjectileData = outputProjectilePin->data->pinProjDataResult;
			RValue overrideConfig;
			g_RunnerInterface.StructCreate(&overrideConfig);
			g_RunnerInterface.StructAddDouble(&overrideConfig, "direction", outputDirectionPin->data->pinNumberResult);
			g_RunnerInterface.StructAddDouble(&overrideConfig, "x", outputXPosPin->data->pinNumberResult);
			g_RunnerInterface.StructAddDouble(&overrideConfig, "y", outputYPosPin->data->pinNumberResult);
			g_RunnerInterface.StructAddDouble(&overrideConfig, "damage", actionProjectileData.projectileDamage.value);
			g_RunnerInterface.StructAddInt(&overrideConfig, "duration", actionProjectileData.projectileDuration.value);
			g_RunnerInterface.StructAddInt(&overrideConfig, "hitCD", actionProjectileData.projectileHitCD.value);
			g_RunnerInterface.StructAddInt(&overrideConfig, "hitLimit", actionProjectileData.projectileHitLimit.value);
			g_RunnerInterface.StructAddInt(&overrideConfig, "range", actionProjectileData.projectileHitRange.value);
			g_RunnerInterface.StructAddDouble(&overrideConfig, "speed", actionProjectileData.projectileSpeed.value);
			g_RunnerInterface.StructAddBool(&overrideConfig, "collides", actionProjectileData.collides);
			g_RunnerInterface.StructAddBool(&overrideConfig, "destroyOnHitLimit", true);
			g_RunnerInterface.StructAddBool(&overrideConfig, "applyWeaponSize", true);
			g_RunnerInterface.StructAddBool(&overrideConfig, "isMain", actionProjectileData.isMain);

			if (actionProjectileData.isMain)
			{
				// TODO: Fix size stamp not working for main weapon projectiles
				auto& charData = charDataMap[playingCharName];
				RValue attackIndexMap = getInstanceVariable(playerCharacter, GML_attacks);
				RValue attack = g_ModuleInterface->CallBuiltin("ds_map_find_value", { attackIndexMap, charData.attackName.c_str() });
				RValue config = getInstanceVariable(attack, GML_config);
				RValue enhancements = getInstanceVariable(config, GML_enhancements);
				setInstanceVariable(overrideConfig, GML_enhancements, enhancements);
				setInstanceVariable(overrideConfig, GML_optionType, "Weapon");
				setInstanceVariable(overrideConfig, GML_onHitEffects, g_ModuleInterface->CallBuiltin("variable_clone", { getInstanceVariable(config, GML_onHitEffects) }));
			}

			// TODO: Check if this can use the global RValue array instead of creating a new one
			RValue** args = new RValue*[3];
			args[0] = new RValue(actionProjectileData.projectileName.c_str());
			args[1] = new RValue(playerCharacter);
			args[2] = new RValue(overrideConfig);
			RValue result;
			origExecuteAttackScript(attackControllerInstance, attackControllerInstance, result, 3, args);

			auto& outputInstancePin = *curNode.outputPinPtrArr[1];
			outputInstancePin.data->pinInstanceResult = result.ToInstance();
			return;
		}
		case nodeEditorNodeType_ApplyBuff:
		{
			if (!isCodeFlow)
			{
				return;
			}
			RValue playerManager = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerManagerIndex, 0 });
			RValue playerCharacter = getInstanceVariable(playerManager, GML_playerCharacter);
			RValue attackController = g_ModuleInterface->CallBuiltin("instance_find", { objAttackControllerIndex, 0 });
			CInstance* attackControllerInstance = attackController.ToInstance();

			auto outputBuffPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& outputBuffNode = *outputBuffPin->parentNodePtr;
			processAction(outputBuffNode, parentInstance, false);

			auto outputStacksPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
			auto& outputStacksNode = *outputStacksPin->parentNodePtr;
			processAction(outputStacksNode, parentInstance, false);

			auto outputTimerPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[3]);
			auto& outputTimerNode = *outputTimerPin->parentNodePtr;
			processAction(outputTimerNode, parentInstance, false);

			// TODO: Do more error checking later

			auto& buffData = outputBuffPin->data->pinBuffDataResult;
			RValue buffsMap = getInstanceVariable(attackController, GML_Buffs);
			RValue buffsMapData = g_ModuleInterface->CallBuiltin("ds_map_find_value", { buffsMap, buffData.buffName.c_str() });
			RValue buffConfig;
			g_RunnerInterface.StructCreate(&buffConfig);
			setInstanceVariable(buffConfig, GML_reapply, true);
			setInstanceVariable(buffConfig, GML_stacks, outputStacksPin->data->pinIntegerResult);
			setInstanceVariable(buffConfig, GML_maxStacks, outputBuffPin->data->pinBuffDataResult.data.maxStacks.value);
			setInstanceVariable(buffConfig, GML_buffName, outputBuffPin->data->pinBuffDataResult.buffName.c_str());
			setInstanceVariable(buffConfig, GML_buffIcon, getInstanceVariable(buffsMapData, GML_buffIcon));

			RValue ApplyBuffMethod = getInstanceVariable(attackController, GML_ApplyBuff);
			RValue ApplyBuffArr = g_ModuleInterface->CallBuiltin("array_create", { 4 });
			ApplyBuffArr[0] = playerCharacter;
			ApplyBuffArr[1] = buffData.buffName.c_str();
			ApplyBuffArr[2] = buffsMapData;
			ApplyBuffArr[3] = buffConfig;
			g_ModuleInterface->CallBuiltin("method_call", { ApplyBuffMethod, ApplyBuffArr });
			return;
		}
		case nodeEditorNodeType_Number:
		{
			auto& outputNumberPin = *curNode.outputPinPtrArr[0];
			outputNumberPin.data->pinNumberResult = outputNumberPin.pinNumberVar;
			return;
		}
		case nodeEditorNodeType_ProjectileData:
		{
			auto& outputProjectilePin = *curNode.outputPinPtrArr[0];
			auto& charData = charDataMap[playingCharName];
			for (auto& projData : charData.projectileDataList)
			{
				if (projData.data.projectileName.compare(outputProjectilePin.pinProjectileDataName) == 0)
				{
					outputProjectilePin.data->pinProjDataResult = projData.data;
					break;
				}
			}

			auto& curProjData = outputProjectilePin.data->pinProjDataResult;

			if (!(*curNode.inputPinPtrArr[0]).nodeLinks.empty())
			{
				auto parentNodeOutputNumberPinDamageOverride = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
				auto& parentNodeOutputNumberNodeDamageOverride = *parentNodeOutputNumberPinDamageOverride->parentNodePtr;
				processAction(parentNodeOutputNumberNodeDamageOverride, parentInstance, false);
				curProjData.projectileDamage.value = parentNodeOutputNumberPinDamageOverride->data->pinNumberResult;
			}
			
			if (!(*curNode.inputPinPtrArr[1]).nodeLinks.empty())
			{
				auto parentNodeOutputNumberPinDurationOverride = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
				auto& parentNodeOutputNumberNodeDurationOverride = *parentNodeOutputNumberPinDurationOverride->parentNodePtr;
				processAction(parentNodeOutputNumberNodeDurationOverride, parentInstance, false);
				curProjData.projectileDuration.value = parentNodeOutputNumberPinDurationOverride->data->pinIntegerResult;
			}

			if (!(*curNode.inputPinPtrArr[2]).nodeLinks.empty())
			{
				auto parentNodeOutputNumberPinHitCooldownOverride = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
				auto& parentNodeOutputNumberNodeHitCooldownOverride = *parentNodeOutputNumberPinHitCooldownOverride->parentNodePtr;
				processAction(parentNodeOutputNumberNodeHitCooldownOverride, parentInstance, false);
				curProjData.projectileHitCD.value = parentNodeOutputNumberPinHitCooldownOverride->data->pinIntegerResult;
			}

			if (!(*curNode.inputPinPtrArr[3]).nodeLinks.empty())
			{
				auto parentNodeOutputNumberPinHitLimitOverride = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[3]);
				auto& parentNodeOutputNumberNodeHitLimitOverride = *parentNodeOutputNumberPinHitLimitOverride->parentNodePtr;
				processAction(parentNodeOutputNumberNodeHitLimitOverride, parentInstance, false);
				curProjData.projectileHitLimit.value = parentNodeOutputNumberPinHitLimitOverride->data->pinIntegerResult;
			}

			if (!(*curNode.inputPinPtrArr[4]).nodeLinks.empty())
			{
				auto parentNodeOutputNumberPinHitRangeOverride = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[4]);
				auto& parentNodeOutputNumberNodeHitRangeOverride = *parentNodeOutputNumberPinHitRangeOverride->parentNodePtr;
				processAction(parentNodeOutputNumberNodeHitRangeOverride, parentInstance, false);
				curProjData.projectileHitRange.value = parentNodeOutputNumberPinHitRangeOverride->data->pinIntegerResult;
			}

			if (!(*curNode.inputPinPtrArr[5]).nodeLinks.empty())
			{
				auto parentNodeOutputNumberPinProjectileSpeedOverride = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[5]);
				auto& parentNodeOutputNumberNodeProjectileSpeedOverride = *parentNodeOutputNumberPinProjectileSpeedOverride->parentNodePtr;
				processAction(parentNodeOutputNumberNodeProjectileSpeedOverride, parentInstance, false);
				curProjData.projectileSpeed.value = parentNodeOutputNumberPinProjectileSpeedOverride->data->pinNumberResult;
			}
			
			return;
		}
		case nodeEditorNodeType_BuffData:
		{
			auto& outputBuffPin = *curNode.outputPinPtrArr[0];
			auto& charData = charDataMap[playingCharName];
			for (auto& buffData : charData.buffDataList)
			{
				if (buffData.buffName.compare(outputBuffPin.pinBuffDataName) == 0)
				{
					outputBuffPin.data->pinBuffDataResult = buffData;
					break;
				}
			}

			auto& curBuffData = outputBuffPin.data->pinBuffDataResult;

			if (!(*curNode.inputPinPtrArr[0]).nodeLinks.empty())
			{
				auto parentNodeOutputIntegerPinMaxStacksOverride = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
				auto& parentNodeOutputIntegerNodeMaxStacksOverride = *parentNodeOutputIntegerPinMaxStacksOverride->parentNodePtr;
				processAction(parentNodeOutputIntegerNodeMaxStacksOverride, parentInstance, false);
				curBuffData.data.maxStacks.isDefined = true;
				curBuffData.data.maxStacks.value = parentNodeOutputIntegerPinMaxStacksOverride->data->pinIntegerResult;
			}

			return;
		}
		case nodeEditorNodeType_SoundData:
		{
			auto& outputSoundPin = *curNode.outputPinPtrArr[0];
			auto& charData = charDataMap[playingCharName];
			for (auto& soundData : charData.soundDataList)
			{
				if (soundData.soundName.compare(outputSoundPin.pinSoundDataName) == 0)
				{
					outputSoundPin.data->pinSoundDataResult = soundData;
					break;
				}
			}
			return;
		}
		case nodeEditorNodeType_ThisInstance:
		{
			auto& outputInstancePin = *curNode.outputPinPtrArr[0];
			outputInstancePin.data->pinInstanceResult = parentInstance;
			return;
		}
		case nodeEditorNodeType_GlobalInstance:
		{
			auto& outputInstancePin = *curNode.outputPinPtrArr[0];
			outputInstancePin.data->pinInstanceResult = globalInstance;
			return;
		}
		case nodeEditorNodeType_PlayerManagerInstance:
		{
			auto& outputInstancePin = *curNode.outputPinPtrArr[0];
			outputInstancePin.data->pinRValueResult = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerManagerIndex, 0 });
			return;
		}
		case nodeEditorNodeType_PlayerInstance:
		{
			auto& outputInstancePin = *curNode.outputPinPtrArr[0];
			outputInstancePin.data->pinRValueResult = g_ModuleInterface->CallBuiltin("instance_find", { objPlayerIndex, 0 });
			return;
		}
		case nodeEditorNodeType_GetVariable:
		{
			auto outputInstancePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& outputInstanceNode = *outputInstancePin->parentNodePtr;
			processAction(outputInstanceNode, parentInstance, false);
			CInstance* outputInstance = outputInstancePin->data->pinInstanceResult;

			auto& inputVarNamePin = *curNode.inputPinPtrArr[1];
			auto& outputVarPin = *curNode.outputPinPtrArr[0];

			if (inputVarNamePin.variableDataType == pinVariableDataType_Direction)
			{
				outputVarPin.data->pinNumberResult = getInstanceVariable(outputInstance, GML_direction).ToDouble();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Creator)
			{
				outputVarPin.data->pinInstanceResult = getInstanceVariable(outputInstance, GML_creator).ToInstance();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_X)
			{
				outputVarPin.data->pinNumberResult = getInstanceVariable(outputInstance, GML_x).ToDouble();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Y)
			{
				outputVarPin.data->pinNumberResult = getInstanceVariable(outputInstance, GML_y).ToDouble();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_CustomRValue)
			{
				auto rvalueInstancePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
				auto& rvalueInstanceNode = *rvalueInstancePin->parentNodePtr;
				processAction(rvalueInstanceNode, parentInstance, false);

				if (rvalueInstancePin->data->isStringGMLHashSet)
				{
					RValueInputArgs[0] = outputInstance;
					RValueInputArgs[1] = rvalueInstancePin->data->stringGMLHash;
					origStructGetFromHashFunc(outputVarPin.data->pinRValueResult, globalInstance, nullptr, 2, RValueInputArgs);
				}
				else
				{
					outputVarPin.data->pinRValueResult = g_ModuleInterface->CallBuiltin("variable_instance_get", { outputInstance, rvalueInstancePin->pinStringVar.c_str() });
				}
			}
			else
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown Variable data name %s", inputVarNamePin.pinVariableDataName.c_str());
			}

			return;
		}
		case nodeEditorNodeType_SetVariable:
		{
			if (!isCodeFlow)
			{
				return;
			}

			auto outputInstancePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& outputInstanceNode = *outputInstancePin->parentNodePtr;
			processAction(outputInstanceNode, parentInstance, false);
			CInstance* outputInstance = outputInstancePin->data->pinInstanceResult;

			auto& inputVarNamePin = *curNode.inputPinPtrArr[2];
			auto& inputVarDataPin = *curNode.inputPinPtrArr[3];

			auto outputVariablePin = getConnectedOutputPinForInputPin(curNode, inputVarDataPin);
			auto& outputVariableNode = *outputVariablePin->parentNodePtr;
			processAction(outputVariableNode, parentInstance, false);

			if (inputVarNamePin.variableDataType == pinVariableDataType_Direction)
			{
				setInstanceVariable(outputInstance, GML_direction, outputVariablePin->data->pinNumberResult);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Creator)
			{
				CInstance* outputCreatorInstance = outputVariablePin->data->pinInstanceResult;
				setInstanceVariable(outputInstance, GML_creator, outputCreatorInstance);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_X)
			{
				setInstanceVariable(outputInstance, GML_x, outputVariablePin->data->pinNumberResult);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Y)
			{
				setInstanceVariable(outputInstance, GML_y, outputVariablePin->data->pinNumberResult);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_CustomRValue)
			{
				auto rvalueInstancePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[4]);
				auto& rvalueInstanceNode = *rvalueInstancePin->parentNodePtr;
				processAction(rvalueInstanceNode, parentInstance, false);

				if (rvalueInstancePin->data->isStringGMLHashSet)
				{
					RValueInputArgs[0] = outputInstance;
					RValueInputArgs[1] = rvalueInstancePin->data->stringGMLHash;
					RValueInputArgs[2] = outputVariablePin->data->pinRValueResult;
					origStructSetFromHashFunc(tempResult, globalInstance, nullptr, 3, RValueInputArgs);
				}
				else
				{
					g_ModuleInterface->CallBuiltin("variable_instance_set", { outputInstance, rvalueInstancePin->pinStringVar.c_str(), outputVariablePin->data->pinRValueResult });
				}
			}
			else
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown Variable data name %s", inputVarNamePin.pinVariableDataName.c_str());
			}

			return;
		}
		case nodeEditorNodeType_GetStructVariable:
		{
			// TODO: Figure out a way to avoid duplicating this code
			auto outputRValuePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& outputRValueNode = *outputRValuePin->parentNodePtr;
			processAction(outputRValueNode, parentInstance, false);

			auto& inputVarNamePin = *curNode.inputPinPtrArr[1];
			auto& outputVarPin = *curNode.outputPinPtrArr[0];

			if (inputVarNamePin.variableDataType == pinVariableDataType_Direction)
			{
				outputVarPin.data->pinNumberResult = getInstanceVariable(outputRValuePin->data->pinRValueResult, GML_direction).ToDouble();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Creator)
			{
				outputVarPin.data->pinInstanceResult = getInstanceVariable(outputRValuePin->data->pinRValueResult, GML_creator).ToInstance();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_X)
			{
				outputVarPin.data->pinNumberResult = getInstanceVariable(outputRValuePin->data->pinRValueResult, GML_x).ToDouble();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Y)
			{
				outputVarPin.data->pinNumberResult = getInstanceVariable(outputRValuePin->data->pinRValueResult, GML_y).ToDouble();
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_CustomRValue)
			{
				auto rvalueInstancePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
				auto& rvalueInstanceNode = *rvalueInstancePin->parentNodePtr;
				processAction(rvalueInstanceNode, parentInstance, false);

				if (rvalueInstancePin->data->isStringGMLHashSet)
				{
					RValueInputArgs[0] = outputRValuePin->data->pinRValueResult;
					RValueInputArgs[1] = rvalueInstancePin->data->stringGMLHash;
					origStructGetFromHashFunc(outputVarPin.data->pinRValueResult, globalInstance, nullptr, 2, RValueInputArgs);
				}
				else
				{
					outputVarPin.data->pinRValueResult = g_ModuleInterface->CallBuiltin("variable_instance_get", { outputRValuePin->data->pinRValueResult, rvalueInstancePin->pinStringVar.c_str() });
				}
			}
			else
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown Variable data name %s", inputVarNamePin.pinVariableDataName.c_str());
			}

			return;
		}
		case nodeEditorNodeType_SetStructVariable:
		{
			// TODO: Figure out a way to avoid duplicating this code
			if (!isCodeFlow)
			{
				return;
			}

			auto outputRValuePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& outputRValueNode = *outputRValuePin->parentNodePtr;
			processAction(outputRValueNode, parentInstance, false);

			auto& inputVarNamePin = *curNode.inputPinPtrArr[2];
			auto& inputVarDataPin = *curNode.inputPinPtrArr[3];

			auto outputVariablePin = getConnectedOutputPinForInputPin(curNode, inputVarDataPin);
			auto& outputVariableNode = *outputVariablePin->parentNodePtr;
			processAction(outputVariableNode, parentInstance, false);

			if (inputVarNamePin.variableDataType == pinVariableDataType_Direction)
			{
				setInstanceVariable(outputRValuePin->data->pinRValueResult, GML_direction, outputVariablePin->data->pinNumberResult);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Creator)
			{
				CInstance* outputCreatorInstance = outputVariablePin->data->pinInstanceResult;
				setInstanceVariable(outputRValuePin->data->pinRValueResult, GML_creator, outputCreatorInstance);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_X)
			{
				setInstanceVariable(outputRValuePin->data->pinRValueResult, GML_x, outputVariablePin->data->pinNumberResult);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_Y)
			{
				setInstanceVariable(outputRValuePin->data->pinRValueResult, GML_y, outputVariablePin->data->pinNumberResult);
			}
			else if (inputVarNamePin.variableDataType == pinVariableDataType_CustomRValue)
			{
				auto rvalueInstancePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[4]);
				auto& rvalueInstanceNode = *rvalueInstancePin->parentNodePtr;
				processAction(rvalueInstanceNode, parentInstance, false);

				if (rvalueInstancePin->data->isStringGMLHashSet)
				{
					RValueInputArgs[0] = outputRValuePin->data->pinRValueResult;
					RValueInputArgs[1] = rvalueInstancePin->data->stringGMLHash;
					RValueInputArgs[2] = outputVariablePin->data->pinRValueResult;
					origStructSetFromHashFunc(tempResult, globalInstance, nullptr, 3, RValueInputArgs);
				}
				else
				{
					g_ModuleInterface->CallBuiltin("variable_instance_set", { outputRValuePin->data->pinRValueResult, rvalueInstancePin->pinStringVar.c_str(), outputVariablePin->data->pinRValueResult });
				}
			}
			else
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown Variable data name %s", inputVarNamePin.pinVariableDataName.c_str());
			}

			return;
		}
		case nodeEditorNodeType_Add:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = parentNodeOutputNumberPinOne->data->pinNumberResult + parentNodeOutputNumberPinTwo->data->pinNumberResult;
			return;
		}
		case nodeEditorNodeType_Subtract:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = parentNodeOutputNumberPinOne->data->pinNumberResult - parentNodeOutputNumberPinTwo->data->pinNumberResult;
			return;
		}
		case nodeEditorNodeType_Multiply:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = parentNodeOutputNumberPinOne->data->pinNumberResult * parentNodeOutputNumberPinTwo->data->pinNumberResult;
			return;
		}
		case nodeEditorNodeType_Divide:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			if (parentNodeOutputNumberPinTwo->data->pinNumberResult == 0)
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Divide by zero error for node %d", curNode.nodeID);
				return;
			}

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = parentNodeOutputNumberPinOne->data->pinNumberResult / parentNodeOutputNumberPinTwo->data->pinNumberResult;
			return;
		}
		case nodeEditorNodeType_Boolean:
		{
			auto& outputBooleanPin = *curNode.outputPinPtrArr[0];
			// TODO: Should change it to not require a string comparison every time
			if (outputBooleanPin.booleanDataType == pinBooleanDataType_True)
			{
				outputBooleanPin.data->pinBoolDataResult = true;
			}
			else
			{
				outputBooleanPin.data->pinBoolDataResult = false;
			}
			return;
		}
		case nodeEditorNodeType_If:
		{
			if (!isCodeFlow)
			{
				return;
			}
			auto parentNodeOutputBooleanPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputBooleanNode = *parentNodeOutputBooleanPin->parentNodePtr;
			processAction(parentNodeOutputBooleanNode, parentInstance, false);
			return;
		}
		case nodeEditorNodeType_Compare:
		{
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			auto& inputCompareTypePin = *curNode.inputPinPtrArr[2];
			auto& currentNodeOutputBooleanPin = *curNode.outputPinPtrArr[0];
			if (inputCompareTypePin.compareDataType == pinCompareDataType_Less)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputNumberPinOne->data->pinNumberResult < parentNodeOutputNumberPinTwo->data->pinNumberResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_LessOrEqual)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputNumberPinOne->data->pinNumberResult <= parentNodeOutputNumberPinTwo->data->pinNumberResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_Equal)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputNumberPinOne->data->pinNumberResult == parentNodeOutputNumberPinTwo->data->pinNumberResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_Greater)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputNumberPinOne->data->pinNumberResult > parentNodeOutputNumberPinTwo->data->pinNumberResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_GreaterOrEqual)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputNumberPinOne->data->pinNumberResult >= parentNodeOutputNumberPinTwo->data->pinNumberResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_NotEqual)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputNumberPinOne->data->pinNumberResult != parentNodeOutputNumberPinTwo->data->pinNumberResult;
			}
			return;
		}
		case nodeEditorNodeType_While:
		{
			if (!isCodeFlow)
			{
				return;
			}
			auto parentNodeOutputBooleanPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputBooleanNode = *parentNodeOutputBooleanPin->parentNodePtr;
			processAction(parentNodeOutputBooleanNode, parentInstance, false);
			return;
		}
		case nodeEditorNodeType_And:
		{
			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];

			auto parentNodeOutputBooleanPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputBooleanNodeOne = *parentNodeOutputBooleanPinOne->parentNodePtr;
			processAction(parentNodeOutputBooleanNodeOne, parentInstance, false);

			// Short circuit
			if (!parentNodeOutputBooleanPinOne->data->pinBoolDataResult)
			{
				currentNodeOutputNumberPin.data->pinBoolDataResult = false;
				return;
			}

			auto parentNodeOutputBooleanPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputBooleanNodeTwo = *parentNodeOutputBooleanPinTwo->parentNodePtr;
			processAction(parentNodeOutputBooleanNodeTwo, parentInstance, false);

			currentNodeOutputNumberPin.data->pinBoolDataResult = parentNodeOutputBooleanPinTwo->data->pinBoolDataResult;
			return;
		}
		case nodeEditorNodeType_Or:
		{
			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];

			auto parentNodeOutputBooleanPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputBooleanNodeOne = *parentNodeOutputBooleanPinOne->parentNodePtr;
			processAction(parentNodeOutputBooleanNodeOne, parentInstance, false);

			// Short circuit
			if (parentNodeOutputBooleanPinOne->data->pinBoolDataResult)
			{
				currentNodeOutputNumberPin.data->pinBoolDataResult = true;
				return;
			}

			auto parentNodeOutputBooleanPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputBooleanNodeTwo = *parentNodeOutputBooleanPinTwo->parentNodePtr;
			processAction(parentNodeOutputBooleanNodeTwo, parentInstance, false);

			currentNodeOutputNumberPin.data->pinBoolDataResult = parentNodeOutputBooleanPinTwo->data->pinBoolDataResult;
			return;
		}
		case nodeEditorNodeType_Not:
		{
			auto parentNodeOutputBooleanPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputBooleanNode = *parentNodeOutputBooleanPin->parentNodePtr;
			processAction(parentNodeOutputBooleanNode, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinBoolDataResult = !parentNodeOutputBooleanPin->data->pinBoolDataResult;
			return;
		}
		case nodeEditorNodeType_String:
		{
			auto& outputStringPin = *curNode.outputPinPtrArr[0];
			if (!outputStringPin.data->isStringGMLHashSet)
			{
				outputStringPin.data->pinStringResult = outputStringPin.pinStringVar;
				outputStringPin.data->stringGMLHash = g_ModuleInterface->CallBuiltin("variable_get_hash", { outputStringPin.pinStringVar.c_str() });
				outputStringPin.data->isStringGMLHashSet = true;
			}
			return;
		}
		case nodeEditorNodeType_Print:
		{
			if (!isCodeFlow)
			{
				return;
			}

			auto parentNodeOutputStringPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputStringNode = *parentNodeOutputStringPin->parentNodePtr;
			processAction(parentNodeOutputStringNode, parentInstance, false);

			DbgPrintEx(LOG_SEVERITY_INFO, "%s", parentNodeOutputStringPin->data->pinStringResult.c_str());
			callbackManagerInterfacePtr->LogToFile(MODNAME, "%s", parentNodeOutputStringPin->data->pinStringResult.c_str());
			return;
		}
		case nodeEditorNodeType_TypeCast:
		{
			auto parentNodeOutputPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNode = *parentNodeOutputPin->parentNodePtr;
			processAction(parentNodeOutputNode, parentInstance, false);

			auto& currentNodeOutputPin = *curNode.outputPinPtrArr[1];

			if (parentNodeOutputPin->pinType == nodeEditorPinType_Integer)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_String)
				{
					currentNodeOutputPin.data->pinStringResult = std::format("{}", parentNodeOutputPin->data->pinIntegerResult);
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_Number)
				{
					currentNodeOutputPin.data->pinNumberResult = parentNodeOutputPin->data->pinIntegerResult;
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinIntegerResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_Boolean)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinBoolDataResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_Number)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_String)
				{
					currentNodeOutputPin.data->pinStringResult = std::format("{}", parentNodeOutputPin->data->pinNumberResult);
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinNumberResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_String)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinStringResult.c_str();
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_RValue)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_String)
				{
					currentNodeOutputPin.data->pinStringResult = parentNodeOutputPin->data->pinRValueResult.ToString();
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_Number)
				{
					currentNodeOutputPin.data->pinNumberResult = parentNodeOutputPin->data->pinRValueResult.ToDouble();
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_Integer)
				{
					currentNodeOutputPin.data->pinIntegerResult = parentNodeOutputPin->data->pinRValueResult.ToInt32();
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_Boolean)
				{
					currentNodeOutputPin.data->pinBoolDataResult = parentNodeOutputPin->data->pinRValueResult.ToBoolean();
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_Instance)
				{
					currentNodeOutputPin.data->pinInstanceResult = parentNodeOutputPin->data->pinRValueResult.ToInstance();
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_Array)
				{
					// TODO: Maybe do some type checking?
					currentNodeOutputPin.data->pinArrayResult = parentNodeOutputPin->data->pinRValueResult;
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_DSMap)
				{
					// TODO: Maybe do some type checking?
					currentNodeOutputPin.data->pinDSMapResult = parentNodeOutputPin->data->pinRValueResult;
				}
				else if (currentNodeOutputPin.pinType == nodeEditorPinType_DSList)
				{
					// TODO: Maybe do some type checking?
					currentNodeOutputPin.data->pinDSListResult = parentNodeOutputPin->data->pinRValueResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_Instance)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinInstanceResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_Array)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinArrayResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_DSMap)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinDSMapResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_DSList)
			{
				if (currentNodeOutputPin.pinType == nodeEditorPinType_RValue)
				{
					currentNodeOutputPin.data->pinRValueResult = parentNodeOutputPin->data->pinDSListResult;
				}
				else
				{
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown type cast to %s %d", (*curNode.outputPinPtrArr[0]).pinVariableTypeName.c_str(), currentNodeOutputPin.pinType);
				}
				}
			else
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown input type cast %s %d", (*curNode.inputPinPtrArr[0]).pinVariableTypeName.c_str(), parentNodeOutputPin->pinType);
			}
			return;
		}
		case nodeEditorNodeType_AppendString:
		{
			auto parentNodeOutputStringPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputStringNodeOne = *parentNodeOutputStringPinOne->parentNodePtr;
			processAction(parentNodeOutputStringNodeOne, parentInstance, false);

			auto parentNodeOutputStringPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputStringNodeTwo = *parentNodeOutputStringPinTwo->parentNodePtr;
			processAction(parentNodeOutputStringNodeTwo, parentInstance, false);

			auto& currentNodeOutputStringPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputStringPin.data->pinStringResult = parentNodeOutputStringPinOne->data->pinStringResult + parentNodeOutputStringPinTwo->data->pinStringResult;
			return;
		}
		case nodeEditorNodeType_Integer:
		{
			auto& outputIntegerPin = *curNode.outputPinPtrArr[0];
			outputIntegerPin.data->pinIntegerResult = outputIntegerPin.pinIntegerVar;
			return;
		}
		case nodeEditorNodeType_IntegerCeiling:
		{
			auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
			processAction(parentNodeOutputNumberNode, parentInstance, false);

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = static_cast<int>(ceil(parentNodeOutputNumberPin->data->pinNumberResult));
			return;
		}
		case nodeEditorNodeType_IntegerFloor:
		{
			auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
			processAction(parentNodeOutputNumberNode, parentInstance, false);

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = static_cast<int>(floor(parentNodeOutputNumberPin->data->pinNumberResult));
			return;
		}
		case nodeEditorNodeType_IntegerRound:
		{
			auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
			processAction(parentNodeOutputNumberNode, parentInstance, false);

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = static_cast<int>(lround(parentNodeOutputNumberPin->data->pinNumberResult));
			return;
		}
		case nodeEditorNodeType_IntegerAdd:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputIntegerPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputIntegerNodeOne = *parentNodeOutputIntegerPinOne->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeOne, parentInstance, false);

			auto parentNodeOutputIntegerPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeTwo = *parentNodeOutputIntegerPinTwo->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeTwo, parentInstance, false);

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult + parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			return;
		}
		case nodeEditorNodeType_IntegerSubtract:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputIntegerPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputIntegerNodeOne = *parentNodeOutputIntegerPinOne->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeOne, parentInstance, false);

			auto parentNodeOutputIntegerPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeTwo = *parentNodeOutputIntegerPinTwo->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeTwo, parentInstance, false);

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult - parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			return;
		}
		case nodeEditorNodeType_IntegerMultiply:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputIntegerPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputIntegerNodeOne = *parentNodeOutputIntegerPinOne->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeOne, parentInstance, false);

			auto parentNodeOutputIntegerPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeTwo = *parentNodeOutputIntegerPinTwo->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeTwo, parentInstance, false);

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult * parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			return;
		}
		case nodeEditorNodeType_IntegerDivide:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputIntegerPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputIntegerNodeOne = *parentNodeOutputIntegerPinOne->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeOne, parentInstance, false);

			auto parentNodeOutputIntegerPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeTwo = *parentNodeOutputIntegerPinTwo->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeTwo, parentInstance, false);

			if (parentNodeOutputIntegerPinTwo->data->pinIntegerResult == 0)
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Divide by zero error for node %d", curNode.nodeID);
				return;
			}

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult / parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			return;
		}
		case nodeEditorNodeType_IntegerCompare:
		{
			auto parentNodeOutputIntegerPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputIntegerNodeOne = *parentNodeOutputIntegerPinOne->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeOne, parentInstance, false);

			auto parentNodeOutputIntegerPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeTwo = *parentNodeOutputIntegerPinTwo->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeTwo, parentInstance, false);

			auto& inputCompareTypePin = *curNode.inputPinPtrArr[2];
			auto& currentNodeOutputBooleanPin = *curNode.outputPinPtrArr[0];
			if (inputCompareTypePin.compareDataType == pinCompareDataType_Less)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult < parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_LessOrEqual)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult <= parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_Equal)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult == parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_Greater)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult > parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_GreaterOrEqual)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult >= parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			}
			else if (inputCompareTypePin.compareDataType == pinCompareDataType_NotEqual)
			{
				currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputIntegerPinOne->data->pinIntegerResult != parentNodeOutputIntegerPinTwo->data->pinIntegerResult;
			}
			return;
		}
		case nodeEditorNodeType_Random:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			std::uniform_real_distribution<double> distribution(parentNodeOutputNumberPinOne->data->pinNumberResult, parentNodeOutputNumberPinTwo->data->pinNumberResult);
			currentNodeOutputNumberPin.data->pinNumberResult = distribution(randomGenerator);
			return;
		}
		case nodeEditorNodeType_IntegerRandom:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputIntegerPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputIntegerNodeOne = *parentNodeOutputIntegerPinOne->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeOne, parentInstance, false);

			auto parentNodeOutputIntegerPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeTwo = *parentNodeOutputIntegerPinTwo->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeTwo, parentInstance, false);

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			std::uniform_int_distribution<int> distribution(parentNodeOutputIntegerPinOne->data->pinIntegerResult, parentNodeOutputIntegerPinTwo->data->pinIntegerResult);
			currentNodeOutputIntegerPin.data->pinIntegerResult = distribution(randomGenerator);
			return;
		}
		case nodeEditorNodeType_Modulus:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinIntegerResult = parentNodeOutputNumberPinOne->data->pinIntegerResult % parentNodeOutputNumberPinTwo->data->pinIntegerResult;
			return;
		}
		case nodeEditorNodeType_SquareRoot:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
			processAction(parentNodeOutputNumberNode, parentInstance, false);

			if (parentNodeOutputNumberPin->data->pinNumberResult < 0)
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Trying to square root %f for node %d", parentNodeOutputNumberPin->data->pinNumberResult, curNode.nodeID);
				return;
			}

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = sqrt(parentNodeOutputNumberPin->data->pinNumberResult);
			return;
		}
		case nodeEditorNodeType_Logarithm:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeOne = *parentNodeOutputNumberPinOne->parentNodePtr;
			processAction(parentNodeOutputNumberNodeOne, parentInstance, false);

			auto parentNodeOutputNumberPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeTwo = *parentNodeOutputNumberPinTwo->parentNodePtr;
			processAction(parentNodeOutputNumberNodeTwo, parentInstance, false);

			if (parentNodeOutputNumberPinOne->data->pinNumberResult <= 0)
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Trying to log %f for node %d", parentNodeOutputNumberPinOne->data->pinNumberResult, curNode.nodeID);
				return;
			}

			if (parentNodeOutputNumberPinTwo->data->pinNumberResult <= 0)
			{
				callbackManagerInterfacePtr->LogToFile(MODNAME, "Trying to log %f for node %d", parentNodeOutputNumberPinTwo->data->pinNumberResult, curNode.nodeID);
				return;
			}

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = log(parentNodeOutputNumberPinOne->data->pinNumberResult) / log(parentNodeOutputNumberPinTwo->data->pinNumberResult);
			return;
		}
		case nodeEditorNodeType_Sine:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
			processAction(parentNodeOutputNumberNode, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = sin(parentNodeOutputNumberPin->data->pinNumberResult);
			return;
		}
		case nodeEditorNodeType_Cosine:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
			processAction(parentNodeOutputNumberNode, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = cos(parentNodeOutputNumberPin->data->pinNumberResult);
			return;
		}
		case nodeEditorNodeType_Tangent:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
			processAction(parentNodeOutputNumberNode, parentInstance, false);

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = tan(parentNodeOutputNumberPin->data->pinNumberResult);
			return;
		}
		case nodeEditorNodeType_AssetGetIndex:
		{
			auto parentNodeOutputStringPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputStringNode = *parentNodeOutputStringPin->parentNodePtr;
			processAction(parentNodeOutputStringNode, parentInstance, false);

			auto& currentNodeOutputStringPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputStringPin.data->pinIntegerResult = g_ModuleInterface->CallBuiltin("asset_get_index", { parentNodeOutputStringPin->data->pinStringResult.c_str() }).ToInt32();
			return;
		}
		case nodeEditorNodeType_IsRValueDefined:
		{
			auto parentNodeOutputRValuePin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputRValueNode = *parentNodeOutputRValuePin->parentNodePtr;
			processAction(parentNodeOutputRValueNode, parentInstance, false);

			auto& currentNodeOutputBooleanPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputBooleanPin.data->pinBoolDataResult = parentNodeOutputRValuePin->data->pinRValueResult.m_Kind != VALUE_UNDEFINED;
			return;
		}
		case nodeEditorNodeType_CollisionCircleList:
		{
			auto parentNodeOutputNumberPinX = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputNumberNodeX = *parentNodeOutputNumberPinX->parentNodePtr;
			processAction(parentNodeOutputNumberNodeX, parentInstance, false);

			auto parentNodeOutputNumberPinY = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNumberNodeY = *parentNodeOutputNumberPinY->parentNodePtr;
			processAction(parentNodeOutputNumberNodeY, parentInstance, false);

			auto parentNodeOutputNumberPinRadius = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
			auto& parentNodeOutputNumberNodeRadius = *parentNodeOutputNumberPinRadius->parentNodePtr;
			processAction(parentNodeOutputNumberNodeRadius, parentInstance, false);

			auto parentNodeOutputIntegerPinObjectIndex = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[3]);
			auto& parentNodeOutputIntegerNodeObjectIndex = *parentNodeOutputIntegerPinObjectIndex->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeObjectIndex, parentInstance, false);

			auto parentNodeOutputBooleanPinOrdered = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[4]);
			auto& parentNodeOutputBooleanNodeOrdered = *parentNodeOutputBooleanPinOrdered->parentNodePtr;
			processAction(parentNodeOutputBooleanNodeOrdered, parentInstance, false);

			auto parentNodeOutputListPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[5]);
			auto& parentNodeOutputListNode = *parentNodeOutputListPin->parentNodePtr;
			processAction(parentNodeOutputListNode, parentInstance, false);

			g_ModuleInterface->CallBuiltin("collision_circle_list",
				{
					parentNodeOutputNumberPinX->data->pinNumberResult,
					parentNodeOutputNumberPinY->data->pinNumberResult,
					parentNodeOutputNumberPinRadius->data->pinNumberResult,
					parentNodeOutputIntegerPinObjectIndex->data->pinIntegerResult,
					false,
					true,
					parentNodeOutputListPin->data->pinDSListResult,
					parentNodeOutputBooleanPinOrdered->data->pinBoolDataResult,
				});

			auto& currentNodeOutputDSListPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputDSListPin.data->pinDSListResult = parentNodeOutputListPin->data->pinDSListResult;
			return;
		}
		case nodeEditorNodeType_DSListGet:
		{
			auto parentNodeOutputDSListPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputDSListNode = *parentNodeOutputDSListPin->parentNodePtr;
			processAction(parentNodeOutputDSListNode, parentInstance, false);

			auto parentNodeOutputIntegerPinIndex = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeIndex = *parentNodeOutputIntegerPinIndex->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeIndex, parentInstance, false);


			RValue ret = g_ModuleInterface->CallBuiltin("ds_list_find_value",
				{
					parentNodeOutputDSListPin->data->pinDSListResult,
					parentNodeOutputIntegerPinIndex->data->pinIntegerResult,
				});

			auto& currentNodeOutputRValuePin = *curNode.outputPinPtrArr[0];
			currentNodeOutputRValuePin.data->pinRValueResult = ret;
			return;
		}
		case nodeEditorNodeType_DSListSize:
		{
			auto parentNodeOutputDSListPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputDSListNode = *parentNodeOutputDSListPin->parentNodePtr;
			processAction(parentNodeOutputDSListNode, parentInstance, false);

			RValue ret = g_ModuleInterface->CallBuiltin("ds_list_size",
				{
					parentNodeOutputDSListPin->data->pinDSListResult,
				});

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = ret.ToInt32();
			return;
		}
		case nodeEditorNodeType_DSListCreate:
		{
			curNode.outputPinPtrArr[0]->data->pinDSListResult = g_ModuleInterface->CallBuiltin("ds_list_create", {});
			return;
		}
		case nodeEditorNodeType_DSListClear:
		{
			if (!isCodeFlow)
			{
				return;
			}

			auto parentNodeOutputDSListPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputDSListNode = *parentNodeOutputDSListPin->parentNodePtr;
			processAction(parentNodeOutputDSListNode, parentInstance, false);

			g_ModuleInterface->CallBuiltin("ds_list_clear", { parentNodeOutputDSListPin->data->pinDSListResult });

			return;
		}
		case nodeEditorNodeType_DSListDestroy:
		{
			if (!isCodeFlow)
			{
				return;
			}

			auto parentNodeOutputDSListPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputDSListNode = *parentNodeOutputDSListPin->parentNodePtr;
			processAction(parentNodeOutputDSListNode, parentInstance, false);

			g_ModuleInterface->CallBuiltin("ds_list_destroy", { parentNodeOutputDSListPin->data->pinDSListResult });

			return;
		}
		case nodeEditorNodeType_DSMapGet:
		{
			auto parentNodeOutputDSMapPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputDSMapNode = *parentNodeOutputDSMapPin->parentNodePtr;
			processAction(parentNodeOutputDSMapNode, parentInstance, false);

			auto parentNodeOutputStringPinIndex = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputStringNodeIndex = *parentNodeOutputStringPinIndex->parentNodePtr;
			processAction(parentNodeOutputStringNodeIndex, parentInstance, false);

			RValue ret = g_ModuleInterface->CallBuiltin("ds_map_find_value",
				{
					parentNodeOutputDSMapPin->data->pinDSMapResult,
					parentNodeOutputStringPinIndex->data->pinStringResult.c_str(),
				});

			auto& currentNodeOutputRValuePin = *curNode.outputPinPtrArr[0];
			currentNodeOutputRValuePin.data->pinRValueResult = ret;
			return;
		}
		case nodeEditorNodeType_DSMapKeysToArray:
		{
			auto parentNodeOutputDSMapPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputDSMapNode = *parentNodeOutputDSMapPin->parentNodePtr;
			processAction(parentNodeOutputDSMapNode, parentInstance, false);

			RValue ret = g_ModuleInterface->CallBuiltin("ds_map_keys_to_array",
				{
					parentNodeOutputDSMapPin->data->pinDSMapResult,
				});

			auto& currentNodeOutputArrayPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputArrayPin.data->pinArrayResult = ret;
			return;
		}
		case nodeEditorNodeType_ArrayGet:
		{
			auto parentNodeOutputArrayPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputArrayNode = *parentNodeOutputArrayPin->parentNodePtr;
			processAction(parentNodeOutputArrayNode, parentInstance, false);

			auto parentNodeOutputIntegerPinIndex = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNodeIndex = *parentNodeOutputIntegerPinIndex->parentNodePtr;
			processAction(parentNodeOutputIntegerNodeIndex, parentInstance, false);

			RValue ret = g_ModuleInterface->CallBuiltin("array_get",
				{
					parentNodeOutputArrayPin->data->pinArrayResult,
					parentNodeOutputIntegerPinIndex->data->pinIntegerResult,
				});

			auto& currentNodeOutputRValuePin = *curNode.outputPinPtrArr[0];
			currentNodeOutputRValuePin.data->pinRValueResult = ret;
			return;
		}
		case nodeEditorNodeType_ArrayLength:
		{
			auto parentNodeOutputArrayPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeOutputArrayNode = *parentNodeOutputArrayPin->parentNodePtr;
			processAction(parentNodeOutputArrayNode, parentInstance, false);

			RValue ret = g_ModuleInterface->CallBuiltin("array_length",
				{
					parentNodeOutputArrayPin->data->pinArrayResult,
				});

			auto& currentNodeOutputIntegerPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputIntegerPin.data->pinIntegerResult = ret.ToInt32();
			return;
		}
		case nodeEditorNodeType_TernaryOperator:
		{
			auto parentNodeOutputPinBoolean = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputNodeBoolean = *parentNodeOutputPinBoolean->parentNodePtr;
			processAction(parentNodeOutputNodeBoolean, parentInstance, false);

			auto& currentNodeOutputPin = *curNode.outputPinPtrArr[0];

			if (parentNodeOutputPinBoolean->data->pinBoolDataResult)
			{
				auto parentNodeOutputPinOne = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
				auto& parentNodeOutputNodeOne = *parentNodeOutputPinOne->parentNodePtr;
				processAction(parentNodeOutputNodeOne, parentInstance, false);

				copyPinValue(parentNodeOutputPinOne, currentNodeOutputPin);
			}
			else
			{
				auto parentNodeOutputPinTwo = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[3]);
				auto& parentNodeOutputNodeTwo = *parentNodeOutputPinTwo->parentNodePtr;
				processAction(parentNodeOutputNodeTwo, parentInstance, false);

				copyPinValue(parentNodeOutputPinTwo, currentNodeOutputPin);
			}
			return;
		}
		case nodeEditorNodeType_SetDebugLevel:
		{
			if (!isCodeFlow)
			{
				return;
			}
			auto parentNodeOutputIntegerPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputIntegerNode = *parentNodeOutputIntegerPin->parentNodePtr;
			processAction(parentNodeOutputIntegerNode, parentInstance, false);

			curLogLevel = parentNodeOutputIntegerPin->data->pinIntegerResult;
			return;
		}
		case nodeEditorNodeType_CacheVariable:
		{			
			auto& currentNodeOutputPin = *curNode.outputPinPtrArr[0];

			if (!currentNodeOutputPin.data->isCached)
			{
				auto parentNodeOutputPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
				auto& parentNodeOutputNode = *parentNodeOutputPin->parentNodePtr;
				processAction(parentNodeOutputNode, parentInstance, false);
				copyPinValue(parentNodeOutputPin, currentNodeOutputPin);
				currentNodeOutputPin.data->isCached = true;
			}
			return;
		}
		case nodeEditorNodeType_PlaySound:
		{
			if (!isCodeFlow)
			{
				return;
			}
			auto outputSoundPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& outputSoundNode = *outputSoundPin->parentNodePtr;
			processAction(outputSoundNode, parentInstance, false);

			auto& soundData = outputSoundPin->data->pinSoundDataResult;
			
			g_ModuleInterface->CallBuiltin("audio_play_sound", { soundData.soundRValue.soundIndex, 10, false });
			
			return;
		}
		case nodeEditorNodeType_PointDirection:
		{
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeSourceNumberPinX = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[0]);
			auto& parentNodeSourceNumberNodeX = *parentNodeSourceNumberPinX->parentNodePtr;
			processAction(parentNodeSourceNumberNodeX, parentInstance, false);

			auto parentNodeSourceNumberPinY = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeSourceNumberNodeY = *parentNodeSourceNumberPinY->parentNodePtr;
			processAction(parentNodeSourceNumberNodeY, parentInstance, false);

			auto parentNodeSourceTargetPinX = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[2]);
			auto& parentNodeSourceTargetNodeX = *parentNodeSourceTargetPinX->parentNodePtr;
			processAction(parentNodeSourceTargetNodeX, parentInstance, false);

			auto parentNodeSourceTargetPinY = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[3]);
			auto& parentNodeSourceTargetNodeY = *parentNodeSourceTargetPinY->parentNodePtr;
			processAction(parentNodeSourceTargetNodeY, parentInstance, false);

			double yDiff = parentNodeSourceTargetPinY->data->pinNumberResult - parentNodeSourceNumberPinY->data->pinNumberResult;
			double xDiff = parentNodeSourceTargetPinX->data->pinNumberResult - parentNodeSourceNumberPinX->data->pinNumberResult;

			auto& currentNodeOutputNumberPin = *curNode.outputPinPtrArr[0];
			currentNodeOutputNumberPin.data->pinNumberResult = fmod(atan2(-yDiff, xDiff) / std::numbers::pi * 180 + 360, 360);
			return;
		}
		case nodeEditorNodeType_FlushCache:
		{
			if (!isCodeFlow)
			{
				return;
			}
			auto curOuputPinPtr = curNode.outputPinPtrArr[1];
			for (auto& nodeLink : curOuputPinPtr->nodeLinks)
			{
				nodeEditorNodePin* outputFlushPin = (nodeLink.startPinID == curOuputPinPtr->pinID) ? nodeLink.endPinPtr : nodeLink.startPinPtr;
				auto curNodePtr = outputFlushPin->parentNodePtr;
				while (curNodePtr->nodeType == nodeEditorNodeType_MergeFlush)
				{
					if (curNodePtr->outputPinPtrArr.empty())
					{
						return;
					}
					curNodePtr = getConnectedOutputPinForInputPin(curNode, *curNodePtr->outputPinPtrArr[0])->parentNodePtr;
				}
				curNodePtr->outputPinPtrArr[0]->data->isCached = false;
			}
			return;
		}
		case nodeEditorNodeType_ArrayCreate:
		{
			curNode.outputPinPtrArr[0]->data->pinArrayResult = g_ModuleInterface->CallBuiltin("array_create", { 0 });
			return;
		}
		case nodeEditorNodeType_ArrayPush:
		{
			if (!isCodeFlow)
			{
				return;
			}
			// TODO: Might be useful to have a dirty flag? Could be useful if some numbers don't change that much
			auto parentNodeOutputArrayPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
			auto& parentNodeOutputArrayNode = *parentNodeOutputArrayPin->parentNodePtr;
			processAction(parentNodeOutputArrayNode, parentInstance, false);

			auto parentNodeOutputPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[3]);
			auto& parentNodeOutputNode = *parentNodeOutputPin->parentNodePtr;
			processAction(parentNodeOutputNode, parentInstance, false);

			if (parentNodeOutputPin->pinType == nodeEditorPinType_Integer)
			{
				g_ModuleInterface->CallBuiltin("array_push", { parentNodeOutputArrayPin->data->pinArrayResult, parentNodeOutputPin->data->pinIntegerResult });
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_Number)
			{
				g_ModuleInterface->CallBuiltin("array_push", { parentNodeOutputArrayPin->data->pinArrayResult, parentNodeOutputPin->data->pinNumberResult });
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_String)
			{
				g_ModuleInterface->CallBuiltin("array_push", { parentNodeOutputArrayPin->data->pinArrayResult, parentNodeOutputPin->data->pinStringResult.c_str() });
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_RValue)
			{
				g_ModuleInterface->CallBuiltin("array_push", { parentNodeOutputArrayPin->data->pinArrayResult, parentNodeOutputPin->data->pinRValueResult });
			}
			else if (parentNodeOutputPin->pinType == nodeEditorPinType_Instance)
			{
				g_ModuleInterface->CallBuiltin("array_push", { parentNodeOutputArrayPin->data->pinArrayResult, parentNodeOutputPin->data->pinInstanceResult });
			}
			return;
		}
		default:
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Unhandled node type %d for node %d in process action", curNode.nodeType, curNode.nodeID);
		}
	}
}

// Should only be used for nodes that use code flow
void processCodeActionFlow(nodeEditorNode& curNode, CInstance* parentInstance, std::vector<nodeEditorNode*>& loopBlockStack)
{
	if (curLogLevel >= 1)
	{
		DbgPrintEx(LOG_SEVERITY_INFO, "node ID: %d node Type: %d node Name: %s", curNode.nodeID, curNode.nodeType, curNode.nodeName.c_str());
		callbackManagerInterfacePtr->LogToFile(MODNAME, "node ID: %d node Type: %d node Name: %s", curNode.nodeID, curNode.nodeType, curNode.nodeName.c_str());
	}

	// TODO: Not sure what's causing an error where some nodes just suddenly lose their pins? Maybe related to quitting out and playing again?
	processAction(curNode, parentInstance, true);

	if (curNode.outputPinIDArr.empty())
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "No output pin for node %s, id: %d", curNode.nodeName.c_str(), curNode.nodeID);
		return;
	}
	
	int nextCodeFlowIndex = 0;
	if (curNode.nodeType == nodeEditorNodeType_If)
	{
		auto parentNodeOutputBooleanPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
		if (parentNodeOutputBooleanPin->data->pinBoolDataResult)
		{
			nextCodeFlowIndex = 0;
		}
		else
		{
			nextCodeFlowIndex = 1;
		}
	}
	else if (curNode.nodeType == nodeEditorNodeType_While)
	{
		auto parentNodeOutputBooleanPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
		if (parentNodeOutputBooleanPin->data->pinBoolDataResult)
		{
			loopBlockStack.push_back(&curNode);
			nextCodeFlowIndex = 0;
		}
		else
		{
			nextCodeFlowIndex = 1;
		}
	}
	auto& outputPin = *curNode.outputPinPtrArr[nextCodeFlowIndex];
	if (outputPin.pinType != nodeEditorPinType_CodeFlow)
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Pin index %d isn't code flow for node %s, id: %d", nextCodeFlowIndex, curNode.nodeName.c_str(), curNode.nodeID);
		return;
	}
	if (outputPin.nodeLinks.empty())
	{
		if (!loopBlockStack.empty())
		{
			auto nextNode = loopBlockStack.back();
			loopBlockStack.pop_back();
			processCodeActionFlow(*nextNode, parentInstance, loopBlockStack);
		}
		return;
	}

	auto startPin = outputPin.nodeLinks[0].startPinID;
	auto endPin = outputPin.nodeLinks[0].endPinID;
	if (startPin != outputPin.pinID && endPin != outputPin.pinID)
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Invalid pin link for node %s, id: %d", curNode.nodeName.c_str(), curNode.nodeID);
		return;
	}
	auto& nextPin = (startPin == outputPin.pinID) ? *outputPin.nodeLinks[0].endPinPtr : *outputPin.nodeLinks[0].startPinPtr;
	if (nextPin.pinType != nodeEditorPinType_CodeFlow)
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "First pin type isn't code flow for node %s, id: %d", curNode.nodeName.c_str(), curNode.nodeID);
		return;
	}
	auto& nextNode = *nextPin.parentNodePtr;
	if (curNode.nodeType == nodeEditorNodeType_Delay)
	{
		auto parentNodeOutputNumberPin = getConnectedOutputPinForInputPin(curNode, *curNode.inputPinPtrArr[1]);
		auto& parentNodeOutputNumberNode = *parentNodeOutputNumberPin->parentNodePtr;
		processAction(parentNodeOutputNumberNode, parentInstance, false);
		
		if (parentNodeOutputNumberPin->data->pinIntegerResult <= 0)
		{
			processCodeActionFlow(nextNode, parentInstance, loopBlockStack);
		}
		else
		{
			actionQueue.push(std::make_pair<int, actionQueueData>(curFrameNum + parentNodeOutputNumberPin->data->pinIntegerResult, actionQueueData(parentInstance, &nextNode, loopBlockStack)));
		}
	}
	else
	{
		processCodeActionFlow(nextNode, parentInstance, loopBlockStack);
	}
}

void checkOnTrigger(CInstance* parentInstance, nodeEditorNodeTypeEnum triggerType)
{
	RValue attackID = getInstanceVariable(parentInstance, GML_attackID);

	auto& charData = charDataMap[playingCharName];
	std::vector<nodeEditorNode*> loopBlockStack;
	if (attackID.m_Kind == VALUE_STRING)
	{
		const char* strAttackID = attackID.ToCString();

		if (charData.attackName.compare(strAttackID) == 0)
		{
			for (auto& curNodePair : charData.mainWeaponNodeEditor.nodeMap)
			{
				auto& curNode = curNodePair.second;
				if (curNode.nodeType == triggerType)
				{
					processCodeActionFlow(curNode, parentInstance, loopBlockStack);
					break;
				}
			}
			return;
		}

		if (charData.specialName.compare(strAttackID) == 0)
		{
			for (auto& curNodePair : charData.specialAttackNodeEditor.nodeMap)
			{
				auto& curNode = curNodePair.second;
				if (curNode.nodeType == triggerType)
				{
					processCodeActionFlow(curNode, parentInstance, loopBlockStack);
					break;
				}
			}
			return;
		}

		auto& curProjectileDataList = projectileDataListMap[strAttackID];
		for (auto& projectileData : curProjectileDataList)
		{
			// TODO: Maybe store the root nodes in a separate list to make it easier to get them
			for (auto& curNodePair : projectileData.nodeEditor.nodeMap)
			{
				auto& curNode = curNodePair.second;
				if (curNode.nodeType == triggerType)
				{
					processCodeActionFlow(curNode, parentInstance, loopBlockStack);
					break;
				}
			}
		}
		return;
	}

	if (triggerType == nodeEditorNodeType_OnSkillApply || triggerType == nodeEditorNodeType_OnFrameStep)
	{
		// TODO: Check to make sure attacks can't accidentally run this somehow
		for (auto& curSkillData : charData.skillDataList)
		{
			for (auto& curNodePair : curSkillData.nodeEditor.nodeMap)
			{
				auto& curNode = curNodePair.second;
				if (curNode.nodeType == triggerType)
				{
					processCodeActionFlow(curNode, parentInstance, loopBlockStack);
					break;
				}
			}
		}
	}
}

void AttackCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	
	checkOnTrigger(Self, nodeEditorNodeType_OnCreateProjectile);
}

void AttackStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);

	checkOnTrigger(Self, nodeEditorNodeType_OnFrameStep);
}

void AttackDestroyBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	CInstance* Self = std::get<0>(Args);
	
	checkOnTrigger(Self, nodeEditorNodeType_OnDestroyProjectile);
}

bool isInPlayerStep = false;

void PlayerStepBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	isInPlayerStep = true;
	curFrameNum++;
	if (!charDataMap.contains(playingCharName))
	{
		return;
	}
	auto& charData = charDataMap[playingCharName];
	while (!actionQueue.empty() && curFrameNum >= actionQueue.top().first)
	{
		auto curActionQueueData = actionQueue.top().second;
		actionQueue.pop();
		// TODO: Maybe should cache the instance exist function
		if (g_ModuleInterface->CallBuiltin("instance_exists", { curActionQueueData.parentInstancePtr }).ToBoolean())
		{
			processCodeActionFlow(*(curActionQueueData.curNodePtr), curActionQueueData.parentInstancePtr, curActionQueueData.loopBlockStack);
		}
		else
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Instance doesn't exist %d", curFrameNum);
		}
	}
}

void PlayerStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	isInPlayerStep = false;
}

void BaseMobStepAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	if (isInPlayerStep)
	{
		CInstance* Self = std::get<0>(Args);

		checkOnTrigger(Self, nodeEditorNodeType_OnFrameStep);
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