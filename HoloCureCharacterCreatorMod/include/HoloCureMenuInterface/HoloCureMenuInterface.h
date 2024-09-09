#pragma once

#include "YYToolkit/Shared.hpp"
using namespace Aurie;
using namespace YYTK;

extern YYTKInterface* g_ModuleInterface;

class menuGridData;
class menuColumnData;
struct menuData;
struct spriteData;

struct HoloCureMenuInterface : AurieInterfaceBase
{
	virtual AurieStatus Create();
	virtual void Destroy();
	virtual void QueryVersion(
		OUT short& Major,
		OUT short& Minor,
		OUT short& Patch
	);

	/*
	* Call this to create a new menu grid. If prevMenuGrid is nullptr, it will add the menu to the title menu and return to the main menu
	*/
	virtual AurieStatus CreateMenuGrid(
		IN const std::string& ModName,
		IN std::string menuGridName,
		IN std::shared_ptr<menuGridData>& prevMenuGridPtr,
		OUT std::shared_ptr<menuGridData>& menuGridPtr
	);

	/*
	* Call this to delete a menu grid
	*/
	virtual AurieStatus DeleteMenuGrid(
		IN const std::string& ModName,
		IN std::shared_ptr<menuGridData>& menuGridPtr
	);

	/*
	* Call this to create a new menu column and add it to the menu grid
	*/
	virtual AurieStatus CreateMenuColumn(
		IN const std::string& ModName,
		IN std::shared_ptr<menuGridData>& menuGridPtr,
		OUT std::shared_ptr<menuColumnData>& menuColumnPtr
	);

	/*
	* Call this to add a menu data to a menu column
	*/
	virtual AurieStatus AddMenuData(
		IN const std::string& ModName,
		IN std::shared_ptr<menuColumnData>& menuColumnPtr,
		IN std::shared_ptr<menuData>& addMenuDataPtr
	);

	/*
	* Call this to swap the current menu grid the one passed in
	*/
	virtual AurieStatus SwapToMenuGrid(
		IN const std::string& ModName,
		IN std::shared_ptr<menuGridData>& menuGridPtr
	);

	/*
	* Call this to get the current selected menu data
	*/
	virtual AurieStatus GetSelectedMenuData(
		IN const std::string& ModName,
		OUT std::shared_ptr<menuData>& menuDataPtr
	);

	/*
	* Call this to get the current menu grid
	*/
	virtual AurieStatus GetCurrentMenuGrid(
		IN const std::string& ModName,
		OUT std::shared_ptr<menuGridData>& menuGridPtr
	);
};

enum MenuDataType
{
	MENUDATATYPE_NONE,
	MENUDATATYPE_Button,
	MENUDATATYPE_TextBoxField,
	MENUDATATYPE_NumberField,
	MENUDATATYPE_Image,
	MENUDATATYPE_Text,
	MENUDATATYPE_Selection
};

using menuFunc = void (*)(void);

struct menuData
{
	int xPos;
	int yPos;
	int width;
	int height;
	int fps;
	std::string menuID;
	std::string labelName;
	std::string textField;
	bool isVisible;
	menuFunc clickMenuFunc;
	menuFunc labelNameFunc;
	MenuDataType menuDataType;
	std::vector<std::string> selectionText;
	std::shared_ptr<spriteData> curSprite;
	int curSubImageIndex;
	int curFrameCount;
	int curSelectionTextIndex;

	menuData(int xPos, int yPos, int width, int height, std::string menuID, std::string labelName, bool isVisible, menuFunc clickMenuFunc, menuFunc labelNameFunc, MenuDataType menuDataType) :
		xPos(xPos), yPos(yPos), width(width), height(height), menuID(menuID), labelName(labelName), isVisible(isVisible), defaultLabelName(labelName),
		defaultIsVisible(isVisible), clickMenuFunc(clickMenuFunc), labelNameFunc(labelNameFunc), menuDataType(menuDataType), fps(0), curSubImageIndex(0), curFrameCount(-1), curSprite(nullptr),
		curSelectionTextIndex(0)
	{
	}

	menuData(int xPos, int yPos, std::string menuID, std::string labelName, bool isVisible, menuFunc clickMenuFunc, menuFunc labelNameFunc, MenuDataType menuDataType, int fps) :
		xPos(xPos), yPos(yPos), width(0), height(0), menuID(menuID), labelName(labelName), isVisible(isVisible), defaultLabelName(labelName),
		defaultIsVisible(isVisible), clickMenuFunc(clickMenuFunc), labelNameFunc(labelNameFunc), menuDataType(menuDataType), fps(fps), curSubImageIndex(0), curFrameCount(-1), curSprite(nullptr),
		curSelectionTextIndex(0)
	{
	}

