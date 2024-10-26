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

std::shared_ptr<menuData> characterCreatorMenuLoadCharacter(new menuDataButton(60, 104 + 29 * 0, 180, 29, "CHARACTERCREATORMENU_LoadCharacter", "Load Character", true, loadCharacterClickButton, nullptr));
std::shared_ptr<menuData> characterCreatorMenuCharacterData(new menuDataButton(60, 104 + 29 * 2, 180, 29, "CHARACTERCREATORMENU_CharacterData", "Character Data", true, characterDataClickButton, nullptr));
std::shared_ptr<menuData> characterCreatorMenuWeaponData(new menuDataButton(60, 104 + 29 * 3, 180, 29, "CHARACTERCREATORMENU_WeaponData", "Weapon Data", true, mainWeaponClickButton, nullptr));
std::shared_ptr<menuData> characterCreatorMenuSkillData(new menuDataButton(60, 104 + 29 * 4, 180, 29, "CHARACTERCREATORMENU_SkillData", "Skill Data", true, skillClickButton, nullptr));
std::shared_ptr<menuData> characterCreatorMenuSpecialData(new menuDataButton(60, 104 + 29 * 5, 180, 29, "CHARACTERCREATORMENU_SpecialData", "Special Data", true, specialClickButton, nullptr));
std::shared_ptr<menuData> characterCreatorMenuBuffData(new menuDataButton(60, 104 + 29 * 6, 180, 29, "CHARACTERCREATORMENU_BuffData", "Buff Data", true, buffClickButton, nullptr));
std::shared_ptr<menuData> characterCreatorMenuExportCharacter(new menuDataButton(60, 104 + 29 * 7, 180, 29, "CHARACTERCREATORMENU_ExportCharacter", "Export Character", true, exportCharacterClickButton, nullptr));

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

std::shared_ptr<menuData> loadCharacterMenuButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton1", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton2", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton3", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton4", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton5", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton6", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton7", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "LOADCHARACTERMENU_LoadCharacterButton8", "", false, loadCharacterDataButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "LOADCHARACTERMENU_LoadCharacterPrevButton", "Prev", true, prevLoadCharacterButton, nullptr));
std::shared_ptr<menuData> loadCharacterMenuNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "LOADCHARACTERMENU_LoadCharacterNextButton", "Next", true, nextLoadCharacterButton, nullptr));

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

std::shared_ptr<menuData> characterDataMenuCharName(new menuDataTextBoxField(60, 20 + 29 * 0, 130, 20, "CHARACTERDATAMENU_CharName", "charName", true, nullptr, nullptr));
std::shared_ptr<menuData> characterDataMenuCharPortrait(new menuDataButton(60, 20 + 29 * 1, 180, 20, "CHARACTERDATAMENU_CharacterPortrait", "Portrait", true, portraitClickButton, nullptr));
std::shared_ptr<menuData> characterDataMenuCharLargePortrait(new menuDataButton(60, 20 + 29 * 2, 180, 20, "CHARACTERDATAMENU_CharacterLargePortrait", "Large Portrait", true, largePortraitClickButton, nullptr));
std::shared_ptr<menuData> characterDataMenuIdleAnimation(new menuDataButton(60, 20 + 29 * 3, 180, 20, "CHARACTERDATAMENU_IdleAnimation", "Idle Animation", true, idleAnimationClickButton, nullptr));
std::shared_ptr<menuData> characterDataMenuRunAnimation(new menuDataButton(60, 20 + 29 * 4, 180, 20, "CHARACTERDATAMENU_RunAnimation", "Run Animation", true, runAnimationClickButton, nullptr));
std::shared_ptr<menuData> characterDataMenuHP(new menuDataNumberField(60, 20 + 29 * 5, 130, 20, "CHARACTERDATAMENU_HP", "HP", true, nullptr, nullptr));
std::shared_ptr<menuData> characterDataMenuATK(new menuDataNumberField(60, 20 + 29 * 6, 130, 20, "CHARACTERDATAMENU_ATK", "ATK", true, nullptr, nullptr));
std::shared_ptr<menuData> characterDataMenuSPD(new menuDataNumberField(60, 20 + 29 * 7, 130, 20, "CHARACTERDATAMENU_SPD", "SPD", true, nullptr, nullptr));
std::shared_ptr<menuData> characterDataMenuCrit(new menuDataNumberField(60, 20 + 29 * 8, 130, 20, "CHARACTERDATAMENU_Crit", "crit", true, nullptr, nullptr));
std::shared_ptr<menuData> characterDataMenuSizeGrade(new menuDataNumberField(330, 20 + 29 * 0, 130, 20, "CHARACTERDATAMENU_SizeGrade", "Size Grade", true, nullptr, nullptr));

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

std::shared_ptr<menuData> portraitMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "PORTRAITMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "PORTRAITMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "PORTRAITMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "PORTRAITMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "PORTRAITMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "PORTRAITMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "PORTRAITMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "PORTRAITMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "PORTRAITMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuIconNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "PORTRAITMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> portraitMenuPreviewImage(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "PORTRAITMENU_PreviewImage", "", true, nullptr, nullptr, 0));


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

std::shared_ptr<menuData> largePortraitMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "LARGEPORTRAITMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "LARGEPORTRAITMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "LARGEPORTRAITMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "LARGEPORTRAITMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "LARGEPORTRAITMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "LARGEPORTRAITMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "LARGEPORTRAITMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "LARGEPORTRAITMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "LARGEPORTRAITMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "LARGEPORTRAITMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> largePortraitMenuPreviewImage(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "LARGEPORTRAITMENU_PreviewImage", "", true, nullptr, nullptr, 0));

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

