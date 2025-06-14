# Holocure Character Creator Mod
A Holocure mod that allows for new characters to be added to the game. Please follow Cover Corp guidelines https://hololivepro.com/en/terms/ when making and distributing your character.

Note: Currently doesn't work with my other mods since I am currently updating how the mods work. Please follow the Installation Steps with only the mods listed in there until I have updated my other mods.
## Installation Steps
- Download `HoloCureCharacterCreatorMod.dll`, `CallbackManagerMod.dll`, and `HoloCureMenuMod.dll` from the latest version of the mod https://github.com/PippleCultist/HoloCureCharacterCreatorMod/releases
- Download `AurieManager.exe` from the latest version of Aurie https://github.com/AurieFramework/Aurie/releases
    - Note: This launcher may be marked as a Trojan by your antivirus. Aurie is opensource and has been used in several modding communities without issues.
- Launch `AurieManager.exe`, click `Add Game`, and select `HoloCure.exe`
    - You can find `HoloCure.exe` through Steam by clicking `Browse local files`
- Click `Install Aurie`
- Click `Add Mods` and add `HoloCureCharacterCreatorMod.dll`, `CallbackManagerMod.dll`, and `HoloCureMenuMod.dll`
- Running the game either using the executable or through Steam should now launch the mods as well
## Creating a Character
To create a character, you can use the mod GUI or directly edit the `charData.json` yourself. An example character has been provided in this repository named `char_test`.

You will need a folder named `CharacterCreatorMod` in the directory `HoloCure.exe` is in. You can put the `char_test` folder in the `CharacterCreatorMod` directory, and the mod will automatically recognize it as long as the folder name starts with `char_`.

When including new images, you will need to put them in the character folder you made for the mod to automatically recognize them.

In order to have Gamemaker recognize animations from a png, you will need to name it in the format `name_stripX.png` where `stripX` is the number of frames in the animation. The animation must be horizontally laid out in a strip for Gamemaker to automatically convert it correctly.
Gamemaker will split the png into `X` equal images, so make sure to make each image of your animation with the same size and equally spaced apart.
