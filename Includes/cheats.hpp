#ifndef CHEATS_H
#define CHEATS_H

#include <CTRPluginFramework.hpp>
#include "Helpers.hpp"
#include "Unicode.h"

namespace CTRPluginFramework
{
    void utilityKeyboard(MenuEntry *entry);
    void fasterLoading(MenuEntry *entry);
    void allStatusMax(MenuEntry *entry);
    void maxHealth(MenuEntry *entry);
    void maxFood(MenuEntry *entry);
    void maxWater(MenuEntry *entry);
    void maxBattery(MenuEntry *entry);
    void maxTemp(MenuEntry *entry);
    void alwaysNight(MenuEntry *entry);
    void alwaysDay(MenuEntry *entry);
    void setStatsColor(MenuEntry *entry);
    void fastServerRefresh(MenuEntry *entry);
    void autoHeal(MenuEntry *entry);
    void flashlightAssignedToKey(MenuEntry *entry);
    void showHideAllIDs(MenuEntry *entry);
    void xrayWorld(MenuEntry *entry);
    void advancedMapZoom(MenuEntry *entry);
    void seeAllNames(MenuEntry *entry);
    void copyPasteInventory(MenuEntry *entry);
    void noItemLoss(MenuEntry *entry);
    void unlockAllOutfits(MenuEntry *entry);
    void hackerInventory(MenuEntry *entry);
    void constantNameChanger(MenuEntry *entry);
    void noEnginePartsNeeded(MenuEntry *entry);
    void noFuelNeeded(MenuEntry *entry);
    void walkOnWater(MenuEntry *entry);
    void walkThroughWalls(MenuEntry *entry);
    void moonJump(MenuEntry *entry);
    void carSpeedHack(MenuEntry *entry);
    void noClip(MenuEntry *entry);
    void stalkEntities(MenuEntry *entry);
    void aimbot(MenuEntry *entry);
    void revivePlayer(MenuEntry *entry);
    void noClip(MenuEntry *entry);
    void rapidFire(MenuEntry *entry);
    void noReload(MenuEntry *entry);
    void noRecoil(MenuEntry *entry);
    void spamDoorSounds(MenuEntry *entry);
    void healKnife(MenuEntry *entry);
    void serverLocker(MenuEntry *entry);
    void cloneServers(MenuEntry *entry);
    std::vector<u32> findStruct();
    void selectPlayerToKick();
    void setCarHackSpeed();
    void nil();
    void wtw();
    void setMoonJump();
    void nameChanger();
    // decl bools, ints, etc
}
#endif
