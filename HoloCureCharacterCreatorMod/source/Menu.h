#pragma once
#pragma once
#include "ModuleMain.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include "nlohmann/json.hpp"
#include "imgui_node_editor/imgui_node_editor.h"
#include <vector>
#include <string>

extern HoloCureMenuInterface* holoCureMenuInterfacePtr;

struct spriteData;

struct characterData;
struct skillData;
struct skillDataWrapper;
struct skillLevelData;
struct weaponLevelData;
struct buffLevelData;
struct buffData;
struct JSONDouble;
struct JSONInt;
struct projectileDataWrapper;
struct projectileData;
struct nodeEditor;
struct projectileNodeEditor;
struct nodeEditorNode;
struct nodeEditorNodeLink;
struct nodeEditorNodePin;
struct pinVariableData;
struct soundData;

void to_json(nlohmann::json& outputJson, const characterData& inputCharData);
void from_json(const nlohmann::json& inputJson, characterData& outputCharData);
void to_json(nlohmann::json& outputJson, const skillData& inputSkillData);
void from_json(const nlohmann::json& inputJson, skillData& outputSkillData);
void to_json(nlohmann::json& outputJson, const skillDataWrapper& inputSkillDataWrapper);
void from_json(const nlohmann::json& inputJson, skillDataWrapper& outputSkillDataWrapper);
void to_json(nlohmann::json& outputJson, const skillLevelData& inputSkillLevelData);
void from_json(const nlohmann::json& inputJson, skillLevelData& outputSkillLevelData);
void to_json(nlohmann::json& outputJson, const weaponLevelData& inputWeaponLevelData);
void from_json(const nlohmann::json& inputJson, weaponLevelData& outputWeaponLevelData);
void to_json(nlohmann::json& outputJson, const buffLevelData& inputBuffLevelData);
void from_json(const nlohmann::json& inputJson, buffLevelData& outputBuffLevelData);
void to_json(nlohmann::json& outputJson, const buffData& inputBuffData);
void from_json(const nlohmann::json& inputJson, buffData& outputBuffData);
void to_json(nlohmann::json& outputJson, const JSONDouble& inputJSONDoubleData);
void from_json(const nlohmann::json& inputJson, JSONDouble& outputJSONDoubleData);
void to_json(nlohmann::json& outputJson, const JSONInt& inputJSONIntData);
void from_json(const nlohmann::json& inputJson, JSONInt& outputJSONIntData);
void to_json(nlohmann::json& outputJson, const projectileData& inputProjectileData);
void from_json(const nlohmann::json& inputJson, projectileData& outputProjectileData);
void to_json(nlohmann::json& outputJson, const projectileDataWrapper& inputProjectileDataWrapper);
void from_json(const nlohmann::json& inputJson, projectileDataWrapper& outputProjectileDataWrapper);
void to_json(nlohmann::json& outputJson, const nodeEditor& inputNodeEditor);
void from_json(const nlohmann::json& inputJson, nodeEditor& outputNodeEditor);
void to_json(nlohmann::json& outputJson, const nodeEditorNodeLink& inputNodeLink);
void from_json(const nlohmann::json& inputJson, nodeEditorNodeLink& outputNodeLink);
void to_json(nlohmann::json& outputJson, const nodeEditorNodePin& inputNodePin);
void from_json(const nlohmann::json& inputJson, nodeEditorNodePin& outputNodePin);
void to_json(nlohmann::json& outputJson, const nodeEditorNode& inputNode);
void from_json(const nlohmann::json& inputJson, nodeEditorNode& outputNode);
void to_json(nlohmann::json& outputJson, const soundData& inputSoundData);
void from_json(const nlohmann::json& inputJson, soundData& outputSoundData);

class TextureData
{
public:
	int width;
	int height;
	int numFrames;
	double curFrame;
	ID3D11ShaderResourceView* texture;
	bool isAnimationPlaying;

	TextureData() : width(0), height(0), numFrames(0), curFrame(0), texture(NULL), isAnimationPlaying(false)
	{
	}
};

struct JSONInt
{
	bool isDefined;
	int value;

	JSONInt() : isDefined(false), value(0)
	{
	}
};

struct JSONDouble
{
	bool isDefined;
	double value;

	JSONDouble() : isDefined(false), value(0)
	{
	}
};

/*
enum projectileActionTriggerTypeEnum
{
	projectileActionTriggerType_NONE,
	projectileActionTriggerType_OnDestroy,
	projectileActionTriggerType_OnCreate,
};

static std::unordered_map<projectileActionTriggerTypeEnum, std::string> projectileActionTriggerTypeMap
{
	{ projectileActionTriggerType_NONE, "NONE" },
	{ projectileActionTriggerType_OnDestroy, "OnDestroy" },
	{ projectileActionTriggerType_OnCreate, "OnCreate" },
};

struct projectileActionData
{
	std::string projectileActionName;
	projectileActionTriggerTypeEnum projectileActionTriggerType;
	std::string triggeredActionName;
};
*/

struct weaponLevelData
{
	std::string attackDescription;
	JSONInt attackTime;
	JSONInt attackCount;
	JSONInt attackDelay;
	JSONDouble damage;
	JSONInt duration;
	JSONInt hitCD;
	JSONInt hitLimit;
	JSONInt range;
	JSONDouble speed;
//	std::vector<projectileActionData> projectileActionList;
};

enum skillTriggerTypeEnum
{
	skillTriggerType_NONE,
	skillTriggerType_OnDebuff,
	skillTriggerType_OnAttackCreate,
	skillTriggerType_OnCriticalHit,
	skillTriggerType_OnHeal,
	skillTriggerType_OnKill,
	skillTriggerType_OnTakeDamage,
	skillTriggerType_OnDodge,
};

