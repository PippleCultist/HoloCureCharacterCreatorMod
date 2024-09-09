#pragma once
#pragma once
#include "ModuleMain.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <string>

extern HoloCureMenuInterface* holoCureMenuInterfacePtr;

struct spriteData;

struct characterData;
struct skillData;
struct skillLevelData;
struct weaponLevelData;
struct buffLevelData;
struct buffData;
struct onTriggerData;
struct JSONDouble;
struct JSONInt;

void to_json(nlohmann::json& outputJson, const characterData& inputCharData);
void from_json(const nlohmann::json& inputJson, characterData& outputCharData);
void to_json(nlohmann::json& outputJson, const skillData& inputSkillData);
void from_json(const nlohmann::json& inputJson, skillData& outputSkillData);
void to_json(nlohmann::json& outputJson, const skillLevelData& inputSkillLevelData);
void from_json(const nlohmann::json& inputJson, skillLevelData& outputSkillLevelData);
void to_json(nlohmann::json& outputJson, const weaponLevelData& inputWeaponLevelData);
void from_json(const nlohmann::json& inputJson, weaponLevelData& outputWeaponLevelData);
void to_json(nlohmann::json& outputJson, const buffLevelData& inputBuffLevelData);
void from_json(const nlohmann::json& inputJson, buffLevelData& outputBuffLevelData);
void to_json(nlohmann::json& outputJson, const buffData& inputBuffData);
void from_json(const nlohmann::json& inputJson, buffData& outputBuffData);
void to_json(nlohmann::json& outputJson, const onTriggerData& inputOnTriggerData);
void from_json(const nlohmann::json& inputJson, onTriggerData& outputOnTriggerData);
void to_json(nlohmann::json& outputJson, const JSONDouble& inputJSONDoubleData);
void from_json(const nlohmann::json& inputJson, JSONDouble& outputJSONDoubleData);
void to_json(nlohmann::json& outputJson, const JSONInt& inputJSONIntData);
void from_json(const nlohmann::json& inputJson, JSONInt& outputJSONIntData);

struct JSONInt
{
	bool isDefined;
	int value;

	JSONInt() : isDefined(false), value(0)
	{
	}
};

struct JSONDouble
{
	bool isDefined;
	double value;

	JSONDouble() : isDefined(false), value(0)
	{
	}
};

struct weaponLevelData
{
	std::string attackDescription;
	JSONInt attackTime;
	JSONInt attackCount;
	JSONInt attackDelay;
	JSONDouble damage;
	JSONInt duration;
	JSONInt hitCD;
	JSONInt hitLimit;
	JSONInt range;
	JSONDouble speed;
};

struct onTriggerData
{
	std::string onTriggerType;
	std::string buffName;
	JSONInt probability;
};

struct skillLevelData
{
	std::string skillDescription;
	JSONInt attackIncrement;
	JSONInt critIncrement;
	JSONInt hasteIncrement;
	JSONInt speedIncrement;
	JSONDouble DRMultiplier;
	JSONDouble healMultiplier;
	JSONDouble food;
	JSONDouble weaponSize;
	onTriggerData skillOnTriggerData;
};

struct skillData
{
	std::vector<skillLevelData> skillLevelDataList;
	std::string skillName;
	std::string skillIconFileName;
};

struct buffLevelData
{
	JSONInt attackIncrement;
	JSONInt critIncrement;
	JSONInt hasteIncrement;
	JSONInt speedIncrement;
	JSONDouble DRMultiplier;
	JSONDouble healMultiplier;
	JSONDouble food;
	JSONDouble weaponSize;
	JSONInt maxStacks;
	JSONInt timer;
};

struct buffData
{
	std::string buffName;
	std::vector<buffLevelData> levels;
	std::string buffIconFileName;
};

struct characterData
{
	std::string charName;
	std::string portraitFileName;
	std::string largePortraitFileName;
	JSONInt idleAnimationFPS;
	std::string idleAnimationFileName;
	JSONInt runAnimationFPS;
	std::string runAnimationFileName;
	JSONDouble hp;
	JSONDouble atk;
	JSONDouble spd;
	JSONDouble crit;
	std::string attackIconFileName;
	std::string attackAwakenedIconFileName;
	std::string attackName;
	std::string specialIconFileName;
	JSONInt attackAnimationFPS;
	std::string attackAnimationFileName;
	JSONInt specialCooldown;
	std::string specialName;
	std::string specialDescription;
	JSONInt specialAnimationFPS;
	std::string specialAnimationFileName;
	JSONDouble specialDamage;
	JSONInt specialDuration;
	JSONInt sizeGrade;
	std::string mainWeaponWeaponType;
	std::vector<weaponLevelData> weaponLevelDataList;
	std::vector<skillData> skillDataList;
	std::vector<buffData> buffDataList;

