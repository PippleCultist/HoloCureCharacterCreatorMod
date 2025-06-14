#pragma once
#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include <CallbackManager/CallbackManagerInterface.h>

#define VERSION_NUM "v1.0.5"
#define MODNAME "Holocure Character Creator Mod " VERSION_NUM

#define SOME_ENUM(DO) \
	DO(idletime) \
	DO(canControl) \
	DO(actionOnePressed) \
	DO(actionTwoPressed) \
	DO(enterPressed) \
	DO(escPressed) \
	DO(moveUpPressed) \
	DO(moveDownPressed) \
	DO(moveLeftPressed) \
	DO(moveRightPressed) \
	DO(charListByGen) \
	DO(characterInfo) \
	DO(attackIndex) \
	DO(defaultConfig) \
	DO(Attack) \
	DO(config) \
	DO(sprite_index) \
	DO(attackTime) \
	DO(duration) \
	DO(damage) \
	DO(hitLimit) \
	DO(speed) \
	DO(hitCD) \
	DO(faceCreatorDirection) \
	DO(attackCount) \
	DO(attackDelay) \
	DO(range) \
	DO(onCreate) \
	DO(customDrawScriptBelow) \
	DO(collides) \
	DO(attackID) \
	DO(optionID) \
	DO(weaponType) \
	DO(isMain) \
	DO(destroyOnHitLimit) \
	DO(levels) \
	DO(optionName) \
	DO(optionDescription) \
	DO(maxLevel) \
	DO(optionIcon) \
	DO(id) \
	DO(PERKS) \
	DO(OnApply) \
	DO(name) \
	DO(selectedLanguage) \
	DO(playerCharacter) \
	DO(ATK) \
	DO(crit) \
	DO(haste) \
	DO(SPD) \
	DO(DR) \
	DO(healMultiplier) \
	DO(food) \
	DO(weaponSize) \
	DO(charName) \
	DO(timer) \
	DO(Apply) \
	DO(Callback) \
	DO(Buffs) \
	DO(buffName) \
	DO(stacks) \
	DO(reapply) \
	DO(maxStacks) \
	DO(ApplyBuff) \
	DO(UpdateBuffIfExists) \
	DO(probability) \
	DO(buffIcon) \
	DO(selectingGen) \
	DO(selectingChar) \
	DO(selectedCharacter) \
	DO(isMelee) \
	DO(stayOnCreator) \
	DO(weapons) \
	DO(level) \
	DO(ExecuteAttack) \
	DO(direction) \
	DO(x) \
	DO(y) \

#define MAKE_ENUM(VAR) GML_ ## VAR,
enum VariableNames
{
	SOME_ENUM(MAKE_ENUM)
};

#define MAKE_STRINGS(VAR) #VAR,
const char* const VariableNamesStringsArr[] =
{
	SOME_ENUM(MAKE_STRINGS)
};

using YYGML_PushContextStack = void(*)(CInstance* arg1);
using YYGML_YYSetScriptRef = void(*)(RValue* arg1, void* arg2, CInstance* arg3);
using YYGML_PopContextStack = void(*)(int arg1);

extern RValue GMLVarIndexMapGMLHash[1001];
extern CInstance* globalInstance;
extern YYTKInterface* g_ModuleInterface;
extern YYRunnerInterface g_RunnerInterface;

extern TRoutine origStructGetFromHashFunc;
extern TRoutine origStructSetFromHashFunc;

extern int objInputManagerIndex;
extern int objAttackControllerIndex;
extern int objPlayerManagerIndex;
extern int sprHudInitButtonsIndex;
extern int sprRhythmButtonsIndex;
extern int jpFont;