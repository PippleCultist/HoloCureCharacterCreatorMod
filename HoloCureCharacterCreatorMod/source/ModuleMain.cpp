#include <Aurie/shared.hpp>
#include <YYToolkit/Shared.hpp>
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

CInstance* globalInstance = nullptr;

int objInputManagerIndex = -1;
int objAttackControllerIndex = -1;
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
		g_ModuleInterface->Print(CM_RED, "Couldn't find pattern %s", Pattern);
		return AURIE_OBJECT_NOT_FOUND;
	}

	outMemoryAddress = reinterpret_cast<PVOID>(patternMatch);
	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	// TODO: Should add levels for buffs
	// TODO: Probably should try to avoid loading the same image multiple times if it is used multiple times
	// TODO: Add delete option for buffs
	// TODO: Should probably improve the onTrigger selection to not need to default to none
	AurieStatus status = AURIE_SUCCESS;
	status = ObGetInterface("callbackManager", (AurieInterfaceBase*&)callbackManagerInterfacePtr);
	if (!AurieSuccess(status))
	{
		printf("Failed to get callback manager interface. Make sure that CallbackManagerMod is located in the mods/Aurie directory.\n");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	// If we can't get the interface, we fail loading.
	if (!AurieSuccess(status))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to get YYTK Interface");
		printf("Failed to get YYTK Interface\n");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	status = ObGetInterface("HoloCureMenuInterface", (AurieInterfaceBase*&)holoCureMenuInterfacePtr);
	if (!AurieSuccess(status))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to get HoloCure Menu interface. Make sure that HoloCureMenuInterfaceMod is located in the mods/Aurie directory.\n");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	initMenu();

	CreateDirectory(L"CharacterCreatorMod", NULL);
	CreateDirectory(L"CharacterCreatorMod/inputData", NULL);

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_get_from_hash", nullptr, nullptr, &origStructGetFromHashFunc)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "struct_get_from_hash");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "struct_set_from_hash", nullptr, nullptr, &origStructSetFromHashFunc)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "struct_set_from_hash");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterBuiltinFunctionCallback(MODNAME, "variable_struct_get", VariableStructGetBefore, nullptr, nullptr)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "variable_struct_get");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_CharacterData_Create_0", nullptr, CharacterDataCreateAfter)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_CharacterData_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_CharSelect_Create_0", nullptr, CharSelectCreateAfter)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_CharSelect_Draw_0", CharSelectDrawBefore, CharSelectDrawAfter)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_CharSelect_Draw_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_AttackController_Create_0", nullptr, AttackControllerCreateAfter)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_AttackController_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_AttackController_Other_11", nullptr, AttackControllerOther11After)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_AttackController_Other_11");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterCodeEventCallback(MODNAME, "gml_Object_obj_PlayerManager_Other_22", nullptr, PlayerManagerOther22After)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Object_obj_PlayerManager_Other_22");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	if (!AurieSuccess(callbackManagerInterfacePtr->RegisterScriptFunctionCallback(MODNAME, "gml_Script_CanSubmitScore_gml_Object_obj_PlayerManager_Create_0", CanSubmitScoreFuncBefore, nullptr, &origCanSubmitScoreScript)))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to register callback for %s", "gml_Script_CanSubmitScore_gml_Object_obj_PlayerManager_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	PVOID tempPushContextStackPtr = nullptr;
	status = FindMemoryPatternAddress(
		UTEXT(
			"\x40\x53"						// PUSH RBX
			"\x48\x83\xEC\x30"				// SUB RSP, 0x30
			"\x80\x3D\x26\x63\xE8\x00\x00"	// CMP byte ptr [DAT_142fd81a3], 0x0
			"\x48\x8B\xD9"					// MOV RBX, RCX
			"\x0F\x84\x82\x00\x00\x00"		// JZ LAB_142151f08
		),
		"xx"
		"xxxx"
		"xxxxxxx"
		"xxx"
		"xxxxxx",
		tempPushContextStackPtr
	);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to find memory address for %s", "PushContextStack");
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to find memory address for %s", "PushContextStack");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	yyGMLPushContextStack = static_cast<YYGML_PushContextStack>(tempPushContextStackPtr);

	PVOID tempYYGMLYYSetScriptRefPtr = nullptr;
	status = FindMemoryPatternAddress(
		UTEXT(
			"\x48\x89\x5C\x24\x10"	// MOV qword ptr [RSP + local_res10], RBX
			"\x48\x89\x6C\x24\x18"	// MOV qword ptr [RSP + local_res18], RBP
			"\x48\x89\x74\x24\x20"	// MOV qword ptr [RSP + local_res20], RSI
			"\x57"					// PUSH RDI
			"\x41\x54"				// PUSH R12
			"\x41\x55"				// PUSH R13
			"\x41\x56"				// PUSH R14
			"\x41\x57"				// PUSH R15
			"\x48\x83\xEC\x20"		// SUB RSP, 0x20
			"\x49\x8B\xD8"			// MOV RBX, R8
		),
		"xxxxx"
		"xxxxx"
		"xxxxx"
		"x"
		"xx"
		"xx"
		"xx"
		"xx"
		"xxxx"
		"xxx",
		tempYYGMLYYSetScriptRefPtr
	);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_RED, "Failed to find memory address for %s", "YYSetScriptRef");
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to find memory address for %s", "YYSetScriptRef");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
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
		g_ModuleInterface->Print(CM_RED, "Failed to find memory address for %s", "PopContextStack");
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Failed to find memory address for %s", "PopContextStack");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	yyGMLPopContextStack = static_cast<YYGML_PopContextStack>(tempYYGMLPopContextStackPtr);

	g_RunnerInterface = g_ModuleInterface->GetRunnerInterface();
	g_ModuleInterface->GetGlobalInstance(&globalInstance);

	objInputManagerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_InputManager" }).AsReal());
	objAttackControllerIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "obj_AttackController" }).AsReal());
	sprHudInitButtonsIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "hud_initButtons" }).AsReal());
	sprRhythmButtonsIndex = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "spr_rhythmButtons" }).AsReal());
	jpFont = static_cast<int>(g_ModuleInterface->CallBuiltin("asset_get_index", { "jpFont" }).AsReal());

	for (int i = 0; i < std::extent<decltype(VariableNamesStringsArr)>::value; i++)
	{
		if (!AurieSuccess(status))
		{
			g_ModuleInterface->Print(CM_RED, "Failed to get hash for %s", VariableNamesStringsArr[i]);
		}
		GMLVarIndexMapGMLHash[i] = std::move(g_ModuleInterface->CallBuiltin("variable_get_hash", { VariableNamesStringsArr[i] }));
	}

	callbackManagerInterfacePtr->LogToFile(MODNAME, "Finished initializing");

	return AURIE_SUCCESS;
}