	menuData(int xPos, int yPos, std::string menuID, std::string labelName, bool isVisible, menuFunc clickMenuFunc, menuFunc labelNameFunc, MenuDataType menuDataType, std::vector<std::string> selectionText) :
		xPos(xPos), yPos(yPos), width(180), height(20), menuID(menuID), labelName(labelName), isVisible(isVisible), defaultLabelName(labelName),
		defaultIsVisible(isVisible), clickMenuFunc(clickMenuFunc), labelNameFunc(labelNameFunc), menuDataType(menuDataType), fps(0), curSubImageIndex(0), curFrameCount(-1), curSprite(nullptr), selectionText(selectionText),
		curSelectionTextIndex(0)
	{
	}

	void resetToDefault();

private:
	std::string defaultLabelName;
	bool defaultIsVisible;
};

class menuColumnData
{
public:
	std::vector<std::shared_ptr<menuData>> menuDataPtrList;
	int curSelectedIndex;

	menuColumnData() : curSelectedIndex(0), defaultCurSelectedIndex(0)
	{
	}

	menuColumnData(std::vector< std::shared_ptr<menuData>> menuDataPtrList, int defaultCurSelectedIndex) :
		menuDataPtrList(menuDataPtrList), curSelectedIndex(defaultCurSelectedIndex), defaultCurSelectedIndex(defaultCurSelectedIndex)
	{
	}

	void addMenuData(std::shared_ptr<menuData> curMenuData)
	{
		menuDataPtrList.push_back(curMenuData);
	}

	std::shared_ptr<menuData>& getSelectedMenuData();

	int getMinVisibleMenuDataIndex(int curMenuDataIndex);

	int getMaxVisibleMenuDataIndex(int curMenuDataIndex);

	void resetToDefault();

private:
	int defaultCurSelectedIndex;
};

class menuGridData
{
public:
	std::vector<std::shared_ptr<menuColumnData>> menuColumnsPtrList;
	int curSelectedColumnIndex;
	std::shared_ptr<menuGridData> prevMenu;
	menuFunc onEnterFunc;

	menuGridData() : curSelectedColumnIndex(0), defaultCurSelectedColumnIndex(0), prevMenu(nullptr), onEnterFunc(nullptr)
	{
	}

	menuGridData(std::vector<std::shared_ptr<menuColumnData>> menuColumnsPtrList, int defaultCurSelectedColumnIndex, std::shared_ptr<menuGridData> prevMenu) :
		menuColumnsPtrList(menuColumnsPtrList), curSelectedColumnIndex(defaultCurSelectedColumnIndex), defaultCurSelectedColumnIndex(defaultCurSelectedColumnIndex), prevMenu(prevMenu), onEnterFunc(nullptr)
	{
	}

	menuGridData(std::vector<std::shared_ptr<menuColumnData>> menuColumnsPtrList, int defaultCurSelectedColumnIndex, std::shared_ptr<menuGridData> prevMenu, menuFunc onEnterFunc) :
		menuColumnsPtrList(menuColumnsPtrList), curSelectedColumnIndex(defaultCurSelectedColumnIndex), defaultCurSelectedColumnIndex(defaultCurSelectedColumnIndex), prevMenu(prevMenu), onEnterFunc(onEnterFunc)
	{
	}

	void addMenuColumn(std::shared_ptr<menuColumnData> menuColumnPtr)
	{
		menuColumnsPtrList.push_back(menuColumnPtr);
	}

	void draw(CInstance* Self);

	void processInput(bool isMouseLeftPressed, bool isMouseRightPressed, bool isActionOnePressed, bool isActionTwoPressed, bool isEnterPressed, bool isEscPressed,
		bool isMoveUpPressed, bool isMoveDownPressed, bool isMoveLeftPressed, bool isMoveRightPressed);

	std::shared_ptr<menuColumnData>& getSelectedMenuColumn();

	int getMinVisibleMenuColumnIndex(int curMenuColumnIndex);

	int getMaxVisibleMenuColumnIndex(int curMenuColumnIndex);

	void resetMenu();

private:
	int defaultCurSelectedColumnIndex;
};

struct spriteData
{
	std::string spritePath;
	std::string spriteFileName;
	RValue spriteRValue;
	int numFrames;

	spriteData(std::string spritePath, std::string spriteFileName, int numFrames) : spritePath(spritePath), spriteFileName(spriteFileName), numFrames(numFrames)
	{
		spriteRValue = g_ModuleInterface->CallBuiltin("sprite_add", { spritePath, numFrames, false, false, 0, 0 });
	}

	~spriteData()
	{
		if (spriteRValue.m_Kind != VALUE_UNDEFINED)
		{
			g_ModuleInterface->CallBuiltin("sprite_delete", { spriteRValue });
		}
	}
};