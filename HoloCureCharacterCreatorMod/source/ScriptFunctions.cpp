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
std::unordered_map<std::string, std::vector<projectileDataWrapper>> projectileDataListMap;

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
	if (!charDataMap.contains(playingCharName))
	{
		return ReturnValue;
	}
	curFrameNum = 0;
	auto& curCharData = charDataMap[playingCharName];
	buffDataListMap.clear();
	projectileDataListMap.clear();
	for (auto& buffData : curCharData.buffDataList)
	{
		buffDataListMap[buffData.buffName].push_back(buffData);
	}
	for (auto& projectileData : curCharData.projectileDataList)
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "%s", projectileData.data.projectileName.c_str());
		projectileDataListMap[projectileData.data.projectileName].push_back(projectileData);
	}
	return ReturnValue;
}