static std::unordered_map<skillTriggerTypeEnum, std::string> skillTriggerTypeMap
{
	{ skillTriggerType_NONE, "NONE" },
	{ skillTriggerType_OnDebuff, "onDebuff" },
	{ skillTriggerType_OnAttackCreate, "onAttackCreate" },
	{ skillTriggerType_OnCriticalHit, "onCriticalHit" },
	{ skillTriggerType_OnHeal, "onHeal" },
	{ skillTriggerType_OnKill, "onKill" },
	{ skillTriggerType_OnTakeDamage, "onTakeDamage" },
	{ skillTriggerType_OnDodge, "onDodge" },
};

struct buffLevelData
{
	JSONInt attackIncrement;
	JSONInt critIncrement;
	JSONInt hasteIncrement;
	JSONInt speedIncrement;
	JSONDouble DRMultiplier;
	JSONDouble healMultiplier;
	JSONDouble food;
	JSONDouble weaponSize;
	JSONDouble pickupRange;
	JSONDouble critMod;
	JSONDouble bonusProjectile;
	JSONInt maxStacks;
	JSONInt timer;
};

struct buffData
{
	std::string buffName;
	buffLevelData data;
	std::string buffIconFileName;
};

struct soundWrapper
{
	std::string soundFile;
	RValue soundIndex;
	soundWrapper()
	{
	}

	~soundWrapper()
	{
		if (soundIndex.m_Kind != VALUE_UNDEFINED)
		{
			g_ModuleInterface->CallBuiltin("audio_destroy_stream", { soundIndex });
		}
	}
};

struct soundData
{
	std::string soundName;
	soundWrapper soundRValue;
};

enum nodeEditorNodeTypeEnum
{
	nodeEditorNodeType_OnCreateProjectile,
	nodeEditorNodeType_OnDestroyProjectile,
	nodeEditorNodeType_SpawnProjectile,
	nodeEditorNodeType_ApplyBuff,
	nodeEditorNodeType_Number,
	nodeEditorNodeType_ProjectileData,
	nodeEditorNodeType_ThisInstance,
	nodeEditorNodeType_InitVariable,
	nodeEditorNodeType_GetVariable,
	nodeEditorNodeType_SetVariable,
	nodeEditorNodeType_Add,
	nodeEditorNodeType_Subtract,
	nodeEditorNodeType_Multiply,
	nodeEditorNodeType_Divide,
	nodeEditorNodeType_Delay,
	nodeEditorNodeType_Boolean,
	nodeEditorNodeType_If,
	nodeEditorNodeType_Compare,
	nodeEditorNodeType_While,
	nodeEditorNodeType_MergeCodeFlow,
	nodeEditorNodeType_And,
	nodeEditorNodeType_Or,
	nodeEditorNodeType_Not,
	nodeEditorNodeType_String,
	nodeEditorNodeType_Print,
	nodeEditorNodeType_TypeCast,
	nodeEditorNodeType_AppendString,
	nodeEditorNodeType_Integer,
	nodeEditorNodeType_IntegerCeiling,
	nodeEditorNodeType_IntegerFloor,
	nodeEditorNodeType_IntegerRound,
	nodeEditorNodeType_IntegerAdd,
	nodeEditorNodeType_IntegerSubtract,
	nodeEditorNodeType_IntegerMultiply,
	nodeEditorNodeType_IntegerDivide,
	nodeEditorNodeType_IntegerCompare,
	nodeEditorNodeType_GlobalInstance,
	nodeEditorNodeType_Random,
	nodeEditorNodeType_IntegerRandom,
	nodeEditorNodeType_Modulus,
	nodeEditorNodeType_SquareRoot,
	nodeEditorNodeType_Logarithm,
	nodeEditorNodeType_Sine,
	nodeEditorNodeType_Cosine,
	nodeEditorNodeType_Tangent,
	nodeEditorNodeType_PlayerManagerInstance,
	nodeEditorNodeType_PlayerInstance,
	nodeEditorNodeType_AssetGetIndex,
	nodeEditorNodeType_GetStructVariable,
	nodeEditorNodeType_SetStructVariable,
	nodeEditorNodeType_OnFrameStep,
	nodeEditorNodeType_OnSkillApply,
	nodeEditorNodeType_IsRValueDefined,
	nodeEditorNodeType_BuffData,
	nodeEditorNodeType_CollisionCircleList,
	nodeEditorNodeType_DSListGet,
	nodeEditorNodeType_DSListSize,
	nodeEditorNodeType_DSMapGet,
	nodeEditorNodeType_DSMapKeysToArray,
	nodeEditorNodeType_ArrayGet,
	nodeEditorNodeType_ArrayLength,
	nodeEditorNodeType_TernaryOperator,
	nodeEditorNodeType_SetDebugLevel,
	nodeEditorNodeType_CacheVariable,
	nodeEditorNodeType_SoundData,
	nodeEditorNodeType_PlaySound,
	nodeEditorNodeType_PointDirection,
	nodeEditorNodeType_FlushCache,
	nodeEditorNodeType_MergeFlush,
	nodeEditorNodeType_ArrayCreate,
	nodeEditorNodeType_ArrayPush,
	nodeEditorNodeType_DSListCreate,
	nodeEditorNodeType_DSListClear,
	nodeEditorNodeType_DSListDestroy,
};

enum nodeEditorPinTypeEnum
{
	nodeEditorPinType_None,
	nodeEditorPinType_CodeFlow,
	nodeEditorPinType_ProjectileData,
	nodeEditorPinType_Number,
	nodeEditorPinType_Instance,
	nodeEditorPinType_VariableName,
	nodeEditorPinType_Boolean,
	nodeEditorPinType_Compare,
	nodeEditorPinType_String,
	nodeEditorPinType_Integer,
	nodeEditorPinType_VariableType,
	nodeEditorPinType_RValue,
	nodeEditorPinType_BuffData,
	nodeEditorPinType_DSList,
	nodeEditorPinType_DSMap,
	nodeEditorPinType_Array,
	nodeEditorPinType_SoundData,
	nodeEditorPinType_FlushCache,
};

