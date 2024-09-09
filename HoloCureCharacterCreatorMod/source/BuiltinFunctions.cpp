#include "BuiltinFunctions.h"
#include "ModuleMain.h"

RValue lastStructVarGetName;

void VariableStructGetBefore(RValue* Result, CInstance* Self, CInstance* Other, int numArgs, RValue* Args)
{
	// TODO: Can probably do something better to figure out the script function name, but it would be pretty difficult
	lastStructVarGetName = Args[1];
}