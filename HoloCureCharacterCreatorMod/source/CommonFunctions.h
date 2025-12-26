#pragma once
#include <YYToolkit/YYTK_Shared.hpp>
#include <CallbackManager/CallbackManagerInterface.h>
#include "ModuleMain.h"

RValue getInstanceVariable(RValue instance, VariableNames variableName);
RValue getInstanceVariable(CInstance* instance, VariableNames variableName);
void setInstanceVariable(RValue instance, VariableNames variableName, RValue setValue);
void setInstanceVariable(CInstance* instance, VariableNames variableName, RValue setValue);