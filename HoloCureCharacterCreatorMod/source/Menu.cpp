#include "Menu.h"
#include "ModuleMain.h"
#include "CodeEvents.h"
#include "CommonFunctions.h"
#include "HoloCureMenuInterface/HoloCureMenuInterface.h"
#include "nlohmann/json.hpp"
#include <queue>
#include <fstream>
#include <regex>

extern CallbackManagerInterface* callbackManagerInterfacePtr;
extern HoloCureMenuInterface* holoCureMenuInterfacePtr;

int spriteDequePage = 0;
std::deque<std::shared_ptr<spriteData>> spriteDeque;

int loadCharacterPage = 0;
std::deque<std::string> loadCharacterDeque;

int weaponLevelMenuIndex = -1;
int skillMenuIndex = -1;
int skillLevelMenuIndex = -1;
int buffMenuIndex = -1;

std::shared_ptr<menuData> characterCreatorMenuLoadCharacter(new menuData(60, 104 + 29 * 0, 180, 29, "CHARACTERCREATORMENU_LoadCharacter", "Load Character", true, loadCharacterClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterCreatorMenuCharacterData(new menuData(60, 104 + 29 * 2, 180, 29, "CHARACTERCREATORMENU_CharacterData", "Character Data", true, characterDataClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterCreatorMenuWeaponData(new menuData(60, 104 + 29 * 3, 180, 29, "CHARACTERCREATORMENU_WeaponData", "Weapon Data", true, mainWeaponClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterCreatorMenuSkillData(new menuData(60, 104 + 29 * 4, 180, 29, "CHARACTERCREATORMENU_SkillData", "Skill Data", true, skillClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterCreatorMenuSpecialData(new menuData(60, 104 + 29 * 5, 180, 29, "CHARACTERCREATORMENU_SpecialData", "Special Data", true, specialClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterCreatorMenuBuffData(new menuData(60, 104 + 29 * 6, 180, 29, "CHARACTERCREATORMENU_BuffData", "Buff Data", true, buffClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterCreatorMenuExportCharacter(new menuData(60, 104 + 29 * 7, 180, 29, "CHARACTERCREATORMENU_ExportCharacter", "Export Character", true, exportCharacterClickButton, nullptr, MENUDATATYPE_Button));

menuGrid characterCreatorMenuGrid({
	menuColumn({
		characterCreatorMenuLoadCharacter,
		characterCreatorMenuCharacterData,
		characterCreatorMenuWeaponData,
		characterCreatorMenuSkillData,
		characterCreatorMenuSpecialData,
		characterCreatorMenuBuffData,
		characterCreatorMenuExportCharacter,
	}),
}, "Create Character", nullptr);

std::shared_ptr<menuData> loadCharacterMenuButton1(new menuData(60, 20 + 29 * 0, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton1", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuButton2(new menuData(60, 20 + 29 * 1, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton2", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuButton3(new menuData(60, 20 + 29 * 2, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton3", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuButton4(new menuData(60, 20 + 29 * 3, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton4", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuButton5(new menuData(60, 20 + 29 * 4, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton5", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuButton6(new menuData(60, 20 + 29 * 5, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton6", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuButton7(new menuData(60, 20 + 29 * 6, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton7", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuButton8(new menuData(60, 20 + 29 * 7, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton8", "", false, loadCharacterDataButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "LOADCHARACTERMENU_LoadCharacterPrevButton", "Prev", true, prevLoadCharacterButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> loadCharacterMenuNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "LOADCHARACTERMENU_LoadCharacterNextButton", "Next", true, nextLoadCharacterButton, nullptr, MENUDATATYPE_Button));

menuGrid loadCharacterMenuGrid({
	menuColumn({
		loadCharacterMenuButton1,
		loadCharacterMenuButton2,
		loadCharacterMenuButton3,
		loadCharacterMenuButton4,
		loadCharacterMenuButton5,
		loadCharacterMenuButton6,
		loadCharacterMenuButton7,
		loadCharacterMenuButton8,
		loadCharacterMenuPrevButton,
		loadCharacterMenuNextButton,
	}),
}, "Load Character", &characterCreatorMenuGrid);

std::shared_ptr<menuData> characterDataMenuCharName(new menuData(60, 20 + 29 * 0, 130, 20, "CHARACTERDATAMENU_CharName", "charName", true, nullptr, nullptr, MENUDATATYPE_TextBoxField));
std::shared_ptr<menuData> characterDataMenuCharPortrait(new menuData(60, 20 + 29 * 1, 180, 20, "CHARACTERDATAMENU_CharacterPortrait", "Portrait", true, portraitClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterDataMenuCharLargePortrait(new menuData(60, 20 + 29 * 2, 180, 20, "CHARACTERDATAMENU_CharacterLargePortrait", "Large Portrait", true, largePortraitClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterDataMenuIdleAnimation(new menuData(60, 20 + 29 * 3, 180, 20, "CHARACTERDATAMENU_IdleAnimation", "Idle Animation", true, idleAnimationClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterDataMenuRunAnimation(new menuData(60, 20 + 29 * 4, 180, 20, "CHARACTERDATAMENU_RunAnimation", "Run Animation", true, runAnimationClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> characterDataMenuHP(new menuData(60, 20 + 29 * 5, 130, 20, "CHARACTERDATAMENU_HP", "HP", true, nullptr, nullptr, MENUDATATYPE_NumberField));
std::shared_ptr<menuData> characterDataMenuATK(new menuData(60, 20 + 29 * 6, 130, 20, "CHARACTERDATAMENU_ATK", "ATK", true, nullptr, nullptr, MENUDATATYPE_NumberField));
std::shared_ptr<menuData> characterDataMenuSPD(new menuData(60, 20 + 29 * 7, 130, 20, "CHARACTERDATAMENU_SPD", "SPD", true, nullptr, nullptr, MENUDATATYPE_NumberField));
std::shared_ptr<menuData> characterDataMenuCrit(new menuData(60, 20 + 29 * 8, 130, 20, "CHARACTERDATAMENU_Crit", "crit", true, nullptr, nullptr, MENUDATATYPE_NumberField));
std::shared_ptr<menuData> characterDataMenuSizeGrade(new menuData(330, 20 + 29 * 0, 130, 20, "CHARACTERDATAMENU_SizeGrade", "Size Grade", true, nullptr, nullptr, MENUDATATYPE_NumberField));

menuGrid characterDataMenuGrid({
	menuColumn({
		characterDataMenuCharName,
		characterDataMenuCharPortrait,
		characterDataMenuCharLargePortrait,
		characterDataMenuIdleAnimation,
		characterDataMenuRunAnimation,
		characterDataMenuHP,
		characterDataMenuATK,
		characterDataMenuSPD,
		characterDataMenuCrit,
	}),
	menuColumn({
		characterDataMenuSizeGrade,
	}),
}, "Character Data", &characterCreatorMenuGrid);

std::shared_ptr<menuData> portraitMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "PORTRAITMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "PORTRAITMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "PORTRAITMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "PORTRAITMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "PORTRAITMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "PORTRAITMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "PORTRAITMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "PORTRAITMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "PORTRAITMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuIconNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "PORTRAITMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> portraitMenuPreviewImage(new menuData(250, 20 + 29 * 0, 0, 0, "PORTRAITMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image));


menuGrid portraitMenuGrid({
	menuColumn({
		portraitMenuIconButton1,
		portraitMenuIconButton2,
		portraitMenuIconButton3,
		portraitMenuIconButton4,
		portraitMenuIconButton5,
		portraitMenuIconButton6,
		portraitMenuIconButton7,
		portraitMenuIconButton8,
		portraitMenuIconPrevButton,
		portraitMenuIconNextButton,
	}),
	menuColumn({
		portraitMenuPreviewImage,
	}),
}, "Portrait", &characterDataMenuGrid);

std::shared_ptr<menuData> largePortraitMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "LARGEPORTRAITMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "LARGEPORTRAITMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "LARGEPORTRAITMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "LARGEPORTRAITMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "LARGEPORTRAITMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "LARGEPORTRAITMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "LARGEPORTRAITMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "LARGEPORTRAITMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "LARGEPORTRAITMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "LARGEPORTRAITMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> largePortraitMenuPreviewImage(new menuData(250, 20 + 29 * 0, 0, 0, "LARGEPORTRAITMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image));

menuGrid largePortraitMenuGrid({
	menuColumn({
		largePortraitMenuIconButton1,
		largePortraitMenuIconButton2,
		largePortraitMenuIconButton3,
		largePortraitMenuIconButton4,
		largePortraitMenuIconButton5,
		largePortraitMenuIconButton6,
		largePortraitMenuIconButton7,
		largePortraitMenuIconButton8,
		largePortraitMenuPrevButton,
		largePortraitMenuNextButton,
	}),
	menuColumn({
		largePortraitMenuPreviewImage,
	}),
}, "Large Portrait", &characterDataMenuGrid);

std::shared_ptr<menuData> idleAnimationMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "IDLEANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "IDLEANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "IDLEANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "IDLEANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "IDLEANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "IDLEANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "IDLEANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "IDLEANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "IDLEANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuPreviewImage(new menuData(250, 20 + 29 * 2, "IDLEANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image, 4));
std::shared_ptr<menuData> idleAnimationMenuPrevImageButton(new menuData(250, 20 + 29 * 0, 180, 20, "IDLEANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuNextImageButton(new menuData(430, 20 + 29 * 0, 180, 20, "IDLEANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuPlayButton(new menuData(250, 20 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuPauseButton(new menuData(250, 20 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> idleAnimationMenuFrameNumberText(new menuData(430, 30 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText, MENUDATATYPE_Text));

menuGrid idleAnimationMenuGrid({
	menuColumn({
		idleAnimationMenuIconButton1,
		idleAnimationMenuIconButton2,
		idleAnimationMenuIconButton3,
		idleAnimationMenuIconButton4,
		idleAnimationMenuIconButton5,
		idleAnimationMenuIconButton6,
		idleAnimationMenuIconButton7,
		idleAnimationMenuIconButton8,
		idleAnimationMenuPrevButton,
		idleAnimationMenuNextButton,
	}),
	menuColumn({
		idleAnimationMenuPreviewImage,
		idleAnimationMenuPrevImageButton,
		idleAnimationMenuNextImageButton,
		idleAnimationMenuPlayButton,
		idleAnimationMenuPauseButton,
		idleAnimationMenuFrameNumberText,
	}),
}, "Idle Animation", &characterDataMenuGrid);

std::shared_ptr<menuData> runAnimationMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "RUNANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "RUNANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "RUNANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "RUNANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "RUNANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "RUNANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "RUNANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "RUNANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "RUNANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "RUNANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuPreviewImage(new menuData(250, 20 + 29 * 2, "RUNANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image, 12));
std::shared_ptr<menuData> runAnimationMenuPrevImageButton(new menuData(250, 20 + 29 * 0, 180, 20, "RUNANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuNextImageButton(new menuData(430, 20 + 29 * 0, 180, 20, "RUNANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuPlayButton(new menuData(250, 20 + 29 * 1, 180, 20, "RUNANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuPauseButton(new menuData(250, 20 + 29 * 1, 180, 20, "RUNANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> runAnimationMenuFrameNumberText(new menuData(430, 30 + 29 * 1, 180, 20, "RUNANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText, MENUDATATYPE_Text));

menuGrid runAnimationMenuGrid({
	menuColumn({
		runAnimationMenuIconButton1,
		runAnimationMenuIconButton2,
		runAnimationMenuIconButton3,
		runAnimationMenuIconButton4,
		runAnimationMenuIconButton5,
		runAnimationMenuIconButton6,
		runAnimationMenuIconButton7,
		runAnimationMenuIconButton8,
		runAnimationMenuPrevButton,
		runAnimationMenuNextButton,
	}),
	menuColumn({
		runAnimationMenuPreviewImage,
		runAnimationMenuPrevImageButton,
		runAnimationMenuNextImageButton,
		runAnimationMenuPlayButton,
		runAnimationMenuPauseButton,
		runAnimationMenuFrameNumberText,
	}),
}, "Run Animation", &characterDataMenuGrid);

std::shared_ptr<menuData> weaponDataMenuWeaponName(new menuData(60, 20 + 29 * 0, 130, 20, "WEAPONDATAMENU_WeaponName", "weaponName", true, nullptr, nullptr, MENUDATATYPE_TextBoxField));
std::shared_ptr<menuData> weaponDataMenuWeaponIcon(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONDATAMENU_WeaponIcon", "weaponIcon", true, weaponIconClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponDataMenuAwakenedWeaponIcon(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONDATAMENU_AwakenedWeaponIcon", "Awakened weaponIcon", true, weaponAwakenedIconClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponDataMenuWeaponLevelMenu(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONDATAMENU_WeaponLevelMenu", "Weapon Levels", true, weaponLevelMenuClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponDataMenuWeaponAnimation(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONDATAMENU_WeaponAnimation", "Weapon Animation", true, weaponAnimationClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponDataMenuWeaponType(new menuData(60, 20 + 29 * 5, "WEAPONDATAMENU_WeaponType", "Weapon Type", true, nullptr, nullptr, MENUDATATYPE_Selection, { "Melee", "Multishot", "Ranged" }));
// TODO: Implement later
//std::shared_ptr<menuData> weaponDataMenuOnTrigger(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONDATAMENU_OnTrigger", "On Trigger Menu", true, nullptr, nullptr, MENUDATATYPE_Button));
//std::shared_ptr<menuData> weaponDataMenuSubWeapon(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONDATAMENU_SubWeapon", "Sub Weapon Menu", true, nullptr, nullptr, MENUDATATYPE_Button));
menuGrid weaponDataMenuGrid({
	menuColumn({
		weaponDataMenuWeaponName,
		weaponDataMenuWeaponIcon,
		weaponDataMenuAwakenedWeaponIcon,
		weaponDataMenuWeaponLevelMenu,
		weaponDataMenuWeaponAnimation,
		weaponDataMenuWeaponType,
	}),
}, "Weapon Data", &characterCreatorMenuGrid);

std::shared_ptr<menuData> weaponIconMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "WEAPONICONMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONICONMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONICONMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONICONMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONICONMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONICONMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONICONMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONICONMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONICONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponIconMenuIconPreviewImage(new menuData(250, 20 + 29 * 0, 0, 0, "WEAPONICONMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image));

menuGrid weaponIconMenuGrid({
	menuColumn({
		weaponIconMenuIconButton1,
		weaponIconMenuIconButton2,
		weaponIconMenuIconButton3,
		weaponIconMenuIconButton4,
		weaponIconMenuIconButton5,
		weaponIconMenuIconButton6,
		weaponIconMenuIconButton7,
		weaponIconMenuIconButton8,
		weaponIconMenuIconPrevButton,
		weaponIconMenuIconNextButton,
	}),
	menuColumn({
		weaponIconMenuIconPreviewImage,
	}),
}, "Weapon Icon", &weaponDataMenuGrid);

std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONAWAKENEDICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONAWAKENEDICONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconPreviewImage(new menuData(250, 20 + 29 * 0, 0, 0, "WEAPONAWAKENEDICONMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image));

menuGrid weaponAwakenedIconMenuGrid({
	menuColumn({
		weaponAwakenedIconMenuIconButton1,
		weaponAwakenedIconMenuIconButton2,
		weaponAwakenedIconMenuIconButton3,
		weaponAwakenedIconMenuIconButton4,
		weaponAwakenedIconMenuIconButton5,
		weaponAwakenedIconMenuIconButton6,
		weaponAwakenedIconMenuIconButton7,
		weaponAwakenedIconMenuIconButton8,
		weaponAwakenedIconMenuIconPrevButton,
		weaponAwakenedIconMenuIconNextButton,
	}),
	menuColumn({
		weaponAwakenedIconMenuIconPreviewImage,
	}),
}, "Weapon Awakened Icon", &weaponDataMenuGrid);

std::shared_ptr<menuData> weaponAnimationMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "WEAPONANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuIconNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuPreviewImage(new menuData(250, 20 + 29 * 2, "WEAPONANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image, 30));
std::shared_ptr<menuData> weaponAnimationMenuPrevImageButton(new menuData(250, 20 + 29 * 0, 180, 20, "WEAPONANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuNextImageButton(new menuData(430, 20 + 29 * 0, 180, 20, "WEAPONANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuPlayButton(new menuData(250, 20 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuPauseButton(new menuData(250, 20 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> weaponAnimationMenuFrameNumberText(new menuData(430, 30 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText, MENUDATATYPE_Text));

menuGrid weaponAnimationMenuGrid({
	menuColumn({
		weaponAnimationMenuIconButton1,
		weaponAnimationMenuIconButton2,
		weaponAnimationMenuIconButton3,
		weaponAnimationMenuIconButton4,
		weaponAnimationMenuIconButton5,
		weaponAnimationMenuIconButton6,
		weaponAnimationMenuIconButton7,
		weaponAnimationMenuIconButton8,
		weaponAnimationMenuIconPrevButton,
		weaponAnimationMenuIconNextButton,
	}),
	menuColumn({
		weaponAnimationMenuPreviewImage,
		weaponAnimationMenuPrevImageButton,
		weaponAnimationMenuNextImageButton,
		weaponAnimationMenuPlayButton,
		weaponAnimationMenuPauseButton,
		weaponAnimationMenuFrameNumberText,
	}),
}, "Weapon Animation", &weaponDataMenuGrid);

std::vector<std::shared_ptr<menuData>> weaponLevelMenuButtonList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 0, 180, 20, "WEAPONLEVELMENU_Button1", "Level 1", true, weaponLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELMENU_Button2", "Level 2", true, weaponLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELMENU_Button3", "Level 3", true, weaponLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELMENU_Button4", "Level 4", true, weaponLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELMENU_Button5", "Level 5", true, weaponLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELMENU_Button6", "Level 6", true, weaponLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELMENU_Button7", "Level 7", true, weaponLevelClickButton, nullptr, MENUDATATYPE_Button)),
});

menuGrid weaponLevelMenuGrid({
	menuColumn(weaponLevelMenuButtonList),
}, "Weapon Level", &weaponDataMenuGrid);

std::shared_ptr<menuData> weaponLevelDataMenuDescription(new menuData(60, 20 + 29 * 0, 180, 20, "WEAPONLEVELDATAMENU_LevelDescription", "Level Description", true, weaponLevelDescriptionClickButton, nullptr, MENUDATATYPE_Button));

std::vector<std::shared_ptr<menuData>> weaponLevelAttackTimeMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime1", "Attack Time", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime2", "Attack Time", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime3", "Attack Time", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime4", "Attack Time", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime5", "Attack Time", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime6", "Attack Time", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime7", "Attack Time", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelDurationMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration1", "Duration", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration2", "Duration", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration3", "Duration", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration4", "Duration", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration5", "Duration", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration6", "Duration", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration7", "Duration", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelDamageMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage1", "Damage", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage2", "Damage", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage3", "Damage", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage4", "Damage", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage5", "Damage", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage6", "Damage", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage7", "Damage", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelHitLimitMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit1", "Hit Limit", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit2", "Hit Limit", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit3", "Hit Limit", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit4", "Hit Limit", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit5", "Hit Limit", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit6", "Hit Limit", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit7", "Hit Limit", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelSpeedMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed1", "Speed", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed2", "Speed", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed3", "Speed", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed4", "Speed", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed5", "Speed", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed6", "Speed", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed7", "Speed", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelHitCDMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD1", "HitCD", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD2", "HitCD", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD3", "HitCD", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD4", "HitCD", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD5", "HitCD", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD6", "HitCD", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD7", "HitCD", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelAttackCountMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount1", "Count", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount2", "Count", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount3", "Count", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount4", "Count", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount5", "Count", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount6", "Count", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount7", "Count", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelAttackDelayMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay1", "Delay", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay2", "Delay", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay3", "Delay", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay4", "Delay", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay5", "Delay", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay6", "Delay", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay7", "Delay", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelRangeMenuList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range1", "Range", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range2", "Range", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range3", "Range", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range4", "Range", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range5", "Range", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range6", "Range", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range7", "Range", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

menuGrid weaponLevelDataMenuGrid(
	[]() -> std::vector<menuColumn> {
		std::vector<menuColumn> menuColumnList;
		for (int i = 0; i < 7; i++)
		{
			menuColumn curMenuColumn({
				weaponLevelDataMenuDescription,
				weaponLevelAttackTimeMenuList[i],
				weaponLevelDurationMenuList[i],
				weaponLevelDamageMenuList[i],
				weaponLevelHitLimitMenuList[i],
				weaponLevelSpeedMenuList[i],
				weaponLevelHitCDMenuList[i],
				weaponLevelAttackCountMenuList[i],
				weaponLevelAttackDelayMenuList[i],
				weaponLevelRangeMenuList[i]
			});
			menuColumnList.push_back(curMenuColumn);
		}
		return menuColumnList;
	}()
, "Weapon Level Data", &weaponLevelMenuGrid);

std::vector<std::shared_ptr<menuData>> weaponLevelDescriptionMenuList({
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description1", "Weapon Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description2", "Weapon Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description3", "Weapon Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description4", "Weapon Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description5", "Weapon Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description6", "Weapon Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description7", "Weapon Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
});

menuGrid weaponLevelDescriptionMenuGrid({
	menuColumn(weaponLevelDescriptionMenuList),
}, "Weapon Level Description", &weaponLevelDataMenuGrid);

std::vector<std::shared_ptr<menuData>> skillsMenuButtonList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 0, 180, 20, "SKILLSMENU_Button1", "Skill 1", true, skillMenuClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "SKILLSMENU_Button2", "Skill 2", true, skillMenuClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "SKILLSMENU_Button3", "Skill 3", true, skillMenuClickButton, nullptr, MENUDATATYPE_Button)),
});

menuGrid skillsMenuGrid({
	menuColumn({
		skillsMenuButtonList[0],
		skillsMenuButtonList[1],
		skillsMenuButtonList[2]
	}),
}, "Skills Menu", &characterCreatorMenuGrid);

std::shared_ptr<menuData> skillIconMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "SKILLICONMENU_IconButton1", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "SKILLICONMENU_IconButton2", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "SKILLICONMENU_IconButton3", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "SKILLICONMENU_IconButton4", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "SKILLICONMENU_IconButton5", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "SKILLICONMENU_IconButton6", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "SKILLICONMENU_IconButton7", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "SKILLICONMENU_IconButton8", "", false, clickSkillIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "SKILLICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> skillIconMenuIconNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "SKILLICONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::vector<std::shared_ptr<menuData>> skillIconMenuIconPreviewImageList({
	std::shared_ptr<menuData>(new menuData(250, 20 + 29 * 0, 0, 0, "SKILLICONMENU_PreviewImage", "", false, nullptr, nullptr, MENUDATATYPE_Image)),
	std::shared_ptr<menuData>(new menuData(250, 20 + 29 * 0, 0, 0, "SKILLICONMENU_PreviewImage", "", false, nullptr, nullptr, MENUDATATYPE_Image)),
	std::shared_ptr<menuData>(new menuData(250, 20 + 29 * 0, 0, 0, "SKILLICONMENU_PreviewImage", "", false, nullptr, nullptr, MENUDATATYPE_Image)),
});

std::shared_ptr<menuData> skillsDataMenuIconButton(new menuData(330, 20 + 29 * 0, 180, 20, "SKILLSMENU_IconButton", "Skill Icon", true, skillIconMenuClickButton, nullptr, MENUDATATYPE_Button));
std::vector<std::shared_ptr<menuData>> skillsDataMenuNameList({
	std::shared_ptr<menuData>(new menuData(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_Name1", "Skill Name", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_Name2", "Skill Name", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_Name3", "Skill Name", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	});
std::vector<std::shared_ptr<menuData>> skillsDataMenuLevelList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 0, 180, 20, "SKILLSMENU_Level1", "Level 1", true, skillLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "SKILLSMENU_Level2", "Level 2", true, skillLevelClickButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "SKILLSMENU_Level3", "Level 3", true, skillLevelClickButton, nullptr, MENUDATATYPE_Button)),
});

menuGrid skillsDataMenuGrid({
	menuColumn(skillsDataMenuLevelList),
	menuColumn({
		skillsDataMenuIconButton,
		skillsDataMenuNameList[0],
		skillsDataMenuNameList[1],
		skillsDataMenuNameList[2],
	}),
}, "Skills Data Menu", &skillsMenuGrid);

menuGrid skillIconMenuGrid({
	menuColumn({
		skillIconMenuIconButton1,
		skillIconMenuIconButton2,
		skillIconMenuIconButton3,
		skillIconMenuIconButton4,
		skillIconMenuIconButton5,
		skillIconMenuIconButton6,
		skillIconMenuIconButton7,
		skillIconMenuIconButton8,
		skillIconMenuIconPrevButton,
		skillIconMenuIconNextButton,
	}),
	menuColumn(skillIconMenuIconPreviewImageList),
}, "Skills Icon Menu", &skillsDataMenuGrid);

std::shared_ptr<menuData> skillsDataLevelMenuDescription(new menuData(330, 20 + 29 * 0, 180, 20, "SKILLSMENU_Description", "Skill Description", true, skillDescriptionClickButton, nullptr, MENUDATATYPE_Button));

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuAttackButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK1_Level1", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK1_Level2", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK1_Level3", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK2_Level1", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK2_Level2", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK2_Level3", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK3_Level1", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK3_Level2", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK3_Level3", "ATK+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuCritButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT1_Level1", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT1_Level2", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT1_Level3", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT2_Level1", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT2_Level2", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT2_Level3", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT3_Level1", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT3_Level2", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT3_Level3", "CRIT+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuHasteButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE1_Level1", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE1_Level2", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE1_Level3", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE2_Level1", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE2_Level2", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE2_Level3", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE3_Level1", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE3_Level2", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE3_Level3", "Haste+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuSpeedButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD1_Level1", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD1_Level2", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD1_Level3", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD2_Level1", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD2_Level2", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD2_Level3", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD3_Level1", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD3_Level2", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD3_Level3", "SPD+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuDRButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR1_Level1", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR1_Level2", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR1_Level3", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR2_Level1", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR2_Level2", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR2_Level3", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR3_Level1", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR3_Level2", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR3_Level3", "DR*", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuHealMultiplierButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier1_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier1_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier1_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier2_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier2_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier2_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier3_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier3_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier3_Level1", "healMultiplier+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuFoodButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food1_Level1", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food1_Level2", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food1_Level3", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food2_Level1", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food2_Level2", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food2_Level3", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food3_Level1", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food3_Level2", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food3_Level3", "food+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuWeaponSizeMultiplierButtonList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier1_Level1", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier1_Level2", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier1_Level3", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier2_Level1", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier2_Level2", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier2_Level3", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier3_Level1", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier3_Level2", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier3_Level3", "weaponSizeMult+", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

std::shared_ptr<menuData> skillsDataLevelMenuOnTriggerButton(new menuData(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_OnTrigger", "OnTrigger", true, skillOnTriggerClickButton, nullptr, MENUDATATYPE_Button));

menuGrid skillsDataLevelMenuGrid(
	[]() -> std::vector<menuColumn> {
		std::vector<menuColumn> menuColumnList;
		for (int i = 0; i < 9; i++)
		{
			menuColumn curMenuColumn({
				skillsDataLevelMenuAttackButtonList[i],
				skillsDataLevelMenuCritButtonList[i],
				skillsDataLevelMenuHasteButtonList[i],
				skillsDataLevelMenuSpeedButtonList[i],
				skillsDataLevelMenuDRButtonList[i],
				skillsDataLevelMenuHealMultiplierButtonList[i],
				skillsDataLevelMenuFoodButtonList[i],
				skillsDataLevelMenuWeaponSizeMultiplierButtonList[i],
			});
			menuColumnList.push_back(curMenuColumn);
		}
		menuColumnList.push_back(menuColumn({ skillsDataLevelMenuDescription, skillsDataLevelMenuOnTriggerButton }));
		return menuColumnList;
	}()
, "Skills Data Level Menu", &skillsDataMenuGrid);

std::vector<std::shared_ptr<menuData>> skillDataLevelDescriptionMenuList({
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description1_Level1", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description1_Level2", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description1_Level3", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description2_Level1", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description2_Level2", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description2_Level3", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description3_Level1", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description3_Level2", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
	std::shared_ptr<menuData>(new menuData(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description3_Level3", "Skill Description", false, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
});

menuGrid skillDataLevelDescriptionMenuGrid({
	menuColumn(skillDataLevelDescriptionMenuList),
}, "Skills Description Menu", &skillsDataLevelMenuGrid);

std::vector<std::shared_ptr<menuData>> skillOnTriggerBuffSelectionMenuList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection1_Level1", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection1_Level2", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection1_Level3", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection2_Level1", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection2_Level2", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection2_Level3", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection3_Level1", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection3_Level2", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, "SKILLONTRIGGERMENU_BuffSelection3_Level3", "Select Buff", false, nullptr, nullptr, MENUDATATYPE_Selection, { "" })),
});

// TODO: Should probably improve the onTrigger selection to not need to default to none
std::vector<std::string> onTriggerTypes{ "NONE", "onDebuff", "onAttackCreate", "onCriticalHit", "onHeal", "onKill", "onTakeDamage", "onDodge" };

std::vector<std::shared_ptr<menuData>> skillOnTriggerTypeSelectionMenuList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection1_Level1", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection1_Level2", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection1_Level3", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection2_Level1", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection2_Level2", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection2_Level3", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection3_Level1", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection3_Level2", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, "SKILLONTRIGGERMENU_TypeSelection3_Level3", "onTrigger", false, nullptr, nullptr, MENUDATATYPE_Selection, onTriggerTypes)),
});

std::vector<std::shared_ptr<menuData>> skillOnTriggerProbabilityMenuList({
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability1_Level1", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability1_Level2", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability1_Level3", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability2_Level1", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability2_Level2", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability2_Level3", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability3_Level1", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability3_Level2", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
	std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability3_Level3", "Probability", false, nullptr, nullptr, MENUDATATYPE_NumberField)),
});

menuGrid skillOnTriggerMenuGrid({
	[]() -> std::vector<menuColumn> {
		std::vector<menuColumn> menuColumnList;
		for (int i = 0; i < 9; i++)
		{
			menuColumn curMenuColumn({
				skillOnTriggerBuffSelectionMenuList[i],
				skillOnTriggerTypeSelectionMenuList[i],
				skillOnTriggerProbabilityMenuList[i],
			});
			menuColumnList.push_back(curMenuColumn);
		}
		return menuColumnList;
	}(),
}, "Skills OnTrigger Menu", &skillsDataLevelMenuGrid);

std::shared_ptr<menuData> specialMenuSpecialAnimation(new menuData(100, 20 + 29 * 0, 180, 20, "SPECIALMENU_SpecialAnimation", "Special Animation", true, specialAnimationClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialMenuDamage(new menuData(100, 20 + 29 * 1, 180, 20, "SPECIALMENU_Damage", "Damage", true, nullptr, nullptr, MENUDATATYPE_NumberField));
std::shared_ptr<menuData> specialMenuDuration(new menuData(100, 20 + 29 * 2, 180, 20, "SPECIALMENU_Duration", "Duration", true, nullptr, nullptr, MENUDATATYPE_NumberField));
std::shared_ptr<menuData> specialMenuSpecialIcon(new menuData(100, 20 + 29 * 3, 180, 20, "SPECIALMENU_SpecialIcon", "Special Icon", true, specialIconClickButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialMenuSpecialCooldown(new menuData(100, 20 + 29 * 4, 130, 20, "SPECIALMENU_SpecialCooldown", "Special Cooldown", true, nullptr, nullptr, MENUDATATYPE_NumberField));
std::shared_ptr<menuData> specialMenuSpecialName(new menuData(100, 20 + 29 * 5, 130, 20, "SPECIALMENU_SpecialName", "Special Name", true, nullptr, nullptr, MENUDATATYPE_TextBoxField));
std::shared_ptr<menuData> specialMenuSpecialDescription(new menuData(100, 20 + 29 * 6, 180, 20, "SPECIALMENU_SpecialDesc", "Special Description", true, specialDescriptionClickButton, nullptr, MENUDATATYPE_Button));

menuGrid specialMenuGrid({
	menuColumn({
		specialMenuSpecialAnimation,
		specialMenuDamage,
		specialMenuDuration,
		specialMenuSpecialIcon,
		specialMenuSpecialCooldown,
		specialMenuSpecialName,
		specialMenuSpecialDescription
	}),
}, "Special Menu", &characterCreatorMenuGrid);

std::shared_ptr<menuData> specialAnimationMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "SPECIALANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "SPECIALANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "SPECIALANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "SPECIALANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "SPECIALANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "SPECIALANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "SPECIALANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "SPECIALANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuIconNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "SPECIALANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuPreviewImage(new menuData(250, 20 + 29 * 2, "SPECIALANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image, 30));
std::shared_ptr<menuData> specialAnimationMenuPrevImageButton(new menuData(250, 20 + 29 * 0, 180, 20, "SPECIALANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuNextImageButton(new menuData(430, 20 + 29 * 0, 180, 20, "SPECIALANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuPlayButton(new menuData(250, 20 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuPauseButton(new menuData(250, 20 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialAnimationMenuFrameNumberText(new menuData(430, 30 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText, MENUDATATYPE_Text));

menuGrid specialAnimationMenuGrid({
	menuColumn({
		specialAnimationMenuIconButton1,
		specialAnimationMenuIconButton2,
		specialAnimationMenuIconButton3,
		specialAnimationMenuIconButton4,
		specialAnimationMenuIconButton5,
		specialAnimationMenuIconButton6,
		specialAnimationMenuIconButton7,
		specialAnimationMenuIconButton8,
		specialAnimationMenuIconPrevButton,
		specialAnimationMenuIconNextButton,
	}),
	menuColumn({
		specialAnimationMenuPreviewImage,
		specialAnimationMenuPrevImageButton,
		specialAnimationMenuNextImageButton,
		specialAnimationMenuPlayButton,
		specialAnimationMenuPauseButton,
		specialAnimationMenuFrameNumberText,
	}),
}, "Special Animation", &specialMenuGrid);


std::shared_ptr<menuData> specialIconMenuIconButton1(new menuData(60, 20 + 29 * 0, 180, 20, "SPECIALICONMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconButton2(new menuData(60, 20 + 29 * 1, 180, 20, "SPECIALICONMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconButton3(new menuData(60, 20 + 29 * 2, 180, 20, "SPECIALICONMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconButton4(new menuData(60, 20 + 29 * 3, 180, 20, "SPECIALICONMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconButton5(new menuData(60, 20 + 29 * 4, 180, 20, "SPECIALICONMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconButton6(new menuData(60, 20 + 29 * 5, 180, 20, "SPECIALICONMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconButton7(new menuData(60, 20 + 29 * 6, 180, 20, "SPECIALICONMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconButton8(new menuData(60, 20 + 29 * 7, 180, 20, "SPECIALICONMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "SPECIALICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "SPECIALICONMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> specialIconMenuIconPreviewImage(new menuData(250, 20 + 29 * 0, 0, 0, "SPECIALICONMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image));

menuGrid specialIconMenuGrid({
	menuColumn({
		specialIconMenuIconButton1,
		specialIconMenuIconButton2,
		specialIconMenuIconButton3,
		specialIconMenuIconButton4,
		specialIconMenuIconButton5,
		specialIconMenuIconButton6,
		specialIconMenuIconButton7,
		specialIconMenuIconButton8,
		specialIconMenuIconPrevButton,
		specialIconMenuIconNextButton,
	}),
	menuColumn({
		specialIconMenuIconPreviewImage,
	}),
}, "Special Icon", &specialMenuGrid);

std::shared_ptr<menuData> specialDescriptionMenuSpecialDescription(new menuData(150, 20 + 29 * 0, 300, 200, "SPECIALDESCRIPTIONMENU_SpecialDescription", "Special Description", true, nullptr, nullptr, MENUDATATYPE_TextBoxField));

menuGrid specialDescriptionMenuGrid({
	menuColumn({
		specialDescriptionMenuSpecialDescription
	}),
}, "Special Description", &specialMenuGrid);

int buffMenuPage = 0;
std::deque<buffDataMenuGrid> buffMenuDeque;

std::vector<std::shared_ptr<menuData>> buffMenuButtonList({
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 0, 180, 20, "BUFFMENU_Button1", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "BUFFMENU_Button2", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "BUFFMENU_Button3", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "BUFFMENU_Button4", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "BUFFMENU_Button5", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "BUFFMENU_Button6", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "BUFFMENU_Button7", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
	std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "BUFFMENU_Button8", "", false, clickBuffMenuButton, nullptr, MENUDATATYPE_Button)),
});
std::shared_ptr<menuData> buffMenuPrevButton(new menuData(60, 20 + 29 * 8, 180, 20, "BUFFMENU_PrevButton", "Prev", true, prevBuffButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> buffMenuNextButton(new menuData(60, 20 + 29 * 9, 180, 20, "BUFFMENU_NextButton", "Next", true, nextBuffButton, nullptr, MENUDATATYPE_Button));
std::shared_ptr<menuData> buffMenuAddBuffButton(new menuData(330, 20 + 29 * 9, 180, 20, "BUFFMENU_AddBuffButton", "Add Buff", true, addBuffMenuButton, nullptr, MENUDATATYPE_Button));

menuGrid buffMenuGrid({
	menuColumn({
		buffMenuButtonList[0],
		buffMenuButtonList[1],
		buffMenuButtonList[2],
		buffMenuButtonList[3],
		buffMenuButtonList[4],
		buffMenuButtonList[5],
		buffMenuButtonList[6],
		buffMenuButtonList[7],
		buffMenuPrevButton,
		buffMenuNextButton,
	}),
	menuColumn({
		buffMenuAddBuffButton
	}),
}, "Buff", &characterCreatorMenuGrid, reloadBuffs);

void initMenu()
{
	menuGrid nullptrMenuGrid;
	characterCreatorMenuGrid.initMenuGrid();
	loadCharacterMenuGrid.initMenuGrid();
	characterDataMenuGrid.initMenuGrid();
	portraitMenuGrid.initMenuGrid();
	largePortraitMenuGrid.initMenuGrid();
	idleAnimationMenuGrid.initMenuGrid();
	runAnimationMenuGrid.initMenuGrid();
	weaponDataMenuGrid.initMenuGrid();
	weaponIconMenuGrid.initMenuGrid();
	weaponAwakenedIconMenuGrid.initMenuGrid();
	weaponAnimationMenuGrid.initMenuGrid();
	weaponLevelMenuGrid.initMenuGrid();
	weaponLevelDataMenuGrid.initMenuGrid();
	weaponLevelDescriptionMenuGrid.initMenuGrid();
	skillsMenuGrid.initMenuGrid();
	skillsDataMenuGrid.initMenuGrid();
	skillIconMenuGrid.initMenuGrid();
	skillsDataLevelMenuGrid.initMenuGrid();
	skillDataLevelDescriptionMenuGrid.initMenuGrid();
	skillOnTriggerMenuGrid.initMenuGrid();
	specialMenuGrid.initMenuGrid();
	specialAnimationMenuGrid.initMenuGrid();
	specialIconMenuGrid.initMenuGrid();
	specialDescriptionMenuGrid.initMenuGrid();
	buffMenuGrid.initMenuGrid();
}

bool loadCharacterData(std::string dirName, characterData& charData)
{
	if (!std::filesystem::exists("CharacterCreatorMod/" + dirName))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the %s directory", dirName.c_str());
		g_ModuleInterface->Print(CM_RED, "Couldn't find the %s directory", dirName.c_str());
		return false;
	}

	if (!std::filesystem::exists("CharacterCreatorMod/" + dirName + "/charData.json"))
	{
		return false;
	}

	callbackManagerInterfacePtr->LogToFile(MODNAME, "Loading %s", dirName.c_str());
	g_ModuleInterface->Print(CM_WHITE, "Loading %s", dirName.c_str());
	std::ifstream inFile;
	inFile.open("CharacterCreatorMod/" + dirName + "/charData.json");
	nlohmann::json inputData = nlohmann::json::parse(inFile);
	charData = inputData.template get<characterData>();
	
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

void loadCharacterDataButton()
{
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	auto dirName = selectedMenuData->labelName;
	characterData charData;
	if (loadCharacterData(dirName, charData))
	{
		std::string dirStr = "CharacterCreatorMod/" + dirName + "/";
		holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, loadCharacterMenuGrid.menuGridPtr->prevMenu);
		characterDataMenuCharName->textField = charData.charName;
		portraitMenuPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.portraitFileName, charData.portraitFileName, 1));
		largePortraitMenuPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.largePortraitFileName, charData.largePortraitFileName, 1));

		idleAnimationMenuPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.idleAnimationFileName, charData.idleAnimationFileName, getSpriteNumFrames(charData.idleAnimationFileName)));
		idleAnimationMenuPreviewImage->fps = charData.idleAnimationFPS.value;

		runAnimationMenuPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.runAnimationFileName, charData.runAnimationFileName, getSpriteNumFrames(charData.runAnimationFileName)));
		runAnimationMenuPreviewImage->fps = charData.runAnimationFPS.value;
		characterDataMenuHP->textField = std::format("{}", charData.hp.value);
		characterDataMenuATK->textField = std::format("{}", charData.atk.value);
		characterDataMenuSPD->textField = std::format("{}", charData.spd.value);
		characterDataMenuCrit->textField = std::format("{}", charData.crit.value);
		weaponIconMenuIconPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackIconFileName, charData.attackIconFileName, 1));
		weaponAwakenedIconMenuIconPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackAwakenedIconFileName, charData.attackAwakenedIconFileName, 1));
		weaponDataMenuWeaponName->textField = charData.attackName;
		if (charData.mainWeaponWeaponType.compare("Melee") == 0)
		{
			weaponDataMenuWeaponType->curSelectionTextIndex = 0;
		}
		else if (charData.mainWeaponWeaponType.compare("Multishot") == 0)
		{
			weaponDataMenuWeaponType->curSelectionTextIndex = 1;
		}
		else if (charData.mainWeaponWeaponType.compare("Ranged") == 0)
		{
			weaponDataMenuWeaponType->curSelectionTextIndex = 2;
		}
		weaponAnimationMenuPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackAnimationFileName, charData.attackAnimationFileName, getSpriteNumFrames(charData.attackAnimationFileName)));
		weaponAnimationMenuPreviewImage->fps = charData.attackAnimationFPS.value;
		characterDataMenuSizeGrade->textField = std::format("{}", charData.sizeGrade.value);
		for (int i = 0; i < charData.weaponLevelDataList.size(); i++)
		{
			auto& curWeaponLevelData = charData.weaponLevelDataList[i];
			weaponLevelDescriptionMenuList[i]->textField = curWeaponLevelData.attackDescription;
			if (curWeaponLevelData.attackTime.isDefined)
			{
				weaponLevelAttackTimeMenuList[i]->textField = std::format("{}", curWeaponLevelData.attackTime.value);
			}
			if (curWeaponLevelData.duration.isDefined)
			{
				weaponLevelDurationMenuList[i]->textField = std::format("{}", curWeaponLevelData.duration.value);
			}
			if (curWeaponLevelData.damage.isDefined)
			{
				weaponLevelDamageMenuList[i]->textField = std::format("{}", curWeaponLevelData.damage.value);
			}
			if (curWeaponLevelData.hitLimit.isDefined)
			{
				weaponLevelHitLimitMenuList[i]->textField = std::format("{}", curWeaponLevelData.hitLimit.value);
			}
			if (curWeaponLevelData.speed.isDefined)
			{
				weaponLevelSpeedMenuList[i]->textField = std::format("{}", curWeaponLevelData.speed.value);
			}
			if (curWeaponLevelData.hitCD.isDefined)
			{
				weaponLevelHitCDMenuList[i]->textField = std::format("{}", curWeaponLevelData.hitCD.value);
			}
			if (curWeaponLevelData.attackCount.isDefined)
			{
				weaponLevelAttackCountMenuList[i]->textField = std::format("{}", curWeaponLevelData.attackCount.value);
			}
			if (curWeaponLevelData.attackDelay.isDefined)
			{
				weaponLevelAttackDelayMenuList[i]->textField = std::format("{}", curWeaponLevelData.attackDelay.value);
			}
			if (curWeaponLevelData.range.isDefined)
			{
				weaponLevelRangeMenuList[i]->textField = std::format("{}", curWeaponLevelData.range.value);
			}
		}
		specialIconMenuIconPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.specialIconFileName, charData.specialIconFileName, 1));
		specialMenuSpecialCooldown->textField = std::format("{}", charData.specialCooldown.value);
		specialMenuSpecialName->textField = charData.specialName;
		specialDescriptionMenuSpecialDescription->textField = charData.specialDescription;
		specialAnimationMenuPreviewImage->fps = charData.specialAnimationFPS.value;
		specialAnimationMenuPreviewImage->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.specialAnimationFileName, charData.specialAnimationFileName, getSpriteNumFrames(charData.specialAnimationFileName)));
		specialMenuDamage->textField = std::format("{}", charData.specialDamage.value);
		specialMenuDuration->textField = std::format("{}", charData.specialDuration.value);

		for (auto& curBuffDataMenuGrid : buffMenuDeque)
		{
			holoCureMenuInterfacePtr->DeleteMenuGrid(MODNAME, curBuffDataMenuGrid.buffDataGrid->menuGridPtr);
			holoCureMenuInterfacePtr->DeleteMenuGrid(MODNAME, curBuffDataMenuGrid.buffIconGrid->menuGridPtr);
		}

		buffMenuDeque.clear();

		for (int i = 0; i < charData.buffDataList.size(); i++)
		{
			auto& curBuffData = charData.buffDataList[i];
			addBuffMenuButton();
			auto& curBuffDataGrid = buffMenuDeque[i].buffDataGrid;
			auto& curBuffIconGrid = buffMenuDeque[i].buffIconGrid;
			curBuffDataGrid->menuColumnList[0].menuDataPtrList[0]->textField = curBuffData.buffName;
			if (curBuffData.levels[0].attackIncrement.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[1]->textField = std::format("{}", curBuffData.levels[0].attackIncrement.value);
			}
			if (curBuffData.levels[0].critIncrement.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[2]->textField = std::format("{}", curBuffData.levels[0].critIncrement.value);
			}
			if (curBuffData.levels[0].hasteIncrement.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[3]->textField = std::format("{}", curBuffData.levels[0].hasteIncrement.value);
			}
			if (curBuffData.levels[0].speedIncrement.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[4]->textField = std::format("{}", curBuffData.levels[0].speedIncrement.value);
			}
			if (curBuffData.levels[0].DRMultiplier.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[5]->textField = std::format("{}", curBuffData.levels[0].DRMultiplier.value);
			}
			if (curBuffData.levels[0].healMultiplier.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[6]->textField = std::format("{}", curBuffData.levels[0].healMultiplier.value);
			}
			if (curBuffData.levels[0].food.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[7]->textField = std::format("{}", curBuffData.levels[0].food.value);
			}
			if (curBuffData.levels[0].weaponSize.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[8]->textField = std::format("{}", curBuffData.levels[0].weaponSize.value);
			}
			if (curBuffData.levels[0].maxStacks.isDefined)
			{
				curBuffDataGrid->menuColumnList[0].menuDataPtrList[9]->textField = std::format("{}", curBuffData.levels[0].maxStacks.value);
			}
			if (curBuffData.levels[0].timer.isDefined)
			{
				curBuffDataGrid->menuColumnList[1].menuDataPtrList[0]->textField = std::format("{}", curBuffData.levels[0].timer.value);
			}
			curBuffIconGrid->menuColumnList[1].menuDataPtrList[0]->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + curBuffData.buffIconFileName, curBuffData.buffIconFileName, 1));
		}
		reloadBuffs();

		for (int i = 0; i < charData.skillDataList.size(); i++)
		{
			auto& curSkillData = charData.skillDataList[i];
			for (int j = 0; j < 3; j++)
			{
				auto& curSkillLevelData = curSkillData.skillLevelDataList[j];
				if (curSkillLevelData.attackIncrement.isDefined)
				{
					skillsDataLevelMenuAttackButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.attackIncrement.value);
				}
				if (curSkillLevelData.critIncrement.isDefined)
				{
					skillsDataLevelMenuCritButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.critIncrement.value);
				}
				if (curSkillLevelData.hasteIncrement.isDefined)
				{
					skillsDataLevelMenuHasteButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.hasteIncrement.value);
				}
				if (curSkillLevelData.speedIncrement.isDefined)
				{
					skillsDataLevelMenuSpeedButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.speedIncrement.value);
				}
				if (curSkillLevelData.DRMultiplier.isDefined)
				{
					skillsDataLevelMenuDRButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.DRMultiplier.value);
				}
				if (curSkillLevelData.healMultiplier.isDefined)
				{
					skillsDataLevelMenuHealMultiplierButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.healMultiplier.value);
				}
				if (curSkillLevelData.food.isDefined)
				{
					skillsDataLevelMenuFoodButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.food.value);
				}
				if (curSkillLevelData.weaponSize.isDefined)
				{
					skillsDataLevelMenuWeaponSizeMultiplierButtonList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.weaponSize.value);
				}
				skillDataLevelDescriptionMenuList[i * 3 + j]->textField = curSkillLevelData.skillDescription;
				for (int buffIndex = 0; buffIndex < skillOnTriggerBuffSelectionMenuList[i * 3 + j]->selectionText.size(); buffIndex++)
				{
					if (skillOnTriggerBuffSelectionMenuList[i * 3 + j]->selectionText[buffIndex].compare(curSkillLevelData.skillOnTriggerData.buffName) == 0)
					{
						skillOnTriggerBuffSelectionMenuList[i * 3 + j]->curSelectionTextIndex = buffIndex;
						break;
					}
				}
				for (int buffIndex = 0; buffIndex < skillOnTriggerTypeSelectionMenuList[i * 3 + j]->selectionText.size(); buffIndex++)
				{
					if (skillOnTriggerTypeSelectionMenuList[i * 3 + j]->selectionText[buffIndex].compare(curSkillLevelData.skillOnTriggerData.onTriggerType) == 0)
					{
						skillOnTriggerTypeSelectionMenuList[i * 3 + j]->curSelectionTextIndex = buffIndex;
						break;
					}
				}
				if (curSkillLevelData.skillOnTriggerData.probability.isDefined)
				{
					skillOnTriggerProbabilityMenuList[i * 3 + j]->textField = std::format("{}", curSkillLevelData.skillOnTriggerData.probability.value);
				}
			}

			skillsDataMenuNameList[i]->textField = curSkillData.skillName;
			skillIconMenuIconPreviewImageList[i]->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + curSkillData.skillIconFileName, curSkillData.skillIconFileName, 1));
		}
	}
}

void reloadLoadCharacter()
{
	auto& menuDataList = loadCharacterMenuGrid.menuColumnList[0].menuDataPtrList;
	for (size_t i = 0; i < menuDataList.size() - 2; i++)
	{
		menuDataList[i]->labelName = "";
		menuDataList[i]->isVisible = false;
	}
	for (int i = 0; i < 8 && loadCharacterPage * 8 + i < loadCharacterDeque.size(); i++)
	{
		auto& menuData = menuDataList[i];
		menuData->labelName = loadCharacterDeque[loadCharacterPage * 8 + i];
		menuData->isVisible = true;
	}
}

void reloadLoadCharacterDeque()
{
	if (!std::filesystem::exists("CharacterCreatorMod"))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the CharacterCreatorMod directory");
		g_ModuleInterface->Print(CM_RED, "Couldn't find the CharacterCreatorMod directory");
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
	reloadLoadCharacter();
}

void loadCharacterClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, loadCharacterMenuGrid.menuGridPtr);
	reloadLoadCharacterDeque();
}

void prevLoadCharacterButton()
{
	if (loadCharacterPage > 0)
	{
		loadCharacterPage--;
		reloadLoadCharacter();
	}
}

void nextLoadCharacterButton()
{
	if (loadCharacterPage < (loadCharacterDeque.size() - 1) / 8)
	{
		loadCharacterPage++;
		reloadLoadCharacter();
	}
}

void reloadIcons()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto& menuDataPtrList = curMenuGrid->menuColumnsPtrList[0]->menuDataPtrList;
	for (size_t i = 0; i < menuDataPtrList.size() - 2; i++)
	{
		menuDataPtrList[i]->labelName = "";
		menuDataPtrList[i]->curSprite = nullptr;
		menuDataPtrList[i]->isVisible = false;
	}
	for (int i = 0; i < 8 && spriteDequePage * 8 + i < spriteDeque.size(); i++)
	{
		auto& menuData = menuDataPtrList[i];
		menuData->labelName = spriteDeque[spriteDequePage * 8 + i]->spriteFileName;
		menuData->curSprite = spriteDeque[spriteDequePage * 8 + i];
		menuData->isVisible = true;
	}
}

void reloadSpriteDeque()
{
	if (!std::filesystem::exists("CharacterCreatorMod/inputData"))
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find the inputData directory");
		g_ModuleInterface->Print(CM_RED, "Couldn't find the inputData directory");
		return;
	}
	spriteDequePage = 0;
	spriteDeque.clear();
	for (const auto& dir : std::filesystem::directory_iterator("CharacterCreatorMod/inputData"))
	{
		auto path = dir.path();
		if (path.extension().compare(".png") == 0 || path.extension().compare(".jpg") == 0)
		{
			std::shared_ptr<spriteData> data(new spriteData(path.generic_string(), path.filename().string(), getSpriteNumFrames(path.string())));
			spriteDeque.push_back(data);
		}
	}
	reloadIcons();
}

void portraitClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, portraitMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void prevIconButton()
{
	if (spriteDequePage > 0)
	{
		spriteDequePage--;
		reloadIcons();
	}
}

void nextIconButton()
{
	if (spriteDequePage < (spriteDeque.size() - 1) / 8)
	{
		spriteDequePage++;
		reloadIcons();
	}
}

void prevImageButton()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto& curMenuData = curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList[0];
	curMenuData->curSubImageIndex--;
	if (curMenuData->curSubImageIndex < 0)
	{
		curMenuData->curSubImageIndex = max(0, static_cast<int>(curMenuData->curSprite->numFrames) - 1);
	}
}

void nextImageButton()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto& curMenuData = curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList[0];
	curMenuData->curSubImageIndex++;
	if (curMenuData->curSubImageIndex >= curMenuData->curSprite->numFrames)
	{
		curMenuData->curSubImageIndex = 0;
	}
}

void playPauseButton()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto& menuDataList = curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList;
	menuDataList[3]->isVisible = !menuDataList[3]->isVisible;
	menuDataList[4]->isVisible = !menuDataList[4]->isVisible;
	menuDataList[0]->curFrameCount = menuDataList[3]->isVisible ? -1 : 0;
}

void animationFrameText()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto& menuDataList = curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList;
	auto& previewImage = menuDataList[0];
	auto& frameText = menuDataList[5];
	int curImageIndex = previewImage->curSprite == nullptr ? 0 : previewImage->curSubImageIndex + 1;
	int maxFrames = previewImage->curSprite == nullptr ? 0 : previewImage->curSprite->numFrames;
	frameText->labelName = std::format("{} / {}", curImageIndex, maxFrames);
}

void clickIconButton()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto& previewImageData = curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList[0];
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	previewImageData->curSprite = selectedMenuData->curSprite;
}

void clickSkillIconButton()
{
	auto& previewImageData = skillIconMenuIconPreviewImageList[skillMenuIndex];
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	previewImageData->curSprite = selectedMenuData->curSprite;
}

void buffIconClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, buffMenuDeque[buffMenuIndex].buffIconGrid->menuGridPtr);
	reloadSpriteDeque();
}

void clickBuffMenuButton()
{
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	int selectedBuffIndex = -1;
	for (int i = 0; i < buffMenuButtonList.size(); i++)
	{
		if (buffMenuButtonList[i].get() == selectedMenuData.get())
		{
			selectedBuffIndex = i;
			break;
		}
	}
	if (selectedBuffIndex == -1)
	{
		buffMenuIndex = -1;
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find buff menu button");
		g_ModuleInterface->Print(CM_RED, "Couldn't find buff menu button");
		return;
	}
	buffMenuIndex = buffMenuPage * 8 + selectedBuffIndex;
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, buffMenuDeque[buffMenuPage * 8 + selectedBuffIndex].buffDataGrid->menuGridPtr);
}

void reloadBuffs()
{
	for (size_t i = 0; i < buffMenuButtonList.size(); i++)
	{
		buffMenuButtonList[i]->labelName = "";
		buffMenuButtonList[i]->curSprite = nullptr;
		buffMenuButtonList[i]->isVisible = false;
	}
	for (int i = 0; i < 8 && buffMenuPage * 8 + i < buffMenuDeque.size(); i++)
	{
		auto& menuData = buffMenuButtonList[i];
		menuData->labelName = buffMenuDeque[buffMenuPage * 8 + i].buffDataGrid->menuColumnList[0].menuDataPtrList[0]->textField;
		menuData->isVisible = true;
	}
	std::vector<std::string> buffNameList;
	for (auto& curBuffMenuGrid : buffMenuDeque)
	{
		auto& buffName = curBuffMenuGrid.buffDataGrid->menuColumnList[0].menuDataPtrList[0]->textField;
		buffNameList.push_back(buffName);
	}
	for (size_t i = 0; i < skillOnTriggerBuffSelectionMenuList.size(); i++)
	{
		skillOnTriggerBuffSelectionMenuList[i]->selectionText = buffNameList;
		skillOnTriggerBuffSelectionMenuList[i]->curSelectionTextIndex = 0;
	}
}

void prevBuffButton()
{
	if (buffMenuPage > 0)
	{
		buffMenuPage--;
		reloadBuffs();
	}
}

void nextBuffButton()
{
	if (buffMenuPage < (buffMenuDeque.size() - 1) / 8)
	{
		buffMenuPage++;
		reloadBuffs();
	}
}

void addBuffMenuButton()
{
	std::shared_ptr<menuGrid> newBuffDataMenuGrid = std::shared_ptr<menuGrid>(new menuGrid({
		menuColumn({
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 0, 130, 20, "BUFFMENU_NAME", "buff name", true, nullptr, nullptr, MENUDATATYPE_TextBoxField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 1, 130, 20, "BUFFMENU_ATK", "ATK+", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 2, 130, 20, "BUFFMENU_CRIT", "CRIT+", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 3, 130, 20, "BUFFMENU_HASTE", "Haste+", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 4, 130, 20, "BUFFMENU_SPEED", "SPD+", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 5, 130, 20, "BUFFMENU_DR", "DR*", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 6, 130, 20, "BUFFMENU_HealMultiplier", "HealMultiplier+", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 7, 130, 20, "BUFFMENU_Food", "Food+", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 8, 130, 20, "BUFFMENU_WeaponSizeMultiplier", "WeaponSizeMult+", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(100, 20 + 29 * 9, 130, 20, "BUFFMENU_MaxStacks", "Max Stacks", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
		}),
		menuColumn({
			std::shared_ptr<menuData>(new menuData(330, 20 + 29 * 0, 130, 20, "BUFFMENU_Timer", "Timer", true, nullptr, nullptr, MENUDATATYPE_NumberField)),
			std::shared_ptr<menuData>(new menuData(330, 20 + 29 * 1, 180, 20, "BUFFMENU_BuffIcon", "Buff Icon", true, buffIconClickButton, nullptr, MENUDATATYPE_Button)),
		}),
	}, "Buff Data Menu", &buffMenuGrid));

	std::shared_ptr<menuGrid> newBuffIconMenuGrid = std::shared_ptr<menuGrid>(new menuGrid({
		menuColumn({
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 0, 180, 20, "BUFFMENU_IconButton1", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 1, 180, 20, "BUFFMENU_IconButton2", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 2, 180, 20, "BUFFMENU_IconButton3", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 3, 180, 20, "BUFFMENU_IconButton4", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 4, 180, 20, "BUFFMENU_IconButton5", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 5, 180, 20, "BUFFMENU_IconButton6", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 6, 180, 20, "BUFFMENU_IconButton7", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 7, 180, 20, "BUFFMENU_IconButton8", "", false, clickIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 8, 180, 20, "BUFFMENU_PrevButton", "Prev", true, prevIconButton, nullptr, MENUDATATYPE_Button)),
			std::shared_ptr<menuData>(new menuData(60, 20 + 29 * 9, 180, 20, "BUFFMENU_NextButton", "Next", true, nextIconButton, nullptr, MENUDATATYPE_Button)),
		}),
		menuColumn({
			std::shared_ptr<menuData>(new menuData(250, 20 + 29 * 2, "BUFFMENU_PreviewImage", "", true, nullptr, nullptr, MENUDATATYPE_Image, 30)),
		}),
	}, "Buff Icon Menu", newBuffDataMenuGrid.get()));

	newBuffDataMenuGrid->menuColumnList[0].menuDataPtrList[0]->textField = "newBuff";
	newBuffDataMenuGrid->initMenuGrid();
	newBuffIconMenuGrid->initMenuGrid();
	buffMenuDeque.push_back(buffDataMenuGrid(newBuffDataMenuGrid, newBuffIconMenuGrid));
	reloadBuffs();
}

void weaponLevelClickButton()
{
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	int selectedMenuIndex = -1;
	for (int i = 0; i < weaponLevelMenuButtonList.size(); i++)
	{
		if (weaponLevelMenuButtonList[i].get() == selectedMenuData.get())
		{
			selectedMenuIndex = i;
			break;
		}
	}
	weaponLevelMenuIndex = selectedMenuIndex;
	if (selectedMenuIndex == -1)
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find weapon level menu button");
		g_ModuleInterface->Print(CM_RED, "Couldn't find weapon level menu button");
		return;
	}
	for (auto& menuData : weaponLevelAttackTimeMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelDescriptionMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelDurationMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelDamageMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelHitLimitMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelSpeedMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelHitCDMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelAttackCountMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelAttackDelayMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : weaponLevelRangeMenuList)
	{
		menuData->isVisible = false;
	}
	weaponLevelAttackTimeMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelDescriptionMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelDurationMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelDamageMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelHitLimitMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelSpeedMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelHitCDMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelAttackCountMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelAttackDelayMenuList[weaponLevelMenuIndex]->isVisible = true;
	weaponLevelRangeMenuList[weaponLevelMenuIndex]->isVisible = true;
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, weaponLevelDataMenuGrid.menuGridPtr);
}

void skillLevelClickButton()
{
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	int selectedMenuIndex = -1;
	for (int i = 0; i < skillsDataMenuLevelList.size(); i++)
	{
		if (skillsDataMenuLevelList[i].get() == selectedMenuData.get())
		{
			selectedMenuIndex = i;
			break;
		}
	}
	skillLevelMenuIndex = selectedMenuIndex;
	if (selectedMenuIndex == -1)
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find weapon level menu button");
		g_ModuleInterface->Print(CM_RED, "Couldn't find weapon level menu button");
		return;
	}
	for (auto& menuData : skillsDataLevelMenuAttackButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataLevelMenuCritButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataLevelMenuHasteButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataLevelMenuSpeedButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataLevelMenuDRButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataLevelMenuHealMultiplierButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataLevelMenuFoodButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataLevelMenuWeaponSizeMultiplierButtonList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillDataLevelDescriptionMenuList)
	{
		menuData->isVisible = false;
	}
	skillsDataLevelMenuAttackButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillsDataLevelMenuCritButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillsDataLevelMenuHasteButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillsDataLevelMenuSpeedButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillsDataLevelMenuDRButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillsDataLevelMenuHealMultiplierButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillsDataLevelMenuFoodButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillsDataLevelMenuWeaponSizeMultiplierButtonList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillDataLevelDescriptionMenuList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, skillsDataLevelMenuGrid.menuGridPtr);
}

void skillMenuClickButton()
{
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	int selectedMenuIndex = -1;
	for (int i = 0; i < skillsMenuButtonList.size(); i++)
	{
		if (skillsMenuButtonList[i].get() == selectedMenuData.get())
		{
			selectedMenuIndex = i;
			break;
		}
	}
	skillMenuIndex = selectedMenuIndex;
	if (selectedMenuIndex == -1)
	{
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Couldn't find skills menu button");
		g_ModuleInterface->Print(CM_RED, "Couldn't find skills menu button");
		return;
	}
	for (auto& menuData : skillIconMenuIconPreviewImageList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillsDataMenuNameList)
	{
		menuData->isVisible = false;
	}
	
	skillIconMenuIconPreviewImageList[skillMenuIndex]->isVisible = true;
	skillsDataMenuNameList[skillMenuIndex]->isVisible = true;

	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, skillsDataMenuGrid.menuGridPtr);
}

void largePortraitClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, largePortraitMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void idleAnimationClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, idleAnimationMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void runAnimationClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, runAnimationMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void specialAnimationClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, specialAnimationMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void weaponLevelDescriptionClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, weaponLevelDescriptionMenuGrid.menuGridPtr);
}

void skillDescriptionClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, skillDataLevelDescriptionMenuGrid.menuGridPtr);
}

void skillOnTriggerClickButton()
{
	for (auto& menuData : skillOnTriggerBuffSelectionMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillOnTriggerTypeSelectionMenuList)
	{
		menuData->isVisible = false;
	}
	for (auto& menuData : skillOnTriggerProbabilityMenuList)
	{
		menuData->isVisible = false;
	}
	skillOnTriggerBuffSelectionMenuList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillOnTriggerTypeSelectionMenuList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	skillOnTriggerProbabilityMenuList[skillMenuIndex * 3 + skillLevelMenuIndex]->isVisible = true;
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, skillOnTriggerMenuGrid.menuGridPtr);
}

void specialDescriptionClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, specialDescriptionMenuGrid.menuGridPtr);
}

void skillIconMenuClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, skillIconMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void specialIconClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, specialIconMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void weaponIconClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, weaponIconMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void weaponAwakenedIconClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, weaponAwakenedIconMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void weaponAnimationClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, weaponAnimationMenuGrid.menuGridPtr);
	reloadSpriteDeque();
}

void weaponLevelMenuClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, weaponLevelMenuGrid.menuGridPtr);
}

void characterDataClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, characterDataMenuGrid.menuGridPtr);
}

void mainWeaponClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, weaponDataMenuGrid.menuGridPtr);
}

void skillClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, skillsMenuGrid.menuGridPtr);
}

void specialClickButton()
{
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, specialMenuGrid.menuGridPtr);
}

void buffClickButton()
{
	buffMenuPage = 0;
	holoCureMenuInterfacePtr->SwapToMenuGrid(MODNAME, buffMenuGrid.menuGridPtr);
	reloadBuffs();
}

std::string getSpriteFileName(std::shared_ptr<menuData> dataPtr)
{
	if (dataPtr->curSprite == nullptr)
	{
		return "";
	}
	return dataPtr->curSprite->spriteFileName;
}

std::string getSpriteFilePath(std::shared_ptr<menuData> dataPtr)
{
	if (dataPtr->curSprite == nullptr)
	{
		return "";
	}
	return dataPtr->curSprite->spritePath;
}