std::shared_ptr<menuData> idleAnimationMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "IDLEANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "IDLEANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "IDLEANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "IDLEANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "IDLEANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "IDLEANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "IDLEANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "IDLEANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "IDLEANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuPreviewImage(new menuDataImageField(250, 20 + 29 * 2, 0, 0, "IDLEANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, 4));
std::shared_ptr<menuData> idleAnimationMenuPrevImageButton(new menuDataButton(250, 20 + 29 * 0, 180, 20, "IDLEANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuNextImageButton(new menuDataButton(430, 20 + 29 * 0, 180, 20, "IDLEANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuPlayButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuPauseButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr));
std::shared_ptr<menuData> idleAnimationMenuFrameNumberText(new menuDataText(430, 30 + 29 * 1, 180, 20, "IDLEANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText));

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

std::shared_ptr<menuData> runAnimationMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "RUNANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "RUNANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "RUNANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "RUNANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "RUNANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "RUNANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "RUNANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "RUNANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "RUNANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "RUNANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuPreviewImage(new menuDataImageField(250, 20 + 29 * 2, 0, 0, "RUNANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, 12));
std::shared_ptr<menuData> runAnimationMenuPrevImageButton(new menuDataButton(250, 20 + 29 * 0, 180, 20, "RUNANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuNextImageButton(new menuDataButton(430, 20 + 29 * 0, 180, 20, "RUNANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuPlayButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "RUNANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuPauseButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "RUNANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr));
std::shared_ptr<menuData> runAnimationMenuFrameNumberText(new menuDataText(430, 30 + 29 * 1, 180, 20, "RUNANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText));

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

std::shared_ptr<menuData> weaponDataMenuWeaponName(new menuDataTextBoxField(60, 20 + 29 * 0, 130, 20, "WEAPONDATAMENU_WeaponName", "weaponName", true, nullptr, nullptr));
std::shared_ptr<menuData> weaponDataMenuWeaponIcon(new menuDataButton(60, 20 + 29 * 1, 180, 20, "WEAPONDATAMENU_WeaponIcon", "weaponIcon", true, weaponIconClickButton, nullptr));
std::shared_ptr<menuData> weaponDataMenuAwakenedWeaponIcon(new menuDataButton(60, 20 + 29 * 2, 180, 20, "WEAPONDATAMENU_AwakenedWeaponIcon", "Awakened weaponIcon", true, weaponAwakenedIconClickButton, nullptr));
std::shared_ptr<menuData> weaponDataMenuWeaponLevelMenu(new menuDataButton(60, 20 + 29 * 3, 180, 20, "WEAPONDATAMENU_WeaponLevelMenu", "Weapon Levels", true, weaponLevelMenuClickButton, nullptr));
std::shared_ptr<menuData> weaponDataMenuWeaponAnimation(new menuDataButton(60, 20 + 29 * 4, 180, 20, "WEAPONDATAMENU_WeaponAnimation", "Weapon Animation", true, weaponAnimationClickButton, nullptr));
std::shared_ptr<menuData> weaponDataMenuWeaponType(new menuDataSelection(60, 20 + 29 * 5, 180, 20, "WEAPONDATAMENU_WeaponType", "Weapon Type", true, nullptr, nullptr, { "Melee", "Multishot", "Ranged" }));
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

std::shared_ptr<menuData> weaponIconMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "WEAPONICONMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "WEAPONICONMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "WEAPONICONMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "WEAPONICONMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "WEAPONICONMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "WEAPONICONMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "WEAPONICONMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "WEAPONICONMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "WEAPONICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "WEAPONICONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> weaponIconMenuIconPreviewImage(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "WEAPONICONMENU_PreviewImage", "", true, nullptr, nullptr, 0));

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

std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "WEAPONAWAKENEDICONMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "WEAPONAWAKENEDICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "WEAPONAWAKENEDICONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> weaponAwakenedIconMenuIconPreviewImage(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "WEAPONAWAKENEDICONMENU_PreviewImage", "", true, nullptr, nullptr, 0));

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

std::shared_ptr<menuData> weaponAnimationMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "WEAPONANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "WEAPONANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "WEAPONANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "WEAPONANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "WEAPONANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "WEAPONANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "WEAPONANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "WEAPONANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuIconNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "WEAPONANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuPreviewImage(new menuDataImageField(250, 20 + 29 * 2, 0, 0, "WEAPONANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, 30));
std::shared_ptr<menuData> weaponAnimationMenuPrevImageButton(new menuDataButton(250, 20 + 29 * 0, 180, 20, "WEAPONANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuNextImageButton(new menuDataButton(430, 20 + 29 * 0, 180, 20, "WEAPONANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuPlayButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuPauseButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr));
std::shared_ptr<menuData> weaponAnimationMenuFrameNumberText(new menuDataText(430, 30 + 29 * 1, 180, 20, "WEAPONANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText));

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
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 0, 180, 20, "WEAPONLEVELMENU_Button1", "Level 1", true, weaponLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELMENU_Button2", "Level 2", true, weaponLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELMENU_Button3", "Level 3", true, weaponLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELMENU_Button4", "Level 4", true, weaponLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELMENU_Button5", "Level 5", true, weaponLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELMENU_Button6", "Level 6", true, weaponLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELMENU_Button7", "Level 7", true, weaponLevelClickButton, nullptr)),
});

menuGrid weaponLevelMenuGrid({
	menuColumn(weaponLevelMenuButtonList),
}, "Weapon Level", &weaponDataMenuGrid);

std::shared_ptr<menuData> weaponLevelDataMenuDescription(new menuDataButton(60, 20 + 29 * 0, 180, 20, "WEAPONLEVELDATAMENU_LevelDescription", "Level Description", true, weaponLevelDescriptionClickButton, nullptr));

std::vector<std::shared_ptr<menuData>> weaponLevelAttackTimeMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime1", "Attack Time", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime2", "Attack Time", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime3", "Attack Time", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime4", "Attack Time", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime5", "Attack Time", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime6", "Attack Time", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 1, 180, 20, "WEAPONLEVELDATAMENU_AttackTime7", "Attack Time", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelDurationMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration1", "Duration", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration2", "Duration", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration3", "Duration", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration4", "Duration", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration5", "Duration", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration6", "Duration", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 2, 180, 20, "WEAPONLEVELDATAMENU_Duration7", "Duration", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelDamageMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage1", "Damage", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage2", "Damage", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage3", "Damage", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage4", "Damage", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage5", "Damage", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage6", "Damage", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 3, 180, 20, "WEAPONLEVELDATAMENU_Damage7", "Damage", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelHitLimitMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit1", "Hit Limit", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit2", "Hit Limit", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit3", "Hit Limit", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit4", "Hit Limit", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit5", "Hit Limit", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit6", "Hit Limit", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 4, 180, 20, "WEAPONLEVELDATAMENU_HitLimit7", "Hit Limit", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelSpeedMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed1", "Speed", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed2", "Speed", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed3", "Speed", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed4", "Speed", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed5", "Speed", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed6", "Speed", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 5, 180, 20, "WEAPONLEVELDATAMENU_Speed7", "Speed", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelHitCDMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD1", "HitCD", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD2", "HitCD", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD3", "HitCD", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD4", "HitCD", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD5", "HitCD", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD6", "HitCD", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 6, 180, 20, "WEAPONLEVELDATAMENU_HitCD7", "HitCD", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelAttackCountMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount1", "Count", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount2", "Count", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount3", "Count", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount4", "Count", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount5", "Count", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount6", "Count", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 7, 180, 20, "WEAPONLEVELDATAMENU_AttackCount7", "Count", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelAttackDelayMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay1", "Delay", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay2", "Delay", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay3", "Delay", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay4", "Delay", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay5", "Delay", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay6", "Delay", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 8, 180, 20, "WEAPONLEVELDATAMENU_AttackDelay7", "Delay", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> weaponLevelRangeMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range1", "Range", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range2", "Range", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range3", "Range", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range4", "Range", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range5", "Range", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range6", "Range", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(60, 20 + 29 * 9, 180, 20, "WEAPONLEVELDATAMENU_Range7", "Range", false, nullptr, nullptr)),
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
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description1", "Weapon Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description2", "Weapon Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description3", "Weapon Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description4", "Weapon Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description5", "Weapon Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description6", "Weapon Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "WEAPONLEVELDESCRIPTIONMENU_Description7", "Weapon Description", false, nullptr, nullptr)),
});

menuGrid weaponLevelDescriptionMenuGrid({
	menuColumn(weaponLevelDescriptionMenuList),
}, "Weapon Level Description", &weaponLevelDataMenuGrid);

std::vector<std::shared_ptr<menuData>> skillsMenuButtonList({
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 0, 180, 20, "SKILLSMENU_Button1", "Skill 1", true, skillMenuClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 1, 180, 20, "SKILLSMENU_Button2", "Skill 2", true, skillMenuClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 2, 180, 20, "SKILLSMENU_Button3", "Skill 3", true, skillMenuClickButton, nullptr)),
});

menuGrid skillsMenuGrid({
	menuColumn({
		skillsMenuButtonList[0],
		skillsMenuButtonList[1],
		skillsMenuButtonList[2]
	}),
}, "Skills Menu", &characterCreatorMenuGrid);