enum nodeEditorPinMenuStateEnum
{
	nodeEditorPinMenuState_Closed,
	nodeEditorPinMenuState_Clicked,
	nodeEditorPinMenuState_Open,
};

static std::unordered_map<nodeEditorNodeTypeEnum, std::string> nodeEditorNodeTypeMap
{
	{ nodeEditorNodeType_OnCreateProjectile, "OnCreateProjectile" },
	{ nodeEditorNodeType_OnDestroyProjectile, "OnDestroyProjectile" },
	{ nodeEditorNodeType_SpawnProjectile, "SpawnProjectile" },
	{ nodeEditorNodeType_ApplyBuff, "ApplyBuff" },
};

struct nodeEditorNodeLink
{
	int linkID;
	int startPinID;
	int endPinID;
	nodeEditorNodePin* startPinPtr = nullptr;
	nodeEditorNodePin* endPinPtr = nullptr;

	nodeEditorNodeLink() : linkID(0), startPinID(0), endPinID(0)
	{
	}

	nodeEditorNodeLink(int& nextID, int startPinID, int endPinID) : linkID(nextID++), startPinID(startPinID), endPinID(endPinID)
	{
	}
};

enum pinVariableDataTypeEnum
{
	pinVariableDataType_None,
	pinVariableDataType_Direction,
	pinVariableDataType_Creator,
	pinVariableDataType_X,
	pinVariableDataType_Y,
	pinVariableDataType_CustomRValue,
};

enum pinBooleanDataTypeEnum
{
	pinBooleanDataType_None,
	pinBooleanDataType_True,
	pinBooleanDataType_False,
};

enum pinCompareDataTypeEnum
{
	pinCompareDataType_None,
	pinCompareDataType_Less,
	pinCompareDataType_LessOrEqual,
	pinCompareDataType_Equal,
	pinCompareDataType_Greater,
	pinCompareDataType_GreaterOrEqual,
	pinCompareDataType_NotEqual,
};

struct nodeEditorNodePin
{
	nodeEditorNode* parentNodePtr = nullptr;
	pinVariableData* data = nullptr;
	int parentNodeID;
	int pinID;
	std::string pinName;
	nodeEditorPinTypeEnum pinType;
	std::vector<nodeEditorNodeLink> nodeLinks;
	bool isInput;

	// variable storage for number pin
	double pinNumberVar = 0;
	// variable storage for projectile data name
	std::string pinProjectileDataName;
	nodeEditorPinMenuStateEnum projectileDataMenuState = nodeEditorPinMenuState_Closed;
	// variable storage for variable name
	std::string pinVariableDataName;
	nodeEditorPinMenuStateEnum variableDataMenuState = nodeEditorPinMenuState_Closed;
	pinVariableDataTypeEnum variableDataType = pinVariableDataType_None;
	// variable storage for boolean type
	std::string pinBooleanDataName;
	nodeEditorPinMenuStateEnum variableBooleanMenuState = nodeEditorPinMenuState_Closed;
	pinBooleanDataTypeEnum booleanDataType = pinBooleanDataType_None;
	// variable storage for compare type
	// TODO: Store this as an enum
	std::string pinCompareDataName;
	nodeEditorPinMenuStateEnum variableCompareMenuState = nodeEditorPinMenuState_Closed;
	pinCompareDataTypeEnum compareDataType = pinCompareDataType_None;
	// variable storage for string pin
	std::string pinStringVar;
	// variable storage for integer pin
	int pinIntegerVar = 0;
	// variable storage for variable type
	std::string pinVariableTypeName;
	nodeEditorPinMenuStateEnum variableTypeMenuState = nodeEditorPinMenuState_Closed;
	// variable storage for buff data name
	std::string pinBuffDataName;
	nodeEditorPinMenuStateEnum buffDataMenuState = nodeEditorPinMenuState_Closed;
	// variable storage for sound data name
	std::string pinSoundDataName;
	nodeEditorPinMenuStateEnum soundDataMenuState = nodeEditorPinMenuState_Closed;

	nodeEditorNodePin(): parentNodeID(0), pinID(0), pinType(nodeEditorPinType_CodeFlow), isInput(false)
	{
	}

	~nodeEditorNodePin();

	nodeEditorNodePin(int parentNodeID, int& nextID, std::string pinName, nodeEditorPinTypeEnum pinType, bool isInput) : parentNodeID(parentNodeID), pinID(nextID++), pinName(pinName), pinType(pinType), isInput(isInput)
	{
	}
};

struct nodeEditor
{
	int nextID;
	bool isEditorOpen;
	bool initEditorPos;
	ImVec2 openPopupPosition;
	ax::NodeEditor::EditorContext* editorContext;
	std::unordered_map<int, nodeEditorNode> nodeMap;
	std::unordered_map<int, nodeEditorNodePin> nodePinMap;
	std::unordered_map<int, nodeEditorNodeLink> nodeLinkMap;

	nodeEditor() : isEditorOpen(false), nextID(1), initEditorPos(false)
	{
		editorContext = ax::NodeEditor::CreateEditor();
	}

	nodeEditor(const nodeEditor& otherNodeEditor);
	nodeEditor& operator=(const nodeEditor& otherNodeEditor);

	// TODO: Make a clear and load function?
};

struct nodeEditorNode
{
	nodeEditor* parentNodeEditor = nullptr;
	int nodeID;
	std::string nodeName;
	nodeEditorNodeTypeEnum nodeType;
	std::vector<int> inputPinIDArr;
	std::vector<int> outputPinIDArr;
	std::vector<nodeEditorNodePin*> inputPinPtrArr;
	std::vector<nodeEditorNodePin*> outputPinPtrArr;
	int pinSpacing;
	bool canDelete;
	double initPosX;
	double initPosY;

	nodeEditorNode() : parentNodeEditor(nullptr), nodeID(-1), nodeType(nodeEditorNodeType_OnCreateProjectile), pinSpacing(100), canDelete(true), initPosX(1e20), initPosY(1e20)
	{
	}

	nodeEditorNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName, nodeEditorNodeTypeEnum nodeType)
		: parentNodeEditor(parentNodeEditor), nodeID(nextID++), nodeName(nodeName), nodeType(nodeType), initPosX(1e20), initPosY(1e20)
	{
		setPinSpacing();
		setCanDelete();
	}

	void addInputPin(int& nextID, std::string pinName, nodeEditorPinTypeEnum pinType)
	{
		auto inputPin = nodeEditorNodePin(nodeID, nextID, pinName, pinType, true);
		parentNodeEditor->nodePinMap[inputPin.pinID] = inputPin;
		inputPinIDArr.push_back(inputPin.pinID);
	}

	void addOutputPin(int& nextID, std::string pinName, nodeEditorPinTypeEnum pinType)
	{
		auto outputPin = nodeEditorNodePin(nodeID, nextID, pinName, pinType, false);
		parentNodeEditor->nodePinMap[outputPin.pinID] = outputPin;
		outputPinIDArr.push_back(outputPin.pinID);
	}

	void setPinSpacing();
	void setCanDelete();

	void drawNode();
};

// TODO: Add array access support
// TODO: Maybe consider having local editor variables? Maybe make a local editor instance?

struct nodeEditorOnCreateProjectileNode : nodeEditorNode
{
	nodeEditorOnCreateProjectileNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_OnCreateProjectile)
	{
		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorOnDestroyProjectileNode : nodeEditorNode
{
	nodeEditorOnDestroyProjectileNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_OnDestroyProjectile)
	{
		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorSpawnProjectileNode : nodeEditorNode
{
	nodeEditorSpawnProjectileNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_SpawnProjectile)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "Projectile Data", nodeEditorPinType_ProjectileData);
		addInputPin(nextID, "Direction", nodeEditorPinType_Number);
		addInputPin(nextID, "x", nodeEditorPinType_Number);
		addInputPin(nextID, "y", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addOutputPin(nextID, "", nodeEditorPinType_Instance);
	}
};

struct nodeEditorApplyBuffNode : nodeEditorNode
{
	nodeEditorApplyBuffNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_ApplyBuff)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_BuffData);
		addInputPin(nextID, "stacks", nodeEditorPinType_Integer);
		addInputPin(nextID, "timer", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorNumberNode : nodeEditorNode
{
	nodeEditorNumberNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Number)
	{
		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorProjectileDataNode : nodeEditorNode
{
	nodeEditorProjectileDataNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_ProjectileData)
	{
		addInputPin(nextID, "damage override", nodeEditorPinType_Number);
		addInputPin(nextID, "duration override", nodeEditorPinType_Integer);
		addInputPin(nextID, "hit cooldown override", nodeEditorPinType_Integer);
		addInputPin(nextID, "hit limit override", nodeEditorPinType_Integer);
		addInputPin(nextID, "hit range override", nodeEditorPinType_Integer);
		addInputPin(nextID, "projectile speed override", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_ProjectileData);
	}
};

struct nodeEditorBuffDataNode : nodeEditorNode
{
	nodeEditorBuffDataNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_BuffData)
	{
		addInputPin(nextID, "max stacks override", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_BuffData);
	}
};

struct nodeEditorThisInstanceNode : nodeEditorNode
{
	nodeEditorThisInstanceNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_ThisInstance)
	{
		addOutputPin(nextID, "", nodeEditorPinType_Instance);
	}
};

struct nodeEditorInitVariableNode : nodeEditorNode
{
	nodeEditorInitVariableNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_InitVariable)
	{
		addInputPin(nextID, "", nodeEditorPinType_Instance);
		addInputPin(nextID, "", nodeEditorPinType_VariableName);
		addInputPin(nextID, "", nodeEditorPinType_None);
	}
};

struct nodeEditorGetVariableNode : nodeEditorNode
{
	nodeEditorGetVariableNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_GetVariable)
	{
		addInputPin(nextID, "", nodeEditorPinType_Instance);
		addInputPin(nextID, "", nodeEditorPinType_VariableName);
		addInputPin(nextID, "", nodeEditorPinType_None); // Custom RValue pin

		addOutputPin(nextID, "", nodeEditorPinType_None); // output variable pin
	}
};

struct nodeEditorSetVariableNode : nodeEditorNode
{
	nodeEditorSetVariableNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_SetVariable)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_Instance);
		addInputPin(nextID, "", nodeEditorPinType_VariableName);
		addInputPin(nextID, "", nodeEditorPinType_None); // input variable pin
		addInputPin(nextID, "", nodeEditorPinType_None); // Custom RValue pin

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorAddNode : nodeEditorNode
{
	nodeEditorAddNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Add)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorSubtractNode : nodeEditorNode
{
	nodeEditorSubtractNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Subtract)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorMultiplyNode : nodeEditorNode
{
	nodeEditorMultiplyNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Multiply)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorDivideNode : nodeEditorNode
{
	nodeEditorDivideNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Divide)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorDelayNode : nodeEditorNode
{
	nodeEditorDelayNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Delay)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorBooleanNode : nodeEditorNode
{
	nodeEditorBooleanNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Boolean)
	{
		addOutputPin(nextID, "", nodeEditorPinType_Boolean);
	}
};

struct nodeEditorIfNode : nodeEditorNode
{
	nodeEditorIfNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_If)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_Boolean);

		addOutputPin(nextID, "True", nodeEditorPinType_CodeFlow);
		addOutputPin(nextID, "False", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorCompareNode : nodeEditorNode
{
	nodeEditorCompareNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Compare)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);
		addInputPin(nextID, "", nodeEditorPinType_Number);
		addInputPin(nextID, "", nodeEditorPinType_Compare); // Compare type pin

		addOutputPin(nextID, "", nodeEditorPinType_Boolean);
	}
};