void copySpriteToDestination(std::shared_ptr<menuData>& imagePtr, std::string destPath)
{
	std::string spriteFilePath = getSpriteFilePath(imagePtr);
	if (spriteFilePath.compare(destPath) == 0)
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
			g_ModuleInterface->Print(CM_RED, "Couldn't parse %s as double", inputStr.c_str());
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
			g_ModuleInterface->Print(CM_RED, "Couldn't parse %s as", inputStr.c_str());
		}
		outputJSONInt.isDefined = false;
		return;
	}
	outputJSONInt.isDefined = true;
}

void exportCharacterClickButton()
{
	CreateDirectory(L"CharacterCreatorMod", NULL);
	auto charName = characterDataMenuCharName->textField;
	std::string charDirName = "CharacterCreatorMod/char_" + charName;
	CreateDirectoryA(charDirName.c_str(), NULL);

	characterData outCharData;
	outCharData.charName = charName;
	auto portraitFileName = getSpriteFileName(portraitMenuPreviewImage);
	outCharData.portraitFileName = portraitFileName;
	copySpriteToDestination(portraitMenuPreviewImage, charDirName + "/" + portraitFileName);
	auto largePortraitFileName = getSpriteFileName(largePortraitMenuPreviewImage);
	outCharData.largePortraitFileName = largePortraitFileName;
	copySpriteToDestination(largePortraitMenuPreviewImage, charDirName + "/" + largePortraitFileName);
	
	outCharData.idleAnimationFPS.isDefined = true;
	outCharData.idleAnimationFPS.value = idleAnimationMenuPreviewImage->fps;
	auto idleAnimationFileName = getSpriteFileName(idleAnimationMenuPreviewImage);
	outCharData.idleAnimationFileName = idleAnimationFileName;
	copySpriteToDestination(idleAnimationMenuPreviewImage, charDirName + "/" + idleAnimationFileName);

	outCharData.runAnimationFPS.isDefined = true;
	outCharData.runAnimationFPS.value = runAnimationMenuPreviewImage->fps;
	auto runAnimationFileName = getSpriteFileName(runAnimationMenuPreviewImage);
	outCharData.runAnimationFileName = runAnimationFileName;
	copySpriteToDestination(runAnimationMenuPreviewImage, charDirName + "/" + runAnimationFileName);

	parseStringToJSONDouble(characterDataMenuHP->textField, outCharData.hp, false);
	parseStringToJSONDouble(characterDataMenuATK->textField, outCharData.atk, false);
	parseStringToJSONDouble(characterDataMenuSPD->textField, outCharData.spd, false);
	parseStringToJSONDouble(characterDataMenuCrit->textField, outCharData.crit, false);

	auto attackIconFileName = getSpriteFileName(weaponIconMenuIconPreviewImage);
	outCharData.attackIconFileName = attackIconFileName;
	copySpriteToDestination(weaponIconMenuIconPreviewImage, charDirName + "/" + attackIconFileName);
	
	auto attackAwakenedIconFileName = getSpriteFileName(weaponAwakenedIconMenuIconPreviewImage);
	outCharData.attackAwakenedIconFileName = attackAwakenedIconFileName;
	copySpriteToDestination(weaponAwakenedIconMenuIconPreviewImage, charDirName + "/" + attackAwakenedIconFileName);
	
	outCharData.attackAnimationFPS.isDefined = true;
	outCharData.attackAnimationFPS.value = weaponAnimationMenuPreviewImage->fps;
	auto attackAnimationFileName = getSpriteFileName(weaponAnimationMenuPreviewImage);
	outCharData.attackAnimationFileName = attackAnimationFileName;
	copySpriteToDestination(weaponAnimationMenuPreviewImage, charDirName + "/" + attackAnimationFileName);
	outCharData.attackName = weaponDataMenuWeaponName->textField;

	outCharData.mainWeaponWeaponType = weaponDataMenuWeaponType->selectionText[weaponDataMenuWeaponType->curSelectionTextIndex];
	auto specialIconFileName = getSpriteFileName(specialIconMenuIconPreviewImage);
	outCharData.specialIconFileName = specialIconFileName;
	copySpriteToDestination(specialIconMenuIconPreviewImage, charDirName + "/" + specialIconFileName);
	parseStringToJSONInt(specialMenuSpecialCooldown->textField, outCharData.specialCooldown);
	outCharData.specialName = specialMenuSpecialName->textField;
	outCharData.specialDescription = specialDescriptionMenuSpecialDescription->textField;
	parseStringToJSONInt(characterDataMenuSizeGrade->textField, outCharData.sizeGrade);
	
	for (int i = 0; i < weaponLevelDescriptionMenuList.size(); i++)
	{
		weaponLevelData curWeaponLevelData;
		curWeaponLevelData.attackDescription = weaponLevelDescriptionMenuList[i]->textField;
		parseStringToJSONInt(weaponLevelAttackTimeMenuList[i]->textField, curWeaponLevelData.attackTime);
		parseStringToJSONInt(weaponLevelDurationMenuList[i]->textField, curWeaponLevelData.duration);
		parseStringToJSONDouble(weaponLevelDamageMenuList[i]->textField, curWeaponLevelData.damage);
		parseStringToJSONInt(weaponLevelHitLimitMenuList[i]->textField, curWeaponLevelData.hitLimit);
		parseStringToJSONDouble(weaponLevelSpeedMenuList[i]->textField, curWeaponLevelData.speed);
		parseStringToJSONInt(weaponLevelHitCDMenuList[i]->textField, curWeaponLevelData.hitCD);
		parseStringToJSONInt(weaponLevelAttackCountMenuList[i]->textField, curWeaponLevelData.attackCount);
		parseStringToJSONInt(weaponLevelAttackDelayMenuList[i]->textField, curWeaponLevelData.attackDelay);
		parseStringToJSONInt(weaponLevelRangeMenuList[i]->textField, curWeaponLevelData.range);
		outCharData.weaponLevelDataList.push_back(curWeaponLevelData);
	}

	for (int i = 0; i < 3; i++)
	{
		skillData curSkillData;
		for (int j = 0; j < 3; j++)
		{
			skillLevelData curSkillLevelData;
			int listIndex = i * 3 + j;
			
			parseStringToJSONInt(skillsDataLevelMenuAttackButtonList[listIndex]->textField, curSkillLevelData.attackIncrement);
			parseStringToJSONInt(skillsDataLevelMenuCritButtonList[listIndex]->textField, curSkillLevelData.critIncrement);
			parseStringToJSONInt(skillsDataLevelMenuHasteButtonList[listIndex]->textField, curSkillLevelData.hasteIncrement);
			parseStringToJSONInt(skillsDataLevelMenuSpeedButtonList[listIndex]->textField, curSkillLevelData.speedIncrement);
			parseStringToJSONDouble(skillsDataLevelMenuDRButtonList[listIndex]->textField, curSkillLevelData.DRMultiplier);
			parseStringToJSONDouble(skillsDataLevelMenuHealMultiplierButtonList[listIndex]->textField, curSkillLevelData.healMultiplier);
			parseStringToJSONDouble(skillsDataLevelMenuFoodButtonList[listIndex]->textField, curSkillLevelData.food);
			parseStringToJSONDouble(skillsDataLevelMenuWeaponSizeMultiplierButtonList[listIndex]->textField, curSkillLevelData.weaponSize);
			curSkillLevelData.skillDescription = skillDataLevelDescriptionMenuList[listIndex]->textField;
			curSkillLevelData.skillOnTriggerData.buffName = skillOnTriggerBuffSelectionMenuList[listIndex]->selectionText[skillOnTriggerBuffSelectionMenuList[listIndex]->curSelectionTextIndex];
			curSkillLevelData.skillOnTriggerData.onTriggerType = skillOnTriggerTypeSelectionMenuList[listIndex]->selectionText[skillOnTriggerTypeSelectionMenuList[listIndex]->curSelectionTextIndex];
			parseStringToJSONInt(skillOnTriggerProbabilityMenuList[listIndex]->textField, curSkillLevelData.skillOnTriggerData.probability);
			curSkillData.skillLevelDataList.push_back(curSkillLevelData);
		}
		curSkillData.skillName = skillsDataMenuNameList[i]->textField;
		auto skillIconFileName = getSpriteFileName(skillIconMenuIconPreviewImageList[i]);
		curSkillData.skillIconFileName = skillIconFileName;
		copySpriteToDestination(skillIconMenuIconPreviewImageList[i], charDirName + "/" + skillIconFileName);

		outCharData.skillDataList.push_back(curSkillData);
	}
	outCharData.specialAnimationFPS.isDefined = true;
	outCharData.specialAnimationFPS.value = specialAnimationMenuPreviewImage->fps;
	auto specialAnimationFileName = getSpriteFileName(specialAnimationMenuPreviewImage);
	outCharData.specialAnimationFileName = specialAnimationFileName;
	copySpriteToDestination(specialAnimationMenuPreviewImage, charDirName + "/" + specialAnimationFileName);
	parseStringToJSONDouble(specialMenuDamage->textField, outCharData.specialDamage, false);
	parseStringToJSONInt(specialMenuDuration->textField, outCharData.specialDuration, false);

	for (auto& buffMenuGrid : buffMenuDeque)
	{
		buffData curBuffData;
		auto& curBuffDataColumn = buffMenuGrid.buffDataGrid->menuColumnList[0];
		auto& curBuffIconColumn = buffMenuGrid.buffIconGrid->menuColumnList[1];
		curBuffData.buffName = curBuffDataColumn.menuDataPtrList[0]->textField;
		buffLevelData curBuffLevelData;
		parseStringToJSONInt(curBuffDataColumn.menuDataPtrList[1]->textField, curBuffLevelData.attackIncrement);
		parseStringToJSONInt(curBuffDataColumn.menuDataPtrList[2]->textField, curBuffLevelData.critIncrement);
		parseStringToJSONInt(curBuffDataColumn.menuDataPtrList[3]->textField, curBuffLevelData.hasteIncrement);
		parseStringToJSONInt(curBuffDataColumn.menuDataPtrList[4]->textField, curBuffLevelData.speedIncrement);
		parseStringToJSONDouble(curBuffDataColumn.menuDataPtrList[5]->textField, curBuffLevelData.DRMultiplier);
		parseStringToJSONDouble(curBuffDataColumn.menuDataPtrList[6]->textField, curBuffLevelData.healMultiplier);
		parseStringToJSONDouble(curBuffDataColumn.menuDataPtrList[7]->textField, curBuffLevelData.food);
		parseStringToJSONDouble(curBuffDataColumn.menuDataPtrList[8]->textField, curBuffLevelData.weaponSize);
		parseStringToJSONInt(curBuffDataColumn.menuDataPtrList[9]->textField, curBuffLevelData.maxStacks);
		parseStringToJSONInt(buffMenuGrid.buffDataGrid->menuColumnList[1].menuDataPtrList[0]->textField, curBuffLevelData.timer);
		curBuffData.levels.push_back(curBuffLevelData);
		auto buffIconFileName = getSpriteFileName(curBuffIconColumn.menuDataPtrList[0]);
		curBuffData.buffIconFileName = buffIconFileName;
		copySpriteToDestination(curBuffIconColumn.menuDataPtrList[0], charDirName + "/" + buffIconFileName);
		outCharData.buffDataList.push_back(curBuffData);
	}

	nlohmann::json outputJSON = outCharData;

	std::ofstream outFile;
	outFile.open(charDirName + "/charData.json");
	outFile << std::setw(4) << outputJSON << "\n";
	outFile.close();
}

