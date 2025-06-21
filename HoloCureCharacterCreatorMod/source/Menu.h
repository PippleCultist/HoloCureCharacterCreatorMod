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
struct skillTriggerData;
struct JSONDouble;
struct JSONInt;
struct actionProjectile;
struct actionBuff;
struct actionData;
struct projectileActionData;
struct projectileData;

void handleProjectileOnTrigger(std::vector<projectileActionData>& projectileActionList);

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
void to_json(nlohmann::json& outputJson, const skillTriggerData& inputSkillTriggerData);
void from_json(const nlohmann::json& inputJson, skillTriggerData& outputSkillTriggerData);
void to_json(nlohmann::json& outputJson, const JSONDouble& inputJSONDoubleData);
void from_json(const nlohmann::json& inputJson, JSONDouble& outputJSONDoubleData);
void to_json(nlohmann::json& outputJson, const JSONInt& inputJSONIntData);
void from_json(const nlohmann::json& inputJson, JSONInt& outputJSONIntData);
void to_json(nlohmann::json& outputJson, const actionProjectile& inputActionProjectile);
void from_json(const nlohmann::json& inputJson, actionProjectile& outputActionProjectile);
void to_json(nlohmann::json& outputJson, const actionBuff& inputActionBuff);
void from_json(const nlohmann::json& inputJson, actionBuff& outputActionBuff);
void to_json(nlohmann::json& outputJson, const actionData& inputActionData);
void from_json(const nlohmann::json& inputJson, actionData& outputActionData);
void to_json(nlohmann::json& outputJson, const projectileActionData& inputProjectileActionData);
void from_json(const nlohmann::json& inputJson, projectileActionData& outputProjectileActionData);
void to_json(nlohmann::json& outputJson, const projectileData& inputProjectileData);
void from_json(const nlohmann::json& inputJson, projectileData& outputProjectileData);

class TextureData
{
public:
	int width;
	int height;
	int numFrames;
	double curFrame;
	ID3D11ShaderResourceView* texture;
	bool isAnimationPlaying;

	TextureData() : width(0), height(0), numFrames(0), curFrame(0), texture(NULL), isAnimationPlaying(false)
	{
	}
};

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

enum projectileActionTriggerTypeEnum
{
	projectileActionTriggerType_NONE,
	projectileActionTriggerType_OnDestroy,
	projectileActionTriggerType_OnCreate,
};

static std::unordered_map<projectileActionTriggerTypeEnum, std::string> projectileActionTriggerTypeMap
{
	{ projectileActionTriggerType_NONE, "NONE" },
	{ projectileActionTriggerType_OnDestroy, "OnDestroy" },
	{ projectileActionTriggerType_OnCreate, "OnCreate" },
};

struct projectileActionData
{
	std::string projectileActionName;
	projectileActionTriggerTypeEnum projectileActionTriggerType;
	std::string triggeredActionName;
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
	std::vector<projectileActionData> projectileActionList;
};

enum skillTriggerTypeEnum
{
	skillTriggerType_NONE,
	skillTriggerType_OnDebuff,
	skillTriggerType_OnAttackCreate,
	skillTriggerType_OnCriticalHit,
	skillTriggerType_OnHeal,
	skillTriggerType_OnKill,
	skillTriggerType_OnTakeDamage,
	skillTriggerType_OnDodge,
};

static std::unordered_map<skillTriggerTypeEnum, std::string> skillTriggerTypeMap
{
	{ skillTriggerType_NONE, "NONE" },
	{ skillTriggerType_OnDebuff, "onDebuff" },
	{ skillTriggerType_OnAttackCreate, "onAttackCreate" },
	{ skillTriggerType_OnCriticalHit, "onCriticalHit" },
	{ skillTriggerType_OnHeal, "onHeal" },
	{ skillTriggerType_OnKill, "onKill" },
	{ skillTriggerType_OnTakeDamage, "onTakeDamage" },
	{ skillTriggerType_OnDodge, "onDodge" },
};

struct skillTriggerData
{
	std::string skillTriggerName;
	skillTriggerTypeEnum skillTriggerType;
	std::string triggeredActionName;
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
	JSONDouble pickupRange;
	JSONDouble critMod;
	JSONDouble bonusProjectile;
	std::vector<skillTriggerData> skillTriggerList;
};

struct skillData
{
	bool isUsingInGameSkill;
	std::string inGameSkillName;