struct nodeEditorWhileNode : nodeEditorNode
{
	nodeEditorWhileNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_While)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_Boolean);

		addOutputPin(nextID, "Loop Body", nodeEditorPinType_CodeFlow);
		addOutputPin(nextID, "Completed", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorMergeCodeFlowNode : nodeEditorNode
{
	nodeEditorMergeCodeFlowNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_MergeCodeFlow)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorAndNode : nodeEditorNode
{
	nodeEditorAndNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_And)
	{
		addInputPin(nextID, "", nodeEditorPinType_Boolean);
		addInputPin(nextID, "", nodeEditorPinType_Boolean);

		addOutputPin(nextID, "", nodeEditorPinType_Boolean);
	}
};

struct nodeEditorOrNode : nodeEditorNode
{
	nodeEditorOrNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Or)
	{
		addInputPin(nextID, "", nodeEditorPinType_Boolean);
		addInputPin(nextID, "", nodeEditorPinType_Boolean);

		addOutputPin(nextID, "", nodeEditorPinType_Boolean);
	}
};

struct nodeEditorNotNode : nodeEditorNode
{
	nodeEditorNotNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Not)
	{
		addInputPin(nextID, "", nodeEditorPinType_Boolean);

		addOutputPin(nextID, "", nodeEditorPinType_Boolean);
	}
};

struct nodeEditorStringNode : nodeEditorNode
{
	nodeEditorStringNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_String)
	{
		addOutputPin(nextID, "", nodeEditorPinType_String);
	}
};

struct nodeEditorPrintNode : nodeEditorNode
{
	nodeEditorPrintNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Print)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_String);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorTypeCastNode : nodeEditorNode
{
	nodeEditorTypeCastNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_TypeCast)
	{
		addInputPin(nextID, "", nodeEditorPinType_VariableType); // Variable type input
		addInputPin(nextID, "", nodeEditorPinType_None); // input type pin

		addOutputPin(nextID, "", nodeEditorPinType_VariableType); // Variable type output
		addOutputPin(nextID, "", nodeEditorPinType_None); // output type pin
	}
};

struct nodeEditorAppendStringNode : nodeEditorNode
{
	nodeEditorAppendStringNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_AppendString)
	{
		addInputPin(nextID, "", nodeEditorPinType_String);
		addInputPin(nextID, "", nodeEditorPinType_String);

		addOutputPin(nextID, "", nodeEditorPinType_String);
	}
};

