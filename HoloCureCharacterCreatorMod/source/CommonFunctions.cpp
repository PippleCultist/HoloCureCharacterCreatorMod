#pragma once
#include "CommonFunctions.h"

RValue result;
RValue inputArgs[3];

RValue getInstanceVariable(RValue instance, VariableNames variableName)
{
	inputArgs[0] = instance;
	inputArgs[1] = GMLVarIndexMapGMLHash[variableName];
	origStructGetFromHashFunc(result, globalInstance, nullptr, 2, inputArgs);
	return result;
}

RValue getInstanceVariable(CInstance* instance, VariableNames variableName)
{
	inputArgs[0] = instance;
	inputArgs[1] = GMLVarIndexMapGMLHash[variableName];
	origStructGetFromHashFunc(result, globalInstance, nullptr, 2, inputArgs);
	return result;
}

void setInstanceVariable(RValue instance, VariableNames variableName, RValue setValue)
{
	inputArgs[0] = instance;
	inputArgs[1] = GMLVarIndexMapGMLHash[variableName];
	inputArgs[2] = setValue;
	origStructSetFromHashFunc(result, globalInstance, nullptr, 3, inputArgs);
}

void setInstanceVariable(CInstance* instance, VariableNames variableName, RValue setValue)
{
	inputArgs[0] = instance;
	inputArgs[1] = GMLVarIndexMapGMLHash[variableName];
	inputArgs[2] = setValue;
	origStructSetFromHashFunc(result, globalInstance, nullptr, 3, inputArgs);
}