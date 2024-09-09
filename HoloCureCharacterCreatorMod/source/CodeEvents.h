#pragma once
#include "ModuleMain.h"
#include <Aurie/shared.hpp>
#include <YYToolkit/shared.hpp>

void CharacterDataCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void CharSelectCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void CharSelectDrawBefore(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void CharSelectDrawAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void AttackControllerCreateAfter(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void AttackControllerOther11After(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);
void PlayerManagerOther22After(std::tuple<CInstance*, CInstance*, CCode*, int, RValue*>& Args);