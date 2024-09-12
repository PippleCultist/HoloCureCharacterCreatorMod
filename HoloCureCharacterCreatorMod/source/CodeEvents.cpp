#include "CommonFunctions.h"
#include "CodeEvents.h"
#include "Menu.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include <random>

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern YYGML_PushContextStack yyGMLPushContextStack;
extern YYGML_YYSetScriptRef yyGMLYYSetScriptRef;
extern YYGML_PopContextStack yyGMLPopContextStack;
extern RValue lastStructVarGetName;

extern PFUNC_YYGMLScript origCanSubmitScoreScript;

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
};

std::unordered_map<std::string, charSpriteData> charSpriteMap;
std::unordered_map<std::string, characterData> charDataMap;
std::vector<std::string> charNameList;

bool hasBackedUpCharacterList = false;
bool isInCharSelectDraw = false;

int rhythmLeftButtonIndex = 100000;
int rhythmRightButtonIndex = 100001;

int charSelectPage = 0;

void CharacterDataCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
}

void CharSelectCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args)
{
	charSelectPage = 0;
	RValue characterDataMap = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterData" });
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

				double attackIconWidth = g_ModuleInterface->CallBuiltin("sprite_get_width", { curCharSpriteData.attackIconPtr->spriteRValue }).m_Real;
				double attackIconHeight = g_ModuleInterface->CallBuiltin("sprite_get_height", { curCharSpriteData.attackIconPtr->spriteRValue }).m_Real;
				g_ModuleInterface->CallBuiltin("sprite_set_offset", { curCharSpriteData.attackIconPtr->spriteRValue, attackIconWidth / 2, attackIconHeight / 2 });
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
				g_RunnerInterface.StructAddRValue(&charDataStruct, "port", &curCharSpriteData.portraitSpritePtr->spriteRValue);
				g_RunnerInterface.StructAddRValue(&charDataStruct, "large_port", &curCharSpriteData.largePortraitSpritePtr->spriteRValue);
				g_RunnerInterface.StructAddRValue(&charDataStruct, "sprite1", &curCharSpriteData.idleSpritePtr->spriteRValue);
				g_RunnerInterface.StructAddRValue(&charDataStruct, "sprite2", &curCharSpriteData.runSpritePtr->spriteRValue);
				g_RunnerInterface.StructAddDouble(&charDataStruct, "HP", charData.hp.value);
				g_RunnerInterface.StructAddDouble(&charDataStruct, "ATK", charData.atk.value);
				g_RunnerInterface.StructAddDouble(&charDataStruct, "SPD", charData.spd.value);
				g_RunnerInterface.StructAddDouble(&charDataStruct, "crit", charData.crit.value);

				g_RunnerInterface.StructAddString(&charDataStruct, "attackID", charData.attackName.c_str());

				g_RunnerInterface.StructAddRValue(&charDataStruct, "attackIcon", &curCharSpriteData.attackIconPtr->spriteRValue);
				g_RunnerInterface.StructAddString(&charDataStruct, "attack", charData.attackName.c_str());
				RValue attackDescArr = g_ModuleInterface->CallBuiltin("array_create", { 1 });
				if (!charData.weaponLevelDataList.empty())
				{
					attackDescArr[0] = charData.weaponLevelDataList[0].attackDescription.c_str();
				}
				g_RunnerInterface.StructAddRValue(&charDataStruct, "attackDesc", &attackDescArr);
				g_RunnerInterface.StructAddRValue(&charDataStruct, "specIcon", &curCharSpriteData.specialIconPtr->spriteRValue);

				g_RunnerInterface.StructAddString(&charDataStruct, "specID", charData.specialName.c_str());

				g_RunnerInterface.StructAddDouble(&charDataStruct, "specCD", charData.specialCooldown.value);
				g_RunnerInterface.StructAddString(&charDataStruct, "specName", charData.specialName.c_str());
				g_RunnerInterface.StructAddString(&charDataStruct, "specDesc", charData.specialDescription.c_str());

				RValue perksStruct;
				g_RunnerInterface.StructCreate(&perksStruct);
				if (charData.skillDataList.size() >= 3)
				{
					g_RunnerInterface.StructAddDouble(&perksStruct, charData.skillDataList[0].skillName.c_str(), 0);
					g_RunnerInterface.StructAddDouble(&perksStruct, charData.skillDataList[1].skillName.c_str(), 0);
					g_RunnerInterface.StructAddDouble(&perksStruct, charData.skillDataList[2].skillName.c_str(), 0);
				}
				g_RunnerInterface.StructAddRValue(&charDataStruct, "perks", &perksStruct);

				g_RunnerInterface.StructAddDouble(&charDataStruct, "sizeGrade", charData.sizeGrade.value);

				g_ModuleInterface->CallBuiltin("ds_map_set", { characterDataMap, charData.charName, charDataStruct });
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
		if (g_ModuleInterface->CallBuiltin("mouse_check_button_pressed", { 1 }).AsBool())
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
		if (g_ModuleInterface->CallBuiltin("mouse_check_button_pressed", { 1 }).AsBool())
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
				RValue curCharData = g_ModuleInterface->CallBuiltin("ds_map_find_value", { characterDataMap, curCharName });
				g_ModuleInterface->CallBuiltin("array_push", { charListByGenArr[i / 10], curCharData });
				g_ModuleInterface->CallBuiltin("array_push", { charInfoArr, curCharData });

				RValue characterFollowings = g_ModuleInterface->CallBuiltin("variable_global_get", { "characterFollowings" });
				RValue tempArr = g_ModuleInterface->CallBuiltin("array_create", { 2 });
				tempArr[0] = curCharName;
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
		RValue newAttack = g_ModuleInterface->CallBuiltin("variable_clone", { attack });
		auto& charData = charDataPair.second;
		auto& curCharSprite = charSpriteMap[charDataPair.first];
		setInstanceVariable(newAttack, GML_attackID, charData.attackName);
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
		setInstanceVariable(config, GML_attackID, charData.attackName);
		setInstanceVariable(config, GML_optionID, charData.attackName);
		setInstanceVariable(config, GML_onCreate, RValue());
		setInstanceVariable(config, GML_customDrawScriptBelow, RValue());
		setInstanceVariable(config, GML_collides, true);
		setInstanceVariable(config, GML_isMain, true);
		setInstanceVariable(config, GML_maxLevel, 7);
		setInstanceVariable(config, GML_weaponType, charData.mainWeaponWeaponType);
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
			setInstanceVariable(tempConfig, GML_optionName, std::format("{} LV {}", charData.attackName, j + 1));
			setInstanceVariable(tempConfig, GML_optionDescription, charData.weaponLevelDataList[j].attackDescription);
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

		RValue newSpecial = g_ModuleInterface->CallBuiltin("variable_clone", { attack });
		setInstanceVariable(newSpecial, GML_attackID, charData.specialName);
		RValue specialConfig = getInstanceVariable(newSpecial, GML_config);
		setInstanceVariable(specialConfig, GML_attackID, charData.specialName);
		setInstanceVariable(specialConfig, GML_optionID, charData.specialName);
		setInstanceVariable(specialConfig, GML_sprite_index, curCharSprite.specialAnimationPtr->spriteRValue);
		setInstanceVariable(specialConfig, GML_duration, charData.specialDuration.value);
		setInstanceVariable(specialConfig, GML_damage, charData.specialDamage.value);
		setInstanceVariable(specialConfig, GML_collides, true);
		setInstanceVariable(specialConfig, GML_isMain, true);
		setInstanceVariable(specialConfig, GML_onCreate, RValue());
		setInstanceVariable(specialConfig, GML_customDrawScriptBelow, RValue());

		g_ModuleInterface->CallBuiltin("ds_map_set", { attackIndexMap, charData.attackName, newAttack });
		g_ModuleInterface->CallBuiltin("ds_map_set", { attackIndexMap, charData.specialName, newSpecial });
	}
}

