#include <Aurie/shared.hpp>
#include <YYToolkit/shared.hpp>
#include "ScriptFunctions.h"
#include "CommonFunctions.h"
#include "CodeEvents.h"
#include "Menu.h"

extern CallbackManagerInterface* callbackManagerInterfacePtr;

RValue& CanSubmitScoreFuncBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args)
{
	ReturnValue.m_Kind = VALUE_BOOL;
	ReturnValue.m_Real = 0;
	callbackManagerInterfacePtr->CancelOriginalFunction();
	return ReturnValue;
}