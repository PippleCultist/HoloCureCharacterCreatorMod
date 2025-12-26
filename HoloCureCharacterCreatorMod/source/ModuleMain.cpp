#include <Aurie/shared.hpp>
#include <YYToolkit/YYTK_Shared.hpp>
#include "CodeEvents.h"
#include "ScriptFunctions.h"
#include "ModuleMain.h"
#include "CallbackManager/CallbackManagerInterface.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include "Menu.h"
#include "BuiltinFunctions.h"
#include <semaphore>
using namespace Aurie;
using namespace YYTK;

extern bool isInCharSelectDraw;
extern int rhythmLeftButtonIndex;
extern int rhythmRightButtonIndex;

RValue GMLVarIndexMapGMLHash[1001];

TRoutine origStructGetFromHashFunc;
TRoutine origStructSetFromHashFunc;

YYGML_PushContextStack yyGMLPushContextStack = nullptr;
YYGML_YYSetScriptRef yyGMLYYSetScriptRef = nullptr;
YYGML_PopContextStack yyGMLPopContextStack = nullptr;

CallbackManagerInterface* callbackManagerInterfacePtr = nullptr;
HoloCureMenuInterface* holoCureMenuInterfacePtr = nullptr;
YYTKInterface* g_ModuleInterface = nullptr;
YYRunnerInterface g_RunnerInterface;

PFUNC_YYGMLScript origCanSubmitScoreScript = nullptr;
PFUNC_YYGMLScript origExecuteAttackScript = nullptr;

CInstance* globalInstance = nullptr;

int objInputManagerIndex = -1;
int objAttackControllerIndex = -1;
int objPlayerManagerIndex = -1;
int objPlayerIndex = -1;
int sprHudInitButtonsIndex = -1;
int sprRhythmButtonsIndex = -1;
int jpFont = -1;

AurieStatus FindMemoryPatternAddress(const unsigned char* Pattern, const char* PatternMask, PVOID& outMemoryAddress)
{
	AurieStatus status = AURIE_SUCCESS;
	std::wstring gameName;
	if (!AurieSuccess(status = MdGetImageFilename(g_ArInitialImage, gameName)))
	{
		return status;
	}

	// Scan for pattern
	size_t patternMatch = MmSigscanModule(
		gameName.c_str(),
		Pattern,
		PatternMask
	);
	if (!patternMatch)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Couldn't find pattern %s", Pattern);
		return AURIE_OBJECT_NOT_FOUND;
	}

	outMemoryAddress = reinterpret_cast<PVOID>(patternMatch);
	return AURIE_SUCCESS;
}

AurieStatus moduleInitStatus = AURIE_MODULE_INITIALIZATION_FAILED;