RValue& buffApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue buffConfig = *Args[1];
	RValue buffName = getInstanceVariable(buffConfig, GML_buffName);
	RValue playerCharacter = *Args[0];
	RValue playerCharName = getInstanceVariable(playerCharacter, GML_charName);
	RValue stacks = getInstanceVariable(buffConfig, GML_stacks);
	auto& charData = charDataMap[std::string(playerCharName.AsString())];

	for (auto& buffData : charData.buffDataList)
	{
		if (buffName.AsString().compare(buffData.buffName) == 0)
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
			break;
		}
	}

	return ReturnValue;
}

RValue& buffRemove(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue buffConfig = *Args[1];
	RValue buffName = getInstanceVariable(buffConfig, GML_buffName);
	RValue playerCharacter = *Args[0];
	RValue playerCharName = getInstanceVariable(playerCharacter, GML_charName);
	RValue stacks = getInstanceVariable(buffConfig, GML_stacks);
	auto& charData = charDataMap[std::string(playerCharName.AsString())];

	for (auto& buffData : charData.buffDataList)
	{
		if (buffName.AsString().compare(buffData.buffName) == 0)
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
			break;
		}
	}
	return ReturnValue;
}

void applyBuff(RValue& playerCharacter)
{
	RValue playerCharName = getInstanceVariable(playerCharacter, GML_charName);
	auto& charData = charDataMap[std::string(playerCharName.AsString())];

	for (auto& buffData : charData.buffDataList)
	{
		if (lastStructVarGetName.AsString().compare(buffData.buffName) == 0)
		{
			RValue attackController = g_ModuleInterface->CallBuiltin("instance_find", { objAttackControllerIndex, 0 });
			RValue buffsMap = getInstanceVariable(attackController, GML_Buffs);
			RValue buffsMapData = g_ModuleInterface->CallBuiltin("ds_map_find_value", { buffsMap, buffData.buffName });
			RValue probability = getInstanceVariable(buffsMapData, GML_probability);
			std::random_device rd;
			std::default_random_engine generator(rd());
			std::uniform_real_distribution<double> distribution(0, 100);
			double randNum = distribution(generator);
			if (randNum < probability.m_Real)
			{
				RValue buffConfig;
				g_RunnerInterface.StructCreate(&buffConfig);
				setInstanceVariable(buffConfig, GML_reapply, true);
				setInstanceVariable(buffConfig, GML_stacks, 1.0);
				setInstanceVariable(buffConfig, GML_maxStacks, buffData.levels[0].maxStacks.value);
				setInstanceVariable(buffConfig, GML_buffName, buffData.buffName);
				setInstanceVariable(buffConfig, GML_buffIcon, getInstanceVariable(buffsMapData, GML_buffIcon));
				// TODO: Should probably replace this with something more efficient
				RValue ApplyBuffMethod = getInstanceVariable(attackController, GML_ApplyBuff);
				RValue ApplyBuffArr = g_ModuleInterface->CallBuiltin("array_create", { 4 });
				ApplyBuffArr[0] = playerCharacter;
				ApplyBuffArr[1] = buffData.buffName;
				ApplyBuffArr[2] = buffsMapData;
				ApplyBuffArr[3] = buffConfig;
				g_ModuleInterface->CallBuiltin("method_call", { ApplyBuffMethod, ApplyBuffArr });
			}
			return;
		}
	}
}