void to_json(nlohmann::json& outputJson, const characterData& inputCharData)
{
	outputJson = nlohmann::json{
		{ "charName", inputCharData.charName },
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
		{ "levels", inputCharData.weaponLevelDataList },
		{ "skills", inputCharData.skillDataList },
		{ "buffs", inputCharData.buffDataList },
	};
}

// TODO: Should probably add some error checking on this stuff

void from_json(const nlohmann::json& inputJson, characterData& outputCharData)
{
	inputJson.at("charName").get_to(outputCharData.charName);
	inputJson.at("portraitFileName").get_to(outputCharData.portraitFileName);
	inputJson.at("largePortraitFileName").get_to(outputCharData.largePortraitFileName);
	inputJson.at("idleAnimationFPS").get_to(outputCharData.idleAnimationFPS);
	inputJson.at("idleAnimationFileName").get_to(outputCharData.idleAnimationFileName);
	inputJson.at("runAnimationFPS").get_to(outputCharData.runAnimationFPS);
	inputJson.at("runAnimationFileName").get_to(outputCharData.runAnimationFileName);
	inputJson.at("HP").get_to(outputCharData.hp);
	inputJson.at("ATK").get_to(outputCharData.atk);
	inputJson.at("SPD").get_to(outputCharData.spd);
	inputJson.at("CRIT").get_to(outputCharData.crit);
	inputJson.at("attackIconFileName").get_to(outputCharData.attackIconFileName);
	inputJson.at("attackAwakenedIconFileName").get_to(outputCharData.attackAwakenedIconFileName);
	inputJson.at("attackName").get_to(outputCharData.attackName);
	inputJson.at("specialIconFileName").get_to(outputCharData.specialIconFileName);
	inputJson.at("attackAnimationFPS").get_to(outputCharData.attackAnimationFPS);
	inputJson.at("attackAnimationFileName").get_to(outputCharData.attackAnimationFileName);
	inputJson.at("specialCooldown").get_to(outputCharData.specialCooldown);
	inputJson.at("specialName").get_to(outputCharData.specialName);
	inputJson.at("specialDescription").get_to(outputCharData.specialDescription);
	inputJson.at("specialAnimationFPS").get_to(outputCharData.specialAnimationFPS);
	inputJson.at("specialAnimationFileName").get_to(outputCharData.specialAnimationFileName);
	inputJson.at("specialDamage").get_to(outputCharData.specialDamage);
	inputJson.at("specialDuration").get_to(outputCharData.specialDuration);
	inputJson.at("sizeGrade").get_to(outputCharData.sizeGrade);
	inputJson.at("weaponType").get_to(outputCharData.mainWeaponWeaponType);
	inputJson.at("levels").get_to(outputCharData.weaponLevelDataList);
	inputJson.at("skills").get_to(outputCharData.skillDataList);
	inputJson.at("buffs").get_to(outputCharData.buffDataList);
}