struct nodeEditorIntegerNode : nodeEditorNode
{
	nodeEditorIntegerNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Integer)
	{
		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerCeilingNode : nodeEditorNode
{
	nodeEditorIntegerCeilingNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerCeiling)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerFloorNode : nodeEditorNode
{
	nodeEditorIntegerFloorNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerFloor)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerRoundNode : nodeEditorNode
{
	nodeEditorIntegerRoundNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerRound)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerAddNode : nodeEditorNode
{
	nodeEditorIntegerAddNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerAdd)
	{
		addInputPin(nextID, "", nodeEditorPinType_Integer);
		addInputPin(nextID, "", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerSubtractNode : nodeEditorNode
{
	nodeEditorIntegerSubtractNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerSubtract)
	{
		addInputPin(nextID, "", nodeEditorPinType_Integer);
		addInputPin(nextID, "", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerMultiplyNode : nodeEditorNode
{
	nodeEditorIntegerMultiplyNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerMultiply)
	{
		addInputPin(nextID, "", nodeEditorPinType_Integer);
		addInputPin(nextID, "", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerDivideNode : nodeEditorNode
{
	nodeEditorIntegerDivideNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerDivide)
	{
		addInputPin(nextID, "", nodeEditorPinType_Integer);
		addInputPin(nextID, "", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorIntegerCompareNode : nodeEditorNode
{
	nodeEditorIntegerCompareNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerCompare)
	{
		addInputPin(nextID, "", nodeEditorPinType_Integer);
		addInputPin(nextID, "", nodeEditorPinType_Integer);
		addInputPin(nextID, "", nodeEditorPinType_Compare); // Compare type pin

		addOutputPin(nextID, "", nodeEditorPinType_Boolean);
	}
};

struct nodeEditorGlobalInstanceNode : nodeEditorNode
{
	nodeEditorGlobalInstanceNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_GlobalInstance)
	{
		addOutputPin(nextID, "", nodeEditorPinType_Instance);
	}
};

struct nodeEditorRandomNode : nodeEditorNode
{
	nodeEditorRandomNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Random)
	{
		addInputPin(nextID, "min", nodeEditorPinType_Number);
		addInputPin(nextID, "max", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorIntegerRandomNode : nodeEditorNode
{
	nodeEditorIntegerRandomNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IntegerRandom)
	{
		addInputPin(nextID, "min", nodeEditorPinType_Integer);
		addInputPin(nextID, "max", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorModulusNode : nodeEditorNode
{
	nodeEditorModulusNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Modulus)
	{
		addInputPin(nextID, "", nodeEditorPinType_Integer);
		addInputPin(nextID, "", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorSquareRootNode : nodeEditorNode
{
	nodeEditorSquareRootNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_SquareRoot)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorLogarithmNode : nodeEditorNode
{
	nodeEditorLogarithmNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Logarithm)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);
		addInputPin(nextID, "base", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorSineNode : nodeEditorNode
{
	nodeEditorSineNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Sine)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorCosineNode : nodeEditorNode
{
	nodeEditorCosineNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Cosine)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorTangentNode : nodeEditorNode
{
	nodeEditorTangentNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_Tangent)
	{
		addInputPin(nextID, "", nodeEditorPinType_Number);

		addOutputPin(nextID, "", nodeEditorPinType_Number);
	}
};

struct nodeEditorPlayerManagerNode : nodeEditorNode
{
	nodeEditorPlayerManagerNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_PlayerManagerInstance)
	{
		addOutputPin(nextID, "", nodeEditorPinType_RValue);
	}
};

struct nodeEditorPlayerNode : nodeEditorNode
{
	nodeEditorPlayerNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_PlayerInstance)
	{
		addOutputPin(nextID, "", nodeEditorPinType_RValue);
	}
};

struct nodeEditorAssetGetIndexNode : nodeEditorNode
{
	nodeEditorAssetGetIndexNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_AssetGetIndex)
	{
		addInputPin(nextID, "", nodeEditorPinType_String);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorGetStructVariableNode : nodeEditorNode
{
	nodeEditorGetStructVariableNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_GetStructVariable)
	{
		addInputPin(nextID, "", nodeEditorPinType_RValue);
		addInputPin(nextID, "", nodeEditorPinType_VariableName);
		addInputPin(nextID, "", nodeEditorPinType_None); // Custom RValue pin

		addOutputPin(nextID, "", nodeEditorPinType_None); // output variable pin
	}
};

struct nodeEditorSetStructVariableNode : nodeEditorNode
{
	nodeEditorSetStructVariableNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_SetStructVariable)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_RValue);
		addInputPin(nextID, "", nodeEditorPinType_VariableName);
		addInputPin(nextID, "", nodeEditorPinType_None); // input variable pin
		addInputPin(nextID, "", nodeEditorPinType_None); // Custom RValue pin

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorOnFrameStepNode : nodeEditorNode
{
	nodeEditorOnFrameStepNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_OnFrameStep)
	{
		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorOnSkillApplyNode : nodeEditorNode
{
	nodeEditorOnSkillApplyNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_OnSkillApply)
	{
		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorIsRValueDefinedNode : nodeEditorNode
{
	nodeEditorIsRValueDefinedNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_IsRValueDefined)
	{
		addInputPin(nextID, "", nodeEditorPinType_RValue);

		addOutputPin(nextID, "", nodeEditorPinType_Boolean);
	}
};

struct nodeEditorCollisionCircleListNode : nodeEditorNode
{
	nodeEditorCollisionCircleListNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_CollisionCircleList)
	{
		addInputPin(nextID, "x", nodeEditorPinType_Number);
		addInputPin(nextID, "y", nodeEditorPinType_Number);
		addInputPin(nextID, "radius", nodeEditorPinType_Number);
		addInputPin(nextID, "object index", nodeEditorPinType_Integer);
		addInputPin(nextID, "ordered", nodeEditorPinType_Boolean);
		addInputPin(nextID, "", nodeEditorPinType_DSList);

		addOutputPin(nextID, "", nodeEditorPinType_DSList);
	}
};

struct nodeEditorDSListGetNode : nodeEditorNode
{
	nodeEditorDSListGetNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_DSListGet)
	{
		addInputPin(nextID, "", nodeEditorPinType_DSList);
		addInputPin(nextID, "index", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_RValue);
	}
};

struct nodeEditorDSListSizeNode : nodeEditorNode
{
	nodeEditorDSListSizeNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_DSListSize)
	{
		addInputPin(nextID, "", nodeEditorPinType_DSList);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorDSMapGetNode : nodeEditorNode
{
	nodeEditorDSMapGetNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_DSMapGet)
	{
		addInputPin(nextID, "", nodeEditorPinType_DSMap);
		addInputPin(nextID, "key", nodeEditorPinType_String);

		addOutputPin(nextID, "", nodeEditorPinType_RValue);
	}
};

struct nodeEditorDSMapKeysToArrayNode : nodeEditorNode
{
	nodeEditorDSMapKeysToArrayNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_DSMapKeysToArray)
	{
		addInputPin(nextID, "", nodeEditorPinType_DSMap);

		addOutputPin(nextID, "", nodeEditorPinType_Array);
	}
};

struct nodeEditorArrayGetNode : nodeEditorNode
{
	nodeEditorArrayGetNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_ArrayGet)
	{
		addInputPin(nextID, "", nodeEditorPinType_Array);
		addInputPin(nextID, "index", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_RValue);
	}
};

struct nodeEditorArrayLengthNode : nodeEditorNode
{
	nodeEditorArrayLengthNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_ArrayLength)
	{
		addInputPin(nextID, "", nodeEditorPinType_Array);

		addOutputPin(nextID, "", nodeEditorPinType_Integer);
	}
};

struct nodeEditorTernaryOperatorNode : nodeEditorNode
{
	nodeEditorTernaryOperatorNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_TernaryOperator)
	{
		addInputPin(nextID, "", nodeEditorPinType_VariableType); // Ternary type
		addInputPin(nextID, "", nodeEditorPinType_Boolean);
		addInputPin(nextID, "True", nodeEditorPinType_None); // input type pin
		addInputPin(nextID, "False", nodeEditorPinType_None); // input type pin

		addOutputPin(nextID, "", nodeEditorPinType_None); // output type pin
	}
};

struct nodeEditorSetDebugLevelNode : nodeEditorNode
{
	nodeEditorSetDebugLevelNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_SetDebugLevel)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "Debug Level", nodeEditorPinType_Integer);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorCacheVariableNode : nodeEditorNode
{
	nodeEditorCacheVariableNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_CacheVariable)
	{
		addInputPin(nextID, "", nodeEditorPinType_VariableType);
		addInputPin(nextID, "", nodeEditorPinType_None);
		addInputPin(nextID, "", nodeEditorPinType_FlushCache);

		addOutputPin(nextID, "", nodeEditorPinType_None);
	}
};

struct nodeEditorSoundDataNode : nodeEditorNode
{
	nodeEditorSoundDataNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_SoundData)
	{
		addOutputPin(nextID, "", nodeEditorPinType_SoundData);
	}
};

struct nodeEditorPlaySoundNode : nodeEditorNode
{
	nodeEditorPlaySoundNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_PlaySound)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_SoundData);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorPointDirectionNode : nodeEditorNode
{
	nodeEditorPointDirectionNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_PointDirection)
	{
		addInputPin(nextID, "Source X", nodeEditorPinType_Number);
		addInputPin(nextID, "Source Y", nodeEditorPinType_Number);
		addInputPin(nextID, "Target X", nodeEditorPinType_Number);
		addInputPin(nextID, "Target Y", nodeEditorPinType_Number);

		addOutputPin(nextID, "Direction", nodeEditorPinType_Number);
	}
};