	characterData()
	{
		idleAnimationFPS.isDefined = true;
		idleAnimationFPS.value = 4;
		runAnimationFPS.isDefined = true;
		runAnimationFPS.value = 12;
		attackAnimationFPS.isDefined = true;
		attackAnimationFPS.value = 30;
	}
};

struct menuColumn
{
	std::shared_ptr<menuColumnData> menuColumnPtr;
	std::vector<std::shared_ptr<menuData>> menuDataPtrList;

	menuColumn(std::vector<std::shared_ptr<menuData>> menuDataPtrList) : menuColumnPtr(nullptr), menuDataPtrList(menuDataPtrList)
	{
	}

	void initMenuColumn(std::shared_ptr<menuGridData>& menuGridPtr)
	{
		holoCureMenuInterfacePtr->CreateMenuColumn(MODNAME, menuGridPtr, menuColumnPtr);
		for (auto& menuDataPtr : menuDataPtrList)
		{
			holoCureMenuInterfacePtr->AddMenuData(MODNAME, menuColumnPtr, menuDataPtr);
		}
	}
};

struct menuGrid
{
	std::string name;
	menuGrid* prevMenuGridPtr;
	std::shared_ptr<menuGridData> menuGridPtr;
	std::vector<menuColumn> menuColumnList;
	menuFunc onEnterFunc;

	menuGrid() : menuGridPtr(nullptr), prevMenuGridPtr(nullptr), onEnterFunc(nullptr)
	{
	}

	menuGrid(std::vector<menuColumn> menuColumnList, std::string name, menuGrid* prevMenuGridPtr) : menuGridPtr(nullptr), name(name), prevMenuGridPtr(prevMenuGridPtr), menuColumnList(menuColumnList), onEnterFunc(nullptr)
	{
	}

	menuGrid(std::vector<menuColumn> menuColumnList, std::string name, menuGrid* prevMenuGridPtr, menuFunc onEnterFunc) : menuGridPtr(nullptr), name(name), prevMenuGridPtr(prevMenuGridPtr), menuColumnList(menuColumnList), onEnterFunc(onEnterFunc)
	{
	}

	void initMenuGrid()
	{
		std::shared_ptr<menuGridData> menuGridDataPtr = nullptr;
		if (prevMenuGridPtr != nullptr)
		{
			menuGridDataPtr = prevMenuGridPtr->menuGridPtr;
		}
		holoCureMenuInterfacePtr->CreateMenuGrid(MODNAME, name, menuGridDataPtr, menuGridPtr);
		menuGridPtr->onEnterFunc = onEnterFunc;
		for (auto& menuColumn : menuColumnList)
		{
			menuColumn.initMenuColumn(menuGridPtr);
		}
	}
};

struct buffDataMenuGrid
{
	std::shared_ptr<menuGrid> buffDataGrid;
	std::shared_ptr<menuGrid> buffIconGrid;

	buffDataMenuGrid(std::shared_ptr<menuGrid> buffDataGrid, std::shared_ptr<menuGrid> buffIconGrid) : buffDataGrid(buffDataGrid), buffIconGrid(buffIconGrid)
	{
	}
};

void initMenu();
bool loadCharacterData(std::string dirName, characterData& charData);
int getSpriteNumFrames(const std::string spritePathStr);

void loadCharacterDataButton();
void loadCharacterClickButton();
void prevLoadCharacterButton();
void nextLoadCharacterButton();
void portraitClickButton();
void prevIconButton();
void nextIconButton();
void clickIconButton();
void clickSkillIconButton();
void buffIconClickButton();
void clickBuffMenuButton();
void reloadBuffs();
void prevBuffButton();
void nextBuffButton();
void addBuffMenuButton();
void weaponLevelClickButton();
void skillLevelClickButton();
void skillMenuClickButton();
void largePortraitClickButton();
void idleAnimationClickButton();
void runAnimationClickButton();
void specialAnimationClickButton();
void weaponLevelDescriptionClickButton();
void skillDescriptionClickButton();
void skillOnTriggerClickButton();
void specialDescriptionClickButton();
void skillIconMenuClickButton();
void specialIconClickButton();
void weaponIconClickButton();
void weaponAwakenedIconClickButton();
void weaponAnimationClickButton();
void weaponLevelMenuClickButton();
void characterDataClickButton();
void mainWeaponClickButton();
void skillClickButton();
void specialClickButton();
void buffClickButton();
void exportCharacterClickButton();
void prevImageButton();
void nextImageButton();
void playPauseButton();
void animationFrameText();