void to_json(nlohmann::json& outputJson, const skillData& inputSkillData)
{
	outputJson = nlohmann::json{
		{ "levels", inputSkillData.skillLevelDataList },
		{ "skillName", inputSkillData.skillName },
		{ "skillIconFileName", inputSkillData.skillIconFileName },
	};
}

void from_json(const nlohmann::json& inputJson, skillData& outputSkillData)
{
	inputJson.at("levels").get_to(outputSkillData.skillLevelDataList);
	inputJson.at("skillName").get_to(outputSkillData.skillName);
	inputJson.at("skillIconFileName").get_to(outputSkillData.skillIconFileName);
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
		{ "skillOnTriggerData", inputSkillLevelData.skillOnTriggerData },
	};
}

void from_json(const nlohmann::json& inputJson, skillLevelData& outputSkillLevelData)
{
	outputSkillLevelData.skillDescription = inputJson["skillDescription"];
	outputSkillLevelData.attackIncrement = inputJson["attackIncrement"];
	outputSkillLevelData.critIncrement = inputJson["critIncrement"];
	outputSkillLevelData.hasteIncrement = inputJson["hasteIncrement"];
	outputSkillLevelData.speedIncrement = inputJson["speedIncrement"];
	outputSkillLevelData.DRMultiplier = inputJson["DRMultiplier"];
	outputSkillLevelData.healMultiplier = inputJson["healMultiplier"];
	outputSkillLevelData.food = inputJson["food"];
	outputSkillLevelData.weaponSize = inputJson["weaponSize"];
	outputSkillLevelData.skillOnTriggerData = inputJson["skillOnTriggerData"];
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
	outputWeaponLevelData.attackDescription = inputJson["attackDescription"];
	outputWeaponLevelData.attackTime = inputJson["attackTime"];
	outputWeaponLevelData.attackCount = inputJson["attackCount"];
	outputWeaponLevelData.attackDelay = inputJson["attackDelay"];
	outputWeaponLevelData.damage = inputJson["damage"];
	outputWeaponLevelData.duration = inputJson["duration"];
	outputWeaponLevelData.hitCD = inputJson["hitCD"];
	outputWeaponLevelData.hitLimit = inputJson["hitLimit"];
	outputWeaponLevelData.range = inputJson["range"];
	outputWeaponLevelData.speed = inputJson["speed"];
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
		{ "maxStacks", inputBuffLevelData.maxStacks },
		{ "timer", inputBuffLevelData.timer },
	};
}