void initHooks()
{
	initMenu();

	CreateDirectory(L"CharacterCreatorMod", NULL);
	CreateDirectory(L"CharacterCreatorMod/inputData", NULL);

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_get_from_hash", nullptr, nullptr, &origStructGetFromHashFunc)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "struct_get_from_hash");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_set_from_hash", nullptr, nullptr, &origStructSetFromHashFunc)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "struct_set_from_hash");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "variable_struct_get", VariableStructGetBefore, nullptr, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "variable_struct_get");
		return;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_CharacterData_Create_0", nullptr, CharacterDataCreateAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_CharacterData_Create_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_CharSelect_Create_0", nullptr, CharSelectCreateAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_CharSelect_Create_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_CharSelect_Draw_0", CharSelectDrawBefore, CharSelectDrawAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_CharSelect_Draw_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_TitleScreen_Draw_0", nullptr, TitleScreenDrawAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_TitleScreen_Draw_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_AttackController_Create_0", nullptr, AttackControllerCreateAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_AttackController_Create_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_AttackController_Other_11", nullptr, AttackControllerOther11After)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_AttackController_Other_11");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_PlayerManager_Other_22", nullptr, PlayerManagerOther22After)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_PlayerManager_Other_22");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_Attack_Create_0", nullptr, AttackCreateAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_Attack_Create_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_Attack_Step_0", AttackStepBefore, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_Attack_Step_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_Attack_Destroy_0", AttackDestroyBefore, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_Attack_Destroy_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_Player_Step_0", PlayerStepBefore, PlayerStepAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_Player_Step_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_BaseMob_Step_0", nullptr, BaseMobStepAfter)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Object_obj_BaseMob_Step_0");
		return;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_CanSubmitScore@gml_Object_obj_PlayerManager_Create_0", CanSubmitScoreFuncBefore, nullptr, &origCanSubmitScoreScript)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Script_CanSubmitScore@gml_Object_obj_PlayerManager_Create_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_ExecuteAttack@gml_Object_obj_AttackController_Create_0", nullptr, nullptr, &origExecuteAttackScript)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Script_ExecuteAttack@gml_Object_obj_AttackController_Create_0");
		return;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_InitializeCharacter@gml_Object_obj_PlayerManager_Create_0", nullptr, InitializeCharacterAfter, nullptr)))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to register callback for %s", "gml_Script_InitializeCharacter@gml_Object_obj_PlayerManager_Create_0");
		return;
	}

	PVOID tempPushContextStackPtr = nullptr;
	AurieStatus status = FindMemoryPatternAddress(
		UTEXT(
			"\x40\x53"						// PUSH RBX
			"\x48\x83\xEC\x30"				// SUB RSP, 0x30
			"\x80\x3D\x00\x00\x00\x00\x00"	// CMP byte ptr [DAT_?], 0x0
			"\x48\x8B\xD9"					// MOV RBX, RCX
			"\x0F\x84\x82\x00\x00\x00"		// JZ LAB_142151f08
		),
		"xx"
		"xxxx"
		"xx????x"
		"xxx"
		"xxxxxx",
		tempPushContextStackPtr
	);
	if (!AurieSuccess(status))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to find memory address for %s", "PushContextStack");
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to find memory address for %s", "PushContextStack");
		return;
	}
	yyGMLPushContextStack = static_cast<YYGML_PushContextStack>(tempPushContextStackPtr);

	PVOID tempYYGMLYYSetScriptRefPtr = nullptr;
	status = FindMemoryPatternAddress(
		UTEXT(
			"\x48\x89\x5C\x24\x10"	// MOV qword ptr [RSP + local_res10], RBX
			"\x48\x89\x74\x24\x18"	// MOV qword ptr [RSP + local_res18], RSI
			"\x48\x89\x7C\x24\x20"	// MOV qword ptr [RSP + local_res20], RDI
			"\x41\x56"				// PUSH R14
			"\x48\x83\xEC\x20"		// SUB RSP, 0x20
			"\x49\x8B\xF8"			// MOV RDI, R8
			"\x48\x8B\xF2"			// MOV RSI, RDX
			"\x4C\x8B\xF1"			// MOV R14, RCX
		),
		"xxxxx"
		"xxxxx"
		"xxxxx"
		"xx"
		"xxxx"
		"xxx"
		"xxx"
		"xxx",
		tempYYGMLYYSetScriptRefPtr
	);
	if (!AurieSuccess(status))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to find memory address for %s", "YYSetScriptRef");
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to find memory address for %s", "YYSetScriptRef");
		return;
	}

	yyGMLYYSetScriptRef = static_cast<YYGML_YYSetScriptRef>(tempYYGMLYYSetScriptRefPtr);

	PVOID tempYYGMLPopContextStackPtr = nullptr;
	status = FindMemoryPatternAddress(
		UTEXT(
			"\x8B\x05\x00\x00\x00\x00"	// MOV EAX, dword ptr [DAT_?]
			"\x2B\xC1"					// SUB EAX, ECX
			"\xB9\x00\x00\x00\x00"		// MOV ECX, 0x0
			"\x0F\x48\xC1"				// CMOVS EAX, ECX
			"\x89\x05\x00\x00\x00\x00"	// MOV dword ptr [DAT_?], EAX
		),
		"xx????"
		"xx"
		"xxxxx"
		"xxx"
		"xx????",
		tempYYGMLPopContextStackPtr
	);
	if (!AurieSuccess(status))
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to find memory address for %s", "PopContextStack");
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to find memory address for %s", "PopContextStack");
		return;
	}

	yyGMLPopContextStack = static_cast<YYGML_PopContextStack>(tempYYGMLPopContextStackPtr);

	g_RunnerInterface = g_ModuleInterface->GetRunnerInterface();
	g_ModuleInterface->GetGlobalInstance(&globalInstance);

	objInputManagerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_InputManager" }).ToInt32());
	objAttackControllerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_AttackController" }).ToInt32());
	objPlayerManagerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_PlayerManager" }).ToInt32());
	objPlayerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_Player" }).ToInt32());
	sprHudInitButtonsIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "hud_initButtons" }).ToInt32());
	sprRhythmButtonsIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "spr_rhythmButtons" }).ToInt32());
	jpFont = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "jpFont" }).ToInt32());

	for (int i = 0; i < std::extent<decltype(VariableNamesStringsArr)>::value; i++)
	{
		if (!AurieSuccess(status))
		{
			DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to get hash for %s", VariableNamesStringsArr[i]);
		}
		GMLVarIndexMapGMLHash[i] = std::move(g_ModuleInterface->CallBuiltin("variable_get_hash", { VariableNamesStringsArr[i] }));
	}

	moduleInitStatus = AURIE_SUCCESS;

	callbackManagerInterfacePtr->LogToFile(MODNAME, "Finished initializing");
}

void runnerInitCallback(FunctionWrapper<void(int)>& dummyWrapper)
{
	// TODO: Top priority - Add actions which will be kind of like mini code blocks that can trigger other stuff (Maybe including other actions?). 
	// This should include spawning projectiles, spawning food, doing other stuff ... Maybe include start action and end action stuff?
	// Probably should add a time based trigger for actions. Also add onTrigger for projectiles that can trigger actions as well
	// 
	// TODO: Add an option to be able to use in game data/code as base
	// 
	// TODO: Should add levels for buffs
	// TODO: Probably should try to avoid loading the same image multiple times if it is used multiple times
	// TODO: Add delete option for buffs
	// TODO: Should probably improve the onTrigger selection to not need to default to none
	AurieStatus status = AURIE_SUCCESS;
	status = ObGetInterface("callbackManager", (AurieInterfaceBase*&)callbackManagerInterfacePtr);
	if (!AurieSuccess(status))
	{
		printf("Failed to get callback manager interface. Make sure that CallbackManagerMod is located in the mods/Aurie directory.\n");
		return;
	}

	status = ObGetInterface("HoloCureMenuInterface", (AurieInterfaceBase*&)holoCureMenuInterfacePtr);
	if (!AurieSuccess(status))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to get HoloCure Menu interface. Make sure that HoloCureMenuInterfaceMod is located in the mods/Aurie directory.\n");
		return;
	}

	callbackManagerInterfacePtr->RegisterInitFunction(initHooks);
}


EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	AurieStatus status = AURIE_SUCCESS;
	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	g_ModuleInterface = GetInterface();

	// If we can't get the interface, we fail loading.
	if (!AurieSuccess(status))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to get YYTK Interface");
		printf("Failed to get YYTK Interface\n");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	g_ModuleInterface->CreateCallback(
		Module,
		EVENT_RUNNER_INIT,
		runnerInitCallback,
		0
	);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	return moduleInitStatus;
}