std::shared_ptr<menuData> skillIconMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "SKILLICONMENU_IconButton1", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "SKILLICONMENU_IconButton2", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "SKILLICONMENU_IconButton3", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "SKILLICONMENU_IconButton4", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "SKILLICONMENU_IconButton5", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "SKILLICONMENU_IconButton6", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "SKILLICONMENU_IconButton7", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "SKILLICONMENU_IconButton8", "", false, clickSkillIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "SKILLICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> skillIconMenuIconNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "SKILLICONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::vector<std::shared_ptr<menuData>> skillIconMenuIconPreviewImageList({
	std::shared_ptr<menuData>(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "SKILLICONMENU_PreviewImage", "", false, nullptr, nullptr, 0)),
	std::shared_ptr<menuData>(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "SKILLICONMENU_PreviewImage", "", false, nullptr, nullptr, 0)),
	std::shared_ptr<menuData>(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "SKILLICONMENU_PreviewImage", "", false, nullptr, nullptr, 0)),
});

std::shared_ptr<menuData> skillsDataMenuIconButton(new menuDataButton(330, 20 + 29 * 0, 180, 20, "SKILLSMENU_IconButton", "Skill Icon", true, skillIconMenuClickButton, nullptr));
std::vector<std::shared_ptr<menuData>> skillsDataMenuNameList({
	std::shared_ptr<menuData>(new menuDataTextBoxField(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_Name1", "Skill Name", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_Name2", "Skill Name", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_Name3", "Skill Name", false, nullptr, nullptr)),
	});
std::vector<std::shared_ptr<menuData>> skillsDataMenuLevelList({
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 0, 180, 20, "SKILLSMENU_Level1", "Level 1", true, skillLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 1, 180, 20, "SKILLSMENU_Level2", "Level 2", true, skillLevelClickButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 2, 180, 20, "SKILLSMENU_Level3", "Level 3", true, skillLevelClickButton, nullptr)),
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

std::shared_ptr<menuData> skillsDataLevelMenuDescription(new menuDataButton(330, 20 + 29 * 0, 180, 20, "SKILLSMENU_Description", "Skill Description", true, skillDescriptionClickButton, nullptr));

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuAttackButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK1_Level1", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK1_Level2", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK1_Level3", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK2_Level1", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK2_Level2", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK2_Level3", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK3_Level1", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK3_Level2", "ATK+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 0, 130, 20, "SKILLSMENU_ATK3_Level3", "ATK+", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuCritButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT1_Level1", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT1_Level2", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT1_Level3", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT2_Level1", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT2_Level2", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT2_Level3", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT3_Level1", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT3_Level2", "CRIT+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "SKILLSMENU_CRIT3_Level3", "CRIT+", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuHasteButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE1_Level1", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE1_Level2", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE1_Level3", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE2_Level1", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE2_Level2", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE2_Level3", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE3_Level1", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE3_Level2", "Haste+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "SKILLSMENU_HASTE3_Level3", "Haste+", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuSpeedButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD1_Level1", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD1_Level2", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD1_Level3", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD2_Level1", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD2_Level2", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD2_Level3", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD3_Level1", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD3_Level2", "SPD+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "SKILLSMENU_SPD3_Level3", "SPD+", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuDRButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR1_Level1", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR1_Level2", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR1_Level3", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR2_Level1", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR2_Level2", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR2_Level3", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR3_Level1", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR3_Level2", "DR*", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SKILLSMENU_DR3_Level3", "DR*", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuHealMultiplierButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier1_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier1_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier1_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier2_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier2_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier2_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier3_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier3_Level1", "healMultiplier+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "SKILLSMENU_HealMultiplier3_Level1", "healMultiplier+", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuFoodButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food1_Level1", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food1_Level2", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food1_Level3", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food2_Level1", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food2_Level2", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food2_Level3", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food3_Level1", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food3_Level2", "food+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "SKILLSMENU_Food3_Level3", "food+", false, nullptr, nullptr)),
});

std::vector<std::shared_ptr<menuData>> skillsDataLevelMenuWeaponSizeMultiplierButtonList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier1_Level1", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier1_Level2", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier1_Level3", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier2_Level1", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier2_Level2", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier2_Level3", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier3_Level1", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier3_Level2", "weaponSizeMult+", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "SKILLSMENU_WeaponSizeMultiplier3_Level3", "weaponSizeMult+", false, nullptr, nullptr)),
});

std::shared_ptr<menuData> skillsDataLevelMenuOnTriggerButton(new menuDataButton(330, 20 + 29 * 1, 180, 20, "SKILLSMENU_OnTrigger", "OnTrigger", true, skillOnTriggerClickButton, nullptr));

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
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description1_Level1", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description1_Level2", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description1_Level3", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description2_Level1", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description2_Level2", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description2_Level3", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description3_Level1", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description3_Level2", "Skill Description", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SKILLDESCRIPTIONMENU_Description3_Level3", "Skill Description", false, nullptr, nullptr)),
});

menuGrid skillDataLevelDescriptionMenuGrid({
	menuColumn(skillDataLevelDescriptionMenuList),
}, "Skills Description Menu", &skillsDataLevelMenuGrid);

std::vector<std::shared_ptr<menuData>> skillOnTriggerBuffSelectionMenuList({
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection1_Level1", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection1_Level2", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection1_Level3", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection2_Level1", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection2_Level2", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection2_Level3", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection3_Level1", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection3_Level2", "Select Buff", false, nullptr, nullptr, { "" })),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 0, 180, 20, "SKILLONTRIGGERMENU_BuffSelection3_Level3", "Select Buff", false, nullptr, nullptr, { "" })),
});

// TODO: Should probably improve the onTrigger selection to not need to default to none
std::vector<std::string> onTriggerTypes{ "NONE", "onDebuff", "onAttackCreate", "onCriticalHit", "onHeal", "onKill", "onTakeDamage", "onDodge" };

std::vector<std::shared_ptr<menuData>> skillOnTriggerTypeSelectionMenuList({
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection1_Level1", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection1_Level2", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection1_Level3", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection2_Level1", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection2_Level2", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection2_Level3", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection3_Level1", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection3_Level2", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
	std::shared_ptr<menuData>(new menuDataSelection(100, 20 + 29 * 1, 180, 20, "SKILLONTRIGGERMENU_TypeSelection3_Level3", "onTrigger", false, nullptr, nullptr, onTriggerTypes)),
});

std::vector<std::shared_ptr<menuData>> skillOnTriggerProbabilityMenuList({
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability1_Level1", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability1_Level2", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability1_Level3", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability2_Level1", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability2_Level2", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability2_Level3", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability3_Level1", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability3_Level2", "Probability", false, nullptr, nullptr)),
	std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SKILLONTRIGGERMENU_Probability3_Level3", "Probability", false, nullptr, nullptr)),
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

std::shared_ptr<menuData> specialMenuSpecialAnimation(new menuDataButton(100, 20 + 29 * 0, 180, 20, "SPECIALMENU_SpecialAnimation", "Special Animation", true, specialAnimationClickButton, nullptr));
std::shared_ptr<menuData> specialMenuDamage(new menuDataNumberField(100, 20 + 29 * 1, 180, 20, "SPECIALMENU_Damage", "Damage", true, nullptr, nullptr));
std::shared_ptr<menuData> specialMenuDuration(new menuDataNumberField(100, 20 + 29 * 2, 180, 20, "SPECIALMENU_Duration", "Duration", true, nullptr, nullptr));
std::shared_ptr<menuData> specialMenuSpecialIcon(new menuDataButton(100, 20 + 29 * 3, 180, 20, "SPECIALMENU_SpecialIcon", "Special Icon", true, specialIconClickButton, nullptr));
std::shared_ptr<menuData> specialMenuSpecialCooldown(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "SPECIALMENU_SpecialCooldown", "Special Cooldown", true, nullptr, nullptr));
std::shared_ptr<menuData> specialMenuSpecialName(new menuDataTextBoxField(100, 20 + 29 * 5, 130, 20, "SPECIALMENU_SpecialName", "Special Name", true, nullptr, nullptr));
std::shared_ptr<menuData> specialMenuSpecialDescription(new menuDataButton(100, 20 + 29 * 6, 180, 20, "SPECIALMENU_SpecialDesc", "Special Description", true, specialDescriptionClickButton, nullptr));

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

