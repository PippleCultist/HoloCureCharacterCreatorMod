#pragma comment(lib, "d3d11.lib")

#include "Menu.h"
#include "ModuleMain.h"
#include "CodeEvents.h"
#include "CommonFunctions.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include "nlohmann/json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_stdlib.h"
#include <d3d11.h>
#include <queue>
#include <fstream>
#include <regex>

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern HoloCureMenuInterface* holoCureMenuInterfacePtr;

int spriteDequePage = 0;
std::deque<std::shared_ptr<spriteData>> spriteDeque;

int loadCharacterPage = 0;
std::deque<std::string> loadCharacterDeque;

int weaponLevelMenuIndex = -1;
int skillMenuIndex = -1;
int skillLevelMenuIndex = -1;
int buffMenuIndex = -1;
int actionMenuIndex = -1;
int projectileMenuIndex = -1;

int curCharIdx = -1;
int loadedCharIdx = -1;
int curBuffDataIdx = -1;
bool hasLoadedData = false;
bool showAttackAnimationWindow = false;
bool showBuffDataWindow = false;
bool showIdleAnimationWindow = false;
bool showRunAnimationWindow = false;
bool showPortraitWindow = false;
bool showLargePortraitWindow = false;
bool showSpecialAnimationWindow = false;
bool showWeaponLevelsWindow = false;
bool showSkillDataWindow = false;
bool showActionDataWindow = false;
bool showProjectileDataWindow = false;
bool isAttackAnimationPlaying = false;
bool isIdleAnimationPlaying = false;
bool isRunAnimationPlaying = false;
bool isSpecialAnimationPlaying = false;

int attackAnimationWidth = 0;
int attackAnimationHeight = 0;
int attackAnimationNumFrames = 0;
double attackAnimationCurFrame = 0;
ID3D11ShaderResourceView* attackAnimationTexture = NULL;

int idleAnimationWidth = 0;
int idleAnimationHeight = 0;
int idleAnimationNumFrames = 0;
double idleAnimationCurFrame = 0;
ID3D11ShaderResourceView* idleAnimationTexture = NULL;

int runAnimationWidth = 0;
int runAnimationHeight = 0;
int runAnimationNumFrames = 0;
double runAnimationCurFrame = 0;
ID3D11ShaderResourceView* runAnimationTexture = NULL;

int buffIconWidth = 0;
int buffIconHeight = 0;
ID3D11ShaderResourceView* buffIconTexture = NULL;

int portraitIconWidth = 0;
int portraitIconHeight = 0;
ID3D11ShaderResourceView* portraitIconTexture = NULL;

int largePortraitIconWidth = 0;
int largePortraitIconHeight = 0;
ID3D11ShaderResourceView* largePortraitIconTexture = NULL;

int specialAnimationWidth = 0;
int specialAnimationHeight = 0;
int specialAnimationNumFrames = 0;
double specialAnimationCurFrame = 0;
ID3D11ShaderResourceView* specialAnimationTexture = NULL;

int specialIconWidth = 0;
int specialIconHeight = 0;
ID3D11ShaderResourceView* specialIconTexture = NULL;

std::vector<std::string> charList;
std::vector<std::string> imageList;
characterData curCharData;

std::shared_ptr<menuData> characterCreatorMenuLoadCharacter(new menuDataButton(60, 46 + 29 * 0, 180, 29, "CHARACTERCREATORMENU_LoadCharacter", "Load Character", true, loadCharacterClickButton, nullptr));

menuGrid characterCreatorMenuGrid({
	menuColumn({
		characterCreatorMenuLoadCharacter,
	}),
}, "Create Character", nullptr);

void initMenu()
{
	characterCreatorMenuGrid.initMenuGrid();
}

bool loadCharacterData(std::string dirName, characterData& charData)
{
	if (!std::filesystem::exists("CharacterCreatorMod/" + dirName))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the %s directory", dirName.c_str());
		g_ModuleInterface->Print(CM_RED, "Couldn't find the %s directory", dirName.c_str());
		return false;
	}

	if (!std::filesystem::exists("CharacterCreatorMod/" + dirName + "/charData.json"))
	{
		return false;
	}

	callbackManagerInterfacePtr->LogToFile(MODNAME, "Loading %s", dirName.c_str());
	g_ModuleInterface->Print(CM_WHITE, "Loading %s", dirName.c_str());
	std::ifstream inFile;
	inFile.open("CharacterCreatorMod/" + dirName + "/charData.json");
	try
	{
		nlohmann::json inputData = nlohmann::json::parse(inFile);
		charData = inputData.template get<characterData>();
	}
	catch (nlohmann::json::parse_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Parse Error: %s when parsing %s", e.what(), dirName.c_str());
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Parse Error: %s when parsing %s", e.what(), dirName.c_str());
	}
	
	return true;
}

int getSpriteNumFrames(const std::string spritePathStr)
{
	int pos = -1;
	const std::regex regexPattern("_strip[0-9]+\\.(png|jpg)");
	for (std::sregex_iterator it = std::sregex_iterator(spritePathStr.begin(), spritePathStr.end(), regexPattern); it != std::sregex_iterator(); it++)
	{
		std::smatch match = *it;
		pos = static_cast<int>(match.position(0));
	}
	int numFrames = 1;
	if (pos != -1)
	{
		std::string frameStr = spritePathStr.substr(pos + 6, spritePathStr.size() - pos - 1 + 4);
		std::from_chars(frameStr.data(), frameStr.data() + frameStr.size(), numFrames);
	}
	return numFrames;
}

void resetCharData()
{
	curCharIdx = -1;
	loadedCharIdx = -1;
	hasLoadedData = false;
	showAttackAnimationWindow = false;
	charList.clear();
	for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod"))
	{
		auto path = dir.path();
		if (path.filename().string().compare(0, 5, "char_") == 0)
		{
			charList.push_back(path.filename().string());
		}
	}
}

void resetAttackAnimationData()
{
	isAttackAnimationPlaying = false;
}

void reloadImageData()
{
	imageList.clear();
	if (std::filesystem::exists("CharacterCreatorMod/char_" + curCharData.charName))
	{
		for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod/char_" + curCharData.charName))
		{
			auto path = dir.path();
			if (path.extension().string().compare(".png") == 0)
			{
				imageList.push_back(path.filename().string());
			}
		}
	}
}

