# Holocure Character Creator Mod
A Holocure mod that allows for new characters to be added to the game. Please follow Cover Corp guidelines https://hololivepro.com/en/terms/ when making and distributing your character.

Note: Currently doesn't work with my other mods since I am currently updating how the mods work. Please follow the Installation Steps with only the mods listed in there until I have updated my other mods.
## Installation Steps
# Follow these instructions if you've used mods before December 2025
- Run `AurieManager.exe` and uninstall Aurie from `HoloCure.exe`
    - The latest version of Aurie is moving away from AurieManager and is instead patching the game to run the mods. This has the benefit of not requiring admin privileges anymore and easily disabling mods by deleting the mods folder or replacing the original exe without crashing.
# Normal installation steps
- Download `HoloCureCharacterCreatorMod.dll`, `CallbackManagerMod.dll`, `HoloCureMenuMod.dll`, `AurieCore.dll`, and `YYToolkit.dll` from the latest version of the mod https://github.com/PippleCultist/HoloCureCharacterCreatorMod/releases
- Download `AurieInstaller.exe` from the latest version of Aurie https://github.com/AurieFramework/Aurie/releases
- Launch `AurieInstaller.exe`, click `Find my game!`, and select `HoloCure.exe`
    - You can find `HoloCure.exe` through Steam by clicking `Browse local files`
- Click `Confirm Version`
- Go to the `mods` folder where `HoloCure.exe` is located and locate the `Aurie` folder and `Native` folder.
    - In the `Aurie` folder, replace `YYToolkit.dll` and copy over `HoloCureCharacterCreatorMod.dll`, `CallbackManagerMod.dll`, and `HoloCureMenuMod.dll`
    - In the `Native` folder, replace `AurieCore.dll`
- Running the game either using the executable or through Steam should now launch the mods as well
## Creating a Character
To create a character, you can use the mod GUI or directly edit the `charData.json` yourself. An example character has been provided in this repository named `char_test`.

You will need a folder named `CharacterCreatorMod` in the directory `HoloCure.exe` is in. You can put the `char_test` folder in the `CharacterCreatorMod` directory, and the mod will automatically recognize it as long as the folder name starts with `char_`.

When including new images, you will need to put them in the character folder you made for the mod to automatically recognize them.

In order to have Gamemaker recognize animations from a png, you will need to name it in the format `name_stripX.png` where `stripX` is the number of frames in the animation. The animation must be horizontally laid out in a strip for Gamemaker to automatically convert it correctly.
Gamemaker will split the png into `X` equal images, so make sure to make each image of your animation with the same size and equally spaced apart.