std::shared_ptr<menuData> specialAnimationMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "SPECIALANIMATIONMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "SPECIALANIMATIONMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "SPECIALANIMATIONMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "SPECIALANIMATIONMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "SPECIALANIMATIONMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "SPECIALANIMATIONMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "SPECIALANIMATIONMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "SPECIALANIMATIONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuIconNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "SPECIALANIMATIONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuPreviewImage(new menuDataImageField(250, 20 + 29 * 2, 0, 0, "SPECIALANIMATIONMENU_PreviewImage", "", true, nullptr, nullptr, 30));
std::shared_ptr<menuData> specialAnimationMenuPrevImageButton(new menuDataButton(250, 20 + 29 * 0, 180, 20, "SPECIALANIMATIONMENU_PrevImageButton", "Prev", true, prevImageButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuNextImageButton(new menuDataButton(430, 20 + 29 * 0, 180, 20, "SPECIALANIMATIONMENU_NextImageButton", "Next", true, nextImageButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuPlayButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_PlayButton", "Play", true, playPauseButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuPauseButton(new menuDataButton(250, 20 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_PauseButton", "Pause", false, playPauseButton, nullptr));
std::shared_ptr<menuData> specialAnimationMenuFrameNumberText(new menuDataText(430, 30 + 29 * 1, 180, 20, "SPECIALANIMATIONMENU_FrameNumberText", "", true, nullptr, animationFrameText));

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


std::shared_ptr<menuData> specialIconMenuIconButton1(new menuDataButton(60, 20 + 29 * 0, 180, 20, "SPECIALICONMENU_IconButton1", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconButton2(new menuDataButton(60, 20 + 29 * 1, 180, 20, "SPECIALICONMENU_IconButton2", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconButton3(new menuDataButton(60, 20 + 29 * 2, 180, 20, "SPECIALICONMENU_IconButton3", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconButton4(new menuDataButton(60, 20 + 29 * 3, 180, 20, "SPECIALICONMENU_IconButton4", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconButton5(new menuDataButton(60, 20 + 29 * 4, 180, 20, "SPECIALICONMENU_IconButton5", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconButton6(new menuDataButton(60, 20 + 29 * 5, 180, 20, "SPECIALICONMENU_IconButton6", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconButton7(new menuDataButton(60, 20 + 29 * 6, 180, 20, "SPECIALICONMENU_IconButton7", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconButton8(new menuDataButton(60, 20 + 29 * 7, 180, 20, "SPECIALICONMENU_IconButton8", "", false, clickIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "SPECIALICONMENU_PrevButton", "Prev", true, prevIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "SPECIALICONMENU_NextButton", "Next", true, nextIconButton, nullptr));
std::shared_ptr<menuData> specialIconMenuIconPreviewImage(new menuDataImageField(250, 20 + 29 * 0, 0, 0, "SPECIALICONMENU_PreviewImage", "", true, nullptr, nullptr, 0));

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

std::shared_ptr<menuData> specialDescriptionMenuSpecialDescription(new menuDataTextBoxField(150, 20 + 29 * 0, 300, 200, "SPECIALDESCRIPTIONMENU_SpecialDescription", "Special Description", true, nullptr, nullptr));

menuGrid specialDescriptionMenuGrid({
	menuColumn({
		specialDescriptionMenuSpecialDescription
	}),
}, "Special Description", &specialMenuGrid);

int buffMenuPage = 0;
std::deque<buffDataMenuGrid> buffMenuDeque;

std::vector<std::shared_ptr<menuData>> buffMenuButtonList({
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 0, 180, 20, "BUFFMENU_Button1", "", false, clickBuffMenuButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 1, 180, 20, "BUFFMENU_Button2", "", false, clickBuffMenuButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 2, 180, 20, "BUFFMENU_Button3", "", false, clickBuffMenuButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 3, 180, 20, "BUFFMENU_Button4", "", false, clickBuffMenuButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 4, 180, 20, "BUFFMENU_Button5", "", false, clickBuffMenuButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 5, 180, 20, "BUFFMENU_Button6", "", false, clickBuffMenuButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 6, 180, 20, "BUFFMENU_Button7", "", false, clickBuffMenuButton, nullptr)),
	std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 7, 180, 20, "BUFFMENU_Button8", "", false, clickBuffMenuButton, nullptr)),
});
std::shared_ptr<menuData> buffMenuPrevButton(new menuDataButton(60, 20 + 29 * 8, 180, 20, "BUFFMENU_PrevButton", "Prev", true, prevBuffButton, nullptr));
std::shared_ptr<menuData> buffMenuNextButton(new menuDataButton(60, 20 + 29 * 9, 180, 20, "BUFFMENU_NextButton", "Next", true, nextBuffButton, nullptr));
std::shared_ptr<menuData> buffMenuAddBuffButton(new menuDataButton(330, 20 + 29 * 9, 180, 20, "BUFFMENU_AddBuffButton", "Add Buff", true, addBuffMenuButton, nullptr));

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
	try
	{
		nlohmann::json inputData = nlohmann::json::parse(inFile);
		charData = inputData.template get<characterData>();
	}
	catch (nlohmann::json::parse_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Parse Error: %s when parsing %s", e.what(), dirName.c_str());
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
		static_cast<menuDataTextBoxField*>(characterDataMenuCharName.get())->textField = charData.charName;
		static_cast<menuDataImageField*>(portraitMenuPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.portraitFileName, charData.portraitFileName, 1));
		static_cast<menuDataImageField*>(largePortraitMenuPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.largePortraitFileName, charData.largePortraitFileName, 1));

		static_cast<menuDataImageField*>(idleAnimationMenuPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.idleAnimationFileName, charData.idleAnimationFileName, getSpriteNumFrames(charData.idleAnimationFileName)));
		static_cast<menuDataImageField*>(idleAnimationMenuPreviewImage.get())->fps = charData.idleAnimationFPS.value;

		static_cast<menuDataImageField*>(runAnimationMenuPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.runAnimationFileName, charData.runAnimationFileName, getSpriteNumFrames(charData.runAnimationFileName)));
		static_cast<menuDataImageField*>(runAnimationMenuPreviewImage.get())->fps = charData.runAnimationFPS.value;
		static_cast<menuDataNumberField*>(characterDataMenuHP.get())->textField = std::format("{}", charData.hp.value);
		static_cast<menuDataNumberField*>(characterDataMenuATK.get())->textField = std::format("{}", charData.atk.value);
		static_cast<menuDataNumberField*>(characterDataMenuSPD.get())->textField = std::format("{}", charData.spd.value);
		static_cast<menuDataNumberField*>(characterDataMenuCrit.get())->textField = std::format("{}", charData.crit.value);
		static_cast<menuDataImageField*>(weaponIconMenuIconPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackIconFileName, charData.attackIconFileName, 1));
		static_cast<menuDataImageField*>(weaponAwakenedIconMenuIconPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackAwakenedIconFileName, charData.attackAwakenedIconFileName, 1));
		static_cast<menuDataTextBoxField*>(weaponDataMenuWeaponName.get())->textField = charData.attackName;
		if (charData.mainWeaponWeaponType.compare("Melee") == 0)
		{
			static_cast<menuDataSelection*>(weaponDataMenuWeaponType.get())->curSelectionTextIndex = 0;
		}
		else if (charData.mainWeaponWeaponType.compare("Multishot") == 0)
		{
			static_cast<menuDataSelection*>(weaponDataMenuWeaponType.get())->curSelectionTextIndex = 1;
		}
		else if (charData.mainWeaponWeaponType.compare("Ranged") == 0)
		{
			static_cast<menuDataSelection*>(weaponDataMenuWeaponType.get())->curSelectionTextIndex = 2;
		}
		static_cast<menuDataImageField*>(weaponAnimationMenuPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.attackAnimationFileName, charData.attackAnimationFileName, getSpriteNumFrames(charData.attackAnimationFileName)));
		static_cast<menuDataImageField*>(weaponAnimationMenuPreviewImage.get())->fps = charData.attackAnimationFPS.value;
		static_cast<menuDataNumberField*>(characterDataMenuSizeGrade.get())->textField = std::format("{}", charData.sizeGrade.value);
		for (int i = 0; i < charData.weaponLevelDataList.size(); i++)
		{
			auto& curWeaponLevelData = charData.weaponLevelDataList[i];
			static_cast<menuDataTextBoxField*>(weaponLevelDescriptionMenuList[i].get())->textField = curWeaponLevelData.attackDescription;
			if (curWeaponLevelData.attackTime.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelAttackTimeMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.attackTime.value);
			}
			if (curWeaponLevelData.duration.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelDurationMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.duration.value);
			}
			if (curWeaponLevelData.damage.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelDamageMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.damage.value);
			}
			if (curWeaponLevelData.hitLimit.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelHitLimitMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.hitLimit.value);
			}
			if (curWeaponLevelData.speed.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelSpeedMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.speed.value);
			}
			if (curWeaponLevelData.hitCD.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelHitCDMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.hitCD.value);
			}
			if (curWeaponLevelData.attackCount.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelAttackCountMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.attackCount.value);
			}
			if (curWeaponLevelData.attackDelay.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelAttackDelayMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.attackDelay.value);
			}
			if (curWeaponLevelData.range.isDefined)
			{
				static_cast<menuDataNumberField*>(weaponLevelRangeMenuList[i].get())->textField = std::format("{}", curWeaponLevelData.range.value);
			}
		}
		static_cast<menuDataImageField*>(specialIconMenuIconPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.specialIconFileName, charData.specialIconFileName, 1));
		static_cast<menuDataNumberField*>(specialMenuSpecialCooldown.get())->textField = std::format("{}", charData.specialCooldown.value);
		static_cast<menuDataTextBoxField*>(specialMenuSpecialName.get())->textField = charData.specialName;
		static_cast<menuDataTextBoxField*>(specialDescriptionMenuSpecialDescription.get())->textField = charData.specialDescription;
		static_cast<menuDataImageField*>(specialAnimationMenuPreviewImage.get())->fps = charData.specialAnimationFPS.value;
		static_cast<menuDataImageField*>(specialAnimationMenuPreviewImage.get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + charData.specialAnimationFileName, charData.specialAnimationFileName, getSpriteNumFrames(charData.specialAnimationFileName)));
		static_cast<menuDataNumberField*>(specialMenuDamage.get())->textField = std::format("{}", charData.specialDamage.value);
		static_cast<menuDataNumberField*>(specialMenuDuration.get())->textField = std::format("{}", charData.specialDuration.value);

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
			static_cast<menuDataTextBoxField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[0].get())->textField = curBuffData.buffName;
			if (curBuffData.levels[0].attackIncrement.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[1].get())->textField = std::format("{}", curBuffData.levels[0].attackIncrement.value);
			}
			if (curBuffData.levels[0].critIncrement.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[2].get())->textField = std::format("{}", curBuffData.levels[0].critIncrement.value);
			}
			if (curBuffData.levels[0].hasteIncrement.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[3].get())->textField = std::format("{}", curBuffData.levels[0].hasteIncrement.value);
			}
			if (curBuffData.levels[0].speedIncrement.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[4].get())->textField = std::format("{}", curBuffData.levels[0].speedIncrement.value);
			}
			if (curBuffData.levels[0].DRMultiplier.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[5].get())->textField = std::format("{}", curBuffData.levels[0].DRMultiplier.value);
			}
			if (curBuffData.levels[0].healMultiplier.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[6].get())->textField = std::format("{}", curBuffData.levels[0].healMultiplier.value);
			}
			if (curBuffData.levels[0].food.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[7].get())->textField = std::format("{}", curBuffData.levels[0].food.value);
			}
			if (curBuffData.levels[0].weaponSize.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[8].get())->textField = std::format("{}", curBuffData.levels[0].weaponSize.value);
			}
			if (curBuffData.levels[0].maxStacks.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[0].menuDataPtrList[9].get())->textField = std::format("{}", curBuffData.levels[0].maxStacks.value);
			}
			if (curBuffData.levels[0].timer.isDefined)
			{
				static_cast<menuDataNumberField*>(curBuffDataGrid->menuColumnList[1].menuDataPtrList[0].get())->textField = std::format("{}", curBuffData.levels[0].timer.value);
			}
			static_cast<menuDataImageField*>(curBuffIconGrid->menuColumnList[1].menuDataPtrList[0].get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + curBuffData.buffIconFileName, curBuffData.buffIconFileName, 1));
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
					static_cast<menuDataNumberField*>(skillsDataLevelMenuAttackButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.attackIncrement.value);
				}
				if (curSkillLevelData.critIncrement.isDefined)
				{
					static_cast<menuDataNumberField*>(skillsDataLevelMenuCritButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.critIncrement.value);
				}
				if (curSkillLevelData.hasteIncrement.isDefined)
				{
					static_cast<menuDataNumberField*>(skillsDataLevelMenuHasteButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.hasteIncrement.value);
				}
				if (curSkillLevelData.speedIncrement.isDefined)
				{
					static_cast<menuDataNumberField*>(skillsDataLevelMenuSpeedButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.speedIncrement.value);
				}
				if (curSkillLevelData.DRMultiplier.isDefined)
				{
					static_cast<menuDataNumberField*>(skillsDataLevelMenuDRButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.DRMultiplier.value);
				}
				if (curSkillLevelData.healMultiplier.isDefined)
				{
					static_cast<menuDataNumberField*>(skillsDataLevelMenuHealMultiplierButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.healMultiplier.value);
				}
				if (curSkillLevelData.food.isDefined)
				{
					static_cast<menuDataNumberField*>(skillsDataLevelMenuFoodButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.food.value);
				}
				if (curSkillLevelData.weaponSize.isDefined)
				{
					static_cast<menuDataNumberField*>(skillsDataLevelMenuWeaponSizeMultiplierButtonList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.weaponSize.value);
				}
				static_cast<menuDataTextBoxField*>(skillDataLevelDescriptionMenuList[i * 3 + j].get())->textField = curSkillLevelData.skillDescription;
				for (int buffIndex = 0; buffIndex < static_cast<menuDataSelection*>(skillOnTriggerBuffSelectionMenuList[i * 3 + j].get())->selectionText.size(); buffIndex++)
				{
					if (static_cast<menuDataSelection*>(skillOnTriggerBuffSelectionMenuList[i * 3 + j].get())->selectionText[buffIndex].compare(curSkillLevelData.skillOnTriggerData.buffName) == 0)
					{
						static_cast<menuDataSelection*>(skillOnTriggerBuffSelectionMenuList[i * 3 + j].get())->curSelectionTextIndex = buffIndex;
						break;
					}
				}
				for (int buffIndex = 0; buffIndex < static_cast<menuDataSelection*>(skillOnTriggerTypeSelectionMenuList[i * 3 + j].get())->selectionText.size(); buffIndex++)
				{
					if (static_cast<menuDataSelection*>(skillOnTriggerTypeSelectionMenuList[i * 3 + j].get())->selectionText[buffIndex].compare(curSkillLevelData.skillOnTriggerData.onTriggerType) == 0)
					{
						static_cast<menuDataSelection*>(skillOnTriggerTypeSelectionMenuList[i * 3 + j].get())->curSelectionTextIndex = buffIndex;
						break;
					}
				}
				if (curSkillLevelData.skillOnTriggerData.probability.isDefined)
				{
					static_cast<menuDataNumberField*>(skillOnTriggerProbabilityMenuList[i * 3 + j].get())->textField = std::format("{}", curSkillLevelData.skillOnTriggerData.probability.value);
				}
			}

			static_cast<menuDataTextBoxField*>(skillsDataMenuNameList[i].get())->textField = curSkillData.skillName;
			static_cast<menuDataImageField*>(skillIconMenuIconPreviewImageList[i].get())->curSprite = std::shared_ptr<spriteData>(new spriteData(dirStr + curSkillData.skillIconFileName, curSkillData.skillIconFileName, 1));
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
		menuDataPtrList[i]->isVisible = false;
	}
	for (int i = 0; i < 8 && spriteDequePage * 8 + i < spriteDeque.size(); i++)
	{
		auto& menuData = menuDataPtrList[i];
		menuData->labelName = spriteDeque[spriteDequePage * 8 + i]->spriteFileName;
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
	auto curMenuData = static_cast<menuDataImageField*>(curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList[0].get());
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
	auto curMenuData = static_cast<menuDataImageField*>(curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList[0].get());
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
	static_cast<menuDataImageField*>(menuDataList[0].get())->curFrameCount = menuDataList[3]->isVisible ? -1 : 0;
}