	std::vector<skillLevelData> skillLevelDataList;
	std::string skillName;
	std::string skillIconFileName;
	TextureData skillIconTextureData;

	skillData(): isUsingInGameSkill(false)
	{
		for (int i = 0; i < 3; i++)
		{
			skillLevelDataList.push_back(skillLevelData());
		}
	}
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
	JSONDouble pickupRange;
	JSONDouble critMod;
	JSONDouble bonusProjectile;
	JSONInt maxStacks;
	JSONInt timer;
};

struct buffData
{
	std::string buffName;
	std::vector<buffLevelData> levels;
	std::string buffIconFileName;
};

struct actionProjectile
{
	JSONDouble relativeSpawnPosX;
	JSONDouble relativeSpawnPosY;
	JSONDouble spawnDir;
	bool isAbsoluteSpawnDir;
	std::string projectileDataName;
};

struct actionBuff
{
	std::string buffName;
};

enum actionTypeEnum
{
	actionType_NONE,
	actionType_SpawnProjectile,
	actionType_ApplyBuff,
};

static std::unordered_map<actionTypeEnum, std::string> actionTypeMap
{
	{ actionType_NONE, "NONE" },
	{ actionType_SpawnProjectile, "SpawnProjectile" },
	{ actionType_ApplyBuff, "ApplyBuff" },
};

struct actionData
{
	std::string actionName;
	actionTypeEnum actionType;
//	std::vector<std::string> nextActionList;
	int probability;
	actionProjectile actionProjectileData;
	actionBuff actionBuffData;

	actionData() : actionType(actionType_NONE), probability(100)
	{
	}
};

struct projectileData
{
	std::string projectileName;
	JSONInt projectileAnimationFPS;
	std::string projectileAnimationFileName;
	JSONDouble projectileDamage;
	JSONInt projectileDuration;
	JSONInt projectileHitCD;
	JSONInt projectileHitLimit;
	JSONInt projectileHitRange;
	JSONDouble projectileSpeed;
	std::vector<projectileActionData> projectileActionList;
	// TODO: Should add a isMain flag

	std::string curSelectedProjectileAction;
	TextureData projectileAnimationTextureData;
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
	std::vector<projectileActionData> specialProjectileActionList;
	JSONInt sizeGrade;
	std::string mainWeaponWeaponType;
	bool isUsingInGameMainWeapon;
	std::string inGameMainWeaponChar;
	bool isUsingInGameIdleSprite;
	std::string inGameIdleSpriteChar;
	bool isUsingInGameRunSprite;
	std::string inGameRunSpriteChar;
	bool isUsingInGamePortraitSprite;
	std::string inGamePortraitSpriteChar;
	bool isUsingInGameLargePortraitSprite;
	std::string inGameLargePortraitSpriteChar;
	bool isUsingInGameSpecial;
	std::string inGameSpecialChar;

	std::vector<weaponLevelData> weaponLevelDataList;
	std::vector<skillData> skillDataList;
	std::vector<buffData> buffDataList;
	std::vector<actionData> actionDataList;
	std::vector<projectileData> projectileDataList;

	characterData() : isUsingInGameMainWeapon(false), isUsingInGameIdleSprite(false), isUsingInGameRunSprite(false), isUsingInGamePortraitSprite(false),
		isUsingInGameLargePortraitSprite(false), isUsingInGameSpecial(false)
	{
		idleAnimationFPS.isDefined = true;
		idleAnimationFPS.value = 4;
		runAnimationFPS.isDefined = true;
		runAnimationFPS.value = 12;
		attackAnimationFPS.isDefined = true;
		attackAnimationFPS.value = 30;
		for (int i = 0; i < 3; i++)
		{
			skillDataList.push_back(skillData());
		}
		for (int i = 0; i < 7; i++)
		{
			weaponLevelDataList.push_back(weaponLevelData());
		}
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

struct actionDataMenuGrid
{
	std::shared_ptr<menuGrid> actionDataGrid;

	actionDataMenuGrid(std::shared_ptr<menuGrid> actionDataGrid) : actionDataGrid(actionDataGrid)
	{
	}
};

void initMenu();
void handleImGUI();
bool loadCharacterData(std::string dirName, characterData& charData);
int getSpriteNumFrames(const std::string spritePathStr);

void loadCharacterClickButton();

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();