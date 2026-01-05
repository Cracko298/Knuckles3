#include "cheats.hpp"
#include "sha256.hpp"
#include "types.h"
#include "3ds.h"
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include <random>

namespace CTRPluginFramework{
    u32 const INV_PAGE = 0x01E81000;
    u32 const TEXT_PAGE = INV_PAGE+0x6F0;
    u32 const NAME_PAGE = TEXT_PAGE+0x24;
    u32 const COORD_PAGE = NAME_PAGE+0x58;
    u32 const CALL_PAGE = COORD_PAGE;
    
    float static moonJumpVar = 2.0;
    float static carSpeedSlow = 5.0;
    float static carSpeedFast = 32.0;

    enum class ItemCategory {
        Empty,
        Weapons,
        Ammo,
        Consumables,
        Tools,
        Clothing,
        Misc
    };

    struct Item {
        u8 id;
        std::string name;
        ItemCategory category;
        bool stackable;
    };

    std::string toLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
                        [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    std::unordered_map<uint8_t, Item> *itemDatabase = nullptr;

    void initItemDatabase(){
        itemDatabase = new std::unordered_map<uint8_t, Item> ({
            {0x00, {0x00, "Empty", ItemCategory::Empty, false}},

            // Weapons
            {0x01, {0x01, "Basic Handgun", ItemCategory::Weapons, false}},
            {0x02, {0x02, "Arctic Handgun", ItemCategory::Weapons, false}},
            {0x03, {0x03, "Arctic Eagle Handgun", ItemCategory::Weapons, false}},
            {0x04, {0x04, "Double Barrel Shotgun", ItemCategory::Weapons, false}},
            {0x05, {0x05, "Hunting Shotgun", ItemCategory::Weapons, false}},
            {0x06, {0x06, "Arctic Hunting Shotgun", ItemCategory::Weapons, false}},
            {0x07, {0x07, "Hunting Rifle", ItemCategory::Weapons, false}},
            {0x08, {0x08, "Scout Hunting Rifle", ItemCategory::Weapons, false}},
            {0x09, {0x09, "Arctic Hunting Rifle", ItemCategory::Weapons, false}},
            {0x0A, {0x0A, "Crossbow", ItemCategory::Weapons, false}},
            {0x0B, {0x0B, "Hunting Crossbow", ItemCategory::Weapons, false}},
            {0x0C, {0x0C, "Arctic Hunting Crossbow", ItemCategory::Weapons, false}},
            {0x0D, {0x0D, "Tranquilizer Gun", ItemCategory::Weapons, false}},
            {0x0E, {0x0E, "Hunting Knife", ItemCategory::Weapons, false}},
            {0x45, {0x45, "Flamethrower", ItemCategory::Weapons, false}},
            {0x46, {0x46, "Ray Gun", ItemCategory::Weapons, false}},
            {0x47, {0x47, "Flare Gun", ItemCategory::Weapons, false}},

            // Ammo
            {0x0F, {0x0F, "Pistol Ammo", ItemCategory::Ammo, true}},
            {0x10, {0x10, "Shotgun Ammo", ItemCategory::Ammo, true}},
            {0x11, {0x11, "Rifle Ammo", ItemCategory::Ammo, true}},
            {0x12, {0x12, "Crossbow Bolts (Ammo)", ItemCategory::Ammo, true}},
            {0x13, {0x13, "Tranquilizer Ammo", ItemCategory::Ammo, true}},

            // Consumables
            {0x1D, {0x1D, "Water Bottle", ItemCategory::Consumables, false}},
            {0x1E, {0x1E, "Snackbar", ItemCategory::Consumables, false}},
            {0x1F, {0x1F, "Tuna", ItemCategory::Consumables, false}},
            {0x20, {0x20, "Beans", ItemCategory::Consumables, false}},
            {0x21, {0x21, "Corned Beef", ItemCategory::Consumables, false}},
            {0x22, {0x22, "Large Water Bottle", ItemCategory::Consumables, false}},
            {0x23, {0x23, "Mint Cake", ItemCategory::Consumables, false}},
            {0x24, {0x24, "Antiseptic", ItemCategory::Consumables, false}},
            {0x25, {0x25, "Antibiotics", ItemCategory::Consumables, false}},
            {0x26, {0x26, "Bandage", ItemCategory::Consumables, false}},
            {0x27, {0x27, "Dressing", ItemCategory::Consumables, false}},
            {0x28, {0x28, "Large Bandage", ItemCategory::Consumables, false}},
            {0x29, {0x29, "Large Dressing", ItemCategory::Consumables, false}},
            {0x3F, {0x3F, "Cooked Goose", ItemCategory::Consumables, true}},
            {0x42, {0x42, "Cooked Fish", ItemCategory::Consumables, true}},

            // Tools
            {0x14, {0x14, "Matches", ItemCategory::Tools, true}},
            {0x15, {0x15, "Fuel", ItemCategory::Tools, false}},
            {0x16, {0x16, "Binoculars", ItemCategory::Tools, false}},
            {0x17, {0x17, "Cooking Stove", ItemCategory::Tools, false}},
            {0x18, {0x18, "Bear Trap", ItemCategory::Tools, false}},
            {0x19, {0x19, "Fishing Rod", ItemCategory::Tools, false}},
            {0x1A, {0x1A, "Field Binoculars", ItemCategory::Tools, false}},
            {0x1B, {0x1B, "Professional Fishing Rod", ItemCategory::Tools, false}},
            {0x1C, {0x1C, "Metal Detector", ItemCategory::Tools, false}},

            // Clothing
            {0x2A, {0x2A, "Jacket", ItemCategory::Clothing, false}},
            {0x2B, {0x2B, "Trousers", ItemCategory::Clothing, false}},
            {0x2C, {0x2C, "Backpack", ItemCategory::Clothing, false}},
            {0x2D, {0x2D, "Cammo Jacket", ItemCategory::Clothing, false}},
            {0x2E, {0x2E, "Cammo Trousers", ItemCategory::Clothing, false}},
            {0x2F, {0x2F, "Cammo Backpack", ItemCategory::Clothing, false}},
            {0x30, {0x30, "Military Jacket", ItemCategory::Clothing, false}},
            {0x31, {0x31, "Military Trousers", ItemCategory::Clothing, false}},
            {0x32, {0x32, "Military Backpack", ItemCategory::Clothing, false}},
            {0x33, {0x33, "Kevlar Jacket", ItemCategory::Clothing, false}},
            {0x34, {0x34, "Kevlar Trousers", ItemCategory::Clothing, false}},
            {0x35, {0x35, "Kevlar Backpack", ItemCategory::Clothing, false}},
            {0x36, {0x36, "Arctic Jacket", ItemCategory::Clothing, false}},
            {0x37, {0x37, "Arctic Trousers", ItemCategory::Clothing, false}},
            {0x38, {0x38, "Arctic Backpack", ItemCategory::Clothing, false}},
            {0x39, {0x39, "Blink Jacket", ItemCategory::Clothing, false}},
            {0x3A, {0x3A, "Bling Trousers", ItemCategory::Clothing, false}},
            {0x3B, {0x3B, "Bling Backpack", ItemCategory::Clothing, false}},

            // Misc
            {0x3C, {0x3C, "Tent", ItemCategory::Misc, false}},
            {0x3D, {0x3D, "Logs", ItemCategory::Misc, true}},
            {0x3E, {0x3E, "Goose", ItemCategory::Misc, true}},
            {0x40, {0x40, "Minnow", ItemCategory::Misc, true}},
            {0x41, {0x41, "Large Fish", ItemCategory::Misc, true}},
            {0x43, {0x43, "Engine Part", ItemCategory::Misc, false}},
            {0x44, {0x44, "Snowboard", ItemCategory::Misc, false}},
        });
    }

    std::vector<Item> getItemsByCategory(ItemCategory category) {
        std::vector<Item> result;
        for (const auto& pair : *itemDatabase) {
            const Item& item = pair.second;
            if (item.category == category)
                result.push_back(item);
        }
        return result;
    }

    std::vector<Item> searchItems(const std::string& searchText) {
        std::vector<Item> results;
        std::string query = toLower(searchText);
    
        for (const auto& pair : *itemDatabase) {
            const Item& item = pair.second;
            std::string itemNameLower = toLower(item.name);
            if (itemNameLower.find(query) != std::string::npos) {
                results.push_back(item);
            }
        }
        return results;
    }

    bool isPlayerDead(){
        u32 hpVal;
        Process::Read32(0x483614, hpVal);
        if (hpVal == 0x00){
            return true;
        } else {
            return false;
        }
    }
    
    static const std::unordered_map<std::string, float> timeFloatMap = {
        {"12pm", 0.000f},
        {"1pm", 0.273f},
        {"2pm", 0.545f},
        {"3pm", 0.818f},
        {"4pm", 1.091f},
        {"5pm", 1.364f},
        {"6pm", 1.636f},
        {"7pm", 1.909f},
        {"8pm", 2.182f},
        {"9pm", 2.455f},
        {"10pm", 2.727f},
        {"11pm", 3.000f},
        {"12am", 3.273f},
        {"1am", 3.545f},
        {"2am", 3.818f},
        {"3am", 4.091f},
        {"4am", 4.364f},
        {"5am", 4.636f},
        {"6am", 4.909f},
        {"7am", 5.182f},
        {"8am", 5.455f},
        {"9am", 5.727f},
        {"10am", 6.000f},
        {"11am", 6.273f}
    };

    
    float GetFloatFromTime(const std::string &input){
        std::string key = input;
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        auto it = timeFloatMap.find(key);
        if (it != timeFloatMap.end())
            return it->second;

        return -1.0f;
    }

    void clearBuffer(u32 address, size_t length){
        for (size_t i = 0; i < length; ++i)
            Process::Write8(address + i, 0x00);

        u32 bufferAddress = address-0x04;
        Process::Write8(bufferAddress, 0x00);
    }
 
    void replaceSymbols(u32 textAdd){
        std::string buffer;
        Process::ReadString(textAdd, buffer, 0x14, StringFormat::Utf8);
        u8 cursorPos;
        Process::Read8(textAdd - 0x04, cursorPos);
        for (size_t i = 0; i + 2 < buffer.size(); ++i){
            if (buffer[i] == '.' && buffer[i + 1] == 's'){
                char numChar = buffer[i + 2];
                if (numChar >= '0' && numChar <= '9'){
                    static const std::string symbols[10] = {
                        "!", "@", "#", "$", "%", "^", "&", "*", "|", "-"
                    };

                    std::string symbolStr = symbols[numChar - '0'];
                    Process::WriteString(textAdd + i, symbolStr, StringFormat::Utf8);
                    size_t end = i + symbolStr.size();
                    for (size_t j = end; j < i + 3 && j < 20; ++j){
                        Process::Write8(textAdd + j, 0x00);
                    }

                    if (cursorPos >= 2)
                        cursorPos -= 2;
                    else
                        cursorPos = 0;

                    Process::Write8(textAdd - 0x04, cursorPos);
                    break;
                }
            }
        }
    }

    void newLines(u32 textAdd){
        std::string buffer;
        u8 bufferSize;
        Process::ReadString(textAdd, buffer, 0x14, StringFormat::Utf8);
        for (size_t i = 0; i + 2 < buffer.size(); ++i){
            if (buffer[i] == '.' && buffer[i+1] == 'n' && buffer[i+2] == 'l'){
                Process::Write8(textAdd + i, 0x0A);
                Process::Write8(textAdd + i + 1, 0x00);
                Process::Write8(textAdd + i + 2, 0x00);
                u32 bufferAddress = textAdd-0x04;
                Process::Read8(bufferAddress, bufferSize);
                Process::Write8(bufferAddress, bufferSize-0x02);
                break;
            }
        }
    }

    void utilityKeyboard(MenuEntry *entry){
        u32 textAdd = 0x429F44; u32 hpAdd = 0x483614; u32 wtrAdd = 0x483620; u32 hngAdd = 0x48361C; u32 tempAdd = 0x483624; u32 batAdd = 0x483628; u32 timeAdd = 0x418740; u8 kbVal;
        std::string commandPrefix;
        std::string commandInfo;

        Process::Read8(0x3041942C, kbVal);
        if (kbVal == 0x00){
            return;
        }

        replaceSymbols(textAdd);
        newLines(textAdd);
        if (Controller::IsKeyPressed(Key::DPadDown)){
            for (int i = 0; i < 20; ++i){
                u8 c;
                Process::Read8(textAdd + i, c);
                if (c == 0x00){
                    if (i > 0){
                        u8 prevChar;
                        Process::Read8(textAdd + (i - 1), prevChar);
                        if (prevChar >= 'a' && prevChar <= 'z')
                            prevChar -= 32;

                        Process::Write8(textAdd + (i - 1), prevChar);
                    }
                    break;
                }
            }
        }

        if (Controller::IsKeyPressed(Key::ZL)){
            if (Process::CopyMemory((void*)TEXT_PAGE, (const void*)0x429F40, 0x24)){
                OSD::Notify("Copied Text to Clipboard.");
                Process::Write64(0x429F40, 0x00);
                Process::Write64(0x429F48, 0x00);
                Process::Write64(0x429F50, 0x00);
            }
        }
        if (Controller::IsKeyPressed(Key::ZR)){
            if (Process::CopyMemory((void*)0x429F40, (const void*)TEXT_PAGE, 0x24)){
                OSD::Notify("Pasted Text from Clipboard.");
            }
        }

        if (!Controller::IsKeyPressed(Key::Y))
            return;

        Process::ReadString(textAdd, commandPrefix, 0x04, StringFormat::Utf8);
        if (commandPrefix == ".hlt") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            u32 hpVal = std::stoi(commandInfo);
            Process::Write32(hpAdd, hpVal);
            clearBuffer(textAdd, 20);
        } if (commandPrefix == ".spn") {
            // do stuff eventually
        } if (commandPrefix == ".wtr") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            u32 wtrVal = std::stoi(commandInfo);
            Process::Write32(wtrAdd, wtrVal);
            clearBuffer(textAdd, 20);
        } if (commandPrefix == ".tmp") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            float tempVal = std::stof(commandInfo);
            Process::WriteFloat(tempAdd, tempVal);
            clearBuffer(textAdd, 20);
        } if (commandPrefix == ".hng") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            u32 hngVal = std::stoi(commandInfo);
            Process::Write32(hngAdd, hngVal);
            clearBuffer(textAdd, 20);
        } if (commandPrefix == ".bat") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            u32 batVal = std::stoi(commandInfo);
            Process::Write32(batAdd, batVal);
            clearBuffer(textAdd, 20);
        } if (commandPrefix == ".tim") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x04, StringFormat::Utf8);
            float timeVal = GetFloatFromTime(commandInfo);
            Process::WriteFloat(timeAdd, timeVal);
            clearBuffer(textAdd, 20);
        }
    }

    void fasterLoading(MenuEntry *entry){
        u32 loadingBuffer = 0x418760;
        Process::Write8(loadingBuffer, 0x00);
    }

    void statusBarColor(){
        u32 clrOff = 0x30390000;
        u32 clrVal = 0xFFFFFFFF;
        Process::Write32(clrOff + 0x2490, clrVal);
        Process::Write32(clrOff + 0x2BD0, clrVal);
        Process::Write32(clrOff + 0x31E0, clrVal);
        Process::Write32(clrOff + 0x3920, clrVal);
        Process::Write32(clrOff + 0x4060, clrVal);
        Process::Write32(clrOff + 0x47A0, clrVal);
        Process::Write32(clrOff + 0x25A8, 0x00);
        Process::Write32(clrOff + 0x25B8, 0x00);
        Process::Write32(clrOff + 0x2588, 0x00);
        Process::Write32(clrOff + 0x2598, 0x00);
        Process::Write32(clrOff + 0x2CC8, 0x00);
        Process::Write32(clrOff + 0x2CD8, 0x00);
        Process::Write32(clrOff + 0x2CE8, 0x00);
        Process::Write32(clrOff + 0x32D8, 0x00);
        Process::Write32(clrOff + 0x32E8, 0x00);
        Process::Write32(clrOff + 0x32F8, 0x00);
        Process::Write32(clrOff + 0x3308, 0x00);
        Process::Write32(clrOff + 0x3A28, 0x00);
        Process::Write32(clrOff + 0x3A38, 0x00);
        Process::Write32(clrOff + 0x3A48, 0x00);
        Process::Write32(clrOff + 0x3A18, 0x00);
        Process::Write32(clrOff + 0x4158, 0x00);
        Process::Write32(clrOff + 0x4168, 0x00);
        Process::Write32(clrOff + 0x4188, 0x00);
        Process::Write32(clrOff + 0x4178, 0x00);
        Process::Write32(clrOff + 0x4898, 0x00);
        Process::Write32(clrOff + 0x48A8, 0x00);
        Process::Write32(clrOff + 0x48B8, 0x00);
        Process::Write32(clrOff + 0x48C8, 0x00);
        Process::Write32(clrOff + 0x48D8, 0x00);
    }

    void allStatusMax(MenuEntry *entry){
        Process::WriteString(0x80120A4, "Hacks By: Cracko298\0");
        Process::WriteString(0x11B884, "Cracko\0\0");
        Process::WriteString(0x3B6598, "Created By Cracko.\n");
        Process::Write32(0x483614, 999999999);
        Process::Write32(0x48361C, 999999999);
        Process::Write32(0x483620, 999999999);
        Process::Write32(0x483628, 999999999);
        Process::Write32(0x42C6AC, 0xFF001200);
        Process::Write32(0x418740, 0xFF001200);
        Process::WriteFloat(0x483624, 9999.0);
        Process::Write8(0x483658, 0x00);
        Process::Write8(0x3044E374, 0x01);
        Process::Write8(0x3044E194, 0x01);
        statusBarColor();
    }

    void maxHealth(MenuEntry *entry){
        Process::Write32(0x483614, 999999999);
    }

    void maxFood(MenuEntry *entry){
        Process::Write32(0x48361C, 999999999);
    }

    void maxWater(MenuEntry *entry){
        Process::Write32(0x483620, 999999999);
    }

    void maxBattery(MenuEntry *entry){
        Process::Write32(0x483628, 999999999);
    }

    void maxTemp(MenuEntry *entry){
        Process::WriteFloat(0x483624, 9999.0);
    }

    void alwaysNight(MenuEntry *entry){
        Process::Write32(0x42C640, 0xFF0F1200);
    }

    void alwaysDay(MenuEntry *entry){
        Process::Write32(0x42C640, 0xFF001200);
    }

    void setStatsColor(MenuEntry *entry){
        statusBarColor();
    }

    void fastServerRefresh(MenuEntry *entry){
        Process::Write8(0x3044E374, 0x01);
        Process::Write8(0x3044E194, 0x01);
    }

    void autoHeal(MenuEntry *entry){
        u32 hpVal; u32 chkVal = 25;
        Process::Read32(0x483614, hpVal);
        if (hpVal <= chkVal){
            Process::Write8(0x483658, 0x00);
            Process::Write32(0x483614, 0x64);
            OSD::Notify("Auto-Healed the Player.");
        }
    }

    void flashlightAssignedToKey(MenuEntry *entry){
        if (Controller::IsKeyPressed(Key::A)){
            Process::Write8(0x8805EE1, 0x01);
            Process::Write8(0x30395170, 0x01);
        } if (Controller::IsKeyPressed(Key::Y)){
            Process::Write8(0x8805EE1, 0x00);
            Process::Write8(0x30395170, 0x00);
        }
    }
    
    void showHideAllIDs(MenuEntry *entry){
        if (Controller::IsKeyPressed(Key::A)){
            Process::Write8(0x8806888, 0x00);
        } if (Controller::IsKeyPressed(Key::Y)){
            Process::Write8(0x8806888, 0x01);
        }
    }

    void xrayWorld(MenuEntry *entry){
        if (Controller::IsKeyDown(Key::Y)){
            Process::Write32(0x884248C, 0x00); 
            Process::Write32(0x880688C, 0x00);
            Process::Write32(0x8806894, 0x00);
            Process::Write32(0x8842484, 0x00);
        }
    }

    void advancedMapZoom(MenuEntry *entry){
        u8 mapVal; float zoomVal; float myVal = 0.5; 
        float maxVal = 15.0; float minVal = 0.0;
        Process::Read8(0x30398110, mapVal);
        if (mapVal == 0x01){
            if (Controller::IsKeyPressed(Key::DPadUp)){
                Process::ReadFloat(0x429CE4, zoomVal);
                if (zoomVal >= maxVal){
                    Process::WriteFloat(0x429CE4, maxVal);
                } else {
                    Process::WriteFloat(0x429CE4, zoomVal+myVal);
                }
            } if (Controller::IsKeyPressed(Key::DPadDown)){
                Process::ReadFloat(0x429CE4, zoomVal);
                if (zoomVal <= minVal){
                    Process::WriteFloat(0x429CE4, minVal);
                } else {
                    Process::WriteFloat(0x429CE4, zoomVal-myVal);
                }
            }
        }
    }

    void seeAllNames(MenuEntry *entry){
        u32 baseAdd = 0x483740;
        for (int i = 0; i < 0x14; i++) {
            Process::Write8(baseAdd + 0x848 * i, 0x01);
        }
    }

    void copyPasteInventory(MenuEntry *entry){
        const u32 srcAddr1 = 0x00429600;
        const u32 dstAddr1 = INV_PAGE;
        const u32 dataSize = 0x6F0;
        const u32 wordCount = dataSize / 0x04;
        if (Controller::IsKeyDown(Key::Y)){
            if (Controller::IsKeyPressed(Key::DPadUp)){
                if (Process::CopyMemory((void*)dstAddr1, (const void*)srcAddr1, dataSize)){
                    OSD::Notify("Copied Player Inventory to Clipboard.");
                }
            }
            if (Controller::IsKeyPressed(Key::DPadDown)){
                if (Process::CopyMemory((void*)srcAddr1, (const void*)dstAddr1, dataSize)){
                    OSD::Notify("Pasted Player Inventory from Clipboard.");
                }
            }
        }
    }

    void unlockAllOutfits(MenuEntry *entry){
        u32 insVal = 0xE3500001;
        Process::Write32(0x00247220, insVal);
        Process::Write32(0x00247240, insVal);
        Process::Write32(0x00247264, insVal);
        Process::Write32(0x00247288, insVal);
        Process::Write32(0x002472AC, insVal);
        Process::Write32(0x002472D0, insVal);
        Process::Write32(0x002472F4, insVal);
        Process::Write32(0x00247318, insVal);
        Process::Write32(0x0024733C, insVal);
        Process::Write32(0x00247360, insVal);
        Process::Write32(0x00247388, insVal);
        Process::Write32(0x002473AC, insVal);
        Process::Write32(0x002473D0, insVal);
        Process::Write32(0x002473F4, insVal);
        Process::Write32(0x00247418, insVal);
    }

    void nil(){
        u32 lsVal = 0xE320F000;
        Process::Write32(0x00145D04, lsVal);
        Process::Write32(0x00145D08, lsVal);
        Process::Write32(0x00145D84, lsVal);
        Process::Write32(0x00145D88, lsVal);
        Process::Write32(0x00145DFC, lsVal);
        Process::Write32(0x00145E00, lsVal);
        Process::Write32(0x00145E4C, lsVal);
        Process::Write32(0x00145E5C, lsVal);
        Process::Write32(0x00145E64, lsVal);
        Process::Write32(0x0014609C, lsVal);
    }

    void noItemLoss(MenuEntry *entry){
        nil();
    }

    void hackerInventory(MenuEntry *entry){
        Process::Write32(0x00429604, 0x0000270F);
        Process::Write32(0x00429600, 0x00FF0E03);
        Process::Write32(0x0042963C, 0x00FF0D03);
        Process::Write32(0x00429678, 0x00FF0303);
        Process::Write32(0x004296B4, 0x00FF0903);
        Process::Write32(0x004296F0, 0x00FF0603);
        Process::Write32(0x0042972C, 0x00FF3B04);
        Process::Write32(0x00429730, 0x0000270F);
        Process::Write32(0x00429768, 0x00FF3904);
        Process::Write32(0x0042976C, 0x0000270F);
        Process::Write32(0x004297A4, 0x00FF3A04);
        Process::Write32(0x004297A8, 0x0000270F);
        Process::Write32(0x004297E0, 0x00FF1304);
        Process::Write32(0x004297E4, 0x0000270F);
        Process::Write32(0x0042981C, 0x00FF0F04);
        Process::Write32(0x00429820, 0x0000270F);
        Process::Write32(0x00429820, 0x0000270F);
        Process::Write32(0x00429858, 0x00FF2803);
        Process::Write32(0x0042985C, 0x00000003);
        Process::Write32(0x00429894, 0x00FF2803);
        Process::Write32(0x004298D0, 0x00FF2803);
        Process::Write32(0x0042990C, 0x00FF4204);
        Process::Write32(0x00429910, 0x0000270F);
        Process::Write32(0x00429948, 0x00FF4204);
        Process::Write32(0x0042994C, 0x0000270F);
        Process::Write32(0x00429984, 0x00FF4204);
        Process::Write32(0x00429988, 0x0000270F);
        Process::Write32(0x004299C0, 0x00FF1104);
        Process::Write32(0x004299C4, 0x0000270F);
        Process::Write32(0x004299FC, 0x00FF1004);
        Process::Write32(0x00429A00, 0x0000270F);
        Process::Write32(0x00429A38, 0x00FF2903);
        Process::Write32(0x00429A74, 0x00FF2903);
        Process::Write32(0x00429AB0, 0x00FF2903);
        Process::Write32(0x00429AEC, 0x00FF4503);
        Process::Write32(0x00429B28, 0x00FF4703);
        Process::Write32(0x00429B64, 0x00FF4603);
        Process::Write32(0x00429BA0, 0x00FF1504);
        Process::Write32(0x00429BA4, 0x00000003);
        Process::Write32(0x00429BDC, 0x00FF4304);
        Process::Write32(0x00429BE0, 0x00000003);
        Process::Write32(0x00429C18, 0x00FF2703);
        Process::Write32(0x00429C54, 0x00FF2703);
        Process::Write32(0x00429C90, 0x00FF2703);
    }

    void nameChanger(){
        std::string newName;
        Keyboard kb("Enter New Player Name (10 Chars Max):");
        if (kb.Open(newName) != 0)
            return;

        if (newName.size() > 10)
            newName = newName.substr(0, 10);

        const u32 addresses[] = {
            0x4813F2, 0x48367E, 0x8E4A01C,
            0x8E69D70, 0x900176C, 0x9810800,
            NAME_PAGE // copied (copied name) name page
        };

        for (u32 addr : addresses){
            std::string original;
            for (int i = 0; i < 10; i++){
                u8 c;
                Process::Read8(addr + (i * 2), c);
                if (c != 0x00){
                    original += static_cast<char>(c);
                }
            }

            size_t len = newName.size();
            for (size_t i = 0; i < len; i++){
                Process::Write8(addr + (i * 2), newName[i]);
                Process::Write8(addr + (i * 2) + 1, 0x00);
            }

            for (size_t i = len; i < original.size(); i++){
                Process::Write8(addr + (i * 2), 0x00);
                Process::Write8(addr + (i * 2) + 1, 0x00);
            }
        }
        OSD::Notify("New Custom Username Applied (Online/Offline).");
    }

    void constantNameChanger(MenuEntry *entry){
        u64 checkVal;
        Process::Read64(NAME_PAGE, checkVal);
        if (checkVal == 0x00) {
            return;
        }

        const u32 addresses[] = {
            0x4813F2, 0x48367E, 0x8E4A01C,
            0x8E69D70, 0x900176C, 0x9810800,
        };

        for (u32 addr : addresses){
            Process::CopyMemory((void*)addr, (const void*)NAME_PAGE, 0x20);
        }
    }

    void noEnginePartsNeeded(MenuEntry *entry){
        for (int i = 0; i < 0x123; ++i) {
            Process::Write8(0x490848+0x7C*i, 0x03);
        }
    }

    void noFuelNeeded(MenuEntry *entry){
        for (int i = 0; i < 0x123; ++i) {
            Process::Write32(0x490844+0x7C*i, 0xFF);
        }
    }

    void setCollision(){
        Process::Write32(0x482F28, 0xBDCCCCCD);
        Process::Write32(0x15FCE0, 0x3F800000);
        Process::Write32(0x12DBD0, 0x0A000002);
        Process::Write32(0x249F47, 0x00000043);
        Process::Write32(0x242214, 0xED800A08);
        Process::Write32(0x2421A4, 0x0A000016);
    }

    void removeCollision(){
        Process::Write32(0x2421A4, 0xEA000016);
        Process::Write32(0x242214, 0xEAFFFFE9);
        Process::Write32(0x12DBD0, 0x1A000002);
        Process::Write32(0x482F28, 0x00);
        Process::Write32(0x4836B4, 0xFFFFFFFF);
    }

    void wtw(){
        setCollision();
        if (Controller::IsKeyDown(Key::Y)){
            removeCollision();
        }
    }

    void walkThroughWalls(MenuEntry *entry){
        wtw();
    }

    void walkOnWater(MenuEntry *entry){
        u32 zCoordAdd = 0x482F10; u32 crntCrdVal;
        Process::Read32(zCoordAdd, crntCrdVal);
        if (crntCrdVal < 0x404FDDB6){
            Process::Write32(zCoordAdd, 0x404FDDB6);
        }
    }

    void setMoonJump(){
        float myVar;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << moonJumpVar;
        std::string kbMsg = "Set the MoonJump Value:\n(Current -> " + ss.str() + ")";
        Keyboard kb(kbMsg);
        if (kb.Open(myVar) != 0)
            return;
        
        moonJumpVar = myVar;
        OSD::Notify("New MoonJump Value has been Set.");
    }

    void moonJump(MenuEntry *entry){
        u32 zCoordAdd = 0x482F10; float crntCrdVal;
        Process::ReadFloat(zCoordAdd, crntCrdVal);
        if (Controller::IsKeyDown(Key::L)){
            Process::WriteFloat(zCoordAdd, crntCrdVal+moonJumpVar);
        }
    }

    void setCarHackSpeed(){
        float myVarSlow; float myVarFast;
        std::ostringstream ssSlow; std::ostringstream ssFast;
        ssSlow << std::fixed << std::setprecision(1) << carSpeedSlow;
        ssFast << std::fixed << std::setprecision(1) << carSpeedFast;
        std::string kbMsgSlow = "Set the Slow(er) CarSpeed Value:\n(Current -> " + ssSlow.str() + ")";
        std::string kbMsgFast = "Set the Fast(er) CarSpeed Value:\n(Current -> " + ssFast.str() + ")";
        Keyboard kbSlow(kbMsgSlow);
        if (kbSlow.Open(myVarSlow) != 0)
            return;
        
        if (myVarSlow == 0.0f){
            carSpeedSlow = carSpeedSlow;
            OSD::Notify("New Slow(er) CarSpeed not Set.");
        } else {
            carSpeedSlow = myVarSlow;
            OSD::Notify("New Slow(er) CarSpeed Set.");
        }
        
        Keyboard kbFast(kbMsgFast);
        if (kbFast.Open(myVarFast) != 0)
            return;

        if (myVarFast == 0.0f){
            carSpeedFast = carSpeedFast;
            OSD::Notify("New Fast(er) CarSpeed not Set.");
        } else {
            carSpeedFast = myVarFast;
            OSD::Notify("New Fast(er) CarSpeed Set.");
        }   
    }

    void carSpeedHack(MenuEntry *entry){
        if (Controller::IsKeyDown(Key::B) && Controller::IsKeyDown(Key::R)){
            Process::Write32(0x482FB4, 0x12);
            Process::Write32(0x482FB4, 0x0E);
            Process::Write32(0x482FE0, 0x00);
            return;
        } if (Controller::IsKeyDown(Key::B)){
            Process::Write32(0x482FB4, 0x12);
            Process::Write32(0x482FB4, 0x0E);
            Process::WriteFloat(0x482FE0, carSpeedSlow);
        } if (Controller::IsKeyDown(Key::X)){
            Process::WriteFloat(0x482FE0, carSpeedFast);
        }
    }

    void stalkEntities(MenuEntry *entry){
        static bool stalkMode = false;
        static int targetIndex = 0;
        static int lastTargetIndex = -1;
        static bool wasStalking = false;

        u32 entityCrdStructAdd = 0x483754;
        u32 playerCrdAdd = 0x482F0C;

        if (Controller::IsKeysDown(Key::L | Key::B)){
            if (stalkMode){
                stalkMode = false;
                OSD::Notify(Color::Red << "Stalk Mode: OFF");
            }
        }

        if (Controller::IsKeysDown(Key::Y) && Controller::IsKeyPressed(Key::DPadLeft)){
            targetIndex = (targetIndex - 1 + 0x0F) % 0x0F;
        } if (Controller::IsKeysDown(Key::Y) && Controller::IsKeyPressed(Key::DPadRight)){
            targetIndex = (targetIndex + 1) % 0x0F;
        } if (Controller::IsKeysDown(Key::Y | Key::L | Key::DPadUp)){
            if (!stalkMode){
                stalkMode = true;
                OSD::Notify(Color::Lime << "Stalk Mode: ON");
            }
        }

        if (stalkMode){
            u32 targetAddr = entityCrdStructAdd + (0x848 * targetIndex);
            float coords[3];
            Process::ReadFloat(targetAddr, coords[0]);
            Process::ReadFloat(targetAddr + 0x4, coords[1]);
            Process::ReadFloat(targetAddr + 0x8, coords[2]);
            float offset = 0.5f; coords[0] += offset; coords[2] += offset;
            Process::WriteFloat(playerCrdAdd, coords[0]);
            Process::WriteFloat(playerCrdAdd + 0x4, coords[1]);
            Process::WriteFloat(playerCrdAdd + 0x8, coords[2]);
            if (targetIndex != lastTargetIndex){
                OSD::Notify(Color::Cyan << "Stalking Entity/Player [" << targetIndex << "]");
                lastTargetIndex = targetIndex;
            }

            wasStalking = true;
        } else if (wasStalking){
            OSD::Notify(Color::Red << "Stalking Stopped.");
            wasStalking = false;
            lastTargetIndex = -1;
        }
    }

    void noClip(MenuEntry *entry){
        u32 playerCoordAdd = 0x482F0C;
        setCollision(); float coords[3];
        Process::ReadFloat(playerCoordAdd, coords[0]);          // X
        Process::ReadFloat(playerCoordAdd + 0x4, coords[1]);    // Y (not needed)
        Process::ReadFloat(playerCoordAdd + 0x8, coords[2]);    // Z
        if(Controller::IsKeysDown(Key::A | Key::DPadRight)){
            removeCollision();
            Process::WriteFloat(playerCoordAdd, coords[0]+2.0f);
        } if(Controller::IsKeysDown(Key::A | Key::DPadLeft)){
            removeCollision();
            Process::WriteFloat(playerCoordAdd, coords[0]-2.0f);
        } if(Controller::IsKeysDown(Key::A | Key::DPadUp)){
            removeCollision();
            Process::WriteFloat(playerCoordAdd+0x08, coords[2]-2.0f);
        } if(Controller::IsKeysDown(Key::A | Key::DPadDown)){
            removeCollision();
            Process::WriteFloat(playerCoordAdd+0x08, coords[2]+2.0f);
        }
    }

    void revivePlayer(MenuEntry *entry){
        setCollision();
        static bool hasRevived = false;
        u32 playerCrdAdd = 0x482F0C;
        u32 playerHpAdd  = 0x483614;

        nil();
        u32 playerHp;
        Process::Read32(playerHpAdd, playerHp);
        if (isPlayerDead() == true) {
            Process::CopyMemory((void*)COORD_PAGE, (const void*)playerCrdAdd, 0x0C);
            hasRevived = false;
        } if (playerHp >= 0x63 && !hasRevived) {
            removeCollision();
            Process::CopyMemory((void*)playerCrdAdd, (const void*)COORD_PAGE, 0x0C);
            OSD::Notify("Player has been Revived.");
            hasRevived = true;
        }
    }

    void aimbot(MenuEntry *entry){
        u32 entityCrdStructAdd = 0x483754; u32 playerCrdAdd = 0x482F0C;
        if (Controller::IsKeysDown(Key::R | Key::Y)){
            for (int i = 0; i < 0x0F; ++i) {
                Process::CopyMemory((void*)entityCrdStructAdd+0x848*i, (const void*)playerCrdAdd, 0x0C);
            }
        }
    }

    void noReload(MenuEntry *entry) {
        if (Controller::IsKeysDown(Key::R | Key::Y)){
            for (int i = 0; i < 0x5; ++i) {
                u32 base = 0x429610 + (0x3C * i); u32 ptr;
                Process::Read32(base, ptr);        
                if (ptr != 0x00) {
                    Process::Write16(ptr + 0xC8, 0x00);
                }
            }
        }
    }

    void rapidFire(MenuEntry *entry){
        if (Controller::IsKeysDown(Key::R | Key::Y)){
            for (int i = 0; i < 0x5; ++i) {
                u32 base = 0x429610 + (0x3C * i); u32 ptr;
                Process::Read32(base, ptr);        
                if (ptr != 0x00) {
                    Process::Write32(ptr + 0x6C, 0x00);
                }
            }
        }
    }

    void noRecoil(MenuEntry *entry){
        if (Controller::IsKeysDown(Key::R | Key::Y)){
            Process::Write32(0x00482FE4, 0x00);
        }
    }

    std::vector<u32> findStruct() {
        static const u32 sAdd = 0x4906F8;          // Base Offset
        static const u32 lpAm = 0x204;             // Amount of Itterations the Struct Exists For.
        static const u32 entrySize = 0x7C;         // Struct Size Per-Item/Entity
        static const u32 doorCheckOffset = 0x1C;   // Offset to ID

        std::vector<u32> doorAddresses;

        for (u32 i = 0; i < lpAm; i++) {
            u32 checkDoorVal = 0;
            u32 baseAddress = sAdd + (entrySize * i);
            Process::Read32(baseAddress + 0x0C + doorCheckOffset, checkDoorVal);

            if (checkDoorVal == 0x01) {
                doorAddresses.push_back(baseAddress);
            }
        }

        return doorAddresses;
    }

    void spamDoorSounds(MenuEntry *entry) {
        std::vector<u32> doorAddresses = findStruct();
        for (u32 addr : doorAddresses){
            Process::Write32(addr+0x0C+0x1C+0x08, 0x01); // baseAddress + Tuple Coords + Offset To ID + Offset to Sound.Play()
        }
    }

    void getItem() {
        u8 itemStackVal; u32 durabilityVal; u32 amountVal;
        std::vector<std::string> menuOptions;
        Keyboard menuKb("Chose an Item Search Option.");
        menuOptions.push_back("Item Categories");
        menuOptions.push_back("Search for Item");
        menuKb.Populate(menuOptions);
        int mChoice = menuKb.Open();
        std::vector<Item> results;

        if (mChoice == 0) {
            std::vector<std::string> categories = {"Empty", "Weapons", "Ammo", "Consumables", "Tools", "Clothing", "Misc"};
            Keyboard catKb("Select a Category:");
            catKb.Populate(categories);
            int catIndex = catKb.Open();
            if (catIndex < 0) return;
            results = getItemsByCategory(static_cast<ItemCategory>(catIndex));
        } else if (mChoice == 1) {
            Keyboard inputKb("Search for an Item Name:");
            std::string searchText;
            if (inputKb.Open(searchText) != 0 || searchText.empty()) return;
            results = searchItems(searchText);
        } else return;

        if (results.empty()) {
            OSD::Notify("No items found.");
            return;
        }

        std::vector<std::string> itemNames;
        for (const auto &item : results) itemNames.push_back(item.name);

        Keyboard selectKb("Select an Item:");
        selectKb.Populate(itemNames);
        int index = selectKb.Open();
        if (index < 0) return;

        u8 selectedItemId = results[index].id;
        std::string itemString = results[index].name;
        itemStackVal = results[index].stackable ? 4 : 3;

        durabilityVal = 100;
        amountVal = 1;

        Keyboard durabilityKb("Durability (Decimal):");
        durabilityKb.IsHexadecimal(false);
        durabilityKb.Open(durabilityVal);

        Keyboard amountKb(Utils::Format("How Many %s?", itemString.c_str()));
        amountKb.IsHexadecimal(false);
        amountKb.Open(amountVal);

        Process::Write8(0x429600, itemStackVal);
        Process::Write8(0x429601, selectedItemId);
        Process::Write16(0x429602, 255);
        Process::Write32(0x429604, amountVal);
        Process::Write32(0x429608, durabilityVal);
        Process::Write32(0x429610, 0x00);

        OSD::Notify(Utils::Format("Gave Player Item: %s", itemString.c_str()));
        OSD::Notify(Utils::Format("Selected Item ID: %u", selectedItemId));
        OSD::Notify(Utils::Format("Durability: %u", durabilityVal));
        OSD::Notify(Utils::Format("Amount: %u", amountVal));
    }

    u16 getPlayerCount(){
        static u16 playerCount;
        Process::Read16(0x8FD8A90, playerCount);
        return playerCount; // Inlcudes the Player themselves...
    }

    std::map<std::string, u32> buildPlayerDictionary() {
        std::map<std::string, u32> playerDict;
        const u32 baseAddr = 0x483EC6;
        const u32 entrySize = 0x748;
        const u32 maxNameLen = 16;

        u32 playerCount = 0x07;
        if (playerCount == 0)
            return playerDict;

        for (u32 i = 0; i <= playerCount; i++) {
            u32 currentAddr = baseAddr + (i * entrySize);
            std::string name;

            for (u32 j = 0; j < maxNameLen; j++) {
                u8 c;
                if (!Process::Read8(currentAddr + (j * 2), c))
                    continue;

                if (c != 0x00)
                    name += static_cast<char>(c); // only take the first byte of UTF-16LE
            }

            if (!name.empty())
                playerDict[name] = currentAddr;
        }

        return playerDict;
    }

    void kickPlayer(u32 index){
        u32 val = 0x000000FD;
        OSD::Notify(Color::Red << Utils::Format("Kicked Player: %u.", index));
        if (index == 0x00){
            Process::Write32(0x8E4BCD0, val);
        } if (index == 0x01) {
            Process::Write32(0x8E4BD48, val);
        } if (index == 0x02) {
            Process::Write32(0x8E4BDC0, val);
        } if (index == 0x03) {
            Process::Write32(0x8E4BE38, val);
        } if (index == 0x04) {
            Process::Write32(0x8E4BEB0, val);
        } if (index == 0x05) {
            Process::Write32(0x8E4BF28, val);
        } if (index == 0x06) {
            Process::Write32(0x8E4BFA0, val);
        }
    }

    void selectPlayerToKick() {
        auto playerDict = buildPlayerDictionary();
        if (playerDict.empty()) {
            OSD::Notify("No Players Found.");
            return;
        }

        std::vector<std::string> names;
        std::vector<u32> addresses;
        for (auto &pair : playerDict) {
            names.push_back(pair.first);
            addresses.push_back(pair.second);
        }

        Keyboard kb("Select a Player to Kick:");
        kb.Populate(names);
        int selection = kb.Open();
        if (selection < 0)
            return;

        std::string chosenName = names[selection];
        u32 chosenAddr = addresses[selection];
        OSD::Notify(Utils::Format("Selected: %s.", chosenName.c_str()));
        kickPlayer(selection);
    }

    void healKnife(MenuEntry *entry){
        u8 itemID; u32 slotID;
        Process::Read8(0x429601, itemID);
        Process::Read32(0x4294AC, slotID);

        if (itemID == 0x0E && slotID == 0x00){
            Process::Write32(0x12494C, 0xE3A000FF);
        } else {
            Process::Write32(0x12494C, 0xE5D800D9);
        }
    }

    void serverLocker(MenuEntry *entry){
        static bool serverLocked = false; u16 playerCount = getPlayerCount();
        if (Controller::IsKeyDown(Key::A) && Controller::IsKeyPressed(Key::DPadUp)){
            serverLocked = true;
            OSD::Notify(Color::Red << "Server Status: Locked.");
        } if (Controller::IsKeyDown(Key::A) && Controller::IsKeyPressed(Key::DPadDown)){
            serverLocked = false;
            OSD::Notify(Color::Lime << "Server Status: Unlocked.");
            Process::Write16(0x8FD8A90, 0x01);
            Process::Write16(0x8FD8A92, 0x08);
        } if (serverLocked == true){
            Process::Write16(0x8FD8A90, 0xFF);
            Process::Write16(0x8FD8A92, 0xFF);
        }
    }

    void cloneServers(MenuEntry *entry){
        u32 val = 0xFFFFFFFF; u32 oVal;
        if (Controller::IsKeyPressed(Key::Start)){
            Process::Write32(0x8E48A50, val);
            Process::Write32(0x8FF5C48, 0x00);
            Process::Write32(0x8FF5F74, 0x00);
            Process::Write32(0x8FF69C4, 0x00);
            Process::Write32(0x90017F4, 0x00);
            Process::Read32(0x8E48A50, oVal);
            if (oVal == val){
                Process::Write32(0x8FF1A58, val);
                OSD::Notify(Color::Red << "Server Cloned Successfully.");
            }
        }
    }

}