RValue& onDebuff(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	applyBuff(playerCharacter);
	return ReturnValue;
}

RValue& onAttackCreate(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	applyBuff(playerCharacter);
	return ReturnValue;
}

RValue& onCriticalHit(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	applyBuff(playerCharacter);
	return *Args[3];
}

RValue& onHeal(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[1];
	applyBuff(playerCharacter);
	return *Args[0];
}

RValue& onKill(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[0];
	applyBuff(playerCharacter);
	return ReturnValue;
}

RValue& onTakeDamage(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[3];
	applyBuff(playerCharacter);
	return *Args[0];
}

RValue& onDodge(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = *Args[3];
	applyBuff(playerCharacter);
	return *Args[0];
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
			g_ModuleInterface->CallBuiltin("ds_map_set", { buffsMap, buffData.buffName, buffStruct });
		}
	}
}

int curSkillIndex = 0;
int curSkillLevel = 0;

RValue& skillApply(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue playerCharacter = getInstanceVariable(Self, GML_playerCharacter);
	RValue playerCharName = getInstanceVariable(Self, GML_charName);
	RValue attackController = g_ModuleInterface->CallBuiltin("instance_find", { objAttackControllerIndex, 0 });
	RValue buffsMap = getInstanceVariable(attackController, GML_Buffs);
	auto& charData = charDataMap[std::string(playerCharName.AsString())];
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
	auto& onTriggerData = curSkillLevelData.skillOnTriggerData;
	if (onTriggerData.onTriggerType.compare("NONE") != 0)
	{
		auto& buffName = onTriggerData.buffName;
		for (auto& buffData : charData.buffDataList)
		{
			if (buffName.compare(buffData.buffName) == 0)
			{
				RValue onTriggerStruct = g_ModuleInterface->CallBuiltin("variable_instance_get", { playerCharacter, onTriggerData.onTriggerType });
				RValue retVal;
				yyGMLPushContextStack(Self);

				// TODO: Can probably do something better than this
				if (onTriggerData.onTriggerType.compare("onDebuff") == 0)
				{
					yyGMLYYSetScriptRef(&retVal, onDebuff, Self);
				}
				else if (onTriggerData.onTriggerType.compare("onAttackCreate") == 0)
				{
					yyGMLYYSetScriptRef(&retVal, onAttackCreate, Self);
				}
				else if (onTriggerData.onTriggerType.compare("onCriticalHit") == 0)
				{
					yyGMLYYSetScriptRef(&retVal, onCriticalHit, Self);
				}
				else if (onTriggerData.onTriggerType.compare("onHeal") == 0)
				{
					yyGMLYYSetScriptRef(&retVal, onHeal, Self);
				}
				else if (onTriggerData.onTriggerType.compare("onKill") == 0)
				{
					yyGMLYYSetScriptRef(&retVal, onKill, Self);
				}
				else if (onTriggerData.onTriggerType.compare("onTakeDamage") == 0)
				{
					yyGMLYYSetScriptRef(&retVal, onTakeDamage, Self);
				}
				else if (onTriggerData.onTriggerType.compare("onDodge") == 0)
				{
					yyGMLYYSetScriptRef(&retVal, onDodge, Self);
				}
				else
				{
					g_ModuleInterface->Print(CM_RED, "Invalid on trigger type for %s", buffData.buffName);
					callbackManagerInterfacePtr->LogToFile(MODNAME, "Invalid on trigger type for %s", buffData.buffName);
				}

				yyGMLPopContextStack(1);
				g_ModuleInterface->CallBuiltin("variable_instance_set", { onTriggerStruct, onTriggerData.buffName, retVal });

				RValue buffsMapStruct = g_ModuleInterface->CallBuiltin("ds_map_find_value", { buffsMap, buffData.buffName });
				setInstanceVariable(buffsMapStruct, GML_probability, static_cast<double>(onTriggerData.probability.value));

				RValue buffConfig;
				g_RunnerInterface.StructCreate(&buffConfig);
				setInstanceVariable(buffConfig, GML_reapply, true);
				setInstanceVariable(buffConfig, GML_stacks, 1.0);
				setInstanceVariable(buffConfig, GML_maxStacks, buffData.levels[0].maxStacks.value);
				setInstanceVariable(buffConfig, GML_buffName, buffData.buffName);
				setInstanceVariable(buffConfig, GML_buffIcon, getInstanceVariable(buffsMapStruct, GML_buffIcon));
				// TODO: Should probably replace this with something more efficient
				RValue UpdateBuffIfExistsMethod = getInstanceVariable(Self, GML_UpdateBuffIfExists);
				RValue UpdateBuffIfExistsArr = g_ModuleInterface->CallBuiltin("array_create", { 2 });
				UpdateBuffIfExistsArr[0] = onTriggerData.buffName;
				UpdateBuffIfExistsArr[1] = buffConfig;
				g_ModuleInterface->CallBuiltin("method_call", { UpdateBuffIfExistsMethod, UpdateBuffIfExistsArr });
				break;
			}
		}
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
			
			setInstanceVariable(newPerk, GML_id, skillData.skillName);
			setInstanceVariable(newPerk, GML_optionIcon, charSpriteMap[charDataPair.first].skillIconPtrList[j]->spriteRValue);
			setInstanceVariable(newPerk, GML_name, skillData.skillName);
			setInstanceVariable(newPerk, GML_optionName, skillData.skillName);
			setInstanceVariable(newPerk, GML_optionDescription, skillData.skillLevelDataList[0].skillDescription);
			setInstanceVariable(newPerk, GML_optionID, skillData.skillName);
			
			RValue descriptionArr = g_ModuleInterface->CallBuiltin("array_create", { 3 });
			RValue skillOnApply = g_ModuleInterface->CallBuiltin("array_create", { 3 });
			for (int level = 0; level < 3; level++)
			{
				RValue retVal;
				yyGMLPushContextStack(Self);
				yyGMLYYSetScriptRef(&retVal, onApplyList[j * 3 + level], Self);
				yyGMLPopContextStack(1);
				skillOnApply[level] = retVal;
				descriptionArr[level] = skillData.skillLevelDataList[level].skillDescription;
			}
			setInstanceVariable(newPerk, GML_OnApply, skillOnApply);

			RValue descContainer;
			g_RunnerInterface.StructCreate(&descContainer);
			setInstanceVariable(descContainer, GML_selectedLanguage, descriptionArr);
			g_ModuleInterface->CallBuiltin("variable_instance_set", { textContainer, skillData.skillName + "Description", descContainer });

			g_ModuleInterface->CallBuiltin("ds_map_set", { perksMap, skillData.skillName, newPerk });
		}
	}
}