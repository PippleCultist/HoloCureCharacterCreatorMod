#pragma comment(lib, "d3d11.lib")
#define IMGUI_DEFINE_MATH_OPERATORS

#include "Menu.h"
#include "ModuleMain.h"
#include "CodeEvents.h"
#include "CommonFunctions.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include "nlohmann/json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_stdlib.h"
#include <d3d11.h>
#include <queue>
#include <fstream>
#include <regex>

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern HoloCureMenuInterface* holoCureMenuInterfacePtr;
extern std::unordered_map<std::string, characterDataStruct> characterDataMap;

void handleNodeEditor(nodeEditor& curNodeEditor);

ax::NodeEditor::PinId prevLinkPinId = 0;

int spriteDequePage = 0;
std::deque<std::shared_ptr<spriteData>> spriteDeque;

int loadCharacterPage = 0;
std::deque<std::string> loadCharacterDeque;

int weaponLevelMenuIndex = -1;
int skillMenuIndex = -1;
int skillLevelMenuIndex = -1;
int buffMenuIndex = -1;
int actionMenuIndex = -1;
int projectileMenuIndex = -1;

int curCharIdx = -1;
int loadedCharIdx = -1;
int curBuffDataIdx = -1;
int curSoundDataIdx = -1;
bool hasLoadedData = false;
bool showBuffDataWindow = false;
bool showIdleAnimationWindow = false;
bool showRunAnimationWindow = false;
bool showPortraitWindow = false;
bool showLargePortraitWindow = false;
bool showSpecialAnimationWindow = false;
bool showWeaponLevelsWindow = false;
bool showSkillDataWindow = false;
bool showProjectileDataWindow = false;
bool showSoundDataWindow = false;
bool isAttackAnimationPlaying = false;
bool isIdleAnimationPlaying = false;
bool isRunAnimationPlaying = false;
bool isSpecialAnimationPlaying = false;

int attackAnimationWidth = 0;
int attackAnimationHeight = 0;
int attackAnimationNumFrames = 0;
double attackAnimationCurFrame = 0;
ID3D11ShaderResourceView* attackAnimationTexture = NULL;

int idleAnimationWidth = 0;
int idleAnimationHeight = 0;
int idleAnimationNumFrames = 0;
double idleAnimationCurFrame = 0;
ID3D11ShaderResourceView* idleAnimationTexture = NULL;

int runAnimationWidth = 0;
int runAnimationHeight = 0;
int runAnimationNumFrames = 0;
double runAnimationCurFrame = 0;
ID3D11ShaderResourceView* runAnimationTexture = NULL;

int attackIconWidth = 0;
int attackIconHeight = 0;
ID3D11ShaderResourceView* attackIconTexture = NULL;

int attackAwakenedIconWidth = 0;
int attackAwakenedIconHeight = 0;
ID3D11ShaderResourceView* attackAwakenedIconTexture = NULL;

int buffIconWidth = 0;
int buffIconHeight = 0;
ID3D11ShaderResourceView* buffIconTexture = NULL;

int portraitIconWidth = 0;
int portraitIconHeight = 0;
ID3D11ShaderResourceView* portraitIconTexture = NULL;

int largePortraitIconWidth = 0;
int largePortraitIconHeight = 0;
ID3D11ShaderResourceView* largePortraitIconTexture = NULL;

int specialAnimationWidth = 0;
int specialAnimationHeight = 0;
int specialAnimationNumFrames = 0;
double specialAnimationCurFrame = 0;
ID3D11ShaderResourceView* specialAnimationTexture = NULL;

int specialIconWidth = 0;
int specialIconHeight = 0;
ID3D11ShaderResourceView* specialIconTexture = NULL;

std::vector<std::string> charList;
std::vector<std::string> imageList;
std::vector<std::string> soundList;
characterData curCharData;

std::shared_ptr<menuData> characterCreatorMenuLoadCharacter(new menuDataButton(60, 46 + 29 * 0, 180, 29, "CHARACTERCREATORMENU_LoadCharacter", "Load Character", true, loadCharacterClickButton, nullptr));

nodeEditorNodePin::~nodeEditorNodePin()
{
	if (data != nullptr)
	{
		delete data;
	}
}

nodeEditor::nodeEditor(const nodeEditor& otherNodeEditor) :
	nextID(otherNodeEditor.nextID), isEditorOpen(otherNodeEditor.isEditorOpen), initEditorPos(otherNodeEditor.initEditorPos), editorContext(otherNodeEditor.editorContext),
	nodeMap(otherNodeEditor.nodeMap), nodePinMap(otherNodeEditor.nodePinMap), nodeLinkMap(otherNodeEditor.nodeLinkMap)
{
	for (auto& curNodePair : nodeMap)
	{
		auto& curNode = curNodePair.second;
		curNodePair.second.parentNodeEditor = this;
		curNode.inputPinPtrArr.clear();
		curNode.outputPinPtrArr.clear();
		for (auto& inputPinID : curNode.inputPinIDArr)
		{
			curNode.inputPinPtrArr.push_back(&nodePinMap[inputPinID]);
		}
		for (auto& outputPinID : curNode.outputPinIDArr)
		{
			curNode.outputPinPtrArr.push_back(&nodePinMap[outputPinID]);
		}
	}
	for (auto& curNodePinPair : nodePinMap)
	{
		curNodePinPair.second.parentNodePtr = &nodeMap[curNodePinPair.second.parentNodeID];
		curNodePinPair.second.data = new pinVariableData();
		for (auto& curNodeLink: curNodePinPair.second.nodeLinks)
		{
			nodeLinkMap[curNodeLink.linkID].startPinPtr = curNodeLink.startPinPtr = &nodePinMap[curNodeLink.startPinID];
			nodeLinkMap[curNodeLink.linkID].endPinPtr = curNodeLink.endPinPtr = &nodePinMap[curNodeLink.endPinID];
		}
	}
}

nodeEditor& nodeEditor::operator=(const nodeEditor& otherNodeEditor)
{
	nextID = otherNodeEditor.nextID;
	isEditorOpen = otherNodeEditor.isEditorOpen;
	initEditorPos = otherNodeEditor.initEditorPos;
	editorContext = otherNodeEditor.editorContext;
	nodeMap = otherNodeEditor.nodeMap;
	nodePinMap = otherNodeEditor.nodePinMap;
	nodeLinkMap = otherNodeEditor.nodeLinkMap;
	for (auto& curNodePair : nodeMap)
	{
		auto& curNode = curNodePair.second;
		curNodePair.second.parentNodeEditor = this;
		curNode.inputPinPtrArr.clear();
		curNode.outputPinPtrArr.clear();
		for (auto& inputPinID : curNode.inputPinIDArr)
		{
			curNode.inputPinPtrArr.push_back(&nodePinMap[inputPinID]);
		}
		for (auto& outputPinID : curNode.outputPinIDArr)
		{
			curNode.outputPinPtrArr.push_back(&nodePinMap[outputPinID]);
		}
	}
	for (auto& curNodePinPair : nodePinMap)
	{
		curNodePinPair.second.parentNodePtr = &nodeMap[curNodePinPair.second.parentNodeID];
		curNodePinPair.second.data = new pinVariableData();
		for (auto& curNodeLink : curNodePinPair.second.nodeLinks)
		{
			nodeLinkMap[curNodeLink.linkID].startPinPtr = curNodeLink.startPinPtr = &nodePinMap[curNodeLink.startPinID];
			nodeLinkMap[curNodeLink.linkID].endPinPtr = curNodeLink.endPinPtr = &nodePinMap[curNodeLink.endPinID];
		}
	}
	return *this;
}

static std::string pinTypeToString(nodeEditorPinTypeEnum pinType)
{
	switch (pinType)
	{
		case nodeEditorPinType_None:
		{
			return "None";
		}
		case nodeEditorPinType_CodeFlow:
		{
			return "Code";
		}
		case nodeEditorPinType_ProjectileData:
		{
			return "Projectile";
		}
		case nodeEditorPinType_Number:
		{
			return "Number";
		}
		case nodeEditorPinType_Instance:
		{
			return "Instance";
		}
		case nodeEditorPinType_VariableName:
		{
			return "VarName";
		}
		case nodeEditorPinType_Boolean:
		{
			return "Boolean";
		}
		case nodeEditorPinType_String:
		{
			return "String";
		}
		case nodeEditorPinType_Integer:
		{
			return "Integer";
		}
		case nodeEditorPinType_RValue:
		{
			return "RValue";
		}
		case nodeEditorPinType_BuffData:
		{
			return "Buff";
		}
		case nodeEditorPinType_DSList:
		{
			return "DS List";
		}
		case nodeEditorPinType_DSMap:
		{
			return "DS Map";
		}
		case nodeEditorPinType_Array:
		{
			return "Array";
		}
		case nodeEditorPinType_SoundData:
		{
			return "Sound";
		}
		case nodeEditorPinType_FlushCache:
		{
			return "Flush Cache";
		}
		default:
		{
			return "Undefined";
		}
	}
}

void nodeEditorNode::setPinSpacing()
{
	switch (nodeType)
	{
		case nodeEditorNodeType_OnCreateProjectile:
		case nodeEditorNodeType_OnDestroyProjectile:
		case nodeEditorNodeType_InitVariable:
		case nodeEditorNodeType_GetVariable:
		case nodeEditorNodeType_SetVariable:
		case nodeEditorNodeType_If:
		case nodeEditorNodeType_Compare:
		case nodeEditorNodeType_While:
		case nodeEditorNodeType_MergeCodeFlow:
		case nodeEditorNodeType_And:
		case nodeEditorNodeType_Or:
		case nodeEditorNodeType_Not:
		case nodeEditorNodeType_Print:
		case nodeEditorNodeType_TypeCast:
		case nodeEditorNodeType_AppendString:
		case nodeEditorNodeType_IntegerCeiling:
		case nodeEditorNodeType_IntegerFloor:
		case nodeEditorNodeType_IntegerRound:
		case nodeEditorNodeType_IntegerCompare:
		case nodeEditorNodeType_GetStructVariable:
		case nodeEditorNodeType_SetStructVariable:
		case nodeEditorNodeType_OnFrameStep:
		case nodeEditorNodeType_OnSkillApply:
		case nodeEditorNodeType_IsRValueDefined:
		case nodeEditorNodeType_CollisionCircleList:
		case nodeEditorNodeType_DSListGet:
		case nodeEditorNodeType_DSListSize:
		case nodeEditorNodeType_DSMapGet:
		case nodeEditorNodeType_DSMapKeysToArray:
		case nodeEditorNodeType_ArrayGet:
		case nodeEditorNodeType_ArrayLength:
		case nodeEditorNodeType_Random:
		case nodeEditorNodeType_IntegerRandom:
		case nodeEditorNodeType_Modulus:
		case nodeEditorNodeType_SquareRoot:
		case nodeEditorNodeType_Logarithm:
		case nodeEditorNodeType_TernaryOperator:
		case nodeEditorNodeType_SetDebugLevel:
		case nodeEditorNodeType_CacheVariable:
		case nodeEditorNodeType_PlaySound:
		case nodeEditorNodeType_ArrayPush:
		case nodeEditorNodeType_DSListClear:
		case nodeEditorNodeType_DSListDestroy:
		{
			pinSpacing = 100;
			break;
		}
		case nodeEditorNodeType_PointDirection:
		{
			pinSpacing = 200;
			break;
		}
		case nodeEditorNodeType_ProjectileData:
		case nodeEditorNodeType_SpawnProjectile:
		case nodeEditorNodeType_BuffData:
		case nodeEditorNodeType_ApplyBuff:
		{
			pinSpacing = 300;
			break;
		}
		case nodeEditorNodeType_Number:
		case nodeEditorNodeType_ThisInstance:
		case nodeEditorNodeType_GlobalInstance:
		case nodeEditorNodeType_PlayerManagerInstance:
		case nodeEditorNodeType_PlayerInstance:
		case nodeEditorNodeType_Add:
		case nodeEditorNodeType_Subtract:
		case nodeEditorNodeType_Multiply:
		case nodeEditorNodeType_Divide:
		case nodeEditorNodeType_Delay:
		case nodeEditorNodeType_Boolean:
		case nodeEditorNodeType_String:
		case nodeEditorNodeType_Integer:
		case nodeEditorNodeType_IntegerAdd:
		case nodeEditorNodeType_IntegerSubtract:
		case nodeEditorNodeType_IntegerMultiply:
		case nodeEditorNodeType_IntegerDivide:
		case nodeEditorNodeType_AssetGetIndex:
		case nodeEditorNodeType_Sine:
		case nodeEditorNodeType_Cosine:
		case nodeEditorNodeType_Tangent:
		case nodeEditorNodeType_SoundData:
		case nodeEditorNodeType_FlushCache:
		case nodeEditorNodeType_MergeFlush:
		case nodeEditorNodeType_ArrayCreate:
		case nodeEditorNodeType_DSListCreate:
		{
			pinSpacing = 0;
			break;
		}
		default:
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Unknown pin type %d", nodeType);
			break;
		}
	}
}

void nodeEditorNode::setCanDelete()
{
	switch (nodeType)
	{
		case nodeEditorNodeType_OnCreateProjectile:
		case nodeEditorNodeType_OnDestroyProjectile:
		case nodeEditorNodeType_OnFrameStep:
		case nodeEditorNodeType_OnSkillApply:
		{
			canDelete = false;
			break;
		}
		default:
		{
			canDelete = true;
		}
	}
}

void nodeEditorNode::drawNode()
{
	// First time init node position if loading from save
	if (abs(initPosX) < 1e10 && abs(initPosY) < 1e10)
	{
		ax::NodeEditor::SetNodePosition(nodeID, ImVec2(initPosX, initPosY));
		initPosX = 1e20;
		initPosY = 1e20;
	}
	// TODO: Make the pins look more like Unreal pins
	ax::NodeEditor::BeginNode(nodeID);
	ImGui::Text(nodeName.c_str());
	size_t maxPins = max(inputPinIDArr.size(), outputPinIDArr.size());
	for (int i = 0; i < maxPins; i++)
	{
		if (i < inputPinIDArr.size())
		{
			auto& inputPin = parentNodeEditor->nodePinMap[inputPinIDArr[i]];
			if (inputPin.pinType == nodeEditorPinType_VariableName)
			{
				ImGui::PushID(inputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(inputPin.pinVariableDataName.c_str()))
				{
					inputPin.variableDataMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
			}
			else if (inputPin.pinType == nodeEditorPinType_Compare)
			{
				ImGui::PushID(inputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(inputPin.pinCompareDataName.c_str()))
				{
					inputPin.variableCompareMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
			}
			else if (inputPin.pinType == nodeEditorPinType_VariableType)
			{
				ImGui::PushID(inputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(inputPin.pinVariableTypeName.c_str()))
				{
					inputPin.variableTypeMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
			}
			else if (inputPin.pinType != nodeEditorPinType_None)
			{
				ax::NodeEditor::BeginPin(inputPinIDArr[i], ax::NodeEditor::PinKind::Input);
				ImGui::Text("->");
				ax::NodeEditor::EndPin();
				ImGui::SameLine();
				ImGui::Text(std::format("({}) {}", pinTypeToString(parentNodeEditor->nodePinMap[inputPinIDArr[i]].pinType), parentNodeEditor->nodePinMap[inputPinIDArr[i]].pinName).c_str());
			}
			else
			{
				ImGui::Text(" ");
			}
		}
		else
		{
			ImGui::Text(" ");
		}
		ImGui::SameLine(pinSpacing);
		if (i < outputPinIDArr.size())
		{
			auto& outputPin = parentNodeEditor->nodePinMap[outputPinIDArr[i]];
			if (nodeType == nodeEditorNodeType_Number && outputPin.pinType == nodeEditorPinType_Number && !outputPin.isInput)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				ImGui::InputDouble("##PinNumberOutput", &outputPin.pinNumberVar);
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::SameLine();
			}
			else if (nodeType == nodeEditorNodeType_ProjectileData && outputPin.pinType == nodeEditorPinType_ProjectileData && !outputPin.isInput)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(outputPin.pinProjectileDataName.c_str()))
				{
					outputPin.projectileDataMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::SameLine();
			}
			else if (nodeType == nodeEditorNodeType_SoundData && outputPin.pinType == nodeEditorPinType_SoundData && !outputPin.isInput)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(outputPin.pinSoundDataName.c_str()))
				{
					outputPin.soundDataMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::SameLine();
			}
			else if (nodeType == nodeEditorNodeType_BuffData && outputPin.pinType == nodeEditorPinType_BuffData && !outputPin.isInput)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(outputPin.pinBuffDataName.c_str()))
				{
					outputPin.buffDataMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::SameLine();
			}
			else if (nodeType == nodeEditorNodeType_Boolean && outputPin.pinType == nodeEditorPinType_Boolean && !outputPin.isInput)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(outputPin.pinBooleanDataName.c_str()))
				{
					outputPin.variableBooleanMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::SameLine();
			}
			else if (nodeType == nodeEditorNodeType_String && outputPin.pinType == nodeEditorPinType_String && !outputPin.isInput)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				ImGui::InputText("##PinStringOutput", &outputPin.pinStringVar);
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::SameLine();
			}
			else if (nodeType == nodeEditorNodeType_Integer && outputPin.pinType == nodeEditorPinType_Integer && !outputPin.isInput)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				ImGui::InputInt("##PinIntegerOutput", &outputPin.pinIntegerVar);
				ImGui::PopItemWidth();
				ImGui::PopID();
				ImGui::SameLine();
			}
			else if (outputPin.pinType == nodeEditorPinType_VariableType)
			{
				ImGui::PushID(outputPin.pinID);
				ImGui::PushItemWidth(100.0f);
				if (ImGui::Button(outputPin.pinVariableTypeName.c_str()))
				{
					outputPin.variableTypeMenuState = nodeEditorPinMenuState_Clicked;
				}
				ImGui::PopItemWidth();
				ImGui::PopID();
				continue;
			}
			if (outputPin.pinType != nodeEditorPinType_None)
			{
				ImGui::Text(std::format("{} ({})", parentNodeEditor->nodePinMap[outputPinIDArr[i]].pinName, pinTypeToString(parentNodeEditor->nodePinMap[outputPinIDArr[i]].pinType)).c_str());
				ImGui::SameLine();
				ax::NodeEditor::BeginPin(outputPin.pinID, ax::NodeEditor::PinKind::Output);
				ImGui::Text("->");
				ax::NodeEditor::EndPin();
			}
			else
			{
				ImGui::Text(" ");
			}
		}
		else
		{
			ImGui::Text(" ");
		}
	}
	ax::NodeEditor::EndNode();
}