void animationFrameText()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto& menuDataList = curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList;
	auto previewImage = static_cast<menuDataImageField*>(menuDataList[0].get());
	auto& frameText = menuDataList[5];
	int curImageIndex = previewImage->curSprite == nullptr ? 0 : previewImage->curSubImageIndex + 1;
	int maxFrames = previewImage->curSprite == nullptr ? 0 : previewImage->curSprite->numFrames;
	frameText->labelName = std::format("{} / {}", curImageIndex, maxFrames);
}

void clickIconButton()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto previewImageData = static_cast<menuDataImageField*>(curMenuGrid->menuColumnsPtrList[1]->menuDataPtrList[0].get());
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	int selectedIconIndex = -1;
	auto& menuDataPtrList = curMenuGrid->menuColumnsPtrList[0]->menuDataPtrList;
	for (int i = 0; i < menuDataPtrList.size(); i++)
	{
		if (menuDataPtrList[i].get() == selectedMenuData.get())
		{
			selectedIconIndex = i;
			break;
		}
	}
	if (selectedIconIndex == -1)
	{
		g_ModuleInterface->Print(CM_RED, "Couldn't find icon");
		return;
	}
	previewImageData->curSprite = spriteDeque[spriteDequePage * 8 + selectedIconIndex];
}