void addImageSelector(std::string name, std::string& fileName, ID3D11ShaderResourceView** texture, int& width, int& height,
						bool isAnimation = false, double* curFrame = nullptr, int* numFrames = nullptr, JSONInt* fps = nullptr, bool* isPlaying = nullptr)
{
	// If no texture is loaded and the fileName is not an empty string, try to load the fileName image.
	// If it fails, then set the fileName to an empty string to prevent it from trying to load again
	if ((*texture) == NULL && !fileName.empty())
	{
		bool hasFoundFileName = false;
		for (int i = 0; i < imageList.size(); i++)
		{
			if (fileName.compare(imageList[i]) == 0)
			{
				hasFoundFileName = true;
				if (isAnimation && numFrames != nullptr)
				{
					*numFrames = getSpriteNumFrames(fileName);
				}
				bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + fileName).c_str(), texture, &width, &height);
				IM_ASSERT(ret);
				break;
			}
		}
		if (!hasFoundFileName)
		{
			fileName = "";
		}
	}
	ImGui::Text("%s", name.c_str());
	if (ImGui::BeginCombo(("##" + name + "Combo").c_str(), fileName.c_str()))
	{
		bool hasSelected = false;
		for (int i = 0; i < imageList.size(); i++)
		{
			if (ImGui::Selectable((imageList[i] + "##" + fileName + std::to_string(i)).c_str(), fileName.compare(imageList[i]) == 0))
			{
				fileName = imageList[i];
				hasSelected = true;
			}
		}
		if (hasSelected)
		{
			if ((*texture) != NULL)
			{
				(*texture)->Release();
				(*texture) = NULL;
			}
			if (isAnimation && numFrames != nullptr)
			{
				*numFrames = getSpriteNumFrames(fileName);
			}
			bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + fileName).c_str(), texture, &width, &height);
			IM_ASSERT(ret);
		}

		ImGui::EndCombo();
	}

	if ((*texture) != NULL)
	{
		if (!isAnimation)
		{
			ImGui::Image((ImTextureID)(intptr_t)(*texture), ImVec2(static_cast<float>(width), static_cast<float>(height)), ImVec2(0, 0), ImVec2(1, 1));
		}
		else if (numFrames != nullptr && curFrame != nullptr && isPlaying != nullptr)
		{
			float size = 1.0f / *numFrames;
			float startU = static_cast<int>(*curFrame) * size;
			ImGui::Image((ImTextureID)(intptr_t)(*texture), ImVec2(static_cast<float>(width / *numFrames), static_cast<float>(height)), ImVec2(startU, 0), ImVec2(startU + size, 1));
			if (*isPlaying)
			{
				*curFrame += fps->value / 60.0f;
			}
			if (*curFrame >= *numFrames)
			{
				*curFrame = 0;
			}
			fps->isDefined |= ImGui::InputInt((name + "FPS").c_str(), &fps->value);
			if (ImGui::Button(("Toggle Play/Pause##" + name).c_str()))
			{
				*isPlaying = !*isPlaying;
			}
		}
	}

}

void handleAttackAnimationWindow()
{
	ImGui::Begin("Attack Animation");

	ImGui::InputText("attackName", &curCharData.attackName);

	addImageSelector("attackAnimation", curCharData.attackAnimationFileName, &attackAnimationTexture, attackAnimationWidth, attackAnimationHeight,
		true, &attackAnimationCurFrame, &attackAnimationNumFrames, &curCharData.attackAnimationFPS, &isAttackAnimationPlaying);

	ImGui::End();
}

void handleBuffDataWindow()
{
	ImGui::Begin("Buff Data");

	if (ImGui::Button("Add Buff Data"))
	{
		buffData newBuffData;
		newBuffData.buffName = "newBuff";
		newBuffData.levels.push_back(buffLevelData());
		curCharData.buffDataList.push_back(newBuffData);
	}

	const char* buffDataComboPreview = "";
	if (curBuffDataIdx >= 0 && curBuffDataIdx < curCharData.buffDataList.size())
	{
		buffDataComboPreview = curCharData.buffDataList[curBuffDataIdx].buffName.c_str();
	}

	if (ImGui::BeginCombo("##BuffDataCombo", buffDataComboPreview))
	{
		for (int i = 0; i < curCharData.buffDataList.size(); i++)
		{
			if (ImGui::Selectable((curCharData.buffDataList[i].buffName + "##" + std::to_string(i)).c_str(), i == curBuffDataIdx))
			{
				curBuffDataIdx = i;
				auto& curBuffData = curCharData.buffDataList[curBuffDataIdx];
				if (buffIconTexture != NULL)
				{
					buffIconTexture->Release();
					buffIconTexture = NULL;
				}
				bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + curBuffData.buffIconFileName).c_str(), &buffIconTexture, &buffIconWidth, &buffIconHeight);
				IM_ASSERT(ret);
			}
		}
		ImGui::EndCombo();
	}

	if (curBuffDataIdx >= 0 && curBuffDataIdx < curCharData.buffDataList.size())
	{
		auto& curBuffData = curCharData.buffDataList[curBuffDataIdx];
		ImGui::InputText("buffName", &curBuffData.buffName);

		curBuffData.levels[0].DRMultiplier.isDefined |= ImGui::InputDouble("DR", &curBuffData.levels[0].DRMultiplier.value);
		curBuffData.levels[0].attackIncrement.isDefined |= ImGui::InputInt("attackIncrement", &curBuffData.levels[0].attackIncrement.value);
		curBuffData.levels[0].critIncrement.isDefined |= ImGui::InputInt("critIncrement", &curBuffData.levels[0].critIncrement.value);
		curBuffData.levels[0].food.isDefined |= ImGui::InputDouble("food", &curBuffData.levels[0].food.value);
		curBuffData.levels[0].hasteIncrement.isDefined |= ImGui::InputInt("hasteIncrement", &curBuffData.levels[0].hasteIncrement.value);
		curBuffData.levels[0].healMultiplier.isDefined |= ImGui::InputDouble("healMultiplier", &curBuffData.levels[0].healMultiplier.value);
		curBuffData.levels[0].maxStacks.isDefined |= ImGui::InputInt("maxStacks", &curBuffData.levels[0].maxStacks.value);
		curBuffData.levels[0].speedIncrement.isDefined |= ImGui::InputInt("speedIncrement", &curBuffData.levels[0].speedIncrement.value);
		curBuffData.levels[0].timer.isDefined |= ImGui::InputInt("timer", &curBuffData.levels[0].timer.value);
		curBuffData.levels[0].weaponSize.isDefined |= ImGui::InputDouble("weaponSize", &curBuffData.levels[0].weaponSize.value);
		
		addImageSelector("buffIcon", curBuffData.buffIconFileName, &buffIconTexture, buffIconWidth, buffIconHeight,
			false, nullptr, nullptr, nullptr, nullptr);

		if (ImGui::Button("Delete Buff"))
		{
			curCharData.buffDataList.erase(curCharData.buffDataList.begin() + curBuffDataIdx);
			curBuffDataIdx = -1;
			if (buffIconTexture != NULL)
			{
				buffIconTexture->Release();
				buffIconTexture = NULL;
			}
		}
	}

	ImGui::End();
}

void handleIdleAnimationWindow()
{
	ImGui::Begin("Idle Animation");

	addImageSelector("idleAnimation", curCharData.idleAnimationFileName, &idleAnimationTexture, idleAnimationWidth, idleAnimationHeight,
		true, &idleAnimationCurFrame, &idleAnimationNumFrames, &curCharData.idleAnimationFPS, &isIdleAnimationPlaying);

	ImGui::End();
}

void handleRunAnimationWindow()
{
	ImGui::Begin("Run Animation");

	addImageSelector("runAnimation", curCharData.runAnimationFileName, &runAnimationTexture, runAnimationWidth, runAnimationHeight,
		true, &runAnimationCurFrame, &runAnimationNumFrames, &curCharData.runAnimationFPS, &isRunAnimationPlaying);

	ImGui::End();
}

void handlePortraitWindow()
{
	ImGui::Begin("Portrait");

	addImageSelector("portrait", curCharData.portraitFileName, &portraitIconTexture, portraitIconWidth, portraitIconHeight,
		false, nullptr, nullptr, nullptr, nullptr);

	ImGui::End();
}

void handleLargePortraitWindow()
{
	ImGui::Begin("Large Portrait");

	addImageSelector("large portrait", curCharData.largePortraitFileName, &largePortraitIconTexture, largePortraitIconWidth, largePortraitIconHeight,
		false, nullptr, nullptr, nullptr, nullptr);

	ImGui::End();
}

