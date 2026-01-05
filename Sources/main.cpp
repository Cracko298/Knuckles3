#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include "cheats.hpp"
#include "sha256.hpp"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <string>
#include <iomanip>   // Include for std::hex
#include <sstream>
#include <vector>

namespace CTRPluginFramework
{
    Handle  processHandle;
    u64 titleID = Process::GetTitleID();

    // This patch the NFC disabling the touchscreen when scanning an amiibo, which prevents ctrpf to be used
    static void    ToggleTouchscreenForceOn(void)
    {
        static u32 original = 0;
        static u32 *patchAddress = nullptr;

        if (patchAddress && original)
        {
            *patchAddress = original;
            return;
        }

        static const std::vector<u32> pattern =
        {
            0xE59F10C0, 0xE5840004, 0xE5841000, 0xE5DD0000,
            0xE5C40008, 0xE28DD03C, 0xE8BD80F0, 0xE5D51001,
            0xE1D400D4, 0xE3510003, 0x159F0034, 0x1A000003
        };

        Result  res;
        s64     textTotalSize = 0;
        s64     startAddress = 0;
        u32 *   found;

        if (R_FAILED(svcOpenProcess(&processHandle, 16)))
            return;

        svcGetProcessInfo(&textTotalSize, processHandle, 0x10002);
        svcGetProcessInfo(&startAddress, processHandle, 0x10005);
        if(R_FAILED(svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, processHandle, (u32)startAddress, textTotalSize)))
            goto exit;

        found = (u32 *)Utils::Search<u32>(0x14000000, (u32)textTotalSize, pattern);

        if (found != nullptr)
        {
            original = found[13];
            patchAddress = (u32 *)PA_FROM_VA((found + 13));
            found[13] = 0xE1A00000;
        }

        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, textTotalSize);