void clickSkillIconButton()
{
	std::shared_ptr<menuGridData> curMenuGrid;
	holoCureMenuInterfacePtr->GetCurrentMenuGrid(MODNAME, curMenuGrid);
	auto previewImageData = static_cast<menuDataImageField*>(skillIconMenuIconPreviewImageList[skillMenuIndex].get());
	std::shared_ptr<menuData> selectedMenuData;
	holoCureMenuInterfacePtr->GetSelectedMenuData(MODNAME, selectedMenuData);
	int selectedIconIndex = -1;
	auto& menuDataPtrList = curMenuGrid->menuColumnsPtrList[0]->menuDataPtrList;
	for (int i = 0; i < menuDataPtrList.size(); i++)
	{
		if (menuDataPtrList[i].get() == selectedMenuData.get())
		{
			selectedIconIndex = i;
			break;
		}
	}
	if (selectedIconIndex == -1)
	{
		g_ModuleInterface->Print(CM_RED, "Couldn't find icon");
		return;
	}
	previewImageData->curSprite = spriteDeque[spriteDequePage * 8 + selectedIconIndex];
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
		buffMenuButtonList[i]->isVisible = false;
	}
	for (int i = 0; i < 8 && buffMenuPage * 8 + i < buffMenuDeque.size(); i++)
	{
		auto& menuData = buffMenuButtonList[i];
		menuData->labelName = static_cast<menuDataTextBoxField*>(buffMenuDeque[buffMenuPage * 8 + i].buffDataGrid->menuColumnList[0].menuDataPtrList[0].get())->textField;
		menuData->isVisible = true;
	}
	std::vector<std::string> buffNameList;
	for (auto& curBuffMenuGrid : buffMenuDeque)
	{
		auto& buffName = static_cast<menuDataTextBoxField*>(curBuffMenuGrid.buffDataGrid->menuColumnList[0].menuDataPtrList[0].get())->textField;
		buffNameList.push_back(buffName);
	}
	for (size_t i = 0; i < skillOnTriggerBuffSelectionMenuList.size(); i++)
	{
		static_cast<menuDataSelection*>(skillOnTriggerBuffSelectionMenuList[i].get())->selectionText = buffNameList;
		static_cast<menuDataSelection*>(skillOnTriggerBuffSelectionMenuList[i].get())->curSelectionTextIndex = 0;
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
			std::shared_ptr<menuData>(new menuDataTextBoxField(100, 20 + 29 * 0, 130, 20, "BUFFMENU_NAME", "buff name", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 1, 130, 20, "BUFFMENU_ATK", "ATK+", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 2, 130, 20, "BUFFMENU_CRIT", "CRIT+", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 3, 130, 20, "BUFFMENU_HASTE", "Haste+", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 4, 130, 20, "BUFFMENU_SPEED", "SPD+", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 5, 130, 20, "BUFFMENU_DR", "DR*", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 6, 130, 20, "BUFFMENU_HealMultiplier", "HealMultiplier+", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 7, 130, 20, "BUFFMENU_Food", "Food+", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 8, 130, 20, "BUFFMENU_WeaponSizeMultiplier", "WeaponSizeMult+", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataNumberField(100, 20 + 29 * 9, 130, 20, "BUFFMENU_MaxStacks", "Max Stacks", true, nullptr, nullptr)),
		}),
		menuColumn({
			std::shared_ptr<menuData>(new menuDataNumberField(330, 20 + 29 * 0, 130, 20, "BUFFMENU_Timer", "Timer", true, nullptr, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(330, 20 + 29 * 1, 180, 20, "BUFFMENU_BuffIcon", "Buff Icon", true, buffIconClickButton, nullptr)),
		}),
	}, "Buff Data Menu", &buffMenuGrid));

	std::shared_ptr<menuGrid> newBuffIconMenuGrid = std::shared_ptr<menuGrid>(new menuGrid({
		menuColumn({
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 0, 180, 20, "BUFFMENU_IconButton1", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 1, 180, 20, "BUFFMENU_IconButton2", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 2, 180, 20, "BUFFMENU_IconButton3", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 3, 180, 20, "BUFFMENU_IconButton4", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 4, 180, 20, "BUFFMENU_IconButton5", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 5, 180, 20, "BUFFMENU_IconButton6", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 6, 180, 20, "BUFFMENU_IconButton7", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 7, 180, 20, "BUFFMENU_IconButton8", "", false, clickIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 8, 180, 20, "BUFFMENU_PrevButton", "Prev", true, prevIconButton, nullptr)),
			std::shared_ptr<menuData>(new menuDataButton(60, 20 + 29 * 9, 180, 20, "BUFFMENU_NextButton", "Next", true, nextIconButton, nullptr)),
		}),
		menuColumn({
			std::shared_ptr<menuData>(new menuDataImageField(250, 20 + 29 * 2, 0, 0, "BUFFMENU_PreviewImage", "", true, nullptr, nullptr, 30)),
		}),
	}, "Buff Icon Menu", newBuffDataMenuGrid.get()));

	static_cast<menuDataTextBoxField*>(newBuffDataMenuGrid->menuColumnList[0].menuDataPtrList[0].get())->textField = "newBuff";
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
	auto charName = static_cast<menuDataTextBoxField*>(characterDataMenuCharName.get())->textField;
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
	outCharData.idleAnimationFPS.value = static_cast<menuDataImageField*>(idleAnimationMenuPreviewImage.get())->fps;
	auto idleAnimationFileName = getSpriteFileName(idleAnimationMenuPreviewImage);
	outCharData.idleAnimationFileName = idleAnimationFileName;
	copySpriteToDestination(idleAnimationMenuPreviewImage, charDirName + "/" + idleAnimationFileName);

	outCharData.runAnimationFPS.isDefined = true;
	outCharData.runAnimationFPS.value = static_cast<menuDataImageField*>(runAnimationMenuPreviewImage.get())->fps;
	auto runAnimationFileName = getSpriteFileName(runAnimationMenuPreviewImage);
	outCharData.runAnimationFileName = runAnimationFileName;
	copySpriteToDestination(runAnimationMenuPreviewImage, charDirName + "/" + runAnimationFileName);

	parseStringToJSONDouble(static_cast<menuDataNumberField*>(characterDataMenuHP.get())->textField, outCharData.hp, false);
	parseStringToJSONDouble(static_cast<menuDataNumberField*>(characterDataMenuATK.get())->textField, outCharData.atk, false);
	parseStringToJSONDouble(static_cast<menuDataNumberField*>(characterDataMenuSPD.get())->textField, outCharData.spd, false);
	parseStringToJSONDouble(static_cast<menuDataNumberField*>(characterDataMenuCrit.get())->textField, outCharData.crit, false);

	auto attackIconFileName = getSpriteFileName(weaponIconMenuIconPreviewImage);
	outCharData.attackIconFileName = attackIconFileName;
	copySpriteToDestination(weaponIconMenuIconPreviewImage, charDirName + "/" + attackIconFileName);
	
	auto attackAwakenedIconFileName = getSpriteFileName(weaponAwakenedIconMenuIconPreviewImage);
	outCharData.attackAwakenedIconFileName = attackAwakenedIconFileName;
	copySpriteToDestination(weaponAwakenedIconMenuIconPreviewImage, charDirName + "/" + attackAwakenedIconFileName);
	
	outCharData.attackAnimationFPS.isDefined = true;
	outCharData.attackAnimationFPS.value = static_cast<menuDataImageField*>(weaponAnimationMenuPreviewImage.get())->fps;
	auto attackAnimationFileName = getSpriteFileName(weaponAnimationMenuPreviewImage);
	outCharData.attackAnimationFileName = attackAnimationFileName;
	copySpriteToDestination(weaponAnimationMenuPreviewImage, charDirName + "/" + attackAnimationFileName);
	outCharData.attackName = static_cast<menuDataTextBoxField*>(weaponDataMenuWeaponName.get())->textField;

	outCharData.mainWeaponWeaponType = static_cast<menuDataSelection*>(weaponDataMenuWeaponType.get())->selectionText[static_cast<menuDataSelection*>(weaponDataMenuWeaponType.get())->curSelectionTextIndex];
	auto specialIconFileName = getSpriteFileName(specialIconMenuIconPreviewImage);
	outCharData.specialIconFileName = specialIconFileName;
	copySpriteToDestination(specialIconMenuIconPreviewImage, charDirName + "/" + specialIconFileName);
	parseStringToJSONInt(static_cast<menuDataNumberField*>(specialMenuSpecialCooldown.get())->textField, outCharData.specialCooldown);
	outCharData.specialName = static_cast<menuDataTextBoxField*>(specialMenuSpecialName.get())->textField;
	outCharData.specialDescription = static_cast<menuDataTextBoxField*>(specialDescriptionMenuSpecialDescription.get())->textField;
	parseStringToJSONInt(static_cast<menuDataNumberField*>(characterDataMenuSizeGrade.get())->textField, outCharData.sizeGrade);
	
	for (int i = 0; i < weaponLevelDescriptionMenuList.size(); i++)
	{
		weaponLevelData curWeaponLevelData;
		curWeaponLevelData.attackDescription = static_cast<menuDataTextBoxField*>(weaponLevelDescriptionMenuList[i].get())->textField;
		parseStringToJSONInt(static_cast<menuDataNumberField*>(weaponLevelAttackTimeMenuList[i].get())->textField, curWeaponLevelData.attackTime);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(weaponLevelDurationMenuList[i].get())->textField, curWeaponLevelData.duration);
		parseStringToJSONDouble(static_cast<menuDataNumberField*>(weaponLevelDamageMenuList[i].get())->textField, curWeaponLevelData.damage);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(weaponLevelHitLimitMenuList[i].get())->textField, curWeaponLevelData.hitLimit);
		parseStringToJSONDouble(static_cast<menuDataNumberField*>(weaponLevelSpeedMenuList[i].get())->textField, curWeaponLevelData.speed);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(weaponLevelHitCDMenuList[i].get())->textField, curWeaponLevelData.hitCD);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(weaponLevelAttackCountMenuList[i].get())->textField, curWeaponLevelData.attackCount);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(weaponLevelAttackDelayMenuList[i].get())->textField, curWeaponLevelData.attackDelay);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(weaponLevelRangeMenuList[i].get())->textField, curWeaponLevelData.range);
		outCharData.weaponLevelDataList.push_back(curWeaponLevelData);
	}

	for (int i = 0; i < 3; i++)
	{
		skillData curSkillData;
		for (int j = 0; j < 3; j++)
		{
			skillLevelData curSkillLevelData;
			int listIndex = i * 3 + j;
			
			parseStringToJSONInt(static_cast<menuDataNumberField*>(skillsDataLevelMenuAttackButtonList[listIndex].get())->textField, curSkillLevelData.attackIncrement);
			parseStringToJSONInt(static_cast<menuDataNumberField*>(skillsDataLevelMenuCritButtonList[listIndex].get())->textField, curSkillLevelData.critIncrement);
			parseStringToJSONInt(static_cast<menuDataNumberField*>(skillsDataLevelMenuHasteButtonList[listIndex].get())->textField, curSkillLevelData.hasteIncrement);
			parseStringToJSONInt(static_cast<menuDataNumberField*>(skillsDataLevelMenuSpeedButtonList[listIndex].get())->textField, curSkillLevelData.speedIncrement);
			parseStringToJSONDouble(static_cast<menuDataNumberField*>(skillsDataLevelMenuDRButtonList[listIndex].get())->textField, curSkillLevelData.DRMultiplier);
			parseStringToJSONDouble(static_cast<menuDataNumberField*>(skillsDataLevelMenuHealMultiplierButtonList[listIndex].get())->textField, curSkillLevelData.healMultiplier);
			parseStringToJSONDouble(static_cast<menuDataNumberField*>(skillsDataLevelMenuFoodButtonList[listIndex].get())->textField, curSkillLevelData.food);
			parseStringToJSONDouble(static_cast<menuDataNumberField*>(skillsDataLevelMenuWeaponSizeMultiplierButtonList[listIndex].get())->textField, curSkillLevelData.weaponSize);
			curSkillLevelData.skillDescription = static_cast<menuDataTextBoxField*>(skillDataLevelDescriptionMenuList[listIndex].get())->textField;
			curSkillLevelData.skillOnTriggerData.buffName = static_cast<menuDataSelection*>(skillOnTriggerBuffSelectionMenuList[listIndex].get())->selectionText[static_cast<menuDataSelection*>(skillOnTriggerBuffSelectionMenuList[listIndex].get())->curSelectionTextIndex];
			curSkillLevelData.skillOnTriggerData.onTriggerType = static_cast<menuDataSelection*>(skillOnTriggerTypeSelectionMenuList[listIndex].get())->selectionText[static_cast<menuDataSelection*>(skillOnTriggerTypeSelectionMenuList[listIndex].get())->curSelectionTextIndex];
			parseStringToJSONInt(static_cast<menuDataNumberField*>(skillOnTriggerProbabilityMenuList[listIndex].get())->textField, curSkillLevelData.skillOnTriggerData.probability);
			curSkillData.skillLevelDataList.push_back(curSkillLevelData);
		}
		curSkillData.skillName = static_cast<menuDataTextBoxField*>(skillsDataMenuNameList[i].get())->textField;
		auto skillIconFileName = getSpriteFileName(skillIconMenuIconPreviewImageList[i]);
		curSkillData.skillIconFileName = skillIconFileName;
		copySpriteToDestination(skillIconMenuIconPreviewImageList[i], charDirName + "/" + skillIconFileName);

		outCharData.skillDataList.push_back(curSkillData);
	}
	outCharData.specialAnimationFPS.isDefined = true;
	outCharData.specialAnimationFPS.value = static_cast<menuDataImageField*>(specialAnimationMenuPreviewImage.get())->fps;
	auto specialAnimationFileName = getSpriteFileName(specialAnimationMenuPreviewImage);
	outCharData.specialAnimationFileName = specialAnimationFileName;
	copySpriteToDestination(specialAnimationMenuPreviewImage, charDirName + "/" + specialAnimationFileName);
	parseStringToJSONDouble(static_cast<menuDataNumberField*>(specialMenuDamage.get())->textField, outCharData.specialDamage, false);
	parseStringToJSONInt(static_cast<menuDataNumberField*>(specialMenuDuration.get())->textField, outCharData.specialDuration, false);

	for (auto& buffMenuGrid : buffMenuDeque)
	{
		buffData curBuffData;
		auto& curBuffDataColumn = buffMenuGrid.buffDataGrid->menuColumnList[0];
		auto& curBuffIconColumn = buffMenuGrid.buffIconGrid->menuColumnList[1];
		curBuffData.buffName = static_cast<menuDataTextBoxField*>(curBuffDataColumn.menuDataPtrList[0].get())->textField;
		buffLevelData curBuffLevelData;
		parseStringToJSONInt(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[1].get())->textField, curBuffLevelData.attackIncrement);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[2].get())->textField, curBuffLevelData.critIncrement);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[3].get())->textField, curBuffLevelData.hasteIncrement);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[4].get())->textField, curBuffLevelData.speedIncrement);
		parseStringToJSONDouble(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[5].get())->textField, curBuffLevelData.DRMultiplier);
		parseStringToJSONDouble(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[6].get())->textField, curBuffLevelData.healMultiplier);
		parseStringToJSONDouble(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[7].get())->textField, curBuffLevelData.food);
		parseStringToJSONDouble(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[8].get())->textField, curBuffLevelData.weaponSize);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(curBuffDataColumn.menuDataPtrList[9].get())->textField, curBuffLevelData.maxStacks);
		parseStringToJSONInt(static_cast<menuDataNumberField*>(buffMenuGrid.buffDataGrid->menuColumnList[1].menuDataPtrList[0].get())->textField, curBuffLevelData.timer);
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

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::string& outputStr)
{
	try
	{
		inputJson.at(varName).get_to(outputStr);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to string", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to string", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to string", e.what(), varName);
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
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to JSONInt", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to JSONInt", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to JSONInt", e.what(), varName);
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
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to JSONDouble", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to JSONDouble", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to JSONDouble", e.what(), varName);
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
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to weaponLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, std::vector<skillData>& outputSkillDataList)
{
	try
	{
		inputJson.at(varName).get_to(outputSkillDataList);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to skillData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to skillData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to skillData list", e.what(), varName);
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
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to buffData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to buffData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to buffData list", e.what(), varName);
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
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to skillLevelData list", e.what(), varName);
	}
}

void parseJSONToVar(const nlohmann::json& inputJson, const char* varName, onTriggerData& outputOnTriggerData)
{
	try
	{
		inputJson.at(varName).get_to(outputOnTriggerData);
	}
	catch (nlohmann::json::type_error& e)
	{
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to onTriggerData", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to onTriggerData", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to onTriggerData", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to onTriggerData", e.what(), varName);
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
		g_ModuleInterface->Print(CM_RED, "Type Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Type Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
	}
	catch (nlohmann::json::out_of_range& e)
	{
		g_ModuleInterface->Print(CM_RED, "Out of Range Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
		callbackManagerInterfacePtr->LogToFile(MODNAME, "Out of Range Error: %s when parsing var %s to buffLevelData list", e.what(), varName);
	}
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

void from_json(const nlohmann::json& inputJson, characterData& outputCharData)
{
	parseJSONToVar(inputJson, "charName", outputCharData.charName);
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
	parseJSONToVar(inputJson, "sizeGrade", outputCharData.sizeGrade);
	parseJSONToVar(inputJson, "weaponType", outputCharData.mainWeaponWeaponType);
	parseJSONToVar(inputJson, "levels", outputCharData.weaponLevelDataList);
	parseJSONToVar(inputJson, "skills", outputCharData.skillDataList);
	parseJSONToVar(inputJson, "buffs", outputCharData.buffDataList);
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
	parseJSONToVar(inputJson, "levels", outputSkillData.skillLevelDataList);
	parseJSONToVar(inputJson, "skillName", outputSkillData.skillName);
	parseJSONToVar(inputJson, "skillIconFileName", outputSkillData.skillIconFileName);
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
	parseJSONToVar(inputJson, "skillDescription", outputSkillLevelData.skillDescription);
	parseJSONToVar(inputJson, "attackIncrement", outputSkillLevelData.attackIncrement);
	parseJSONToVar(inputJson, "critIncrement", outputSkillLevelData.critIncrement);
	parseJSONToVar(inputJson, "hasteIncrement", outputSkillLevelData.hasteIncrement);
	parseJSONToVar(inputJson, "speedIncrement", outputSkillLevelData.speedIncrement);
	parseJSONToVar(inputJson, "DRMultiplier", outputSkillLevelData.DRMultiplier);
	parseJSONToVar(inputJson, "healMultiplier", outputSkillLevelData.healMultiplier);
	parseJSONToVar(inputJson, "food", outputSkillLevelData.food);
	parseJSONToVar(inputJson, "weaponSize", outputSkillLevelData.weaponSize);
	parseJSONToVar(inputJson, "skillOnTriggerData", outputSkillLevelData.skillOnTriggerData);
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
	parseJSONToVar(inputJson, "maxStacks", outputBuffLevelData.maxStacks);
	parseJSONToVar(inputJson, "timer", outputBuffLevelData.timer);
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
	parseJSONToVar(inputJson, "buffName", outputBuffData.buffName);
	parseJSONToVar(inputJson, "levels", outputBuffData.levels);
	parseJSONToVar(inputJson, "buffIconFileName", outputBuffData.buffIconFileName);
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
	parseJSONToVar(inputJson, "onTriggerType", outputOnTriggerData.onTriggerType);
	parseJSONToVar(inputJson, "buffName", outputOnTriggerData.buffName);
	parseJSONToVar(inputJson, "probability", outputOnTriggerData.probability);
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