menuGrid characterCreatorMenuGrid({
	menuColumn({
		characterCreatorMenuLoadCharacter,
	}),
}, "Create Character", nullptr);

void initMenu()
{
	characterCreatorMenuGrid.initMenuGrid();
}

bool loadCharacterData(std::string dirName, characterData& charData)
{
	if (!std::filesystem::exists("CharacterCreatorMod/" + dirName))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the %s directory", dirName.c_str());
		DbgPrintEx(LOG_SEVERITY_ERROR, "Couldn't find the %s directory", dirName.c_str());
		return false;
	}

	if (!std::filesystem::exists("CharacterCreatorMod/" + dirName + "/charData.json"))
	{
		return false;
	}

	callbackManagerInterfacePtr->LogToFile(MODNAME, "Loading %s", dirName.c_str());
	DbgPrintEx(LOG_SEVERITY_INFO, "Loading %s", dirName.c_str());
	std::ifstream inFile;
	inFile.open("CharacterCreatorMod/" + dirName + "/charData.json");
	try
	{
		nlohmann::json inputData = nlohmann::json::parse(inFile);
		charData = inputData.template get<characterData>();
		charData.charName = dirName.substr(5);
	}
	catch (nlohmann::json::parse_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Parse Error: %s when parsing %s", e.what(), dirName.c_str());
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Parse Error: %s when parsing %s", e.what(), dirName.c_str());
	}
	
	return true;
}

int getSpriteNumFrames(const std::string spritePathStr)
{
	int pos = -1;
	const std::regex regexPattern("_strip[0-9]+\\.(png|jpg)");
	for (std::sregex_iterator it = std::sregex_iterator(spritePathStr.begin(), spritePathStr.end(), regexPattern); it != std::sregex_iterator(); it++)
	{
		std::smatch match = *it;
		pos = static_cast<int>(match.position(0));
	}
	int numFrames = 1;
	if (pos != -1)
	{
		std::string frameStr = spritePathStr.substr(pos + 6, spritePathStr.size() - pos - 1 + 4);
		std::from_chars(frameStr.data(), frameStr.data() + frameStr.size(), numFrames);
	}
	return numFrames;
}

void resetCharData()
{
	curCharIdx = -1;
	loadedCharIdx = -1;
	hasLoadedData = false;
	charList.clear();
	for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod"))
	{
		auto path = dir.path();
		if (path.filename().string().compare(0, 5, "char_") == 0)
		{
			charList.push_back(path.filename().string());
		}
	}
}

void resetAttackAnimationData()
{
	isAttackAnimationPlaying = false;
}

void reloadImageData()
{
	imageList.clear();
	if (std::filesystem::exists("CharacterCreatorMod/char_" + curCharData.charName))
	{
		for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod/char_" + curCharData.charName))
		{
			auto path = dir.path();
			if (path.extension().string().compare(".png") == 0)
			{
				imageList.push_back(path.filename().string());
			}
		}
	}
}

void reloadSoundData()
{
	soundList.clear();
	if (std::filesystem::exists("CharacterCreatorMod/char_" + curCharData.charName))
	{
		for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod/char_" + curCharData.charName))
		{
			auto path = dir.path();
			if (path.extension().string().compare(".ogg") == 0)
			{
				soundList.push_back(path.filename().string());
			}
		}
	}
}

void addImageSelector(std::string name, std::string& fileName, ID3D11ShaderResourceView** texture, int& width, int& height,
						bool isAnimation = false, double* curFrame = nullptr, int* numFrames = nullptr, JSONInt* fps = nullptr, bool* isPlaying = nullptr)
{
	// If no texture is loaded and the fileName is not an empty string, try to load the fileName image.
	// If it fails, then set the fileName to an empty string to prevent it from trying to load again
	if ((*texture) == NULL && !fileName.empty())
	{
		bool hasFoundFileName = false;
		for (int i = 0; i < imageList.size(); i++)
		{
			if (fileName.compare(imageList[i]) == 0)
			{
				hasFoundFileName = true;
				if (isAnimation && numFrames != nullptr)
				{
					*numFrames = getSpriteNumFrames(fileName);
				}
				bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + fileName).c_str(), texture, &width, &height);
				IM_ASSERT(ret);
				break;
			}
		}
		if (!hasFoundFileName)
		{
			fileName = "";
		}
	}
	ImGui::Text("%s", name.c_str());
	if (ImGui::BeginCombo(("##" + name + "Combo").c_str(), fileName.c_str()))
	{
		bool hasSelected = false;
		for (int i = 0; i < imageList.size(); i++)
		{
			if (ImGui::Selectable((imageList[i] + "##" + fileName + std::to_string(i)).c_str(), fileName.compare(imageList[i]) == 0))
			{
				fileName = imageList[i];
				hasSelected = true;
			}
		}
		if (hasSelected)
		{
			if ((*texture) != NULL)
			{
				(*texture)->Release();
				(*texture) = NULL;
			}
			if (isAnimation && numFrames != nullptr)
			{
				*numFrames = getSpriteNumFrames(fileName);
			}
			bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + fileName).c_str(), texture, &width, &height);
			IM_ASSERT(ret);
		}

		ImGui::EndCombo();
	}

	if ((*texture) != NULL)
	{
		if (!isAnimation)
		{
			ImGui::Image((ImTextureID)(intptr_t)(*texture), ImVec2(static_cast<float>(width), static_cast<float>(height)), ImVec2(0, 0), ImVec2(1, 1));
		}
		else if (numFrames != nullptr && curFrame != nullptr && isPlaying != nullptr)
		{
			float size = 1.0f / *numFrames;
			float startU = static_cast<int>(*curFrame) * size;
			ImGui::Image((ImTextureID)(intptr_t)(*texture), ImVec2(static_cast<float>(width / *numFrames), static_cast<float>(height)), ImVec2(startU, 0), ImVec2(startU + size, 1));
			if (*isPlaying)
			{
				*curFrame += fps->value / 60.0f;
			}
			if (*curFrame >= *numFrames)
			{
				*curFrame = 0;
			}
			fps->isDefined |= ImGui::InputInt((name + "FPS").c_str(), &fps->value);
			if (ImGui::Button(("Toggle Play/Pause##" + name).c_str()))
			{
				*isPlaying = !*isPlaying;
			}
		}
	}

}

void handleBuffDataWindow()
{
	ImGui::Begin("Buff Data");

	if (ImGui::Button("Add Buff Data"))
	{
		buffData newBuffData;
		newBuffData.buffName = "newBuff";
		curCharData.buffDataList.push_back(newBuffData);
	}

	const char* buffDataComboPreview = "";
	if (curBuffDataIdx >= 0 && curBuffDataIdx < curCharData.buffDataList.size())
	{
		buffDataComboPreview = curCharData.buffDataList[curBuffDataIdx].buffName.c_str();
	}

	if (ImGui::BeginCombo("##BuffDataCombo", buffDataComboPreview))
	{
		for (int i = 0; i < curCharData.buffDataList.size(); i++)
		{
			if (ImGui::Selectable((curCharData.buffDataList[i].buffName + "##" + std::to_string(i)).c_str(), i == curBuffDataIdx))
			{
				curBuffDataIdx = i;
				auto& curBuffData = curCharData.buffDataList[curBuffDataIdx];
				if (buffIconTexture != NULL)
				{
					buffIconTexture->Release();
					buffIconTexture = NULL;
				}
				bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + curBuffData.buffIconFileName).c_str(), &buffIconTexture, &buffIconWidth, &buffIconHeight);
				IM_ASSERT(ret);
			}
		}
		ImGui::EndCombo();
	}

	if (curBuffDataIdx >= 0 && curBuffDataIdx < curCharData.buffDataList.size())
	{
		auto& curBuffData = curCharData.buffDataList[curBuffDataIdx];
		ImGui::InputText("buffName", &curBuffData.buffName);

		curBuffData.data.DRMultiplier.isDefined |= ImGui::InputDouble("DR", &curBuffData.data.DRMultiplier.value);
		curBuffData.data.attackIncrement.isDefined |= ImGui::InputInt("attackIncrement", &curBuffData.data.attackIncrement.value);
		curBuffData.data.critIncrement.isDefined |= ImGui::InputInt("critIncrement", &curBuffData.data.critIncrement.value);
		curBuffData.data.food.isDefined |= ImGui::InputDouble("food", &curBuffData.data.food.value);
		curBuffData.data.hasteIncrement.isDefined |= ImGui::InputInt("hasteIncrement", &curBuffData.data.hasteIncrement.value);
		curBuffData.data.healMultiplier.isDefined |= ImGui::InputDouble("healMultiplier", &curBuffData.data.healMultiplier.value);
		curBuffData.data.maxStacks.isDefined |= ImGui::InputInt("maxStacks", &curBuffData.data.maxStacks.value);
		curBuffData.data.speedIncrement.isDefined |= ImGui::InputInt("speedIncrement", &curBuffData.data.speedIncrement.value);
		curBuffData.data.timer.isDefined |= ImGui::InputInt("timer", &curBuffData.data.timer.value);
		curBuffData.data.weaponSize.isDefined |= ImGui::InputDouble("weaponSize", &curBuffData.data.weaponSize.value);
		curBuffData.data.pickupRange.isDefined |= ImGui::InputDouble("pickupRange", &curBuffData.data.pickupRange.value);
		curBuffData.data.critMod.isDefined |= ImGui::InputDouble("critDamage", &curBuffData.data.critMod.value);
		curBuffData.data.bonusProjectile.isDefined |= ImGui::InputDouble("bonusProjectile", &curBuffData.data.bonusProjectile.value);
		
		addImageSelector("buffIcon", curBuffData.buffIconFileName, &buffIconTexture, buffIconWidth, buffIconHeight,
			false, nullptr, nullptr, nullptr, nullptr);

		if (ImGui::Button("Delete Buff"))
		{
			curCharData.buffDataList.erase(curCharData.buffDataList.begin() + curBuffDataIdx);
			curBuffDataIdx = -1;
			if (buffIconTexture != NULL)
			{
				buffIconTexture->Release();
				buffIconTexture = NULL;
			}
		}
	}

	ImGui::End();
}