struct nodeEditorFlushCacheNode : nodeEditorNode
{
	nodeEditorFlushCacheNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_FlushCache)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addOutputPin(nextID, "", nodeEditorPinType_FlushCache);
	}
};

struct nodeEditorMergeFlushNode : nodeEditorNode
{
	nodeEditorMergeFlushNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_MergeFlush)
	{
		addInputPin(nextID, "", nodeEditorPinType_FlushCache);
		addInputPin(nextID, "", nodeEditorPinType_FlushCache);

		addOutputPin(nextID, "", nodeEditorPinType_FlushCache);
	}
};

struct nodeEditorArrayCreateNode : nodeEditorNode
{
	nodeEditorArrayCreateNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_ArrayCreate)
	{
		addOutputPin(nextID, "", nodeEditorPinType_Array);
	}
};

struct nodeEditorArrayPushNode : nodeEditorNode
{
	nodeEditorArrayPushNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_ArrayPush)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_Array);
		addInputPin(nextID, "", nodeEditorPinType_VariableType);
		addInputPin(nextID, "", nodeEditorPinType_None);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorDSListCreateNode : nodeEditorNode
{
	nodeEditorDSListCreateNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_DSListCreate)
	{
		addOutputPin(nextID, "", nodeEditorPinType_DSList);
	}
};

struct nodeEditorDSListClearNode : nodeEditorNode
{
	nodeEditorDSListClearNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_DSListClear)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_DSList);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct nodeEditorDSListDestroyNode : nodeEditorNode
{
	nodeEditorDSListDestroyNode(nodeEditor* parentNodeEditor, int& nextID, std::string nodeName) : nodeEditorNode(parentNodeEditor, nextID, nodeName, nodeEditorNodeType_DSListDestroy)
	{
		addInputPin(nextID, "", nodeEditorPinType_CodeFlow);
		addInputPin(nextID, "", nodeEditorPinType_DSList);

		addOutputPin(nextID, "", nodeEditorPinType_CodeFlow);
	}
};

struct projectileNodeEditor : nodeEditor
{
	projectileNodeEditor()
	{
		nodeEditorNode curNode = nodeEditorOnCreateProjectileNode(this, nextID, "On Create Projectile");
		nodeMap[curNode.nodeID] = curNode;
		curNode = nodeEditorOnDestroyProjectileNode(this, nextID, "On Destroy Projectile");
		nodeMap[curNode.nodeID] = curNode;
		curNode = nodeEditorOnFrameStepNode(this, nextID, "On Frame Step");
		nodeMap[curNode.nodeID] = curNode;
	}
};

struct weaponNodeEditor : nodeEditor
{
	weaponNodeEditor()
	{
		nodeEditorNode curNode = nodeEditorOnCreateProjectileNode(this, nextID, "On Create Projectile");
		nodeMap[curNode.nodeID] = curNode;
		curNode = nodeEditorOnDestroyProjectileNode(this, nextID, "On Destroy Projectile");
		nodeMap[curNode.nodeID] = curNode;
		curNode = nodeEditorOnFrameStepNode(this, nextID, "On Frame Step");
		nodeMap[curNode.nodeID] = curNode;
	}
};

struct skillNodeEditor : nodeEditor
{
	skillNodeEditor()
	{
		nodeEditorNode curNode = nodeEditorOnFrameStepNode(this, nextID, "On Frame Step");
		nodeMap[curNode.nodeID] = curNode;
		curNode = nodeEditorOnSkillApplyNode(this, nextID, "On Skill Apply");
		nodeMap[curNode.nodeID] = curNode;
	}
};

struct specialNodeEditor : nodeEditor
{
	specialNodeEditor()
	{
		nodeEditorNode curNode = nodeEditorOnCreateProjectileNode(this, nextID, "On Create Projectile");
		nodeMap[curNode.nodeID] = curNode;
		curNode = nodeEditorOnDestroyProjectileNode(this, nextID, "On Destroy Projectile");
		nodeMap[curNode.nodeID] = curNode;
		curNode = nodeEditorOnFrameStepNode(this, nextID, "On Frame Step");
		nodeMap[curNode.nodeID] = curNode;
	}
};

struct projectileData
{
	std::string projectileName;
	JSONInt projectileAnimationFPS;
	std::string projectileAnimationFileName;
	JSONDouble projectileDamage;
	JSONInt projectileDuration;
	JSONInt projectileHitCD;
	JSONInt projectileHitLimit;
	JSONInt projectileHitRange;
	JSONDouble projectileSpeed;
	bool isMain;

	// TODO: Should add projectile weapon type
	// TODO: Should add isSkill flag
	bool collides;
};

struct projectileDataWrapper
{
	projectileData data;

	projectileNodeEditor nodeEditor;
	std::string curSelectedProjectileAction;
	TextureData projectileAnimationTextureData;
	bool showProjectileEditor;
};

struct skillLevelData
{
	std::string skillDescription;
	JSONInt attackIncrement;
	JSONInt critIncrement;
	JSONInt hasteIncrement;
	JSONInt speedIncrement;
	JSONDouble DRMultiplier;
	JSONDouble healMultiplier;
	JSONDouble food;
	JSONDouble weaponSize;
	JSONDouble pickupRange;
	JSONDouble critMod;
	JSONDouble bonusProjectile;
};

struct skillData
{
	bool isUsingInGameSkill;
	std::string inGameSkillName;

	std::vector<skillLevelData> skillLevelDataList;
	std::string skillName;
	std::string skillIconFileName;
	TextureData skillIconTextureData;

	skillData() : isUsingInGameSkill(false)
	{
		for (int i = 0; i < 3; i++)
		{
			skillLevelDataList.push_back(skillLevelData());
		}
	}
};

struct skillDataWrapper
{
	skillData data;
	skillNodeEditor nodeEditor;
};