void from_json(const nlohmann::json& inputJson, buffLevelData& outputBuffLevelData)
{
	outputBuffLevelData.attackIncrement = inputJson["attackIncrement"];
	outputBuffLevelData.critIncrement = inputJson["critIncrement"];
	outputBuffLevelData.hasteIncrement = inputJson["hasteIncrement"];
	outputBuffLevelData.speedIncrement = inputJson["speedIncrement"];
	outputBuffLevelData.DRMultiplier = inputJson["DR"];
	outputBuffLevelData.healMultiplier = inputJson["healMultiplier"];
	outputBuffLevelData.food = inputJson["food"];
	outputBuffLevelData.weaponSize = inputJson["weaponSize"];
	outputBuffLevelData.maxStacks = inputJson["maxStacks"];
	outputBuffLevelData.timer = inputJson["timer"];
}

void to_json(nlohmann::json& outputJson, const buffData& inputBuffData)
{
	outputJson = nlohmann::json{
		{ "buffName", inputBuffData.buffName },
		{ "levels", inputBuffData.levels },
		{ "buffIconFileName", inputBuffData.buffIconFileName },
	};
}

void from_json(const nlohmann::json& inputJson, buffData& outputBuffData)
{
	outputBuffData.buffName = inputJson["buffName"];
	outputBuffData.levels = inputJson["levels"];
	outputBuffData.buffIconFileName = inputJson["buffIconFileName"];
}

void to_json(nlohmann::json& outputJson, const onTriggerData& inputOnTriggerData)
{
	outputJson = nlohmann::json{
		{ "onTriggerType", inputOnTriggerData.onTriggerType },
		{ "buffName", inputOnTriggerData.buffName },
		{ "probability", inputOnTriggerData.probability },
	};
}

void from_json(const nlohmann::json& inputJson, onTriggerData& outputOnTriggerData)
{
	outputOnTriggerData.onTriggerType = inputJson["onTriggerType"];
	outputOnTriggerData.buffName = inputJson["buffName"];
	outputOnTriggerData.probability = inputJson["probability"];
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