void handleSpecialAnimationWindow()
{
	ImGui::Begin("Special Animation");

	ImGui::InputText("specialName", &curCharData.specialName);
	ImGui::InputTextMultiline("specialDescription", &curCharData.specialDescription);
	curCharData.specialCooldown.isDefined |= ImGui::InputInt("specialCooldown", &curCharData.specialCooldown.value);
	curCharData.specialDamage.isDefined |= ImGui::InputDouble("specialDamage", &curCharData.specialDamage.value);
	curCharData.specialDuration.isDefined |= ImGui::InputInt("specialDuration", &curCharData.specialDuration.value);

	addImageSelector("specialIcon", curCharData.specialIconFileName, &specialIconTexture, specialIconWidth, specialIconHeight,
		false, nullptr, nullptr, nullptr, nullptr);

	addImageSelector("specialAnimation", curCharData.specialAnimationFileName, &specialAnimationTexture, specialAnimationWidth, specialAnimationHeight,
		true, &specialAnimationCurFrame, &specialAnimationNumFrames, &curCharData.specialAnimationFPS, &isSpecialAnimationPlaying);

	ImGui::End();
}

void handleProjectileOnTrigger(std::vector<projectileActionData>& projectileActionList)
{
	if (ImGui::Button("Add On Trigger"))
	{
		projectileActionData curProjectileActionData;
		curProjectileActionData.projectileActionName = "newOnTrigger";
		curProjectileActionData.projectileActionTriggerType = projectileActionTriggerType_NONE;
		projectileActionList.push_back(curProjectileActionData);
	}
	for (int j = 0; j < projectileActionList.size(); j++)
	{
		auto& curProjectileActionData = projectileActionList[j];
		if (ImGui::TreeNode((void*)(intptr_t)j, curProjectileActionData.projectileActionName.c_str()))
		{
			ImGui::InputText("projectileActionName", &curProjectileActionData.projectileActionName);
			if (ImGui::BeginCombo("projectileActionTriggerType", projectileActionTriggerTypeMap[curProjectileActionData.projectileActionTriggerType].c_str()))
			{
				for (const auto& [key, value] : projectileActionTriggerTypeMap)
				{
					bool isSelectable = key == curProjectileActionData.projectileActionTriggerType;
					if (ImGui::Selectable(value.c_str(), isSelectable))
					{
						curProjectileActionData.projectileActionTriggerType = key;
					}
				}
				ImGui::EndCombo();
			}

			if (curProjectileActionData.projectileActionTriggerType != projectileActionTriggerType_NONE)
			{
				if (ImGui::BeginCombo("triggeredActionName", curProjectileActionData.triggeredActionName.c_str()))
				{
					for (int k = 0; k < curCharData.actionDataList.size(); k++)
					{
						const bool is_selected = (curProjectileActionData.triggeredActionName.compare(curCharData.actionDataList[k].actionName) == 0);
						if (ImGui::Selectable(curCharData.actionDataList[k].actionName.c_str(), is_selected))
						{
							curProjectileActionData.triggeredActionName = curCharData.actionDataList[k].actionName;
						}
					}
					ImGui::EndCombo();
				}
			}

			if (ImGui::Button("Delete onTrigger"))
			{
				projectileActionList.erase(projectileActionList.begin() + j);
				j--;
			}
			ImGui::TreePop();
		}
	}
}