struct characterData
{
	std::string charName;
	std::string portraitFileName;
	std::string largePortraitFileName;
	JSONInt idleAnimationFPS;
	std::string idleAnimationFileName;
	JSONInt runAnimationFPS;
	std::string runAnimationFileName;
	JSONDouble hp;
	JSONDouble atk;
	JSONDouble spd;
	JSONDouble crit;
	std::string attackIconFileName;
	std::string attackAwakenedIconFileName;
	std::string attackName;
	bool attackCollides = true;
	std::string specialIconFileName;
	JSONInt attackAnimationFPS;
	std::string attackAnimationFileName;
	JSONInt specialCooldown;
	std::string specialName;
	std::string specialDescription;
	JSONInt specialAnimationFPS;
	std::string specialAnimationFileName;
	JSONDouble specialDamage;
	JSONInt specialDuration;
//	std::vector<projectileActionData> specialProjectileActionList;
	JSONInt sizeGrade;
	std::string mainWeaponWeaponType;
	bool isUsingInGameMainWeapon;
	std::string inGameMainWeaponChar;
	bool isUsingInGameIdleSprite;
	std::string inGameIdleSpriteChar;
	bool isUsingInGameRunSprite;
	std::string inGameRunSpriteChar;
	bool isUsingInGamePortraitSprite;
	std::string inGamePortraitSpriteChar;
	bool isUsingInGameLargePortraitSprite;
	std::string inGameLargePortraitSpriteChar;
	bool isUsingInGameSpecial;
	std::string inGameSpecialChar;

	std::vector<weaponLevelData> weaponLevelDataList;
	std::vector<skillDataWrapper> skillDataList;
	std::vector<buffData> buffDataList;
	std::vector<projectileDataWrapper> projectileDataList;
	std::vector<soundData> soundDataList;
	weaponNodeEditor mainWeaponNodeEditor;
	specialNodeEditor specialAttackNodeEditor;

	characterData() : isUsingInGameMainWeapon(false), isUsingInGameIdleSprite(false), isUsingInGameRunSprite(false), isUsingInGamePortraitSprite(false),
		isUsingInGameLargePortraitSprite(false), isUsingInGameSpecial(false)
	{
		idleAnimationFPS.isDefined = true;
		idleAnimationFPS.value = 4;
		runAnimationFPS.isDefined = true;
		runAnimationFPS.value = 12;
		attackAnimationFPS.isDefined = true;
		attackAnimationFPS.value = 30;
		skillDataList = std::vector<skillDataWrapper>(3);
		for (int i = 0; i < 3; i++)
		{
			skillDataList[i] = std::move(skillDataWrapper());
		}
		for (int i = 0; i < 7; i++)
		{
			weaponLevelDataList.push_back(weaponLevelData());
		}
	}
};

struct menuColumn
{
	std::shared_ptr<menuColumnData> menuColumnPtr;
	std::vector<std::shared_ptr<menuData>> menuDataPtrList;

	menuColumn(std::vector<std::shared_ptr<menuData>> menuDataPtrList) : menuColumnPtr(nullptr), menuDataPtrList(menuDataPtrList)
	{
	}

	void initMenuColumn(std::shared_ptr<menuGridData>& menuGridPtr)
	{
		holoCureMenuInterfacePtr->CreateMenuColumn(MODNAME, menuGridPtr, menuColumnPtr);
		for (auto& menuDataPtr : menuDataPtrList)
		{
			holoCureMenuInterfacePtr->AddMenuData(MODNAME, menuColumnPtr, menuDataPtr);
		}
	}
};

struct menuGrid
{
	std::string name;
	menuGrid* prevMenuGridPtr;
	std::shared_ptr<menuGridData> menuGridPtr;
	std::vector<menuColumn> menuColumnList;
	menuFunc onEnterFunc;

	menuGrid() : menuGridPtr(nullptr), prevMenuGridPtr(nullptr), onEnterFunc(nullptr)
	{
	}

	menuGrid(std::vector<menuColumn> menuColumnList, std::string name, menuGrid* prevMenuGridPtr) : menuGridPtr(nullptr), name(name), prevMenuGridPtr(prevMenuGridPtr), menuColumnList(menuColumnList), onEnterFunc(nullptr)
	{
	}

	menuGrid(std::vector<menuColumn> menuColumnList, std::string name, menuGrid* prevMenuGridPtr, menuFunc onEnterFunc) : menuGridPtr(nullptr), name(name), prevMenuGridPtr(prevMenuGridPtr), menuColumnList(menuColumnList), onEnterFunc(onEnterFunc)
	{
	}

	void initMenuGrid()
	{
		std::shared_ptr<menuGridData> menuGridDataPtr = nullptr;
		if (prevMenuGridPtr != nullptr)
		{
			menuGridDataPtr = prevMenuGridPtr->menuGridPtr;
		}
		holoCureMenuInterfacePtr->CreateMenuGrid(MODNAME, name, menuGridDataPtr, menuGridPtr);
		menuGridPtr->onEnterFunc = onEnterFunc;
		for (auto& menuColumn : menuColumnList)
		{
			menuColumn.initMenuColumn(menuGridPtr);
		}
	}
};

struct buffDataMenuGrid
{
	std::shared_ptr<menuGrid> buffDataGrid;
	std::shared_ptr<menuGrid> buffIconGrid;

	buffDataMenuGrid(std::shared_ptr<menuGrid> buffDataGrid, std::shared_ptr<menuGrid> buffIconGrid) : buffDataGrid(buffDataGrid), buffIconGrid(buffIconGrid)
	{
	}
};

struct actionDataMenuGrid
{
	std::shared_ptr<menuGrid> actionDataGrid;

	actionDataMenuGrid(std::shared_ptr<menuGrid> actionDataGrid) : actionDataGrid(actionDataGrid)
	{
	}
};

void initMenu();
void handleImGUI();
bool loadCharacterData(std::string dirName, characterData& charData);
int getSpriteNumFrames(const std::string spritePathStr);

void loadCharacterClickButton();

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();