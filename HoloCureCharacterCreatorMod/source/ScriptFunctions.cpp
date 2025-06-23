#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include "ScriptFunctions.h"
#include "CommonFunctions.h"
#include "CodeEvents.h"
#include "Menu.h"

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern std::unordered_map<std::string, characterData> charDataMap;
std::string playingCharName;
std::unordered_map<std::string, std::vector<buffData>> buffDataListMap;
std::unordered_map<std::string, std::vector<actionData>> actionDataListMap;
std::unordered_map<std::string, std::vector<projectileData>> projectileDataListMap;

int curFrameNum = 0;

RValue& CanSubmitScoreFuncBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	ReturnValue.m_Kind = VALUE_BOOL;
	ReturnValue.m_Real = 0;
	callbackManagerInterfacePtr->CancelOriginalFunction();
	return ReturnValue;
}

RValue& InitializeCharacterAfter(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	RValue charData = getInstanceVariable(Self, GML_charData);
	RValue charName = getInstanceVariable(charData, GML_charName);
	playingCharName = charName.ToString();
	curFrameNum = 0;
	auto& curCharData = charDataMap[playingCharName];
	buffDataListMap.clear();
	actionDataListMap.clear();
	projectileDataListMap.clear();
	for (auto& buffData : curCharData.buffDataList)
	{
		buffDataListMap[buffData.buffName].push_back(buffData);
	}
	for (auto& actionData : curCharData.actionDataList)
	{
		actionDataListMap[actionData.actionName].push_back(actionData);
	}
	for (auto& projectileData : curCharData.projectileDataList)
	{
		projectileDataListMap[projectileData.projectileName].push_back(projectileData);
	}
	return ReturnValue;
}