void handleIdleAnimationWindow()
{
	ImGui::Begin("Idle Animation");

	ImGui::Checkbox("Use in game idle sprite", &curCharData.isUsingInGameIdleSprite);
	if (curCharData.isUsingInGameIdleSprite)
	{
		if (ImGui::BeginCombo("Idle Sprite Char Name", curCharData.inGameIdleSpriteChar.c_str()))
		{
			for (const auto& [key, value] : characterDataMap)
			{
				const bool is_selected = curCharData.inGameIdleSpriteChar.compare(key);
				if (ImGui::Selectable(key.c_str(), is_selected))
				{
					curCharData.inGameIdleSpriteChar = key;
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		addImageSelector("idleAnimation", curCharData.idleAnimationFileName, &idleAnimationTexture, idleAnimationWidth, idleAnimationHeight,
			true, &idleAnimationCurFrame, &idleAnimationNumFrames, &curCharData.idleAnimationFPS, &isIdleAnimationPlaying);
	}

	ImGui::End();
}

void handleRunAnimationWindow()
{
	ImGui::Begin("Run Animation");

	ImGui::Checkbox("Use in game run sprite", &curCharData.isUsingInGameRunSprite);
	if (curCharData.isUsingInGameRunSprite)
	{
		if (ImGui::BeginCombo("Run Sprite Char Name", curCharData.inGameRunSpriteChar.c_str()))
		{
			for (const auto& [key, value] : characterDataMap)
			{
				const bool is_selected = curCharData.inGameRunSpriteChar.compare(key);
				if (ImGui::Selectable(key.c_str(), is_selected))
				{
					curCharData.inGameRunSpriteChar = key;
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		addImageSelector("runAnimation", curCharData.runAnimationFileName, &runAnimationTexture, runAnimationWidth, runAnimationHeight,
			true, &runAnimationCurFrame, &runAnimationNumFrames, &curCharData.runAnimationFPS, &isRunAnimationPlaying);
	}

	ImGui::End();
}

void handlePortraitWindow()
{
	ImGui::Begin("Portrait");

	ImGui::Checkbox("Use in game portrait sprite", &curCharData.isUsingInGamePortraitSprite);
	if (curCharData.isUsingInGamePortraitSprite)
	{
		if (ImGui::BeginCombo("Portrait Sprite Char Name", curCharData.inGamePortraitSpriteChar.c_str()))
		{
			for (const auto& [key, value] : characterDataMap)
			{
				const bool is_selected = curCharData.inGamePortraitSpriteChar.compare(key);
				if (ImGui::Selectable(key.c_str(), is_selected))
				{
					curCharData.inGamePortraitSpriteChar = key;
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		addImageSelector("portrait", curCharData.portraitFileName, &portraitIconTexture, portraitIconWidth, portraitIconHeight,
			false, nullptr, nullptr, nullptr, nullptr);
	}

	ImGui::End();
}

void handleLargePortraitWindow()
{
	ImGui::Begin("Large Portrait");

	ImGui::Checkbox("Use in game large portrait sprite", &curCharData.isUsingInGameLargePortraitSprite);
	if (curCharData.isUsingInGameLargePortraitSprite)
	{
		if (ImGui::BeginCombo("Large Portrait Char Name", curCharData.inGameLargePortraitSpriteChar.c_str()))
		{
			for (const auto& [key, value] : characterDataMap)
			{
				const bool is_selected = curCharData.inGameLargePortraitSpriteChar.compare(key);
				if (ImGui::Selectable(key.c_str(), is_selected))
				{
					curCharData.inGameLargePortraitSpriteChar = key;
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		addImageSelector("large portrait", curCharData.largePortraitFileName, &largePortraitIconTexture, largePortraitIconWidth, largePortraitIconHeight,
			false, nullptr, nullptr, nullptr, nullptr);
	}

	ImGui::End();
}

void handleSpecialAnimationWindow()
{
	ImGui::Begin("Special Animation");

	// TODO: Add a way to add actions to the special as well

	ImGui::Checkbox("Use in game special", &curCharData.isUsingInGameSpecial);
	if (curCharData.isUsingInGameSpecial)
	{
		if (ImGui::BeginCombo("Special Char Name", curCharData.inGameSpecialChar.c_str()))
		{
			for (const auto& [key, value] : characterDataMap)
			{
				const bool is_selected = curCharData.inGameSpecialChar.compare(key);
				if (ImGui::Selectable(key.c_str(), is_selected))
				{
					curCharData.inGameSpecialChar = key;
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		ImGui::InputText("specialName", &curCharData.specialName);
		ImGui::InputTextMultiline("specialDescription", &curCharData.specialDescription);
		curCharData.specialCooldown.isDefined |= ImGui::InputInt("specialCooldown", &curCharData.specialCooldown.value);
		curCharData.specialDamage.isDefined |= ImGui::InputDouble("specialDamage", &curCharData.specialDamage.value);
		curCharData.specialDuration.isDefined |= ImGui::InputInt("specialDuration", &curCharData.specialDuration.value);

		addImageSelector("specialIcon", curCharData.specialIconFileName, &specialIconTexture, specialIconWidth, specialIconHeight,
			false, nullptr, nullptr, nullptr, nullptr);

		addImageSelector("specialAnimation", curCharData.specialAnimationFileName, &specialAnimationTexture, specialAnimationWidth, specialAnimationHeight,
			true, &specialAnimationCurFrame, &specialAnimationNumFrames, &curCharData.specialAnimationFPS, &isSpecialAnimationPlaying);

		if (ImGui::Button("Toggle Special Editor Window"))
		{
			curCharData.specialAttackNodeEditor.isEditorOpen = !curCharData.specialAttackNodeEditor.isEditorOpen;
			// Save all the node positions when closing the editor
			if (!curCharData.specialAttackNodeEditor.isEditorOpen)
			{
				ax::NodeEditor::SetCurrentEditor(curCharData.specialAttackNodeEditor.editorContext);
				for (auto& nodePair : curCharData.specialAttackNodeEditor.nodeMap)
				{
					auto& node = nodePair.second;
					auto nodePos = ax::NodeEditor::GetNodePosition(node.nodeID);
					node.initPosX = nodePos.x;
					node.initPosY = nodePos.y;
				}
				ax::NodeEditor::SetCurrentEditor(nullptr);
			}
		}

		if (curCharData.specialAttackNodeEditor.isEditorOpen)
		{
			ImGui::Begin(("Special Editor - " + curCharData.specialName + "##").c_str());
			handleNodeEditor(curCharData.specialAttackNodeEditor);
			ImGui::End();
		}
	}

	ImGui::End();
}

void deleteNodeLink(nodeEditor& curNodeEditor, nodeEditorNodeLink& nodeLink)
{
	auto& startPin = curNodeEditor.nodePinMap[nodeLink.startPinID];
	auto& endPin = curNodeEditor.nodePinMap[nodeLink.endPinID];
	for (int i = 0; i < startPin.nodeLinks.size(); i++)
	{
		if (nodeLink.linkID == startPin.nodeLinks[i].linkID)
		{
			startPin.nodeLinks.erase(startPin.nodeLinks.begin() + i);
			break;
		}
	}
	for (int i = 0; i < endPin.nodeLinks.size(); i++)
	{
		if (nodeLink.linkID == endPin.nodeLinks[i].linkID)
		{
			endPin.nodeLinks.erase(endPin.nodeLinks.begin() + i);
			break;
		}
	}
	curNodeEditor.nodeLinkMap.erase(nodeLink.linkID);
}

void handleNodeEditorCreate(nodeEditor& curNodeEditor)
{
	if (ax::NodeEditor::BeginCreate())
	{
		auto showLabel = [](const char* label, ImColor color)
			{
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
				auto size = ImGui::CalcTextSize(label);

				auto padding = ImGui::GetStyle().FramePadding;
				auto spacing = ImGui::GetStyle().ItemSpacing;

				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

				auto rectMin = ImGui::GetCursorScreenPos() - padding;
				auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

				auto drawList = ImGui::GetWindowDrawList();
				drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
				ImGui::TextUnformatted(label);
			};
		ax::NodeEditor::PinId startPinID = 0, endPinID = 0;
		if (ax::NodeEditor::QueryNewLink(&startPinID, &endPinID))
		{
			if (curNodeEditor.nodePinMap.contains(static_cast<int>(startPinID.Get())) && curNodeEditor.nodePinMap.contains(static_cast<int>(endPinID.Get())))
			{
				auto& startPin = curNodeEditor.nodePinMap[static_cast<int>(startPinID.Get())];
				auto& endPin = curNodeEditor.nodePinMap[static_cast<int>(endPinID.Get())];
				if (startPin.pinType == endPin.pinType && startPin.isInput != endPin.isInput)
				{
					bool doesLinkExist = false;
					for (auto& nodeLink : startPin.nodeLinks)
					{
						if ((nodeLink.startPinID == static_cast<int>(startPinID.Get()) && nodeLink.endPinID == static_cast<int>(endPinID.Get())) ||
							(nodeLink.startPinID == static_cast<int>(endPinID.Get()) && nodeLink.endPinID == static_cast<int>(startPinID.Get())))
						{
							doesLinkExist = true;
							break;
						}
					}
					if (doesLinkExist || startPin.parentNodeID == endPin.parentNodeID)
					{
						//						printf("Rejecting link %d %d %d\n", doesLinkExist, startPin.parentNodeID, endPin.parentNodeID);
						ax::NodeEditor::RejectNewItem(ImColor(255, 0, 0), 2.0f);
					}
					else
					{
						showLabel("+ Create Link", ImColor(32, 45, 32, 180));
						if (ax::NodeEditor::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
						{
							auto& inputPin = (startPin.isInput) ? startPin : endPin;
							auto& outputPin = (startPin.isInput) ? endPin : startPin;
							// Break off input pin links
							for (auto& pinLink : inputPin.nodeLinks)
							{
								deleteNodeLink(curNodeEditor, pinLink);
							}
							// Break off the link for the output pin if it's a code flow pin
							if (outputPin.pinType == nodeEditorPinType_CodeFlow)
							{
								for (auto& pinLink : outputPin.nodeLinks)
								{
									deleteNodeLink(curNodeEditor, pinLink);
								}
							}
							auto nodeLink = nodeEditorNodeLink(curNodeEditor.nextID, static_cast<int>(startPinID.Get()), static_cast<int>(endPinID.Get()));
							curNodeEditor.nodeLinkMap[nodeLink.linkID] = nodeLink;
							startPin.nodeLinks.push_back(nodeLink);
							endPin.nodeLinks.push_back(nodeLink);
						}
					}
				}
				else
				{
					ax::NodeEditor::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				}
			}
		}

		ax::NodeEditor::PinId pinId = 0;
		if (ax::NodeEditor::QueryNewNode(&pinId))
		{
			auto nodePinPair = curNodeEditor.nodePinMap.find(static_cast<int>(pinId.Get()));
			if (nodePinPair != curNodeEditor.nodePinMap.end())
			{
				showLabel("+ Create Node", ImColor(32, 45, 32, 180));

				if (ax::NodeEditor::AcceptNewItem())
				{
					auto& nodePin = nodePinPair->second;
					if (nodePin.isInput || !nodePin.isInput && nodePin.pinType == nodeEditorPinType_CodeFlow)
					{
						for (auto& pinLink : nodePin.nodeLinks)
						{
							deleteNodeLink(curNodeEditor, pinLink);
						}
					}
					prevLinkPinId = pinId;
					curNodeEditor.openPopupPosition = ax::NodeEditor::ScreenToCanvas(ImGui::GetMousePos());
					ax::NodeEditor::Suspend();
					ImGui::OpenPopup("Create New Node");
					ax::NodeEditor::Resume();
				}
			}
		}
	}
	ax::NodeEditor::EndCreate();
}

void handleNodeEditorDelete(nodeEditor& curNodeEditor)
{
	if (ax::NodeEditor::BeginDelete())
	{
		ax::NodeEditor::LinkId deletedLinkId;
		while (ax::NodeEditor::QueryDeletedLink(&deletedLinkId))
		{
			if (ax::NodeEditor::AcceptDeletedItem())
			{
				auto& nodeLink = curNodeEditor.nodeLinkMap[static_cast<int>(deletedLinkId.Get())];
				deleteNodeLink(curNodeEditor, nodeLink);
			}
		}

		ax::NodeEditor::NodeId deletedNodeId;
		while (ax::NodeEditor::QueryDeletedNode(&deletedNodeId))
		{
			auto& node = curNodeEditor.nodeMap[static_cast<int>(deletedNodeId.Get())];
			if (!node.canDelete)
			{
				ax::NodeEditor::RejectDeletedItem();
				continue;
			}
			if (ax::NodeEditor::AcceptDeletedItem())
			{
				for (auto& inputPinID : node.inputPinIDArr)
				{
					auto& nodeLinks = curNodeEditor.nodePinMap[inputPinID].nodeLinks;
					for (auto& nodeLink : nodeLinks)
					{
						deleteNodeLink(curNodeEditor, nodeLink);
					}
					curNodeEditor.nodePinMap.erase(inputPinID);
				}
				for (auto& outputPinID : node.outputPinIDArr)
				{
					auto& nodeLinks = curNodeEditor.nodePinMap[outputPinID].nodeLinks;
					for (auto& nodeLink : nodeLinks)
					{
						deleteNodeLink(curNodeEditor, nodeLink);
					}
					curNodeEditor.nodePinMap.erase(outputPinID);
				}
				curNodeEditor.nodeMap.erase(static_cast<int>(deletedNodeId.Get()));
			}
		}
	}
	ax::NodeEditor::EndDelete();
}

void handleNodeEditorDropdown(nodeEditor& curNodeEditor)
{
	for (auto& curNodePair : curNodeEditor.nodeMap)
	{
		auto& curNode = curNodePair.second;
		auto dropDownMenuButton = [](std::string menuName, nodeEditor& curNodeEditor, nodeEditorNodePin& resPin, nodeEditorPinMenuStateEnum& menuState, std::string& dataName, std::vector<std::pair<std::string, nodeEditorPinTypeEnum>> menuItemArr)
			-> bool
			{
				bool hasPinUpdated = false;
				if (menuState == nodeEditorPinMenuState_Clicked)
				{
					ImGui::OpenPopup(menuName.c_str());
					menuState = nodeEditorPinMenuState_Open;
				}
				if (menuState == nodeEditorPinMenuState_Open)
				{
					if (!ImGui::IsPopupOpen(menuName.c_str()))
					{
						menuState = nodeEditorPinMenuState_Closed;
					}
					else if (ImGui::BeginPopup(menuName.c_str()))
					{
						for (auto& menuItemName : menuItemArr)
						{
							if (ImGui::MenuItem(menuItemName.first.c_str()))
							{
								dataName = menuItemName.first;
								menuState = nodeEditorPinMenuState_Closed;
								if (menuItemName.second != nodeEditorPinType_None)
								{
									// Change the output pin type if it's different
									if (resPin.pinType != menuItemName.second)
									{
										resPin.pinType = menuItemName.second;
										// Need to disconnect the previous link
										for (auto& resPinLink : resPin.nodeLinks)
										{
											deleteNodeLink(curNodeEditor, resPinLink);
										}
										hasPinUpdated = true;
									}
								}
							}
						}
						ImGui::EndPopup();
					}
				}
				return hasPinUpdated;
			};

		if (curNode.nodeType == nodeEditorNodeType_ProjectileData)
		{
			std::vector<std::pair<std::string, nodeEditorPinTypeEnum>> projectileDataPairList(curCharData.projectileDataList.size());
			int projectileDataNamePos = 0;
			for (auto& curProjectileData : curCharData.projectileDataList)
			{
				projectileDataPairList[projectileDataNamePos] = std::make_pair(curProjectileData.data.projectileName, nodeEditorPinType_None);
				projectileDataNamePos++;
			}
			auto& curPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			dropDownMenuButton("Projectile Data Menu##" + curPin.pinID, curNodeEditor, curPin, curPin.projectileDataMenuState, curPin.pinProjectileDataName, projectileDataPairList);
		}
		else if (curNode.nodeType == nodeEditorNodeType_GetVariable || curNode.nodeType == nodeEditorNodeType_GetStructVariable)
		{
			// TODO: Probably should add some type checking. Show different variable menu based on the instance type?
			auto& curPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[1]];
			auto& outputPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			bool hasPinChanged = dropDownMenuButton("Variable Data Menu" + curPin.pinID, curNodeEditor, outputPin, curPin.variableDataMenuState, curPin.pinVariableDataName,
				{
					{"direction", nodeEditorPinType_Number},
					{"creator", nodeEditorPinType_Instance},
					{"x", nodeEditorPinType_Number},
					{"y", nodeEditorPinType_Number},
					{"Custom RValue", nodeEditorPinType_RValue},
				});
			if (hasPinChanged)
			{
				auto& rvalueNamePin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[2]];
				if (outputPin.pinType == nodeEditorPinType_RValue)
				{
					rvalueNamePin.pinType = nodeEditorPinType_String;
				}
				else
				{
					rvalueNamePin.pinType = nodeEditorPinType_None;
					// Need to disconnect the previous link
					for (auto& pinLink : rvalueNamePin.nodeLinks)
					{
						deleteNodeLink(curNodeEditor, pinLink);
					}
				}
			}
		}
		else if (curNode.nodeType == nodeEditorNodeType_SetVariable || curNode.nodeType == nodeEditorNodeType_SetStructVariable)
		{
			// TODO: Probably should add some type checking. Show different variable menu based on the instance type?
			auto& curPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[2]];
			auto& inputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[3]];
			bool hasPinChanged = dropDownMenuButton("Variable Data Menu" + curPin.pinID, curNodeEditor, inputPin, curPin.variableDataMenuState, curPin.pinVariableDataName,
				{
					{"direction", nodeEditorPinType_Number},
					{"creator", nodeEditorPinType_Instance},
					{"x", nodeEditorPinType_Number},
					{"y", nodeEditorPinType_Number},
					{"Custom RValue", nodeEditorPinType_RValue},
				});
			if (hasPinChanged)
			{
				auto& rvalueNamePin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[4]];
				if (inputPin.pinType == nodeEditorPinType_RValue)
				{
					rvalueNamePin.pinType = nodeEditorPinType_String;
				}
				else
				{
					rvalueNamePin.pinType = nodeEditorPinType_None;
					// Need to disconnect the previous link
					for (auto& pinLink : rvalueNamePin.nodeLinks)
					{
						deleteNodeLink(curNodeEditor, pinLink);
					}
				}
			}
		}
		else if (curNode.nodeType == nodeEditorNodeType_Boolean)
		{
			auto& curPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			dropDownMenuButton("Boolean Menu" + curPin.pinID, curNodeEditor, curPin, curPin.variableBooleanMenuState, curPin.pinBooleanDataName,
				{
					{"True", nodeEditorPinType_Boolean},
					{"False", nodeEditorPinType_Boolean},
				});
		}
		else if (curNode.nodeType == nodeEditorNodeType_Compare)
		{
			auto& curPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[2]];
			auto& outputPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			dropDownMenuButton("Compare Menu" + curPin.pinID, curNodeEditor, outputPin, curPin.variableCompareMenuState, curPin.pinCompareDataName,
				{
					{"<", nodeEditorPinType_Boolean},
					{"<=", nodeEditorPinType_Boolean},
					{"==", nodeEditorPinType_Boolean},
					{">", nodeEditorPinType_Boolean},
					{">=", nodeEditorPinType_Boolean},
					{"!=", nodeEditorPinType_Boolean},
				});
		}
		else if (curNode.nodeType == nodeEditorNodeType_IntegerCompare)
		{
			auto& curPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[2]];
			auto& outputPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			dropDownMenuButton("Integer Compare Menu" + curPin.pinID, curNodeEditor, outputPin, curPin.variableCompareMenuState, curPin.pinCompareDataName,
				{
					{"<", nodeEditorPinType_Boolean},
					{"<=", nodeEditorPinType_Boolean},
					{"==", nodeEditorPinType_Boolean},
					{">", nodeEditorPinType_Boolean},
					{">=", nodeEditorPinType_Boolean},
					{"!=", nodeEditorPinType_Boolean},
				});
		}
		else if (curNode.nodeType == nodeEditorNodeType_TypeCast)
		{
			auto& variableTypeInputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[0]];
			auto& inputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[1]];

			auto& variableTypeOutputPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			auto& outputPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[1]];
			bool hasPinChanged = dropDownMenuButton("Type Cast Input Menu" + inputPin.pinID, curNodeEditor, inputPin, variableTypeInputPin.variableTypeMenuState, variableTypeInputPin.pinVariableTypeName,
				{
					{"Integer", nodeEditorPinType_Integer},
					{"Boolean", nodeEditorPinType_Boolean},
					{"Number", nodeEditorPinType_Number},
					{"String", nodeEditorPinType_String},
					{"RValue", nodeEditorPinType_RValue},
					{"Instance", nodeEditorPinType_Instance},
					{"Array", nodeEditorPinType_Array},
					{"DS Map", nodeEditorPinType_DSMap},
					{"DS List", nodeEditorPinType_DSList},
				});
			std::vector<std::pair<std::string, nodeEditorPinTypeEnum>> typeCastPairList;
			if (inputPin.pinType == nodeEditorPinType_Integer)
			{
				typeCastPairList = {
					{"String", nodeEditorPinType_String},
					{"Number", nodeEditorPinType_Number},
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_Boolean)
			{
				typeCastPairList = {
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_Number)
			{
				typeCastPairList = {
					{"String", nodeEditorPinType_String},
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_String)
			{
				typeCastPairList = {
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_RValue)
			{
				typeCastPairList = {
					{"Integer", nodeEditorPinType_Integer},
					{"Boolean", nodeEditorPinType_Boolean},
					{"String", nodeEditorPinType_String},
					{"Number", nodeEditorPinType_Number},
					{"Instance", nodeEditorPinType_Instance},
					{"Array", nodeEditorPinType_Array},
					{"DS Map", nodeEditorPinType_DSMap},
					{"DS List", nodeEditorPinType_DSList},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_Instance)
			{
				typeCastPairList = {
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_Array)
			{
				typeCastPairList = {
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_DSMap)
			{
				typeCastPairList = {
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			else if (inputPin.pinType == nodeEditorPinType_DSList)
			{
				typeCastPairList = {
					{"RValue", nodeEditorPinType_RValue},
				};
			}
			if (hasPinChanged)
			{
				outputPin.pinType = nodeEditorPinType_None;
				variableTypeOutputPin.pinVariableTypeName = "";
				for (auto& outputPinLink : outputPin.nodeLinks)
				{
					deleteNodeLink(curNodeEditor, outputPinLink);
				}
			}
			dropDownMenuButton("Type Cast Output Menu" + outputPin.pinID, curNodeEditor, outputPin, variableTypeOutputPin.variableTypeMenuState, variableTypeOutputPin.pinVariableTypeName, typeCastPairList);
		}
		else if (curNode.nodeType == nodeEditorNodeType_BuffData)
		{
			std::vector<std::pair<std::string, nodeEditorPinTypeEnum>> buffDataPairList(curCharData.buffDataList.size());
			int buffDataNamePos = 0;
			for (auto& curBuffData : curCharData.buffDataList)
			{
				buffDataPairList[buffDataNamePos] = std::make_pair(curBuffData.buffName, nodeEditorPinType_None);
				buffDataNamePos++;
			}
			auto& curPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			dropDownMenuButton("Buff Data Menu" + curPin.pinID, curNodeEditor, curPin, curPin.buffDataMenuState, curPin.pinBuffDataName, buffDataPairList);
		}
		else if (curNode.nodeType == nodeEditorNodeType_TernaryOperator)
		{
			auto& variableTypeInputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[0]];

			auto& inputPinOne = curNodeEditor.nodePinMap[curNode.inputPinIDArr[2]];
			auto& inputPinTwo = curNodeEditor.nodePinMap[curNode.inputPinIDArr[3]];
			auto& outputPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			bool hasPinChanged = dropDownMenuButton("Ternary Operator Input Menu" + variableTypeInputPin.pinID, curNodeEditor, outputPin, variableTypeInputPin.variableTypeMenuState, variableTypeInputPin.pinVariableTypeName,
				{
					{"Integer", nodeEditorPinType_Integer},
					{"Number", nodeEditorPinType_Number},
					{"String", nodeEditorPinType_String},
					{"RValue", nodeEditorPinType_RValue},
					{"Instance", nodeEditorPinType_Instance},
					{"DS List", nodeEditorPinType_DSList},
					{"DS Map", nodeEditorPinType_DSMap},
					{"Array", nodeEditorPinType_Array},
				});
			if (hasPinChanged)
			{
				inputPinOne.pinType = outputPin.pinType;
				inputPinTwo.pinType = outputPin.pinType;
				for (auto& pinLink : inputPinOne.nodeLinks)
				{
					deleteNodeLink(curNodeEditor, pinLink);
				}
				for (auto& pinLink : inputPinTwo.nodeLinks)
				{
					deleteNodeLink(curNodeEditor, pinLink);
				}
			}
		}
		else if (curNode.nodeType == nodeEditorNodeType_CacheVariable)
		{
			auto& variableTypeInputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[0]];

			auto& inputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[1]];
			auto& outputPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			bool hasPinChanged = dropDownMenuButton("Cache Variable Menu" + variableTypeInputPin.pinID, curNodeEditor, outputPin, variableTypeInputPin.variableTypeMenuState, variableTypeInputPin.pinVariableTypeName,
				{
					{"Integer", nodeEditorPinType_Integer},
					{"Number", nodeEditorPinType_Number},
					{"String", nodeEditorPinType_String},
					{"RValue", nodeEditorPinType_RValue},
					{"Instance", nodeEditorPinType_Instance},
					{"DS List", nodeEditorPinType_DSList},
					{"DS Map", nodeEditorPinType_DSMap},
					{"Array", nodeEditorPinType_Array},
				});
			if (hasPinChanged)
			{
				inputPin.pinType = outputPin.pinType;
				for (auto& pinLink : inputPin.nodeLinks)
				{
					deleteNodeLink(curNodeEditor, pinLink);
				}
			}
		}
		else if (curNode.nodeType == nodeEditorNodeType_SoundData)
		{
			std::vector<std::pair<std::string, nodeEditorPinTypeEnum>> soundDataPairList(curCharData.soundDataList.size());
			int soundDataNamePos = 0;
			for (auto& curSoundData : curCharData.soundDataList)
			{
				soundDataPairList[soundDataNamePos] = std::make_pair(curSoundData.soundName, nodeEditorPinType_None);
				soundDataNamePos++;
			}
			auto& curPin = curNodeEditor.nodePinMap[curNode.outputPinIDArr[0]];
			dropDownMenuButton("Sound Data Menu##" + curPin.pinID, curNodeEditor, curPin, curPin.soundDataMenuState, curPin.pinSoundDataName, soundDataPairList);
		}
		else if (curNode.nodeType == nodeEditorNodeType_ArrayPush)
		{
			auto& variableTypeInputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[2]];

			auto& inputPin = curNodeEditor.nodePinMap[curNode.inputPinIDArr[3]];
			bool hasPinChanged = dropDownMenuButton("Array Push Variable Menu" + variableTypeInputPin.pinID, curNodeEditor, inputPin, variableTypeInputPin.variableTypeMenuState, variableTypeInputPin.pinVariableTypeName,
				{
					{"Integer", nodeEditorPinType_Integer},
					{"Number", nodeEditorPinType_Number},
					{"String", nodeEditorPinType_String},
					{"RValue", nodeEditorPinType_RValue},
					{"Instance", nodeEditorPinType_Instance},
				});
		}
	}
}

void handleNodeEditor(nodeEditor& curNodeEditor)
{
	// Add node editor stuff here
	ax::NodeEditor::SetCurrentEditor(curNodeEditor.editorContext);
	ax::NodeEditor::Begin("My Editor");
	// TODO: Not sure if I like this. Probably should try to fix the issue where a default pin is created and causes an invalid node to be created
	std::erase_if(curNodeEditor.nodeMap, [](std::pair<const int, nodeEditorNode> curNodePair) { return curNodePair.second.nodeID == -1; });
	for (auto& curActionNode : curNodeEditor.nodeMap)
	{
		curActionNode.second.drawNode();
	}

	for (auto& nodeLinkPair : curNodeEditor.nodeLinkMap)
	{
		auto& nodeLink = nodeLinkPair.second;
		ax::NodeEditor::Link(nodeLink.linkID, nodeLink.startPinID, nodeLink.endPinID);
	}

	handleNodeEditorCreate(curNodeEditor);

	handleNodeEditorDelete(curNodeEditor);

	ax::NodeEditor::Suspend();

	if (ax::NodeEditor::IsBackgroundDoubleClicked())
	{
		ImGui::OpenPopup("Create New Node");
		curNodeEditor.openPopupPosition = ax::NodeEditor::ScreenToCanvas(ImGui::GetMousePos());
		prevLinkPinId = 0;
	}

	// ImGui::SetNextWindowSizeConstraints ? Might be able to limit the size
	if (ImGui::BeginPopup("Create New Node"))
	{
		auto linkPrevPin = [](nodeEditor& curNodeEditor, nodeEditorNode& curNode)
		{
			if (prevLinkPinId.Get() != 0)
			{
				auto& prevLinkPin = curNodeEditor.nodePinMap[static_cast<int>(prevLinkPinId.Get())];
				auto& curNodePinIDArr = prevLinkPin.isInput ? curNode.outputPinIDArr : curNode.inputPinIDArr;
				for (auto& curPinId : curNodePinIDArr)
				{
					auto& curPin = curNodeEditor.nodePinMap[curPinId];
					if (curPin.pinType == prevLinkPin.pinType && curPin.isInput != prevLinkPin.isInput)
					{
						auto nodeLink = nodeEditorNodeLink(curNodeEditor.nextID, curPinId, prevLinkPin.pinID);
						curNodeEditor.nodeLinkMap[nodeLink.linkID] = nodeLink;
						curPin.nodeLinks.push_back(nodeLink);
						prevLinkPin.nodeLinks.push_back(nodeLink);
						break;
					}
				}
				prevLinkPinId = 0;
			}
		};
		if (ImGui::MenuItem("Center to Nodes"))
		{
			ax::NodeEditor::NavigateToContent();
		}
		if (ImGui::MenuItem("Spawn Projectile"))
		{
			nodeEditorNode curNode = nodeEditorSpawnProjectileNode(&curNodeEditor, curNodeEditor.nextID, "Spawn Projectile");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Number"))
		{
			nodeEditorNode curNode = nodeEditorNumberNode(&curNodeEditor, curNodeEditor.nextID, "Number");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Integer"))
		{
			nodeEditorNode curNode = nodeEditorIntegerNode(&curNodeEditor, curNodeEditor.nextID, "Integer");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Projectile Data"))
		{
			nodeEditorNode curNode = nodeEditorProjectileDataNode(&curNodeEditor, curNodeEditor.nextID, "Projectile Data");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("This Instance"))
		{
			nodeEditorNode curNode = nodeEditorThisInstanceNode(&curNodeEditor, curNodeEditor.nextID, "This Instance");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Global Instance"))
		{
			nodeEditorNode curNode = nodeEditorGlobalInstanceNode(&curNodeEditor, curNodeEditor.nextID, "Global Instance");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Player Manager Instance"))
		{
			nodeEditorNode curNode = nodeEditorPlayerManagerNode(&curNodeEditor, curNodeEditor.nextID, "Player Manager Instance");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Player Instance"))
		{
			nodeEditorNode curNode = nodeEditorPlayerNode(&curNodeEditor, curNodeEditor.nextID, "Player Instance");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Get Variable"))
		{
			nodeEditorNode curNode = nodeEditorGetVariableNode(&curNodeEditor, curNodeEditor.nextID, "Get Variable");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Set Variable"))
		{
			nodeEditorNode curNode = nodeEditorSetVariableNode(&curNodeEditor, curNodeEditor.nextID, "Set Variable");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Get Struct Variable"))
		{
			nodeEditorNode curNode = nodeEditorGetStructVariableNode(&curNodeEditor, curNodeEditor.nextID, "Get Struct Variable");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Set Struct Variable"))
		{
			nodeEditorNode curNode = nodeEditorSetStructVariableNode(&curNodeEditor, curNodeEditor.nextID, "Set Struct Variable");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::BeginMenu("Math"))
		{
			if (ImGui::MenuItem("Add"))
			{
				nodeEditorNode curNode = nodeEditorAddNode(&curNodeEditor, curNodeEditor.nextID, "Add");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Subtract"))
			{
				nodeEditorNode curNode = nodeEditorSubtractNode(&curNodeEditor, curNodeEditor.nextID, "Subtract");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Multiply"))
			{
				nodeEditorNode curNode = nodeEditorMultiplyNode(&curNodeEditor, curNodeEditor.nextID, "Multiply");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Divide"))
			{
				nodeEditorNode curNode = nodeEditorDivideNode(&curNodeEditor, curNodeEditor.nextID, "Divide");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Ceiling"))
			{
				nodeEditorNode curNode = nodeEditorIntegerCeilingNode(&curNodeEditor, curNodeEditor.nextID, "Integer Ceiling");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Floor"))
			{
				nodeEditorNode curNode = nodeEditorIntegerCeilingNode(&curNodeEditor, curNodeEditor.nextID, "Integer Floor");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Round"))
			{
				nodeEditorNode curNode = nodeEditorIntegerRoundNode(&curNodeEditor, curNodeEditor.nextID, "Integer Round");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Add"))
			{
				nodeEditorNode curNode = nodeEditorIntegerAddNode(&curNodeEditor, curNodeEditor.nextID, "Integer Add");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Subtract"))
			{
				nodeEditorNode curNode = nodeEditorIntegerSubtractNode(&curNodeEditor, curNodeEditor.nextID, "Integer Subtract");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Multiply"))
			{
				nodeEditorNode curNode = nodeEditorIntegerMultiplyNode(&curNodeEditor, curNodeEditor.nextID, "Integer Multiply");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Divide"))
			{
				nodeEditorNode curNode = nodeEditorIntegerDivideNode(&curNodeEditor, curNodeEditor.nextID, "Integer Divide");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Random"))
			{
				nodeEditorNode curNode = nodeEditorRandomNode(&curNodeEditor, curNodeEditor.nextID, "Random");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Random"))
			{
				nodeEditorNode curNode = nodeEditorIntegerRandomNode(&curNodeEditor, curNodeEditor.nextID, "Integer Random");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Modulus"))
			{
				nodeEditorNode curNode = nodeEditorModulusNode(&curNodeEditor, curNodeEditor.nextID, "Modulus");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Square Root"))
			{
				nodeEditorNode curNode = nodeEditorSquareRootNode(&curNodeEditor, curNodeEditor.nextID, "Square Root");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Logarithm"))
			{
				nodeEditorNode curNode = nodeEditorLogarithmNode(&curNodeEditor, curNodeEditor.nextID, "Logarithm");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Sine"))
			{
				nodeEditorNode curNode = nodeEditorSineNode(&curNodeEditor, curNodeEditor.nextID, "Sine");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Cosine"))
			{
				nodeEditorNode curNode = nodeEditorCosineNode(&curNodeEditor, curNodeEditor.nextID, "Cosine");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Tangent"))
			{
				nodeEditorNode curNode = nodeEditorTangentNode(&curNodeEditor, curNodeEditor.nextID, "Tangent");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Point Direction"))
			{
				nodeEditorNode curNode = nodeEditorPointDirectionNode(&curNodeEditor, curNodeEditor.nextID, "Point Direction");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Delay"))
		{
			nodeEditorNode curNode = nodeEditorDelayNode(&curNodeEditor, curNodeEditor.nextID, "Delay");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::BeginMenu("Boolean Functions"))
		{
			if (ImGui::MenuItem("Boolean"))
			{
				nodeEditorNode curNode = nodeEditorBooleanNode(&curNodeEditor, curNodeEditor.nextID, "Boolean");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("If"))
			{
				nodeEditorNode curNode = nodeEditorIfNode(&curNodeEditor, curNodeEditor.nextID, "If");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Compare"))
			{
				nodeEditorNode curNode = nodeEditorCompareNode(&curNodeEditor, curNodeEditor.nextID, "Compare");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Integer Compare"))
			{
				nodeEditorNode curNode = nodeEditorIntegerCompareNode(&curNodeEditor, curNodeEditor.nextID, "Integer Compare");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("While"))
			{
				nodeEditorNode curNode = nodeEditorWhileNode(&curNodeEditor, curNodeEditor.nextID, "While");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("And"))
			{
				nodeEditorNode curNode = nodeEditorAndNode(&curNodeEditor, curNodeEditor.nextID, "And");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Or"))
			{
				nodeEditorNode curNode = nodeEditorOrNode(&curNodeEditor, curNodeEditor.nextID, "Or");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Not"))
			{
				nodeEditorNode curNode = nodeEditorNotNode(&curNodeEditor, curNodeEditor.nextID, "Not");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Is RValue Defined"))
			{
				nodeEditorNode curNode = nodeEditorIsRValueDefinedNode(&curNodeEditor, curNodeEditor.nextID, "Is RValue Defined");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Ternary Operator"))
			{
				nodeEditorNode curNode = nodeEditorTernaryOperatorNode(&curNodeEditor, curNodeEditor.nextID, "Ternary Operator");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Merge Code Flow"))
		{
			nodeEditorNode curNode = nodeEditorMergeCodeFlowNode(&curNodeEditor, curNodeEditor.nextID, "Merge Code Flow");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("String"))
		{
			nodeEditorNode curNode = nodeEditorStringNode(&curNodeEditor, curNodeEditor.nextID, "String");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Print"))
		{
			nodeEditorNode curNode = nodeEditorPrintNode(&curNodeEditor, curNodeEditor.nextID, "Print");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Type Cast"))
		{
			nodeEditorNode curNode = nodeEditorTypeCastNode(&curNodeEditor, curNodeEditor.nextID, "Type Cast");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Append String"))
		{
			nodeEditorNode curNode = nodeEditorAppendStringNode(&curNodeEditor, curNodeEditor.nextID, "Append String");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Asset Get Index"))
		{
			nodeEditorNode curNode = nodeEditorAssetGetIndexNode(&curNodeEditor, curNodeEditor.nextID, "Asset Get Index");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Apply Buff"))
		{
			nodeEditorNode curNode = nodeEditorApplyBuffNode(&curNodeEditor, curNodeEditor.nextID, "Apply Buff");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Buff Data"))
		{
			nodeEditorNode curNode = nodeEditorBuffDataNode(&curNodeEditor, curNodeEditor.nextID, "Buff Data");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Collision Circle List"))
		{
			nodeEditorNode curNode = nodeEditorCollisionCircleListNode(&curNodeEditor, curNodeEditor.nextID, "Collision Circle List");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::BeginMenu("List Functions"))
		{
			if (ImGui::MenuItem("DS List Create"))
			{
				nodeEditorNode curNode = nodeEditorDSListCreateNode(&curNodeEditor, curNodeEditor.nextID, "DS List Create");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("DS List Get"))
			{
				nodeEditorNode curNode = nodeEditorDSListGetNode(&curNodeEditor, curNodeEditor.nextID, "DS List Get");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("DS List Size"))
			{
				nodeEditorNode curNode = nodeEditorDSListSizeNode(&curNodeEditor, curNodeEditor.nextID, "DS List Size");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("DS List Clear"))
			{
				nodeEditorNode curNode = nodeEditorDSListClearNode(&curNodeEditor, curNodeEditor.nextID, "DS List Clear");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("DS List Destroy"))
			{
				nodeEditorNode curNode = nodeEditorDSListDestroyNode(&curNodeEditor, curNodeEditor.nextID, "DS List Destroy");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("DS Map Get"))
		{
			nodeEditorNode curNode = nodeEditorDSMapGetNode(&curNodeEditor, curNodeEditor.nextID, "DS Map Get");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("DS Map Keys To Array"))
		{
			nodeEditorNode curNode = nodeEditorDSMapKeysToArrayNode(&curNodeEditor, curNodeEditor.nextID, "DS Map Keys To Array");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::BeginMenu("Array Functions"))
		{
			if (ImGui::MenuItem("Array Create"))
			{
				nodeEditorNode curNode = nodeEditorArrayCreateNode(&curNodeEditor, curNodeEditor.nextID, "Array Create");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Array Get"))
			{
				nodeEditorNode curNode = nodeEditorArrayGetNode(&curNodeEditor, curNodeEditor.nextID, "Array Get");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Array Length"))
			{
				nodeEditorNode curNode = nodeEditorArrayLengthNode(&curNodeEditor, curNodeEditor.nextID, "Array Length");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Array Push"))
			{
				nodeEditorNode curNode = nodeEditorArrayPushNode(&curNodeEditor, curNodeEditor.nextID, "Array Push");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Set Debug Level"))
		{
			nodeEditorNode curNode = nodeEditorSetDebugLevelNode(&curNodeEditor, curNodeEditor.nextID, "Set Debug Level");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Sound Data"))
		{
			nodeEditorNode curNode = nodeEditorSoundDataNode(&curNodeEditor, curNodeEditor.nextID, "Sound Data");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::MenuItem("Play Sound"))
		{
			nodeEditorNode curNode = nodeEditorPlaySoundNode(&curNodeEditor, curNodeEditor.nextID, "Play Sound");
			curNodeEditor.nodeMap[curNode.nodeID] = curNode;
			ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
			linkPrevPin(curNodeEditor, curNode);
		}
		if (ImGui::BeginMenu("Caching"))
		{
			if (ImGui::MenuItem("Cache Variable"))
			{
				nodeEditorNode curNode = nodeEditorCacheVariableNode(&curNodeEditor, curNodeEditor.nextID, "Cache Variable");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Flush Cache"))
			{
				nodeEditorNode curNode = nodeEditorFlushCacheNode(&curNodeEditor, curNodeEditor.nextID, "Flush Cache");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			if (ImGui::MenuItem("Merge Flush"))
			{
				nodeEditorNode curNode = nodeEditorMergeFlushNode(&curNodeEditor, curNodeEditor.nextID, "Merge Flush");
				curNodeEditor.nodeMap[curNode.nodeID] = curNode;
				ax::NodeEditor::SetNodePosition(curNode.nodeID, curNodeEditor.openPopupPosition);
				linkPrevPin(curNodeEditor, curNode);
			}
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
	handleNodeEditorDropdown(curNodeEditor);
	ax::NodeEditor::Resume();

	ax::NodeEditor::End();
	if (!curNodeEditor.initEditorPos)
	{
		ax::NodeEditor::NavigateToContent();
		curNodeEditor.initEditorPos = true;
	}
	ax::NodeEditor::SetCurrentEditor(nullptr);
}

void handleWeaponLevelsWindow()
{
	ImGui::Begin("Weapon Levels");

	ImGui::Checkbox("Use in game main weapon", &curCharData.isUsingInGameMainWeapon);

	if (curCharData.isUsingInGameMainWeapon)
	{
		// TODO: Probably want to make it so that actions can still be attached to the main weapon attack
		if (ImGui::BeginCombo("Main Weapon Name", curCharData.inGameMainWeaponChar.c_str()))
		{
			for (const auto& [key, value] : characterDataMap)
			{
				const bool is_selected = curCharData.inGameMainWeaponChar.compare(key);
				if (ImGui::Selectable(key.c_str(), is_selected))
				{
					curCharData.inGameMainWeaponChar = key;
				}
			}
			ImGui::EndCombo();
		}
	}
	else
	{
		ImGui::InputText("attackName", &curCharData.attackName);

		addImageSelector("attackAnimation", curCharData.attackAnimationFileName, &attackAnimationTexture, attackAnimationWidth, attackAnimationHeight,
			true, &attackAnimationCurFrame, &attackAnimationNumFrames, &curCharData.attackAnimationFPS, &isAttackAnimationPlaying);

		addImageSelector("attackIcon", curCharData.attackIconFileName, &attackIconTexture, attackIconWidth, attackIconHeight,
			false, nullptr, nullptr, nullptr, nullptr);

		addImageSelector("attackAwakenedIcon", curCharData.attackAwakenedIconFileName, &attackAwakenedIconTexture, attackAwakenedIconWidth, attackAwakenedIconHeight,
			false, nullptr, nullptr, nullptr, nullptr);

		ImGui::Checkbox("collides", &curCharData.attackCollides);

		for (int i = 0; i < curCharData.weaponLevelDataList.size(); i++)
		{
			std::string strLevel = "Weapon Level " + std::to_string(i + 1);
			if (ImGui::TreeNode(strLevel.c_str()))
			{
				auto& curWeaponLevelData = curCharData.weaponLevelDataList[i];
				// TODO: Probably need a disable option to not input anything?
				curWeaponLevelData.attackCount.isDefined |= ImGui::InputInt(("attackCount##" + strLevel).c_str(), &curWeaponLevelData.attackCount.value);
				curWeaponLevelData.attackDelay.isDefined |= ImGui::InputInt(("attackDelay##" + strLevel).c_str(), &curWeaponLevelData.attackDelay.value);
				ImGui::InputText(("attackDescription##" + strLevel).c_str(), &curWeaponLevelData.attackDescription);
				curWeaponLevelData.attackTime.isDefined |= ImGui::InputInt(("attackTime##" + strLevel).c_str(), &curWeaponLevelData.attackTime.value);
				curWeaponLevelData.damage.isDefined |= ImGui::InputDouble(("damage##" + strLevel).c_str(), &curWeaponLevelData.damage.value);
				curWeaponLevelData.duration.isDefined |= ImGui::InputInt(("duration##" + strLevel).c_str(), &curWeaponLevelData.duration.value);
				curWeaponLevelData.hitCD.isDefined |= ImGui::InputInt(("hitCD##" + strLevel).c_str(), &curWeaponLevelData.hitCD.value);
				curWeaponLevelData.hitLimit.isDefined |= ImGui::InputInt(("hitLimit##" + strLevel).c_str(), &curWeaponLevelData.hitLimit.value);
				curWeaponLevelData.range.isDefined |= ImGui::InputInt(("range##" + strLevel).c_str(), &curWeaponLevelData.range.value);
				curWeaponLevelData.speed.isDefined |= ImGui::InputDouble(("speed##" + strLevel).c_str(), &curWeaponLevelData.speed.value);

				ImGui::TreePop();
			}
		}

		if (ImGui::Button("Toggle Weapon Editor Window"))
		{
			curCharData.mainWeaponNodeEditor.isEditorOpen = !curCharData.mainWeaponNodeEditor.isEditorOpen;
			// Save all the node positions when closing the editor
			if (!curCharData.mainWeaponNodeEditor.isEditorOpen)
			{
				ax::NodeEditor::SetCurrentEditor(curCharData.mainWeaponNodeEditor.editorContext);
				for (auto& nodePair : curCharData.mainWeaponNodeEditor.nodeMap)
				{
					auto& node = nodePair.second;
					auto nodePos = ax::NodeEditor::GetNodePosition(node.nodeID);
					node.initPosX = nodePos.x;
					node.initPosY = nodePos.y;
				}
				ax::NodeEditor::SetCurrentEditor(nullptr);
			}
		}

		if (curCharData.mainWeaponNodeEditor.isEditorOpen)
		{
			ImGui::Begin(("Weapon Editor - " + curCharData.attackName + "##").c_str());
			handleNodeEditor(curCharData.mainWeaponNodeEditor);
			ImGui::End();
		}
	}

	ImGui::End();
}

void handleSkillDataWindow()
{
	ImGui::Begin("Skill Data");
	
	for (int i = 0; i < curCharData.skillDataList.size(); i++)
	{
		std::string strLevel = "Skill " + std::to_string(i + 1);
		if (ImGui::TreeNode(strLevel.c_str()))
		{
			auto& curSkillData = curCharData.skillDataList[i];

			ImGui::Checkbox("Use in game skill", &curSkillData.data.isUsingInGameSkill);
			if (curSkillData.data.isUsingInGameSkill)
			{
				if (ImGui::BeginCombo("Skill Name", curSkillData.data.inGameSkillName.c_str()))
				{
					for (const auto& [key, value] : characterDataMap)
					{
						for (int j = 0; j < 3; j++)
						{
							const bool is_selected = curSkillData.data.inGameSkillName.compare(value.perksStringArr[j]);
							if (ImGui::Selectable(value.perksStringArr[j].c_str(), is_selected))
							{
								curSkillData.data.inGameSkillName = value.perksStringArr[j];
							}
						}
					}
					ImGui::EndCombo();
				}
			}
			else
			{
				// TODO: Replace all of this with a node editor
				ImGui::InputText(("skillName##" + strLevel).c_str(), &curSkillData.data.skillName);
				for (int j = 0; j < curSkillData.data.skillLevelDataList.size(); j++)
				{
					if (ImGui::TreeNode(("Level " + std::to_string(j + 1) + "##" + strLevel).c_str()))
					{
						auto& curSkillLevel = curSkillData.data.skillLevelDataList[j];
						std::string skillLevel = "##" + strLevel + " Level " + std::to_string(j + 1);
						curSkillLevel.DRMultiplier.isDefined |= ImGui::InputDouble(("DRMMultiplier" + skillLevel).c_str(), &curSkillLevel.DRMultiplier.value);
						curSkillLevel.healMultiplier.isDefined |= ImGui::InputDouble(("healMultiplier" + skillLevel).c_str(), &curSkillLevel.healMultiplier.value);
						curSkillLevel.attackIncrement.isDefined |= ImGui::InputInt(("attackIncrement" + skillLevel).c_str(), &curSkillLevel.attackIncrement.value);
						curSkillLevel.critIncrement.isDefined |= ImGui::InputInt(("critIncrement" + skillLevel).c_str(), &curSkillLevel.critIncrement.value);
						curSkillLevel.hasteIncrement.isDefined |= ImGui::InputInt(("hasteIncrement" + skillLevel).c_str(), &curSkillLevel.hasteIncrement.value);
						curSkillLevel.speedIncrement.isDefined |= ImGui::InputInt(("speedIncrement" + skillLevel).c_str(), &curSkillLevel.speedIncrement.value);
						curSkillLevel.food.isDefined |= ImGui::InputDouble(("food" + skillLevel).c_str(), &curSkillLevel.food.value);
						curSkillLevel.weaponSize.isDefined |= ImGui::InputDouble(("weaponSize" + skillLevel).c_str(), &curSkillLevel.weaponSize.value);
						curSkillLevel.pickupRange.isDefined |= ImGui::InputDouble(("pickupRange" + skillLevel).c_str(), &curSkillLevel.pickupRange.value);
						curSkillLevel.critMod.isDefined |= ImGui::InputDouble(("critDamage" + skillLevel).c_str(), &curSkillLevel.critMod.value);
						curSkillLevel.bonusProjectile.isDefined |= ImGui::InputDouble(("bonusProjectile" + skillLevel).c_str(), &curSkillLevel.bonusProjectile.value);
						
						ImGui::InputTextMultiline(("skillDescription" + skillLevel).c_str(), &curSkillLevel.skillDescription);
						ImGui::TreePop();
					}
				}

				auto& curSkillIconTextureData = curSkillData.data.skillIconTextureData;
				addImageSelector("skillIcon", curSkillData.data.skillIconFileName, &curSkillIconTextureData.texture, curSkillIconTextureData.width, curSkillIconTextureData.height,
					false, nullptr, nullptr, nullptr, nullptr);

				if (ImGui::Button("Toggle Skill Editor Window"))
				{
					curSkillData.nodeEditor.isEditorOpen = !curSkillData.nodeEditor.isEditorOpen;
					// Save all the node positions when closing the editor
					if (!curSkillData.nodeEditor.isEditorOpen)
					{
						ax::NodeEditor::SetCurrentEditor(curSkillData.nodeEditor.editorContext);
						for (auto& nodePair : curSkillData.nodeEditor.nodeMap)
						{
							auto& node = nodePair.second;
							auto nodePos = ax::NodeEditor::GetNodePosition(node.nodeID);
							node.initPosX = nodePos.x;
							node.initPosY = nodePos.y;
						}
						ax::NodeEditor::SetCurrentEditor(nullptr);
					}
				}

				if (curSkillData.nodeEditor.isEditorOpen)
				{
					ImGui::Begin(("Skill Editor - " + curSkillData.data.skillName + "##" + std::to_string(i)).c_str());
					handleNodeEditor(curSkillData.nodeEditor);
					ImGui::End();
				}
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void handleProjectileWindow()
{
	ImGui::Begin("Projectile Data");

	if (ImGui::Button("Add projectile"))
	{
		projectileDataWrapper newProjectileData;
		newProjectileData.data.projectileName = "newProjectile";
		curCharData.projectileDataList.push_back(newProjectileData);
	}

	for (int i = 0; i < curCharData.projectileDataList.size(); i++)
	{
		std::string strAppendProjectileNumber = "##projectile" + std::to_string(i);
		if (ImGui::TreeNode((void*)(intptr_t)i, curCharData.projectileDataList[i].data.projectileName.c_str()))
		{
			auto& curProjectileData = curCharData.projectileDataList[i];
			ImGui::InputText(("projectileName" + strAppendProjectileNumber).c_str(), &curProjectileData.data.projectileName);
			curProjectileData.data.projectileDamage.isDefined |= ImGui::InputDouble(("projectileDamage" + strAppendProjectileNumber).c_str(), &curProjectileData.data.projectileDamage.value);
			curProjectileData.data.projectileDuration.isDefined |= ImGui::InputInt(("projectileDuration" + strAppendProjectileNumber).c_str(), &curProjectileData.data.projectileDuration.value);
			curProjectileData.data.projectileHitCD.isDefined |= ImGui::InputInt(("projectileHitCD" + strAppendProjectileNumber).c_str(), &curProjectileData.data.projectileHitCD.value);
			curProjectileData.data.projectileHitLimit.isDefined |= ImGui::InputInt(("projectileHitLimit" + strAppendProjectileNumber).c_str(), &curProjectileData.data.projectileHitLimit.value);
			curProjectileData.data.projectileHitRange.isDefined |= ImGui::InputInt(("projectileHitRange" + strAppendProjectileNumber).c_str(), &curProjectileData.data.projectileHitRange.value);
			curProjectileData.data.projectileSpeed.isDefined |= ImGui::InputDouble(("projectileSpeed" + strAppendProjectileNumber).c_str(), &curProjectileData.data.projectileSpeed.value);
			ImGui::Checkbox("collides", &curProjectileData.data.collides);
			ImGui::Checkbox("isMain", &curProjectileData.data.isMain);

			if (ImGui::Button("Toggle Projectile Editor Window"))
			{
				curProjectileData.showProjectileEditor = !curProjectileData.showProjectileEditor;
				curProjectileData.nodeEditor.isEditorOpen = curProjectileData.showProjectileEditor;
				// Save all the node positions when closing the editor
				if (!curProjectileData.nodeEditor.isEditorOpen)
				{
					ax::NodeEditor::SetCurrentEditor(curProjectileData.nodeEditor.editorContext);
					for (auto& nodePair : curProjectileData.nodeEditor.nodeMap)
					{
						auto& node = nodePair.second;
						auto nodePos = ax::NodeEditor::GetNodePosition(node.nodeID);
						node.initPosX = nodePos.x;
						node.initPosY = nodePos.y;
					}
					ax::NodeEditor::SetCurrentEditor(nullptr);
				}
			}

			if (curProjectileData.showProjectileEditor)
			{
				ImGui::Begin(("Action Editor - " + curProjectileData.data.projectileName + "##" + std::to_string(i)).c_str());
				handleNodeEditor(curProjectileData.nodeEditor);
				ImGui::End();
			}

			auto& curProjectileAnimationTextureData = curProjectileData.projectileAnimationTextureData;

			addImageSelector("projectileAnimation", curProjectileData.data.projectileAnimationFileName, &curProjectileAnimationTextureData.texture, curProjectileAnimationTextureData.width, curProjectileAnimationTextureData.height,
				true, &curProjectileAnimationTextureData.curFrame, &curProjectileAnimationTextureData.numFrames, &curProjectileData.data.projectileAnimationFPS, &curProjectileAnimationTextureData.isAnimationPlaying);
			if (ImGui::Button(("Delete projectile" + strAppendProjectileNumber).c_str()))
			{
				if (curProjectileAnimationTextureData.texture != NULL)
				{
					curProjectileAnimationTextureData.texture->Release();
				}
				curCharData.projectileDataList.erase(curCharData.projectileDataList.begin() + i);
				i--;
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void handleSoundDataWindow()
{
	ImGui::Begin("Sound Data");

	if (ImGui::Button("Add Sound Data"))
	{
		soundData newSoundData;
		newSoundData.soundName = "newSound";
		curCharData.soundDataList.push_back(newSoundData);
	}

	const char* soundDataComboPreview = "";
	if (curSoundDataIdx >= 0 && curSoundDataIdx < curCharData.soundDataList.size())
	{
		soundDataComboPreview = curCharData.soundDataList[curSoundDataIdx].soundName.c_str();
	}

	if (ImGui::BeginCombo("##SoundDataCombo", soundDataComboPreview))
	{
		for (int i = 0; i < curCharData.soundDataList.size(); i++)
		{
			if (ImGui::Selectable((curCharData.soundDataList[i].soundName + "##" + std::to_string(i)).c_str(), i == curSoundDataIdx))
			{
				curSoundDataIdx = i;
				auto& curSoundData = curCharData.soundDataList[curSoundDataIdx];
			}
		}
		ImGui::EndCombo();
	}

	if (curSoundDataIdx >= 0 && curSoundDataIdx < curCharData.soundDataList.size())
	{
		auto& curSoundData = curCharData.soundDataList[curSoundDataIdx];
		ImGui::InputText("soundName", &curSoundData.soundName);

		if (ImGui::BeginCombo(("soundFile##" + curSoundData.soundName + "Combo").c_str(), curSoundData.soundRValue.soundFile.c_str()))
		{
			bool hasSelected = false;
			for (int i = 0; i < soundList.size(); i++)
			{
				if (ImGui::Selectable((soundList[i] + "##" + curSoundData.soundRValue.soundFile + std::to_string(i)).c_str(), curSoundData.soundRValue.soundFile.compare(imageList[i]) == 0))
				{
					curSoundData.soundRValue.soundFile = soundList[i];
					hasSelected = true;
				}
			}

			ImGui::EndCombo();
		}


		if (ImGui::Button("Delete Sound"))
		{
			curCharData.soundDataList.erase(curCharData.soundDataList.begin() + curSoundDataIdx);
			curSoundDataIdx = -1;
		}
	}

	ImGui::End();
}

void handleImGUI()
{
	ImGui::Begin("Character List");

	if (ImGui::Button("Reload Character List"))
	{
		if (!std::filesystem::exists("CharacterCreatorMod"))
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the CharacterCreatorMod directory");
			DbgPrintEx(LOG_SEVERITY_ERROR, "Couldn't find the CharacterCreatorMod directory");
		}
		else
		{
			resetCharData();
		}
	}

	// TODO: Need to add character button?

	if (ImGui::BeginListBox("##CharListBox"))
	{
		for (int i = 0; i < charList.size(); i++)
		{
			bool isSelected = false;
			if (ImGui::Selectable((charList[i] + "##" + std::to_string(i)).c_str(), i == curCharIdx))
			{
				curCharIdx = i;
			}
		}
		ImGui::EndListBox();
	}

	ImGui::End();

	if (!charList.empty() && curCharIdx >= 0 && curCharIdx < charList.size())
	{
		ImGui::Begin("Character Data");
		if (curCharIdx != loadedCharIdx)
		{
			hasLoadedData = loadCharacterData(charList[curCharIdx], curCharData);
			loadedCharIdx = curCharIdx;
			reloadImageData();
			resetAttackAnimationData();
			reloadSoundData();
			bool ret = LoadTextureFromFile(("CharacterCreatorMod/char_" + curCharData.charName + "/" + curCharData.attackAnimationFileName).c_str(), &attackAnimationTexture, &attackAnimationWidth, &attackAnimationHeight);
			IM_ASSERT(ret);
			attackAnimationNumFrames = getSpriteNumFrames(curCharData.attackAnimationFileName);
			curBuffDataIdx = -1;
		}

		if (!hasLoadedData)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "FAILED TO LOAD DATA");
		}
		else
		{
			if (ImGui::Button("Save Data"))
			{
				ImGui::SameLine();
				if (!charList.empty() && loadedCharIdx >= 0 && loadedCharIdx < charList.size())
				{
					CreateDirectory(L"CharacterCreatorMod", NULL);
					std::string charDirName = "CharacterCreatorMod/" + charList[loadedCharIdx];
					CreateDirectoryA(charDirName.c_str(), NULL);
					nlohmann::json outputJSON = curCharData;

					std::ofstream outFile;
					outFile.open(charDirName + "/charData.json");
					outFile << std::setw(4) << outputJSON << "\n";
					outFile.close();
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "Saved to %s", charDirName.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "FAILED TO SAVE DATA");
				}
			}
			curCharData.atk.isDefined |= ImGui::InputDouble("ATK", &curCharData.atk.value);
			curCharData.crit.isDefined |= ImGui::InputDouble("CRIT", &curCharData.crit.value);
			curCharData.hp.isDefined |= ImGui::InputDouble("HP", &curCharData.hp.value);
			curCharData.spd.isDefined |= ImGui::InputDouble("SPD", &curCharData.spd.value);
			curCharData.sizeGrade.isDefined |= ImGui::SliderInt("sizeGrade", &curCharData.sizeGrade.value, 0, 4);
			if (ImGui::BeginCombo("weaponType", curCharData.mainWeaponWeaponType.c_str()))
			{
				const char* items[] = { "Melee", "Ranged", "Multishot" };
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					const bool is_selected = (curCharData.mainWeaponWeaponType.compare(items[n]) == 0);
					if (ImGui::Selectable(items[n], is_selected))
					{
						curCharData.mainWeaponWeaponType = items[n];
					}
				}
				ImGui::EndCombo();
			}
			if (ImGui::Button("Toggle Buff Data Window"))
			{
				showBuffDataWindow = !showBuffDataWindow;
			}
			if (ImGui::Button("Toggle Idle Animation Data Window"))
			{
				showIdleAnimationWindow = !showIdleAnimationWindow;
			}
			if (ImGui::Button("Toggle Run Animation Data Window"))
			{
				showRunAnimationWindow = !showRunAnimationWindow;
			}
			if (ImGui::Button("Toggle Portrait Window"))
			{
				showPortraitWindow = !showPortraitWindow;
			}
			if (ImGui::Button("Toggle Large Portrait Window"))
			{
				showLargePortraitWindow = !showLargePortraitWindow;
			}
			if (ImGui::Button("Toggle Special Animation Data Window"))
			{
				showSpecialAnimationWindow = !showSpecialAnimationWindow;
			}
			if (ImGui::Button("Toggle Weapon Levels Window"))
			{
				showWeaponLevelsWindow = !showWeaponLevelsWindow;
			}
			if (ImGui::Button("Toggle Skill Data Window"))
			{
				showSkillDataWindow = !showSkillDataWindow;
			}
			if (ImGui::Button("Toggle Projectile Data Window"))
			{
				showProjectileDataWindow = !showProjectileDataWindow;
			}
			if (ImGui::Button("Toggle Sound Data Window"))
			{
				showSoundDataWindow = !showSoundDataWindow;
				if (showSoundDataWindow)
				{
					reloadSoundData();
				}
			}
		}

		ImGui::End();
		
		if (showBuffDataWindow)
		{
			handleBuffDataWindow();
		}

		if (showIdleAnimationWindow)
		{
			handleIdleAnimationWindow();
		}

		if (showRunAnimationWindow)
		{
			handleRunAnimationWindow();
		}

		if (showPortraitWindow)
		{
			handlePortraitWindow();
		}

		if (showLargePortraitWindow)
		{
			handleLargePortraitWindow();
		}

		if (showSpecialAnimationWindow)
		{
			handleSpecialAnimationWindow();
		}

		if (showWeaponLevelsWindow)
		{
			handleWeaponLevelsWindow();
		}

		if (showSkillDataWindow)
		{
			handleSkillDataWindow();
		}

		if (showProjectileDataWindow)
		{
			handleProjectileWindow();
		}

		if (showSoundDataWindow)
		{
			handleSoundDataWindow();
		}
	}
}

void reloadLoadCharacterDeque()
{
	if (!std::filesystem::exists("CharacterCreatorMod"))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the CharacterCreatorMod directory");
		DbgPrintEx(LOG_SEVERITY_ERROR, "Couldn't find the CharacterCreatorMod directory");
		return;
	}
	loadCharacterPage = 0;
	loadCharacterDeque.clear();
	for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod"))
	{
		auto path = dir.path();
		if (path.filename().string().compare(0, 5, "char_") == 0)
		{
			loadCharacterDeque.push_back(path.filename().string());
		}
	}
//	reloadLoadCharacter();
}

std::string getSpriteFileName(std::shared_ptr<menuData> dataPtr)
{
	if (static_cast<menuDataImageField*>(dataPtr.get())->curSprite == nullptr)
	{
		return "";
	}
	return static_cast<menuDataImageField*>(dataPtr.get())->curSprite->spriteFileName;
}

std::string getSpriteFilePath(std::shared_ptr<menuData> dataPtr)
{
	if (static_cast<menuDataImageField*>(dataPtr.get())->curSprite == nullptr)
	{
		return "";
	}
	return static_cast<menuDataImageField*>(dataPtr.get())->curSprite->spritePath;
}

void copySpriteToDestination(std::shared_ptr<menuData>& imagePtr, std::string destPath)
{
	std::string spriteFilePath = getSpriteFilePath(imagePtr);
	if (spriteFilePath.empty() || spriteFilePath.compare(destPath) == 0)
	{
		return;
	}
	std::filesystem::copy_file(spriteFilePath, destPath, std::filesystem::copy_options::overwrite_existing);
}

void parseStringToJSONDouble(const std::string& inputStr, JSONDouble& outputJSONDouble, bool isSilent = true)
{
	auto [ptr, ec] = std::from_chars(inputStr.data(), inputStr.data() + inputStr.size(), outputJSONDouble.value);
	if (ec != std::errc{})
	{
		if (!isSilent)
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't parse %s as double", inputStr.c_str());
			DbgPrintEx(LOG_SEVERITY_ERROR, "Couldn't parse %s as double", inputStr.c_str());
		}
		outputJSONDouble.isDefined = false;
		return;
	}
	outputJSONDouble.isDefined = true;
}

void parseStringToJSONInt(const std::string& inputStr, JSONInt& outputJSONInt, bool isSilent = true)
{
	auto [ptr, ec] = std::from_chars(inputStr.data(), inputStr.data() + inputStr.size(), outputJSONInt.value);
	if (ec != std::errc{})
	{
		if (!isSilent)
		{
			callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't parse %s as int", inputStr.c_str());
			DbgPrintEx(LOG_SEVERITY_ERROR, "Couldn't parse %s as", inputStr.c_str());
		}
		outputJSONInt.isDefined = false;
		return;
	}
	outputJSONInt.isDefined = true;
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::string& outputStr)
{
	try
	{
		inputJson.at(varName).get_to(outputStr);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to string", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to string", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to string", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to string", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, JSONInt& outputJSONInt)
{
	try
	{
		inputJson.at(varName).get_to(outputJSONInt);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to JSONInt", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to JSONInt", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to JSONInt", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to JSONInt", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, JSONDouble& outputJSONDouble)
{
	try
	{
		inputJson.at(varName).get_to(outputJSONDouble);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to JSONDouble", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to JSONDouble", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to JSONDouble", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to JSONDouble", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<weaponLevelData>& outputWeaponLevelDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputWeaponLevelDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<skillDataWrapper>& outputSkillDataWrapperList)
{
	try
	{
		inputJson.at(varName).get_to(outputSkillDataWrapperList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to skillData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to skillData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to skillData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to skillData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<buffData>& outputBuffDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputBuffDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to buffData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to buffData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to buffData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to buffData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<skillLevelData>& outputSkillLevelDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputSkillLevelDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<buffLevelData>& outputBuffLevelDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputBuffLevelDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<std::string>& outputStringList)
{
	try
	{
		inputJson.at(varName).get_to(outputStringList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to string list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to string list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to string list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to string list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<projectileDataWrapper>& outputProjectileDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputProjectileDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to projectileData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to projectileData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to projectileData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to projectileData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, skillTriggerTypeEnum& outputSkillTriggerTypeEnum)
{
	try
	{
		std::string jsonString;
		inputJson.at(varName).get_to(jsonString);
		for (const auto& [key, value] : skillTriggerTypeMap)
		{
			if (jsonString.compare(value) == 0)
			{
				outputSkillTriggerTypeEnum = key;
				break;
			}
		}
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to skillTriggerTypeEnum", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to skillTriggerTypeEnum", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to skillTriggerTypeEnum", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to skillTriggerTypeEnum", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, int& outputInt)
{
	try
	{
		inputJson.at(varName).get_to(outputInt);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to int", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to int", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to int", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to int", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, bool& outputBool)
{
	try
	{
		inputJson.at(varName).get_to(outputBool);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to bool", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to bool", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to bool", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to bool", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<soundData>& outputSoundDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputSoundDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Type Error: %s when parsing var %s to soundData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to soundData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Out of Range Error: %s when parsing var %s to soundData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to soundData list", e.what(), varName);
	}
}

void to_json(nlohmann::json& outputJson, const characterData& inputCharData)
{
	outputJson = nlohmann::json{
		{ "portraitFileName", inputCharData.portraitFileName },
		{ "largePortraitFileName", inputCharData.largePortraitFileName },
		{ "idleAnimationFPS", inputCharData.idleAnimationFPS },
		{ "idleAnimationFileName", inputCharData.idleAnimationFileName },
		{ "runAnimationFPS", inputCharData.runAnimationFPS },
		{ "runAnimationFileName", inputCharData.runAnimationFileName },
		{ "HP", inputCharData.hp },
		{ "ATK", inputCharData.atk },
		{ "SPD", inputCharData.spd },
		{ "CRIT", inputCharData.crit },
		{ "attackIconFileName", inputCharData.attackIconFileName },
		{ "attackAwakenedIconFileName", inputCharData.attackAwakenedIconFileName },
		{ "attackName", inputCharData.attackName },
		{ "attackCollides", inputCharData.attackCollides },
		{ "specialIconFileName", inputCharData.specialIconFileName },
		{ "attackAnimationFPS", inputCharData.attackAnimationFPS },
		{ "attackAnimationFileName", inputCharData.attackAnimationFileName },
		{ "specialCooldown", inputCharData.specialCooldown },
		{ "specialName", inputCharData.specialName },
		{ "specialDescription", inputCharData.specialDescription },
		{ "specialAnimationFPS", inputCharData.specialAnimationFPS },
		{ "specialAnimationFileName", inputCharData.specialAnimationFileName },
		{ "specialDamage", inputCharData.specialDamage },
		{ "specialDuration", inputCharData.specialDuration },
		{ "sizeGrade", inputCharData.sizeGrade },
		{ "weaponType", inputCharData.mainWeaponWeaponType },
		{ "isUsingInGameMainWeapon", inputCharData.isUsingInGameMainWeapon },
		{ "inGameMainWeaponChar", inputCharData.inGameMainWeaponChar },
		{ "isUsingInGameIdleSprite", inputCharData.isUsingInGameIdleSprite },
		{ "inGameIdleSpriteChar", inputCharData.inGameIdleSpriteChar },
		{ "isUsingInGameRunSprite", inputCharData.isUsingInGameRunSprite },
		{ "inGameRunSpriteChar", inputCharData.inGameRunSpriteChar },
		{ "isUsingInGamePortraitSprite", inputCharData.isUsingInGamePortraitSprite },
		{ "inGamePortraitSpriteChar", inputCharData.inGamePortraitSpriteChar },
		{ "isUsingInGameLargePortraitSprite", inputCharData.isUsingInGameLargePortraitSprite },
		{ "inGameLargePortraitSpriteChar", inputCharData.inGameLargePortraitSpriteChar },
		{ "isUsingInGameSpecial", inputCharData.isUsingInGameSpecial },
		{ "inGameSpecialChar", inputCharData.inGameSpecialChar },
		{ "levels", inputCharData.weaponLevelDataList },
		{ "skills", inputCharData.skillDataList },
		{ "buffs", inputCharData.buffDataList },
		{ "projectileDataList", inputCharData.projectileDataList },
		{ "soundDataList", inputCharData.soundDataList },
		{ "mainWeaponNodeEditor", inputCharData.mainWeaponNodeEditor },
		{ "specialAttackNodeEditor", inputCharData.specialAttackNodeEditor },
	};
}

void from_json(const nlohmann::json& inputJson, characterData& outputCharData)
{
	parseJSONToVar(inputJson, "portraitFileName", outputCharData.portraitFileName);
	parseJSONToVar(inputJson, "largePortraitFileName", outputCharData.largePortraitFileName);
	parseJSONToVar(inputJson, "idleAnimationFPS", outputCharData.idleAnimationFPS);
	parseJSONToVar(inputJson, "idleAnimationFileName", outputCharData.idleAnimationFileName);
	parseJSONToVar(inputJson, "runAnimationFPS", outputCharData.runAnimationFPS);
	parseJSONToVar(inputJson, "runAnimationFileName", outputCharData.runAnimationFileName);
	parseJSONToVar(inputJson, "HP", outputCharData.hp);
	parseJSONToVar(inputJson, "ATK", outputCharData.atk);
	parseJSONToVar(inputJson, "SPD", outputCharData.spd);
	parseJSONToVar(inputJson, "CRIT", outputCharData.crit);
	parseJSONToVar(inputJson, "attackIconFileName", outputCharData.attackIconFileName);
	parseJSONToVar(inputJson, "attackAwakenedIconFileName", outputCharData.attackAwakenedIconFileName);
	parseJSONToVar(inputJson, "attackName", outputCharData.attackName);
	parseJSONToVar(inputJson, "attackCollides", outputCharData.attackCollides);
	parseJSONToVar(inputJson, "specialIconFileName", outputCharData.specialIconFileName);
	parseJSONToVar(inputJson, "attackAnimationFPS", outputCharData.attackAnimationFPS);
	parseJSONToVar(inputJson, "attackAnimationFileName", outputCharData.attackAnimationFileName);
	parseJSONToVar(inputJson, "specialCooldown", outputCharData.specialCooldown);
	parseJSONToVar(inputJson, "specialName", outputCharData.specialName);
	parseJSONToVar(inputJson, "specialDescription", outputCharData.specialDescription);
	parseJSONToVar(inputJson, "specialAnimationFPS", outputCharData.specialAnimationFPS);
	parseJSONToVar(inputJson, "specialAnimationFileName", outputCharData.specialAnimationFileName);
	parseJSONToVar(inputJson, "specialDamage", outputCharData.specialDamage);
	parseJSONToVar(inputJson, "specialDuration", outputCharData.specialDuration);
	/*
	auto& specialProjectileActionList = inputJson["specialProjectileActionList"];
	if (specialProjectileActionList.is_array())
	{
		outputCharData.specialProjectileActionList = specialProjectileActionList;
	}
	*/
	parseJSONToVar(inputJson, "sizeGrade", outputCharData.sizeGrade);
	parseJSONToVar(inputJson, "weaponType", outputCharData.mainWeaponWeaponType);
	parseJSONToVar(inputJson, "isUsingInGameMainWeapon", outputCharData.isUsingInGameMainWeapon);
	parseJSONToVar(inputJson, "inGameMainWeaponChar", outputCharData.inGameMainWeaponChar);
	parseJSONToVar(inputJson, "isUsingInGameIdleSprite", outputCharData.isUsingInGameIdleSprite);
	parseJSONToVar(inputJson, "inGameIdleSpriteChar", outputCharData.inGameIdleSpriteChar);
	parseJSONToVar(inputJson, "isUsingInGameRunSprite", outputCharData.isUsingInGameRunSprite);
	parseJSONToVar(inputJson, "inGameRunSpriteChar", outputCharData.inGameRunSpriteChar);
	parseJSONToVar(inputJson, "isUsingInGamePortraitSprite", outputCharData.isUsingInGamePortraitSprite);
	parseJSONToVar(inputJson, "inGamePortraitSpriteChar", outputCharData.inGamePortraitSpriteChar);
	parseJSONToVar(inputJson, "isUsingInGameLargePortraitSprite", outputCharData.isUsingInGameLargePortraitSprite);
	parseJSONToVar(inputJson, "inGameLargePortraitSpriteChar", outputCharData.inGameLargePortraitSpriteChar);
	parseJSONToVar(inputJson, "isUsingInGameSpecial", outputCharData.isUsingInGameSpecial);
	parseJSONToVar(inputJson, "inGameSpecialChar", outputCharData.inGameSpecialChar);

	parseJSONToVar(inputJson, "levels", outputCharData.weaponLevelDataList);
	parseJSONToVar(inputJson, "skills", outputCharData.skillDataList);
	parseJSONToVar(inputJson, "buffs", outputCharData.buffDataList);
	parseJSONToVar(inputJson, "projectileDataList", outputCharData.projectileDataList);
	parseJSONToVar(inputJson, "soundDataList", outputCharData.soundDataList);
	from_json(inputJson["mainWeaponNodeEditor"], outputCharData.mainWeaponNodeEditor);
	from_json(inputJson["specialAttackNodeEditor"], outputCharData.specialAttackNodeEditor);
}

void to_json(nlohmann::json& outputJson, const skillData& inputSkillData)
{
	outputJson = nlohmann::json{
		{ "isUsingInGameSkill", inputSkillData.isUsingInGameSkill },
		{ "inGameSkillName", inputSkillData.inGameSkillName },
		{ "levels", inputSkillData.skillLevelDataList },
		{ "skillName", inputSkillData.skillName },
		{ "skillIconFileName", inputSkillData.skillIconFileName },
	};
}

void from_json(const nlohmann::json& inputJson, skillData& outputSkillData)
{
	auto& isUsingInGameSkill = inputJson["isUsingInGameSkill"];
	if (isUsingInGameSkill.is_boolean())
	{
		outputSkillData.isUsingInGameSkill = isUsingInGameSkill;
	}
	parseJSONToVar(inputJson, "inGameSkillName", outputSkillData.inGameSkillName);
	parseJSONToVar(inputJson, "levels", outputSkillData.skillLevelDataList);
	parseJSONToVar(inputJson, "skillName", outputSkillData.skillName);
	parseJSONToVar(inputJson, "skillIconFileName", outputSkillData.skillIconFileName);
}

void to_json(nlohmann::json& outputJson, const skillDataWrapper& inputSkillDataWrapper)
{
	to_json(outputJson, inputSkillDataWrapper.data);
	outputJson["nodeEditor"] = inputSkillDataWrapper.nodeEditor;
}

void from_json(const nlohmann::json& inputJson, skillDataWrapper& outputSkillDataWrapper)
{
	from_json(inputJson, outputSkillDataWrapper.data);
	from_json(inputJson["nodeEditor"], outputSkillDataWrapper.nodeEditor);
}

void to_json(nlohmann::json& outputJson, const skillLevelData& inputSkillLevelData)
{
	outputJson = nlohmann::json{
		{ "skillDescription", inputSkillLevelData.skillDescription },
		{ "attackIncrement", inputSkillLevelData.attackIncrement },
		{ "critIncrement", inputSkillLevelData.critIncrement },
		{ "hasteIncrement", inputSkillLevelData.hasteIncrement },
		{ "speedIncrement", inputSkillLevelData.speedIncrement },
		{ "DRMultiplier", inputSkillLevelData.DRMultiplier },
		{ "healMultiplier", inputSkillLevelData.healMultiplier },
		{ "food", inputSkillLevelData.food },
		{ "weaponSize", inputSkillLevelData.weaponSize },
		{ "pickupRange", inputSkillLevelData.pickupRange },
		{ "critMod", inputSkillLevelData.critMod },
		{ "bonusProjectile", inputSkillLevelData.bonusProjectile },
	};
}

void from_json(const nlohmann::json& inputJson, skillLevelData& outputSkillLevelData)
{
	parseJSONToVar(inputJson, "skillDescription", outputSkillLevelData.skillDescription);
	parseJSONToVar(inputJson, "attackIncrement", outputSkillLevelData.attackIncrement);
	parseJSONToVar(inputJson, "critIncrement", outputSkillLevelData.critIncrement);
	parseJSONToVar(inputJson, "hasteIncrement", outputSkillLevelData.hasteIncrement);
	parseJSONToVar(inputJson, "speedIncrement", outputSkillLevelData.speedIncrement);
	parseJSONToVar(inputJson, "DRMultiplier", outputSkillLevelData.DRMultiplier);
	parseJSONToVar(inputJson, "healMultiplier", outputSkillLevelData.healMultiplier);
	parseJSONToVar(inputJson, "food", outputSkillLevelData.food);
	parseJSONToVar(inputJson, "weaponSize", outputSkillLevelData.weaponSize);
	parseJSONToVar(inputJson, "pickupRange", outputSkillLevelData.pickupRange);
	parseJSONToVar(inputJson, "critMod", outputSkillLevelData.critMod);
	parseJSONToVar(inputJson, "bonusProjectile", outputSkillLevelData.bonusProjectile);
}

void to_json(nlohmann::json& outputJson, const weaponLevelData& inputWeaponLevelData)
{
	outputJson = nlohmann::json{
		{ "attackDescription", inputWeaponLevelData.attackDescription },
		{ "attackTime", inputWeaponLevelData.attackTime },
		{ "attackCount", inputWeaponLevelData.attackCount },
		{ "attackDelay", inputWeaponLevelData.attackDelay },
		{ "damage", inputWeaponLevelData.damage },
		{ "duration", inputWeaponLevelData.duration },
		{ "hitCD", inputWeaponLevelData.hitCD },
		{ "hitLimit", inputWeaponLevelData.hitLimit },
		{ "range", inputWeaponLevelData.range },
		{ "speed", inputWeaponLevelData.speed },
	};
}

void from_json(const nlohmann::json& inputJson, weaponLevelData& outputWeaponLevelData)
{
	parseJSONToVar(inputJson, "attackDescription", outputWeaponLevelData.attackDescription);
	parseJSONToVar(inputJson, "attackTime", outputWeaponLevelData.attackTime);
	parseJSONToVar(inputJson, "attackCount", outputWeaponLevelData.attackCount);
	parseJSONToVar(inputJson, "attackDelay", outputWeaponLevelData.attackDelay);
	parseJSONToVar(inputJson, "damage", outputWeaponLevelData.damage);
	parseJSONToVar(inputJson, "duration", outputWeaponLevelData.duration);
	parseJSONToVar(inputJson, "hitCD", outputWeaponLevelData.hitCD);
	parseJSONToVar(inputJson, "hitLimit", outputWeaponLevelData.hitLimit);
	parseJSONToVar(inputJson, "range", outputWeaponLevelData.range);
	parseJSONToVar(inputJson, "speed", outputWeaponLevelData.speed);
}

void to_json(nlohmann::json& outputJson, const buffLevelData& inputBuffLevelData)
{
	outputJson = nlohmann::json{
		{ "attackIncrement", inputBuffLevelData.attackIncrement },
		{ "critIncrement", inputBuffLevelData.critIncrement },
		{ "hasteIncrement", inputBuffLevelData.hasteIncrement },
		{ "speedIncrement", inputBuffLevelData.speedIncrement },
		{ "DR", inputBuffLevelData.DRMultiplier },
		{ "healMultiplier", inputBuffLevelData.healMultiplier },
		{ "food", inputBuffLevelData.food },
		{ "weaponSize", inputBuffLevelData.weaponSize },
		{ "pickupRange", inputBuffLevelData.pickupRange },
		{ "critMod", inputBuffLevelData.critMod },
		{ "bonusProjectile", inputBuffLevelData.bonusProjectile },
		{ "maxStacks", inputBuffLevelData.maxStacks },
		{ "timer", inputBuffLevelData.timer },
	};
}

void from_json(const nlohmann::json& inputJson, buffLevelData& outputBuffLevelData)
{
	parseJSONToVar(inputJson, "attackIncrement", outputBuffLevelData.attackIncrement);
	parseJSONToVar(inputJson, "critIncrement", outputBuffLevelData.critIncrement);
	parseJSONToVar(inputJson, "hasteIncrement", outputBuffLevelData.hasteIncrement);
	parseJSONToVar(inputJson, "speedIncrement", outputBuffLevelData.speedIncrement);
	parseJSONToVar(inputJson, "DR", outputBuffLevelData.DRMultiplier);
	parseJSONToVar(inputJson, "healMultiplier", outputBuffLevelData.healMultiplier);
	parseJSONToVar(inputJson, "food", outputBuffLevelData.food);
	parseJSONToVar(inputJson, "weaponSize", outputBuffLevelData.weaponSize);
	parseJSONToVar(inputJson, "pickupRange", outputBuffLevelData.pickupRange);
	parseJSONToVar(inputJson, "critMod", outputBuffLevelData.critMod);
	parseJSONToVar(inputJson, "bonusProjectile", outputBuffLevelData.bonusProjectile);
	parseJSONToVar(inputJson, "maxStacks", outputBuffLevelData.maxStacks);
	parseJSONToVar(inputJson, "timer", outputBuffLevelData.timer);
}

void to_json(nlohmann::json& outputJson, const buffData& inputBuffData)
{
	outputJson = nlohmann::json{
		{ "buffName", inputBuffData.buffName },
		{ "data", inputBuffData.data },
		{ "buffIconFileName", inputBuffData.buffIconFileName },
	};
}

void from_json(const nlohmann::json& inputJson, buffData& outputBuffData)
{
	parseJSONToVar(inputJson, "buffName", outputBuffData.buffName);
	from_json(inputJson["data"], outputBuffData.data);
	parseJSONToVar(inputJson, "buffIconFileName", outputBuffData.buffIconFileName);
}

void to_json(nlohmann::json& outputJson, const soundData& inputSoundData)
{
	outputJson = nlohmann::json{
		{ "soundName", inputSoundData.soundName },
		{ "soundFile", inputSoundData.soundRValue.soundFile },
	};
}

void from_json(const nlohmann::json& inputJson, soundData& outputSoundData)
{
	parseJSONToVar(inputJson, "soundName", outputSoundData.soundName);
	parseJSONToVar(inputJson, "soundFile", outputSoundData.soundRValue.soundFile);
}

void to_json(nlohmann::json& outputJson, const JSONDouble& inputJSONDoubleData)
{
	if (inputJSONDoubleData.isDefined)
	{
		outputJson = inputJSONDoubleData.value;
	}
	else
	{
		outputJson = nullptr;
	}
}

void from_json(const nlohmann::json& inputJson, JSONDouble& outputJSONDoubleData)
{
	if (inputJson.is_number())
	{
		outputJSONDoubleData.isDefined = true;
		outputJSONDoubleData.value = inputJson;
	}
	else
	{
		outputJSONDoubleData.isDefined = false;
	}
}

void to_json(nlohmann::json& outputJson, const JSONInt& inputJSONIntData)
{
	if (inputJSONIntData.isDefined)
	{
		outputJson = inputJSONIntData.value;
	}
	else
	{
		outputJson = nullptr;
	}
}

void from_json(const nlohmann::json& inputJson, JSONInt& outputJSONIntData)
{
	if (inputJson.is_number_integer())
	{
		outputJSONIntData.isDefined = true;
		outputJSONIntData.value = inputJson;
	}
	else
	{
		outputJSONIntData.isDefined = false;
	}
}

void to_json(nlohmann::json& outputJson, const projectileData& inputProjectileData)
{
	outputJson = nlohmann::json{
		{ "projectileName", inputProjectileData.projectileName },
		{ "projectileAnimationFPS", inputProjectileData.projectileAnimationFPS },
		{ "projectileAnimationFileName", inputProjectileData.projectileAnimationFileName },
		{ "projectileDamage", inputProjectileData.projectileDamage },
		{ "projectileDuration", inputProjectileData.projectileDuration },
		{ "projectileHitCD", inputProjectileData.projectileHitCD },
		{ "projectileHitLimit", inputProjectileData.projectileHitLimit },
		{ "projectileHitRange", inputProjectileData.projectileHitRange },
		{ "projectileSpeed", inputProjectileData.projectileSpeed },
		{ "collides", inputProjectileData.collides },
		{ "isMain", inputProjectileData.isMain },
	};
}

void from_json(const nlohmann::json& inputJson, projectileData& outputProjectileData)
{
	parseJSONToVar(inputJson, "projectileName", outputProjectileData.projectileName);
	parseJSONToVar(inputJson, "projectileAnimationFPS", outputProjectileData.projectileAnimationFPS);
	parseJSONToVar(inputJson, "projectileAnimationFileName", outputProjectileData.projectileAnimationFileName);
	parseJSONToVar(inputJson, "projectileDamage", outputProjectileData.projectileDamage);
	parseJSONToVar(inputJson, "projectileDuration", outputProjectileData.projectileDuration);
	parseJSONToVar(inputJson, "projectileHitCD", outputProjectileData.projectileHitCD);
	parseJSONToVar(inputJson, "projectileHitLimit", outputProjectileData.projectileHitLimit);
	parseJSONToVar(inputJson, "projectileHitRange", outputProjectileData.projectileHitRange);
	parseJSONToVar(inputJson, "projectileSpeed", outputProjectileData.projectileSpeed);
	parseJSONToVar(inputJson, "collides", outputProjectileData.collides);
	parseJSONToVar(inputJson, "isMain", outputProjectileData.isMain);
}

void to_json(nlohmann::json& outputJson, const projectileDataWrapper& inputProjectileDataWrapper)
{
	to_json(outputJson, inputProjectileDataWrapper.data);
	outputJson["nodeEditor"] = inputProjectileDataWrapper.nodeEditor;
}

void from_json(const nlohmann::json& inputJson, projectileDataWrapper& outputProjectileDataWrapper)
{
	from_json(inputJson, outputProjectileDataWrapper.data);
	from_json(inputJson["nodeEditor"], outputProjectileDataWrapper.nodeEditor);
}

void to_json(nlohmann::json& outputJson, const nodeEditor& inputNodeEditor)
{
	outputJson = nlohmann::json{
		{ "nextID", inputNodeEditor.nextID },
		{ "nodeMap", inputNodeEditor.nodeMap },
		{ "nodePinMap", inputNodeEditor.nodePinMap },
		{ "nodeLinkMap", inputNodeEditor.nodeLinkMap },
	};
}

void from_json(const nlohmann::json& inputJson, nodeEditor& outputNodeEditor)
{
	if (inputJson.is_null())
	{
		return;
	}
	outputNodeEditor.nextID = inputJson["nextID"];
	outputNodeEditor.nodeMap = inputJson["nodeMap"];
	outputNodeEditor.nodePinMap = inputJson["nodePinMap"];
	outputNodeEditor.nodeLinkMap = inputJson["nodeLinkMap"];
}

void to_json(nlohmann::json& outputJson, const nodeEditorNodeLink& inputNodeLink)
{
	outputJson = nlohmann::json{
		{ "linkID", inputNodeLink.linkID },
		{ "startPinID", inputNodeLink.startPinID },
		{ "endPinID", inputNodeLink.endPinID },
	};
}

void from_json(const nlohmann::json& inputJson, nodeEditorNodeLink& outputNodeLink)
{
	if (inputJson.is_null())
	{
		return;
	}
	parseJSONToVar(inputJson, "linkID", outputNodeLink.linkID);
	outputNodeLink.startPinID = inputJson["startPinID"];
	outputNodeLink.endPinID = inputJson["endPinID"];
}

void to_json(nlohmann::json& outputJson, const nodeEditorNodePin& inputNodePin)
{
	outputJson = nlohmann::json{
		{ "parentNodeID", inputNodePin.parentNodeID },
		{ "pinID", inputNodePin.pinID },
		{ "pinName", inputNodePin.pinName },
		{ "pinType", inputNodePin.pinType },
		{ "nodeLinks", inputNodePin.nodeLinks },
		{ "isInput", inputNodePin.isInput },
	};
	switch (inputNodePin.pinType)
	{
		case nodeEditorPinType_Number:
		{
			if (!inputNodePin.isInput)
			{
				outputJson["pinNumberVar"] = inputNodePin.pinNumberVar;
			}
			break;
		}
		case nodeEditorPinType_ProjectileData:
		{
			if (!inputNodePin.isInput)
			{
				outputJson["pinProjectileDataName"] = inputNodePin.pinProjectileDataName;
			}
			break;
		}
		case nodeEditorPinType_BuffData:
		{
			if (!inputNodePin.isInput)
			{
				outputJson["pinBuffDataName"] = inputNodePin.pinBuffDataName;
			}
			break;
		}
		case nodeEditorPinType_SoundData:
		{
			if (!inputNodePin.isInput)
			{
				outputJson["pinSoundDataName"] = inputNodePin.pinSoundDataName;
			}
			break;
		}
		case nodeEditorPinType_VariableName:
		{
			outputJson["pinVariableDataName"] = inputNodePin.pinVariableDataName;
			break;
		}
		case nodeEditorPinType_Boolean:
		{
			outputJson["pinBooleanDataName"] = inputNodePin.pinBooleanDataName;
			break;
		}
		case nodeEditorPinType_Compare:
		{
			outputJson["pinCompareDataName"] = inputNodePin.pinCompareDataName;
			break;
		}
		case nodeEditorPinType_String:
		{
			outputJson["pinStringVar"] = inputNodePin.pinStringVar;
			break;
		}
		case nodeEditorPinType_Integer:
		{
			outputJson["pinIntegerVar"] = inputNodePin.pinIntegerVar;
			break;
		}
		case nodeEditorPinType_VariableType:
		{
			outputJson["pinVariableTypeName"] = inputNodePin.pinVariableTypeName;
			break;
		}
	}
}

void from_json(const nlohmann::json& inputJson, nodeEditorNodePin& outputNodePin)
{
	if (inputJson.is_null())
	{
		return;
	}
	parseJSONToVar(inputJson, "parentNodeID", outputNodePin.parentNodeID);
	parseJSONToVar(inputJson, "pinID", outputNodePin.pinID);
	parseJSONToVar(inputJson, "pinName", outputNodePin.pinName);
	outputNodePin.pinType = inputJson["pinType"];
	outputNodePin.nodeLinks = inputJson["nodeLinks"];
	outputNodePin.isInput = inputJson["isInput"];
	switch (outputNodePin.pinType)
	{
		case nodeEditorPinType_Number:
		{
			if (!outputNodePin.isInput)
			{
				outputNodePin.pinNumberVar = inputJson["pinNumberVar"];
			}
			break;
		}
		case nodeEditorPinType_ProjectileData:
		{
			if (!outputNodePin.isInput)
			{
				parseJSONToVar(inputJson, "pinProjectileDataName", outputNodePin.pinProjectileDataName);
			}
			break;
		}
		case nodeEditorPinType_BuffData:
		{
			if (!outputNodePin.isInput)
			{
				parseJSONToVar(inputJson, "pinBuffDataName", outputNodePin.pinBuffDataName);
			}
			break;
		}
		case nodeEditorPinType_SoundData:
		{
			if (!outputNodePin.isInput)
			{
				parseJSONToVar(inputJson, "pinSoundDataName", outputNodePin.pinSoundDataName);
			}
			break;
		}
		case nodeEditorPinType_VariableName:
		{
			parseJSONToVar(inputJson, "pinVariableDataName", outputNodePin.pinVariableDataName);
			if (outputNodePin.pinVariableDataName.compare("direction") == 0)
			{
				outputNodePin.variableDataType = pinVariableDataType_Direction;
			}
			else if (outputNodePin.pinVariableDataName.compare("creator") == 0)
			{
				outputNodePin.variableDataType = pinVariableDataType_Creator;
			}
			else if (outputNodePin.pinVariableDataName.compare("x") == 0)
			{
				outputNodePin.variableDataType = pinVariableDataType_X;
			}
			else if (outputNodePin.pinVariableDataName.compare("y") == 0)
			{
				outputNodePin.variableDataType = pinVariableDataType_Y;
			}
			else if (outputNodePin.pinVariableDataName.compare("Custom RValue") == 0)
			{
				outputNodePin.variableDataType = pinVariableDataType_CustomRValue;
			}
			break;
		}
		case nodeEditorPinType_Boolean:
		{
			parseJSONToVar(inputJson, "pinBooleanDataName", outputNodePin.pinBooleanDataName);
			if (outputNodePin.pinBooleanDataName.compare("True") == 0)
			{
				outputNodePin.booleanDataType = pinBooleanDataType_True;
			}
			else if (outputNodePin.pinBooleanDataName.compare("False") == 0)
			{
				outputNodePin.booleanDataType = pinBooleanDataType_False;
			}
			break;
		}
		case nodeEditorPinType_Compare:
		{
			parseJSONToVar(inputJson, "pinCompareDataName", outputNodePin.pinCompareDataName);
			if (outputNodePin.pinCompareDataName.compare("<") == 0)
			{
				outputNodePin.compareDataType = pinCompareDataType_Less;
			}
			else if (outputNodePin.pinCompareDataName.compare("<=") == 0)
			{
				outputNodePin.compareDataType = pinCompareDataType_LessOrEqual;
			}
			else if (outputNodePin.pinCompareDataName.compare("==") == 0)
			{
				outputNodePin.compareDataType = pinCompareDataType_Equal;
			}
			else if (outputNodePin.pinCompareDataName.compare(">") == 0)
			{
				outputNodePin.compareDataType = pinCompareDataType_Greater;
			}
			else if (outputNodePin.pinCompareDataName.compare(">=") == 0)
			{
				outputNodePin.compareDataType = pinCompareDataType_GreaterOrEqual;
			}
			else if (outputNodePin.pinCompareDataName.compare("!=") == 0)
			{
				outputNodePin.compareDataType = pinCompareDataType_NotEqual;
			}
			break;
		}
		case nodeEditorPinType_String:
		{
			parseJSONToVar(inputJson, "pinStringVar", outputNodePin.pinStringVar);
			break;
		}
		case nodeEditorPinType_Integer:
		{
			parseJSONToVar(inputJson, "pinIntegerVar", outputNodePin.pinIntegerVar);
			break;
		}
		case nodeEditorPinType_VariableType:
		{
			parseJSONToVar(inputJson, "pinVariableTypeName", outputNodePin.pinVariableTypeName);
			break;
		}
	}
}

void to_json(nlohmann::json& outputJson, const nodeEditorNode& inputNode)
{
	ImVec2 nodePos;
	if (inputNode.parentNodeEditor != nullptr && inputNode.parentNodeEditor->isEditorOpen)
	{
		ax::NodeEditor::SetCurrentEditor(inputNode.parentNodeEditor->editorContext);
		nodePos = ax::NodeEditor::GetNodePosition(inputNode.nodeID); // TODO: Seems like this doesn't work if the window isn't actually open?
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}
	else
	{
		nodePos.x = static_cast<float>(inputNode.initPosX);
		nodePos.y = static_cast<float>(inputNode.initPosY);
	}
	
	outputJson = nlohmann::json{
		{ "nodeID", inputNode.nodeID },
		{ "nodeName", inputNode.nodeName },
		{ "nodeType", inputNode.nodeType },
		{ "inputPinIDArr", inputNode.inputPinIDArr },
		{ "outputPinIDArr", inputNode.outputPinIDArr },
		{ "nodePosX", nodePos.x },
		{ "nodePosY", nodePos.y },
	};
}

void from_json(const nlohmann::json& inputJson, nodeEditorNode& outputNode)
{
	if (inputJson.is_null())
	{
		return;
	}
	parseJSONToVar(inputJson, "nodeID", outputNode.nodeID);
	parseJSONToVar(inputJson, "nodeName", outputNode.nodeName);
	outputNode.nodeType = inputJson["nodeType"];
	outputNode.setPinSpacing();
	outputNode.setCanDelete();
	auto& inputPinIDArr = inputJson["inputPinIDArr"];
	if (inputPinIDArr.is_array())
	{
		outputNode.inputPinIDArr = inputPinIDArr.get<std::vector<int>>();
	}
	auto& outputPinIDArr = inputJson["outputPinIDArr"];
	if (outputPinIDArr.is_array())
	{
		outputNode.outputPinIDArr = outputPinIDArr.get<std::vector<int>>();
	}
	auto& nodePosX = inputJson["nodePosX"];
	auto& nodePosY = inputJson["nodePosY"];
	// TODO: Seems like it might give default values if the editor was never initialized?
	// TODO: Might also be crashing due to trying to set the node position before the editor is initialized? Could try to delay it
	if (nodePosX.is_number() && nodePosY.is_number())
	{
		outputNode.initPosX = nodePosX;
		outputNode.initPosY = nodePosY;
	}
}