void handleWeaponLevelsWindow()
{
	ImGui::Begin("Weapon Levels");

	for (int i = 0; i < curCharData.weaponLevelDataList.size(); i++)
	{
		std::string strLevel = "Weapon Level " + std::to_string(i + 1);
		if (ImGui::TreeNode(strLevel.c_str()))
		{
			auto& curWeaponLevelData = curCharData.weaponLevelDataList[i];
			// TODO: Probably need a disable option to not input anything?
			curWeaponLevelData.attackCount.isDefined |= ImGui::InputInt(("attackCount##" + strLevel).c_str(), &curWeaponLevelData.attackCount.value);
			curWeaponLevelData.attackDelay.isDefined |= ImGui::InputInt(("attackDelay##" + strLevel).c_str(), &curWeaponLevelData.attackDelay.value);
			ImGui::InputText(("attackDescription##" + strLevel).c_str(), &curWeaponLevelData.attackDescription);
			curWeaponLevelData.attackTime.isDefined |= ImGui::InputInt(("attackTime##" + strLevel).c_str(), &curWeaponLevelData.attackTime.value);
			curWeaponLevelData.damage.isDefined |= ImGui::InputDouble(("damage##" + strLevel).c_str(), &curWeaponLevelData.damage.value);
			curWeaponLevelData.duration.isDefined |= ImGui::InputInt(("duration##" + strLevel).c_str(), &curWeaponLevelData.duration.value);
			curWeaponLevelData.hitCD.isDefined |= ImGui::InputInt(("hitCD##" + strLevel).c_str(), &curWeaponLevelData.hitCD.value);
			curWeaponLevelData.hitLimit.isDefined |= ImGui::InputInt(("hitLimit##" + strLevel).c_str(), &curWeaponLevelData.hitLimit.value);
			curWeaponLevelData.range.isDefined |= ImGui::InputInt(("range##" + strLevel).c_str(), &curWeaponLevelData.range.value);
			curWeaponLevelData.speed.isDefined |= ImGui::InputDouble(("speed##" + strLevel).c_str(), &curWeaponLevelData.speed.value);

			handleProjectileOnTrigger(curWeaponLevelData.projectileActionList);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void handleSkillDataWindow()
{
	ImGui::Begin("Skill Data");
	
	for (int i = 0; i < curCharData.skillDataList.size(); i++)
	{
		std::string strLevel = "Skill " + std::to_string(i + 1);
		if (ImGui::TreeNode(strLevel.c_str()))
		{
			auto& curSkillData = curCharData.skillDataList[i];
			ImGui::InputText(("skillName##" + strLevel).c_str(), &curSkillData.skillName);
			for (int j = 0; j < curSkillData.skillLevelDataList.size(); j++)
			{
				if (ImGui::TreeNode(("Level " + std::to_string(j + 1) + "##" + strLevel).c_str()))
				{
					auto& curSkillLevel = curSkillData.skillLevelDataList[j];
					std::string skillLevel = "##" + strLevel + " Level " + std::to_string(j + 1);
					curSkillLevel.DRMultiplier.isDefined |= ImGui::InputDouble(("DRMMultiplier" + skillLevel).c_str(), &curSkillLevel.DRMultiplier.value);
					curSkillLevel.healMultiplier.isDefined |= ImGui::InputDouble(("healMultiplier" + skillLevel).c_str(), &curSkillLevel.healMultiplier.value);
					curSkillLevel.attackIncrement.isDefined |= ImGui::InputInt(("attackIncrement" + skillLevel).c_str(), &curSkillLevel.attackIncrement.value);
					curSkillLevel.critIncrement.isDefined |= ImGui::InputInt(("critIncrement" + skillLevel).c_str(), &curSkillLevel.critIncrement.value);
					curSkillLevel.hasteIncrement.isDefined |= ImGui::InputInt(("hasteIncrement" + skillLevel).c_str(), &curSkillLevel.hasteIncrement.value);
					curSkillLevel.speedIncrement.isDefined |= ImGui::InputInt(("speedIncrement" + skillLevel).c_str(), &curSkillLevel.speedIncrement.value);
					curSkillLevel.food.isDefined |= ImGui::InputDouble(("food" + skillLevel).c_str(), &curSkillLevel.food.value);
					curSkillLevel.weaponSize.isDefined |= ImGui::InputDouble(("weaponSize" + skillLevel).c_str(), &curSkillLevel.weaponSize.value);
					if (ImGui::TreeNode(("onTrigger" + skillLevel).c_str()))
					{	
						auto& curOnTriggerData = curSkillLevel.skillOnTriggerData;
						if (ImGui::BeginCombo(("buffName" + skillLevel).c_str(), curOnTriggerData.buffName.c_str()))
						{
							for (int k = 0; k < curCharData.buffDataList.size(); k++)
							{
								const bool is_selected = (curOnTriggerData.buffName.compare(curCharData.buffDataList[k].buffName) == 0);
								if (ImGui::Selectable(curCharData.buffDataList[k].buffName.c_str(), is_selected))
								{
									curOnTriggerData.buffName = curCharData.buffDataList[k].buffName;
								}
							}
							ImGui::EndCombo();
						}
						curOnTriggerData.probability.isDefined |= ImGui::InputInt(("probability" + skillLevel).c_str(), &curOnTriggerData.probability.value);
						if (ImGui::BeginCombo(("onTriggerType" + skillLevel).c_str(), curOnTriggerData.onTriggerType.c_str()))
						{
							const char* items[] = { "NONE", "onDebuff", "onAttackCreate", "onCriticalHit", "onHeal", "onKill", "onTakeDamage", "onDodge" };
							for (int n = 0; n < IM_ARRAYSIZE(items); n++)
							{
								const bool is_selected = (curOnTriggerData.onTriggerType.compare(items[n]) == 0);
								if (ImGui::Selectable(items[n], is_selected))
								{
									curOnTriggerData.onTriggerType = items[n];
								}
							}
							ImGui::EndCombo();
						}
						ImGui::TreePop();
					}
					ImGui::InputTextMultiline(("skillDescription" + skillLevel).c_str(), &curSkillLevel.skillDescription);
					ImGui::TreePop();
				}
			}

			auto& curSkillIconTextureData = curSkillData.skillIconTextureData;
			addImageSelector("skillIcon", curSkillData.skillIconFileName, &curSkillIconTextureData.texture, curSkillIconTextureData.width, curSkillIconTextureData.height,
				false, nullptr, nullptr, nullptr, nullptr);

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void handleActionDataWindow()
{
	ImGui::Begin("Action Data");
	
	if (ImGui::Button("Add Action##action"))
	{
		actionData newActionData;
		newActionData.actionName = "newAction";
		newActionData.actionType = actionType_NONE;
		curCharData.actionDataList.push_back(newActionData);
	}

	for (int i = 0; i < curCharData.actionDataList.size(); i++)
	{
		std::string strAppendActionNumber = "##action" + std::to_string(i);
		if (ImGui::TreeNode((void*)(intptr_t)i, curCharData.actionDataList[i].actionName.c_str()))
		{
			ImGui::InputText(("actionName" + strAppendActionNumber).c_str(), &curCharData.actionDataList[i].actionName);
			if (ImGui::BeginCombo(("actionType" + strAppendActionNumber).c_str(), actionTypeMap[curCharData.actionDataList[i].actionType].c_str()))
			{
				for (const auto& [key, value] : actionTypeMap)
				{
					const bool is_selected = curCharData.actionDataList[i].actionType == key;
					if (ImGui::Selectable(value.c_str(), is_selected))
					{
						curCharData.actionDataList[i].actionType = key;
					}
				}
				ImGui::EndCombo();
			}

			if (curCharData.actionDataList[i].actionType == actionType_SpawnProjectile)
			{
				auto& curActionProjectileData = curCharData.actionDataList[i].actionProjectileData;
				curActionProjectileData.relativeSpawnPosX.isDefined |= ImGui::InputDouble(("relativeSpawnPosX" + strAppendActionNumber).c_str(), &curActionProjectileData.relativeSpawnPosX.value);
				curActionProjectileData.relativeSpawnPosY.isDefined |= ImGui::InputDouble(("relativeSpawnPosY" + strAppendActionNumber).c_str(), &curActionProjectileData.relativeSpawnPosY.value);
				curActionProjectileData.spawnDir.isDefined |= ImGui::InputDouble(("spawnDir" + strAppendActionNumber).c_str(), &curActionProjectileData.spawnDir.value);
				ImGui::Checkbox("Is Absolute Spawn Dir", &curActionProjectileData.isAbsoluteSpawnDir);
				if (ImGui::BeginCombo(("projectileDataName" + strAppendActionNumber).c_str(), curActionProjectileData.projectileDataName.c_str()))
				{
					for (int k = 0; k < curCharData.projectileDataList.size(); k++)
					{
						const bool is_selected = (curActionProjectileData.projectileDataName.compare(curCharData.projectileDataList[k].projectileName) == 0);
						if (ImGui::Selectable(curCharData.projectileDataList[k].projectileName.c_str(), is_selected))
						{
							curActionProjectileData.projectileDataName = curCharData.projectileDataList[k].projectileName;
						}
					}
					ImGui::EndCombo();
				}
			}
			
			if (ImGui::Button(("Delete action" + strAppendActionNumber).c_str()))
			{
				curCharData.actionDataList.erase(curCharData.actionDataList.begin() + i);
				i--;
			}

			// TODO: add code to add to next action list
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void handleProjectileWindow()
{
	ImGui::Begin("Projectile Data");

	if (ImGui::Button("Add projectile"))
	{
		projectileData newProjectileData;
		newProjectileData.projectileName = "newProjectile";
		curCharData.projectileDataList.push_back(newProjectileData);
	}

	for (int i = 0; i < curCharData.projectileDataList.size(); i++)
	{
		std::string strAppendProjectileNumber = "##projectile" + std::to_string(i);
		if (ImGui::TreeNode((void*)(intptr_t)i, curCharData.projectileDataList[i].projectileName.c_str()))
		{
			auto& curProjectileData = curCharData.projectileDataList[i];
			ImGui::InputText(("projectileName" + strAppendProjectileNumber).c_str(), &curProjectileData.projectileName);
			curProjectileData.projectileDamage.isDefined |= ImGui::InputDouble(("projectileDamage" + strAppendProjectileNumber).c_str(), &curProjectileData.projectileDamage.value);
			curProjectileData.projectileDuration.isDefined |= ImGui::InputInt(("projectileDuration" + strAppendProjectileNumber).c_str(), &curProjectileData.projectileDuration.value);
			curProjectileData.projectileHitCD.isDefined |= ImGui::InputInt(("projectileHitCD" + strAppendProjectileNumber).c_str(), &curProjectileData.projectileHitCD.value);
			curProjectileData.projectileHitLimit.isDefined |= ImGui::InputInt(("projectileHitLimit" + strAppendProjectileNumber).c_str(), &curProjectileData.projectileHitLimit.value);
			curProjectileData.projectileHitRange.isDefined |= ImGui::InputInt(("projectileHitRange" + strAppendProjectileNumber).c_str(), &curProjectileData.projectileHitRange.value);
			curProjectileData.projectileSpeed.isDefined |= ImGui::InputDouble(("projectileSpeed" + strAppendProjectileNumber).c_str(), &curProjectileData.projectileSpeed.value);
			
			handleProjectileOnTrigger(curProjectileData.projectileActionList);

			auto& curProjectileAnimationTextureData = curProjectileData.projectileAnimationTextureData;

			addImageSelector("projectileAnimation", curProjectileData.projectileAnimationFileName, &curProjectileAnimationTextureData.texture, curProjectileAnimationTextureData.width, curProjectileAnimationTextureData.height,
				true, &curProjectileAnimationTextureData.curFrame, &curProjectileAnimationTextureData.numFrames, &curProjectileData.projectileAnimationFPS, &curProjectileAnimationTextureData.isAnimationPlaying);
			if (ImGui::Button(("Delete projectile" + strAppendProjectileNumber).c_str()))
			{
				if (curProjectileAnimationTextureData.texture != NULL)
				{
					curProjectileAnimationTextureData.texture->Release();
				}
				curCharData.projectileDataList.erase(curCharData.projectileDataList.begin() + i);
				i--;
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void handleImGUI()
{
	ImGui::Begin("Character List");

	if (ImGui::Button("Reload Character List"))
	{
		if (!std::filesystem::exists("CharacterCreatorMod"))
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the CharacterCreatorMod directory");
			g_ModuleInterface->Print(CM_RED, "Couldn't find the CharacterCreatorMod directory");
		}
		else
		{
			resetCharData();
		}
	}

	// TODO: Need to add character button?

	if (ImGui::BeginListBox("##CharListBox"))
	{
		for (int i = 0; i < charList.size(); i++)
		{
			bool isSelected = false;
			if (ImGui::Selectable((charList[i] + "##" + std::to_string(i)).c_str(), i == curCharIdx))
			{
				curCharIdx = i;
			}
		}
		ImGui::EndListBox();
	}

	ImGui::End();

	if (!charList.empty() && curCharIdx >= 0 && curCharIdx < charList.size())
	{
		ImGui::Begin("Character Data");
		if (curCharIdx != loadedCharIdx)
		{
			hasLoadedData = loadCharacterData(charList[curCharIdx], curCharData);
			loadedCharIdx = curCharIdx;
			reloadImageData();
			resetAttackAnimationData();
			bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + curCharData.attackAnimationFileName).c_str(), &attackAnimationTexture, &attackAnimationWidth, &attackAnimationHeight);
			IM_ASSERT(ret);
			attackAnimationNumFrames = getSpriteNumFrames(curCharData.attackAnimationFileName);
			curBuffDataIdx = -1;
		}

		if (!hasLoadedData)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "FAILED TO LOAD DATA");
		}
		else
		{
			if (ImGui::Button("Save Data"))
			{
				ImGui::SameLine();
				if (!charList.empty() && loadedCharIdx >= 0 && loadedCharIdx < charList.size())
				{
					CreateDirectory(L"CharacterCreatorMod", NULL);
					std::string charDirName = "CharacterCreatorMod/" + charList[loadedCharIdx];
					CreateDirectoryA(charDirName.c_str(), NULL);
					nlohmann::json outputJSON = curCharData;

					std::ofstream outFile;
					outFile.open(charDirName + "/charData.json");
					outFile << std::setw(4) << outputJSON << "\n";
					outFile.close();
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "Saved to %s", charDirName.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "FAILED TO SAVE DATA");
				}
			}
			ImGui::InputText("charName", &curCharData.charName);
			curCharData.atk.isDefined |= ImGui::InputDouble("ATK", &curCharData.atk.value);
			curCharData.crit.isDefined |= ImGui::InputDouble("CRIT", &curCharData.crit.value);
			curCharData.hp.isDefined |= ImGui::InputDouble("HP", &curCharData.hp.value);
			curCharData.spd.isDefined |= ImGui::InputDouble("SPD", &curCharData.hp.value);
			curCharData.sizeGrade.isDefined |= ImGui::SliderInt("sizeGrade", &curCharData.sizeGrade.value, 0, 4);
			if (ImGui::BeginCombo("weaponType", curCharData.mainWeaponWeaponType.c_str()))
			{
				const char* items[] = { "Melee", "Ranged", "Multishot" };
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					const bool is_selected = (curCharData.mainWeaponWeaponType.compare(items[n]) == 0);
					if (ImGui::Selectable(items[n], is_selected))
					{
						curCharData.mainWeaponWeaponType = items[n];
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Toggle Attack Animation Window"))
			{
				showAttackAnimationWindow = !showAttackAnimationWindow;
			}
			if (ImGui::Button("Toggle Buff Data Window"))
			{
				showBuffDataWindow = !showBuffDataWindow;
			}
			if (ImGui::Button("Toggle Idle Animation Data Window"))
			{
				showIdleAnimationWindow = !showIdleAnimationWindow;
			}
			if (ImGui::Button("Toggle Run Animation Data Window"))
			{
				showRunAnimationWindow = !showRunAnimationWindow;
			}
			if (ImGui::Button("Toggle Portrait Window"))
			{
				showPortraitWindow = !showPortraitWindow;
			}
			if (ImGui::Button("Toggle Large Portrait Window"))
			{
				showLargePortraitWindow = !showLargePortraitWindow;
			}
			if (ImGui::Button("Toggle Special Animation Data Window"))
			{
				showSpecialAnimationWindow = !showSpecialAnimationWindow;
			}
			if (ImGui::Button("Toggle Weapon Levels Window"))
			{
				showWeaponLevelsWindow = !showWeaponLevelsWindow;
			}
			if (ImGui::Button("Toggle Skill Data Window"))
			{
				showSkillDataWindow = !showSkillDataWindow;
			}
			if (ImGui::Button("Toggle Action Data Window"))
			{
				showActionDataWindow = !showActionDataWindow;
			}
			if (ImGui::Button("Toggle Projectile Data Window"))
			{
				showProjectileDataWindow = !showProjectileDataWindow;
			}
		}

		ImGui::End();

		if (showAttackAnimationWindow)
		{
			handleAttackAnimationWindow();
		}
		
		if (showBuffDataWindow)
		{
			handleBuffDataWindow();
		}

		if (showIdleAnimationWindow)
		{
			handleIdleAnimationWindow();
		}

		if (showRunAnimationWindow)
		{
			handleRunAnimationWindow();
		}

		if (showPortraitWindow)
		{
			handlePortraitWindow();
		}

		if (showLargePortraitWindow)
		{
			handleLargePortraitWindow();
		}

		if (showSpecialAnimationWindow)
		{
			handleSpecialAnimationWindow();
		}

		if (showWeaponLevelsWindow)
		{
			handleWeaponLevelsWindow();
		}

		if (showSkillDataWindow)
		{
			handleSkillDataWindow();
		}

		if (showActionDataWindow)
		{
			handleActionDataWindow();
		}

		if (showProjectileDataWindow)
		{
			handleProjectileWindow();
		}
	}
}

void reloadLoadCharacterDeque()
{
	if (!std::filesystem::exists("CharacterCreatorMod"))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the CharacterCreatorMod directory");
		g_ModuleInterface->Print(CM_RED, "Couldn't find the CharacterCreatorMod directory");
		return;
	}
	loadCharacterPage = 0;
	loadCharacterDeque.clear();
	for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod"))
	{
		auto path = dir.path();
		if (path.filename().string().compare(0, 5, "char_") == 0)
		{
			loadCharacterDeque.push_back(path.filename().string());
		}
	}
//	reloadLoadCharacter();
}

std::string getSpriteFileName(std::shared_ptr<menuData> dataPtr)
{
	if (static_cast<menuDataImageField*>(dataPtr.get())->curSprite == nullptr)
	{
		return "";
	}
	return static_cast<menuDataImageField*>(dataPtr.get())->curSprite->spriteFileName;
}

std::string getSpriteFilePath(std::shared_ptr<menuData> dataPtr)
{
	if (static_cast<menuDataImageField*>(dataPtr.get())->curSprite == nullptr)
	{
		return "";
	}
	return static_cast<menuDataImageField*>(dataPtr.get())->curSprite->spritePath;
}

void copySpriteToDestination(std::shared_ptr<menuData>& imagePtr, std::string destPath)
{
	std::string spriteFilePath = getSpriteFilePath(imagePtr);
	if (spriteFilePath.empty() || spriteFilePath.compare(destPath) == 0)
	{
		return;
	}
	std::filesystem::copy_file(spriteFilePath, destPath, std::filesystem::copy_options::overwrite_existing);
}

void parseStringToJSONDouble(const std::string& inputStr, JSONDouble& outputJSONDouble, bool isSilent = true)
{
	auto [ptr, ec] = std::from_chars(inputStr.data(), inputStr.data() + inputStr.size(), outputJSONDouble.value);
	if (ec != std::errc{})
	{
		if (!isSilent)
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't parse %s as double", inputStr.c_str());
			g_ModuleInterface->Print(CM_RED, "Couldn't parse %s as double", inputStr.c_str());
		}
		outputJSONDouble.isDefined = false;
		return;
	}
	outputJSONDouble.isDefined = true;
}

void parseStringToJSONInt(const std::string& inputStr, JSONInt& outputJSONInt, bool isSilent = true)
{
	auto [ptr, ec] = std::from_chars(inputStr.data(), inputStr.data() + inputStr.size(), outputJSONInt.value);
	if (ec != std::errc{})
	{
		if (!isSilent)
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't parse %s as int", inputStr.c_str());
			g_ModuleInterface->Print(CM_RED, "Couldn't parse %s as", inputStr.c_str());
		}
		outputJSONInt.isDefined = false;
		return;
	}
	outputJSONInt.isDefined = true;
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::string& outputStr)
{
	try
	{
		inputJson.at(varName).get_to(outputStr);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to string", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to string", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to string", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to string", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, JSONInt& outputJSONInt)
{
	try
	{
		inputJson.at(varName).get_to(outputJSONInt);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to JSONInt", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to JSONInt", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to JSONInt", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to JSONInt", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, JSONDouble& outputJSONDouble)
{
	try
	{
		inputJson.at(varName).get_to(outputJSONDouble);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to JSONDouble", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to JSONDouble", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to JSONDouble", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to JSONDouble", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<weaponLevelData>& outputWeaponLevelDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputWeaponLevelDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<skillData>& outputSkillDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputSkillDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to skillData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to skillData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to skillData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to skillData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<buffData>& outputBuffDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputBuffDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to buffData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to buffData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to buffData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to buffData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<skillLevelData>& outputSkillLevelDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputSkillLevelDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, onTriggerData& outputOnTriggerData)
{
	try
	{
		inputJson.at(varName).get_to(outputOnTriggerData);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to onTriggerData", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to onTriggerData", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to onTriggerData", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to onTriggerData", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<buffLevelData>& outputBuffLevelDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputBuffLevelDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<std::string>& outputStringList)
{
	try
	{
		inputJson.at(varName).get_to(outputStringList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to string list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to string list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to string list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to string list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<actionData>& outputActionDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputActionDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to actionData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to actionData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to actionData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to actionData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<projectileData>& outputProjectileDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputProjectileDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to projectileData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to projectileData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to projectileData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to projectileData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, projectileActionTriggerTypeEnum& outputProjectileActionTriggerTypeEnum)
{
	try
	{
		std::string jsonString;
		inputJson.at(varName).get_to(jsonString);
		for (const auto& [key, value] : projectileActionTriggerTypeMap)
		{
			if (jsonString.compare(value) == 0)
			{
				outputProjectileActionTriggerTypeEnum = key;
				break;
			}
		}
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to projectileActionTriggerTypeEnum", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to projectileActionTriggerTypeEnum", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to projectileActionTriggerTypeEnum", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to projectileActionTriggerTypeEnum", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, actionTypeEnum& outputActionTypeEnum)
{
	try
	{
		std::string jsonString;
		inputJson.at(varName).get_to(jsonString);
		for (const auto& [key, value] : actionTypeMap)
		{
			if (jsonString.compare(value) == 0)
			{
				outputActionTypeEnum = key;
				break;
			}
		}
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to actionTypeEnum", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to actionTypeEnum", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to actionTypeEnum", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to actionTypeEnum", e.what(), varName);
	}
}

void to_json(nlohmann::json& outputJson, const characterData& inputCharData)
{
	outputJson = nlohmann::json{
		{ "charName", inputCharData.charName },
		{ "portraitFileName", inputCharData.portraitFileName },
		{ "largePortraitFileName", inputCharData.largePortraitFileName },
		{ "idleAnimationFPS", inputCharData.idleAnimationFPS },
		{ "idleAnimationFileName", inputCharData.idleAnimationFileName },
		{ "runAnimationFPS", inputCharData.runAnimationFPS },
		{ "runAnimationFileName", inputCharData.runAnimationFileName },
		{ "HP", inputCharData.hp },
		{ "ATK", inputCharData.atk },
		{ "SPD", inputCharData.spd },
		{ "CRIT", inputCharData.crit },
		{ "attackIconFileName", inputCharData.attackIconFileName },
		{ "attackAwakenedIconFileName", inputCharData.attackAwakenedIconFileName },
		{ "attackName", inputCharData.attackName },
		{ "specialIconFileName", inputCharData.specialIconFileName },
		{ "attackAnimationFPS", inputCharData.attackAnimationFPS },
		{ "attackAnimationFileName", inputCharData.attackAnimationFileName },
		{ "specialCooldown", inputCharData.specialCooldown },
		{ "specialName", inputCharData.specialName },
		{ "specialDescription", inputCharData.specialDescription },
		{ "specialAnimationFPS", inputCharData.specialAnimationFPS },
		{ "specialAnimationFileName", inputCharData.specialAnimationFileName },
		{ "specialDamage", inputCharData.specialDamage },
		{ "specialDuration", inputCharData.specialDuration },
		{ "sizeGrade", inputCharData.sizeGrade },
		{ "weaponType", inputCharData.mainWeaponWeaponType },
		{ "levels", inputCharData.weaponLevelDataList },
		{ "skills", inputCharData.skillDataList },
		{ "buffs", inputCharData.buffDataList },
		{ "actionDataList", inputCharData.actionDataList },
		{ "projectileDataList", inputCharData.projectileDataList },
	};
}

void from_json(const nlohmann::json& inputJson, characterData& outputCharData)
{
	parseJSONToVar(inputJson, "charName", outputCharData.charName);
	parseJSONToVar(inputJson, "portraitFileName", outputCharData.portraitFileName);
	parseJSONToVar(inputJson, "largePortraitFileName", outputCharData.largePortraitFileName);
	parseJSONToVar(inputJson, "idleAnimationFPS", outputCharData.idleAnimationFPS);
	parseJSONToVar(inputJson, "idleAnimationFileName", outputCharData.idleAnimationFileName);
	parseJSONToVar(inputJson, "runAnimationFPS", outputCharData.runAnimationFPS);
	parseJSONToVar(inputJson, "runAnimationFileName", outputCharData.runAnimationFileName);
	parseJSONToVar(inputJson, "HP", outputCharData.hp);
	parseJSONToVar(inputJson, "ATK", outputCharData.atk);
	parseJSONToVar(inputJson, "SPD", outputCharData.spd);
	parseJSONToVar(inputJson, "CRIT", outputCharData.crit);
	parseJSONToVar(inputJson, "attackIconFileName", outputCharData.attackIconFileName);
	parseJSONToVar(inputJson, "attackAwakenedIconFileName", outputCharData.attackAwakenedIconFileName);
	parseJSONToVar(inputJson, "attackName", outputCharData.attackName);
	parseJSONToVar(inputJson, "specialIconFileName", outputCharData.specialIconFileName);
	parseJSONToVar(inputJson, "attackAnimationFPS", outputCharData.attackAnimationFPS);
	parseJSONToVar(inputJson, "attackAnimationFileName", outputCharData.attackAnimationFileName);
	parseJSONToVar(inputJson, "specialCooldown", outputCharData.specialCooldown);
	parseJSONToVar(inputJson, "specialName", outputCharData.specialName);
	parseJSONToVar(inputJson, "specialDescription", outputCharData.specialDescription);
	parseJSONToVar(inputJson, "specialAnimationFPS", outputCharData.specialAnimationFPS);
	parseJSONToVar(inputJson, "specialAnimationFileName", outputCharData.specialAnimationFileName);
	parseJSONToVar(inputJson, "specialDamage", outputCharData.specialDamage);
	parseJSONToVar(inputJson, "specialDuration", outputCharData.specialDuration);
	parseJSONToVar(inputJson, "sizeGrade", outputCharData.sizeGrade);
	parseJSONToVar(inputJson, "weaponType", outputCharData.mainWeaponWeaponType);
	parseJSONToVar(inputJson, "levels", outputCharData.weaponLevelDataList);
	parseJSONToVar(inputJson, "skills", outputCharData.skillDataList);
	parseJSONToVar(inputJson, "buffs", outputCharData.buffDataList);
	parseJSONToVar(inputJson, "actionDataList", outputCharData.actionDataList);
	parseJSONToVar(inputJson, "projectileDataList", outputCharData.projectileDataList);
}

void to_json(nlohmann::json& outputJson, const skillData& inputSkillData)
{
	outputJson = nlohmann::json{
		{ "levels", inputSkillData.skillLevelDataList },
		{ "skillName", inputSkillData.skillName },
		{ "skillIconFileName", inputSkillData.skillIconFileName },
	};
}

void from_json(const nlohmann::json& inputJson, skillData& outputSkillData)
{
	parseJSONToVar(inputJson, "levels", outputSkillData.skillLevelDataList);
	parseJSONToVar(inputJson, "skillName", outputSkillData.skillName);
	parseJSONToVar(inputJson, "skillIconFileName", outputSkillData.skillIconFileName);
}

void to_json(nlohmann::json& outputJson, const skillLevelData& inputSkillLevelData)
{
	outputJson = nlohmann::json{
		{ "skillDescription", inputSkillLevelData.skillDescription },
		{ "attackIncrement", inputSkillLevelData.attackIncrement },
		{ "critIncrement", inputSkillLevelData.critIncrement },
		{ "hasteIncrement", inputSkillLevelData.hasteIncrement },
		{ "speedIncrement", inputSkillLevelData.speedIncrement },
		{ "DRMultiplier", inputSkillLevelData.DRMultiplier },
		{ "healMultiplier", inputSkillLevelData.healMultiplier },
		{ "food", inputSkillLevelData.food },
		{ "weaponSize", inputSkillLevelData.weaponSize },
		{ "skillOnTriggerData", inputSkillLevelData.skillOnTriggerData },
	};
}

void from_json(const nlohmann::json& inputJson, skillLevelData& outputSkillLevelData)
{
	parseJSONToVar(inputJson, "skillDescription", outputSkillLevelData.skillDescription);
	parseJSONToVar(inputJson, "attackIncrement", outputSkillLevelData.attackIncrement);
	parseJSONToVar(inputJson, "critIncrement", outputSkillLevelData.critIncrement);
	parseJSONToVar(inputJson, "hasteIncrement", outputSkillLevelData.hasteIncrement);
	parseJSONToVar(inputJson, "speedIncrement", outputSkillLevelData.speedIncrement);
	parseJSONToVar(inputJson, "DRMultiplier", outputSkillLevelData.DRMultiplier);
	parseJSONToVar(inputJson, "healMultiplier", outputSkillLevelData.healMultiplier);
	parseJSONToVar(inputJson, "food", outputSkillLevelData.food);
	parseJSONToVar(inputJson, "weaponSize", outputSkillLevelData.weaponSize);
	parseJSONToVar(inputJson, "skillOnTriggerData", outputSkillLevelData.skillOnTriggerData);
}

void to_json(nlohmann::json& outputJson, const weaponLevelData& inputWeaponLevelData)
{
	outputJson = nlohmann::json{
		{ "attackDescription", inputWeaponLevelData.attackDescription },
		{ "attackTime", inputWeaponLevelData.attackTime },
		{ "attackCount", inputWeaponLevelData.attackCount },
		{ "attackDelay", inputWeaponLevelData.attackDelay },
		{ "damage", inputWeaponLevelData.damage },
		{ "duration", inputWeaponLevelData.duration },
		{ "hitCD", inputWeaponLevelData.hitCD },
		{ "hitLimit", inputWeaponLevelData.hitLimit },
		{ "range", inputWeaponLevelData.range },
		{ "speed", inputWeaponLevelData.speed },
		{ "projectileActionList", inputWeaponLevelData.projectileActionList },
	};
}

void from_json(const nlohmann::json& inputJson, weaponLevelData& outputWeaponLevelData)
{
	parseJSONToVar(inputJson, "attackDescription", outputWeaponLevelData.attackDescription);
	parseJSONToVar(inputJson, "attackTime", outputWeaponLevelData.attackTime);
	parseJSONToVar(inputJson, "attackCount", outputWeaponLevelData.attackCount);
	parseJSONToVar(inputJson, "attackDelay", outputWeaponLevelData.attackDelay);
	parseJSONToVar(inputJson, "damage", outputWeaponLevelData.damage);
	parseJSONToVar(inputJson, "duration", outputWeaponLevelData.duration);
	parseJSONToVar(inputJson, "hitCD", outputWeaponLevelData.hitCD);
	parseJSONToVar(inputJson, "hitLimit", outputWeaponLevelData.hitLimit);
	parseJSONToVar(inputJson, "range", outputWeaponLevelData.range);
	parseJSONToVar(inputJson, "speed", outputWeaponLevelData.speed);
	auto& projectileActionList = inputJson["projectileActionList"];
	if (projectileActionList.is_array())
	{
		outputWeaponLevelData.projectileActionList = projectileActionList;
	}
}

void to_json(nlohmann::json& outputJson, const buffLevelData& inputBuffLevelData)
{
	outputJson = nlohmann::json{
		{ "attackIncrement", inputBuffLevelData.attackIncrement },
		{ "critIncrement", inputBuffLevelData.critIncrement },
		{ "hasteIncrement", inputBuffLevelData.hasteIncrement },
		{ "speedIncrement", inputBuffLevelData.speedIncrement },
		{ "DR", inputBuffLevelData.DRMultiplier },
		{ "healMultiplier", inputBuffLevelData.healMultiplier },
		{ "food", inputBuffLevelData.food },
		{ "weaponSize", inputBuffLevelData.weaponSize },
		{ "maxStacks", inputBuffLevelData.maxStacks },
		{ "timer", inputBuffLevelData.timer },
	};
}

void from_json(const nlohmann::json& inputJson, buffLevelData& outputBuffLevelData)
{
	parseJSONToVar(inputJson, "attackIncrement", outputBuffLevelData.attackIncrement);
	parseJSONToVar(inputJson, "critIncrement", outputBuffLevelData.critIncrement);
	parseJSONToVar(inputJson, "hasteIncrement", outputBuffLevelData.hasteIncrement);
	parseJSONToVar(inputJson, "speedIncrement", outputBuffLevelData.speedIncrement);
	parseJSONToVar(inputJson, "DR", outputBuffLevelData.DRMultiplier);
	parseJSONToVar(inputJson, "healMultiplier", outputBuffLevelData.healMultiplier);
	parseJSONToVar(inputJson, "food", outputBuffLevelData.food);
	parseJSONToVar(inputJson, "weaponSize", outputBuffLevelData.weaponSize);
	parseJSONToVar(inputJson, "maxStacks", outputBuffLevelData.maxStacks);
	parseJSONToVar(inputJson, "timer", outputBuffLevelData.timer);
}

void to_json(nlohmann::json& outputJson, const buffData& inputBuffData)
{
	outputJson = nlohmann::json{
		{ "buffName", inputBuffData.buffName },
		{ "levels", inputBuffData.levels },
		{ "buffIconFileName", inputBuffData.buffIconFileName },
	};
}

void from_json(const nlohmann::json& inputJson, buffData& outputBuffData)
{
	parseJSONToVar(inputJson, "buffName", outputBuffData.buffName);
	parseJSONToVar(inputJson, "levels", outputBuffData.levels);
	parseJSONToVar(inputJson, "buffIconFileName", outputBuffData.buffIconFileName);
}

void to_json(nlohmann::json& outputJson, const onTriggerData& inputOnTriggerData)
{
	outputJson = nlohmann::json{
		{ "onTriggerType", inputOnTriggerData.onTriggerType },
		{ "buffName", inputOnTriggerData.buffName },
		{ "probability", inputOnTriggerData.probability },
	};
}

void from_json(const nlohmann::json& inputJson, onTriggerData& outputOnTriggerData)
{
	parseJSONToVar(inputJson, "onTriggerType", outputOnTriggerData.onTriggerType);
	parseJSONToVar(inputJson, "buffName", outputOnTriggerData.buffName);
	parseJSONToVar(inputJson, "probability", outputOnTriggerData.probability);
}

void to_json(nlohmann::json& outputJson, const JSONDouble& inputJSONDoubleData)
{
	if (inputJSONDoubleData.isDefined)
	{
		outputJson = inputJSONDoubleData.value;
	}
	else
	{
		outputJson = nullptr;
	}
}

void from_json(const nlohmann::json& inputJson, JSONDouble& outputJSONDoubleData)
{
	if (inputJson.is_number())
	{
		outputJSONDoubleData.isDefined = true;
		outputJSONDoubleData.value = inputJson;
	}
	else
	{
		outputJSONDoubleData.isDefined = false;
	}
}

void to_json(nlohmann::json& outputJson, const JSONInt& inputJSONIntData)
{
	if (inputJSONIntData.isDefined)
	{
		outputJson = inputJSONIntData.value;
	}
	else
	{
		outputJson = nullptr;
	}
}

void from_json(const nlohmann::json& inputJson, JSONInt& outputJSONIntData)
{
	if (inputJson.is_number_integer())
	{
		outputJSONIntData.isDefined = true;
		outputJSONIntData.value = inputJson;
	}
	else
	{
		outputJSONIntData.isDefined = false;
	}
}

void to_json(nlohmann::json& outputJson, const actionProjectile& inputActionProjectile)
{
	outputJson = nlohmann::json{
		{ "relativeSpawnPosX", inputActionProjectile.relativeSpawnPosX },
		{ "relativeSpawnPosY", inputActionProjectile.relativeSpawnPosY },
		{ "spawnDir", inputActionProjectile.spawnDir },
		{ "isAbsoluteSpawnDir", inputActionProjectile.isAbsoluteSpawnDir },
		{ "projectileDataName", inputActionProjectile.projectileDataName },
	};
}

void from_json(const nlohmann::json& inputJson, actionProjectile& outputActionProjectile)
{
	parseJSONToVar(inputJson, "relativeSpawnPosX", outputActionProjectile.relativeSpawnPosX);
	parseJSONToVar(inputJson, "relativeSpawnPosY", outputActionProjectile.relativeSpawnPosY);
	parseJSONToVar(inputJson, "spawnDir", outputActionProjectile.spawnDir);
	auto& isAbsoluteSpawnDir = inputJson["isAbsoluteSpawnDir"];
	if (isAbsoluteSpawnDir.is_boolean())
	{
		outputActionProjectile.isAbsoluteSpawnDir = isAbsoluteSpawnDir;
	}
	auto& projectileDataName = inputJson["projectileDataName"];
	if (projectileDataName.is_string())
	{
		outputActionProjectile.projectileDataName = projectileDataName;
	}
}

void to_json(nlohmann::json& outputJson, const actionData& inputActionData)
{
	outputJson = nlohmann::json{
		{ "actionName", inputActionData.actionName },
		{ "actionType", actionTypeMap[inputActionData.actionType] },
//		{ "nextActionList", inputActionData.nextActionList },
	};
	if (inputActionData.actionType == actionType_SpawnProjectile)
	{
		nlohmann::json tempJson;
		to_json(tempJson, inputActionData.actionProjectileData);
		outputJson["actionProjectileData"] = tempJson;
	}
}

void from_json(const nlohmann::json& inputJson, actionData& outputActionData)
{
	parseJSONToVar(inputJson, "actionName", outputActionData.actionName);
	parseJSONToVar(inputJson, "actionType", outputActionData.actionType);
//	parseJSONToVar(inputJson, "nextActionList", outputActionData.nextActionList);
	if (outputActionData.actionType == actionType_SpawnProjectile)
	{
		from_json(inputJson["actionProjectileData"], outputActionData.actionProjectileData);
	}
}

void to_json(nlohmann::json& outputJson, const projectileActionData& inputProjectileActionData)
{
	outputJson = nlohmann::json{
		{ "projectileActionName", inputProjectileActionData.projectileActionName },
		{ "projectileActionTriggerType", projectileActionTriggerTypeMap[inputProjectileActionData.projectileActionTriggerType] },
		{ "triggeredActionName", inputProjectileActionData.triggeredActionName },
	};
}

void from_json(const nlohmann::json& inputJson, projectileActionData& outputProjectileActionData)
{
	parseJSONToVar(inputJson, "projectileActionName", outputProjectileActionData.projectileActionName);
	parseJSONToVar(inputJson, "projectileActionTriggerType", outputProjectileActionData.projectileActionTriggerType);
	parseJSONToVar(inputJson, "triggeredActionName", outputProjectileActionData.triggeredActionName);
}

void to_json(nlohmann::json& outputJson, const projectileData& inputProjectileData)
{
	outputJson = nlohmann::json{
		{ "projectileName", inputProjectileData.projectileName },
		{ "projectileAnimationFPS", inputProjectileData.projectileAnimationFPS },
		{ "projectileAnimationFileName", inputProjectileData.projectileAnimationFileName },
		{ "projectileDamage", inputProjectileData.projectileDamage },
		{ "projectileDuration", inputProjectileData.projectileDuration },
		{ "projectileHitCD", inputProjectileData.projectileHitCD },
		{ "projectileHitLimit", inputProjectileData.projectileHitLimit },
		{ "projectileHitRange", inputProjectileData.projectileHitRange },
		{ "projectileSpeed", inputProjectileData.projectileSpeed },
		{ "projectileActionList", inputProjectileData.projectileActionList },
	};
}

void from_json(const nlohmann::json& inputJson, projectileData& outputProjectileData)
{
	parseJSONToVar(inputJson, "projectileName", outputProjectileData.projectileName);
	parseJSONToVar(inputJson, "projectileAnimationFPS", outputProjectileData.projectileAnimationFPS);
	parseJSONToVar(inputJson, "projectileAnimationFileName", outputProjectileData.projectileAnimationFileName);
	parseJSONToVar(inputJson, "projectileDamage", outputProjectileData.projectileDamage);
	parseJSONToVar(inputJson, "projectileDuration", outputProjectileData.projectileDuration);
	parseJSONToVar(inputJson, "projectileHitCD", outputProjectileData.projectileHitCD);
	parseJSONToVar(inputJson, "projectileHitLimit", outputProjectileData.projectileHitLimit);
	parseJSONToVar(inputJson, "projectileHitRange", outputProjectileData.projectileHitRange);
	parseJSONToVar(inputJson, "projectileSpeed", outputProjectileData.projectileSpeed);
	auto& projectileActionList = inputJson["projectileActionList"];
	if (projectileActionList.is_array())
	{
		outputProjectileData.projectileActionList = projectileActionList;
	}
}