exit:
        svcCloseHandle(processHandle);
    }

    // This function is called before main and before the game starts
    // Useful to do code edits safely
    void PatchProcess(FwkSettings &settings){

    }

    // This function is called when the process exits
    // Useful to save settings, undo patchs or clean up things
    void    OnProcessExit(void)
    {
        if (!Process::IsPaused())  
        {
            ToggleTouchscreenForceOn();
        }
        svcSleepThread(1000000000ULL);
        Process::ReturnToHomeMenu();
    }

    void InitMenu(PluginMenu &menu)
    {
        MenuFolder *statusFolder = new MenuFolder("Status Codes"); //       Status Folder Section
        MenuFolder *statusExtraFolder = new MenuFolder("Extras");
        MenuFolder *miniGameFolder = new MenuFolder("Mini-Game Codes"); //  Maybe move MiniGame and Disable Codes to a Misc Folder? And shorten the list for stuff not related to stats. i.e. subfolder called "Extras" or sum...
        MenuFolder *disableFolder = new MenuFolder("Disable Codes");
        MenuFolder *serverFolder = new MenuFolder("Server Codes");

        statusFolder->Append(statusExtraFolder);
        // statusFolder->Append(miniGameFolder); // add these to new location... misc
        // statusFolder->Append(disableFolder); // that includes this one too...

        MenuFolder *weaponsFolder = new MenuFolder("Weapon Codes"); //      Weapons Folder Section
        MenuFolder *textFolder = new MenuFolder("Massaging/Name Codes"); // Maybe seperate this folder into Messaging, then a name folder? Not much here anyway...

        MenuFolder *movementFolder = new MenuFolder("Movement Codes"); //   Movement Folder Section
        MenuFolder *teleportFolder = new MenuFolder("Teleportation Codes");
        movementFolder->Append(teleportFolder);

        MenuFolder *inventoryFolder = new MenuFolder("Inventory Codes"); // Inventory Folder Section.
        MenuFolder *dropsFolder = new MenuFolder("Drop(s) Codes"); //       Custom Drops, i.e. vehciles/doors
        MenuFolder *slotFolder = new MenuFolder("Slot Codes"); //           Slots belonged under Inv ngl...
        inventoryFolder->Append(slotFolder);
        inventoryFolder->Append(dropsFolder);

        MenuFolder *miscFolder = new MenuFolder("Miscellaneous Codes");
        miscFolder->Append(miniGameFolder);
        miscFolder->Append(disableFolder);

        statusFolder->Append(new MenuEntry("Faster Load Times", fasterLoading, "Disables a timer for Loading, should be near instantanious for loading now."));
        statusFolder->Append(new MenuEntry("Revive Player", revivePlayer, "Revives the Player if they die. In the same spot, and nothing lost."));
        statusFolder->Append(new MenuEntry("All Status' Max", allStatusMax, "A very advanced version of Maxing Stats."));
        statusFolder->Append(new MenuEntry("Max Health", maxHealth, "Gives the Player, the most amount of Health."));
        statusFolder->Append(new MenuEntry("Max Food", maxFood, "Gives the Player, the most amount of Food."));
        statusFolder->Append(new MenuEntry("Max Water", maxWater, "Gives the Player, the most amount of Water."));
        statusFolder->Append(new MenuEntry("Max Battery", maxBattery, "Gives the Player, the most Battery Charge."));
        statusFolder->Append(new MenuEntry("Always Day", alwaysDay, "Sets the time to always be day (noon)."));
        statusFolder->Append(new MenuEntry("Always Night", alwaysNight, "Sets the time to always be night (midnight)."));
        statusExtraFolder->Append(new MenuEntry("Always Server Refresh", fastServerRefresh, "The refresh button is always shown, and usable."));
        statusExtraFolder->Append(new MenuEntry("HotKey'd Flashlight", flashlightAssignedToKey, "Sets the Flashlight to HotKeys.\n\nA = Turn on Flashlight.\nY = Turn off Flashlight."));
        statusExtraFolder->Append(new MenuEntry("Custom Stats Color", setStatsColor, "Cracko298's Custom Status' Colors."));
        statusExtraFolder->Append(new MenuEntry("Advanced Map Zoom", advancedMapZoom, "Map has new Zoom-In Options."));
        statusExtraFolder->Append(new MenuEntry("See All Names", seeAllNames, "See the names of all Players, as if they where friendly/waved to you."));
        statusExtraFolder->Append(new MenuEntry("HotKey'd IDs", showHideAllIDs, "Changes the IDs and Names via HotKeys.\n\nA = Show Player Names.\nY = Show Player IDs."));
        statusExtraFolder->Append(new MenuEntry("Auto Heal", autoHeal, "Automatically heals the Player to 100hp, if the Player is below 25hp."));

        inventoryFolder->Append(new MenuEntry("Give Player an Item", nullptr, [](MenuEntry *entry){
            getItem();
        }));
        inventoryFolder->Append(new MenuEntry("Copy/Paste Inventory", copyPasteInventory, "Copy your Inventory from one Save to Another. You need to be in-game to copy/paste. Then Save your Game.\n\nY-DPadUp = Copy SaveGame Inv.\nY-DPadDown = Paste SaveGame Inv."));
        inventoryFolder->Append(new MenuEntry("Keep Items After Death", noItemLoss, "If the player somehow dies, this will keep all items inside their Inventory."));
        inventoryFolder->Append(new MenuEntry("Unlock all Outfits", unlockAllOutfits, "Unlocks all outfits in-game"));
        inventoryFolder->Append(new MenuEntry("Hacker Inventory", hackerInventory, "A decent inventory code with everything you'll ever need in-game."));
        
        textFolder->Append(new MenuEntry("Change Player Username", nullptr, [](MenuEntry *entry){
            nameChanger();
        }));
        textFolder->Append(new MenuEntry("Force Username Change", constantNameChanger, "Forces the 'Custom' Player Username. Use if Custom Username doesn't work after loading a SaveGame."));
        textFolder->Append(new MenuEntry("Utility Keyboard", utilityKeyboard, "A Utility Keyboard with Symbols, New Lines, Capitalzation, and a few Basic Cheats/Commands."));
        // Put an ID Changer, Chat Spammer, 
        movementFolder->Append(new MenuEntry("Change Car-Speed Hack Velocity", nullptr, [](MenuEntry *entry){
            setCarHackSpeed();
        }));
        movementFolder->Append(new MenuEntry("Change MoonJump Multiplier", nullptr, [](MenuEntry *entry){
            setMoonJump();
        }));
        
        movementFolder->Append(new MenuEntry("No Engine Parts Needed", noEnginePartsNeeded, "Helicopters no longer require Engine Parts for you."));
        movementFolder->Append(new MenuEntry("Stalk Players/Entities", stalkEntities, "Start Stalking: L+DPadUp+Y\nScroll Entities: Y+DPadLeft/Right\nStop Stalking: L+B\n\nHighly suggest to use 'Walk Through Walls' with this Code."));
        movementFolder->Append(new MenuEntry("Walk Through Walls", walkThroughWalls, "Walk Through Walls/Terrain with 'Y'."));
        movementFolder->Append(new MenuEntry("X-Ray the World", xrayWorld, "Hold 'Y' to X-Ray all Terrain/Meshes (not entities)."));
        movementFolder->Append(new MenuEntry("No Fuel Needed", noFuelNeeded, "You can drive any vehcile without fuel. Yes, this includes the dogsled."));
        movementFolder->Append(new MenuEntry("Car-Speed Hack", carSpeedHack, "Press 'B' to Start a Vehcile, and hold for an increase in Speed. Press 'X' to go super-fast while in this Mode. Press 'A' to exit the Mode."));
        movementFolder->Append(new MenuEntry("Walk on Water", walkOnWater, "Walk on Water (Always stay above Water)."));
        movementFolder->Append(new MenuEntry("MoonJump", moonJump, "MoonJump by pressing 'L'."));
        movementFolder->Append(new MenuEntry("No-Clip", noClip, "No-Clip: A+DPad\n\nHighly suggest to use MoonJump with this Code."));

        weaponsFolder->Append(new MenuEntry("Heal/HP Knife", healKnife, "If a Knife is placed in the first slot, and used. It will give Players' HP instead of Damaging them."));
        weaponsFolder->Append(new MenuEntry("Rapid Fire", rapidFire, "Rapid Fire on all Weapons."));
        weaponsFolder->Append(new MenuEntry("No Reload", noReload, "No Reload on all Weapons."));
        weaponsFolder->Append(new MenuEntry("No Recoil", noRecoil, "No Recoil on all Weapons."));
        weaponsFolder->Append(new MenuEntry("Aimbot", aimbot, "Shoot normally, and you'll shoot all Entities/Players."));

        miscFolder->Append(new MenuEntry("Spam Door Sounds", spamDoorSounds, "Spam the Door sound. Online, and offline."));
        
        serverFolder->Append(new MenuEntry("Kick Players from Game", nullptr, [](MenuEntry *entry){
            selectPlayerToKick();
        }));
        serverFolder->Append(new MenuEntry("Server Locker", serverLocker, "Lock Server: A+DPadUp.\nUnlock Server: A+DPadDown."));
        serverFolder->Append(new MenuEntry("Clone Servers", cloneServers, "Press 'start' after you Create a Server."));


        
        // codesFolder->Append(new MenuEntry("Drop Everything In-Hand", dropEverything));
        
        //toolsFolder->Append(new MenuEntry("Player Model Editor", nullptr, [](MenuEntry *entry)
        //{
        //    MessageBox("Are you sure?", "The model editor will permanently change your skin attributes.\nThis requires the game to be restarted for changes to take effect.", DialogType::DialogOk, ClearScreen::Both)()
        //        selectAndModifyOffset();
        //    } else{
        //        OSD::Notify("Operation cancelled.");
        //    }
        //    
        //}));
        
        //funFolder->Append(new MenuEntry("Change Player Scaling", nullptr, [](MenuEntry *entry)
        //{
        //    float userValue;

        //    Keyboard kb("Enter a float value (Recommended 0-10):");
        //    float input;

        //    if (kb.Open(input) != -1)
        //    {
        //        float userValue = input;

        //        Process::WriteFloat(0x600BF0, userValue);
        //        Process::WriteFloat(0x60370C, userValue);
        //        Process::WriteFloat(0x607270, userValue);
        //        Process::WriteFloat(0x607274, userValue);
        //        Process::WriteFloat(0x60804C, userValue);
        //        Process::WriteFloat(0x7218F4, userValue);
        //        Process::WriteFloat(0x735020, userValue);
        //        Process::WriteFloat(0x988BB8, userValue);

        //        OSD::Notify(Utils::Format("Written: %.2f to Scaling Address'", userValue));
        //        OSD::Notify("Requires a world restart, or for you to load a world.");
        //        OSD::Notify("This does NOT scale the player skin.");
        //}
        //}));
        
        
        menu += statusFolder;
        menu += inventoryFolder;
        menu += textFolder;
        menu += movementFolder;
        menu += weaponsFolder;
        menu += miscFolder;
        menu += serverFolder;

    }

    int main(void)
    {
        
        PluginMenu *menu = nullptr;
        menu = new PluginMenu("ISZ Ultimate Plugin", 1, 0, 0, "A CTRPF Plugin for ISZ 3DS.");

        menu->SynchronizeWithFrame(true);
        InitMenu(*menu);
        OSD::Notify("Plugin has Loaded Successfully.");

        menu->ShowWelcomeMessage(false);
        initItemDatabase();
        menu->Run();
        delete menu;
        OnProcessExit();
        svcCloseHandle(processHandle);
        return (0);

    }
}
