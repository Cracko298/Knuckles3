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
#include <iomanip>
#include <cstring>
#include <cstdint>
#include <random>

namespace CTRPluginFramework
{

    u32 const NAME_PAGE = 0x01E81000+0x6F0+0x24;
    u32 const TEXT_PAGE = 0x01E81000+0x6F0;
    u32 const INVENTORY_PAGE = 0x01E81000;
    float static moonJumpVar = 2.0;
    float static carSpeedSlow = 5.0;
    float static carSpeedFast = 32.0;

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

    void ClearBuffer(u32 address, size_t length){
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
            for (int i = 0; i < 20; ++i)
            {
                u8 c;
                Process::Read8(textAdd + i, c);

                if (c == 0x00)
                {
                    if (i > 0)
                    {
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
            ClearBuffer(textAdd, 20);
        } if (commandPrefix == ".spn") {
            // do stuff eventually
        } if (commandPrefix == ".wtr") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            u32 wtrVal = std::stoi(commandInfo);
            Process::Write32(wtrAdd, wtrVal);
            ClearBuffer(textAdd, 20);
        } if (commandPrefix == ".tmp") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            float tempVal = std::stof(commandInfo);
            Process::WriteFloat(tempAdd, tempVal);
            ClearBuffer(textAdd, 20);
        } if (commandPrefix == ".hng") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            u32 hngVal = std::stoi(commandInfo);
            Process::Write32(hngAdd, hngVal);
            ClearBuffer(textAdd, 20);
        } if (commandPrefix == ".bat") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x10, StringFormat::Utf8);
            u32 batVal = std::stoi(commandInfo);
            Process::Write32(batAdd, batVal);
            ClearBuffer(textAdd, 20);
        } if (commandPrefix == ".tim") {
            Process::ReadString(textAdd + 0x04, commandInfo, 0x04, StringFormat::Utf8);
            float timeVal = GetFloatFromTime(commandInfo);
            Process::WriteFloat(timeAdd, timeVal);
            ClearBuffer(textAdd, 20);
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
        const u32 dstAddr1 = INVENTORY_PAGE;
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

    void noItemLoss(MenuEntry *entry){
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

    void walkThroughWalls(MenuEntry *entry){
        Process::Write32(0x482F28, 0xBDCCCCCD);
        Process::Write32(0x15FCE0, 0x3F800000);
        Process::Write32(0x12DBD0, 0x0A000002);
        Process::Write32(0x249F47, 0x00000043);
        Process::Write32(0x242214, 0xED800A08);
        Process::Write32(0x2421A4, 0x0A000016);
        if (Controller::IsKeyDown(Key::Y)){
            Process::Write32(0x2421A4, 0xEA000016);
            Process::Write32(0x242214, 0xEAFFFFE9);
            Process::Write32(0x12DBD0, 0x1A000002);
            Process::Write32(0x482F28, 0x00);
            Process::Write32(0x4836B4, 0xFFFFFFFF);
        }
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


}