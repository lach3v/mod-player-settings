#include "ScriptMgr.h"
#include "ScriptMgrMacros.h"
#include "MapManager.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "PlayerSettings.h"

PSScriptMgr *PSScriptMgr::instance() {
    static PSScriptMgr instance;
    return &instance;
}

bool PSScriptMgr::OnBeforeModifyAttributes(Creature *creature, uint32 &instancePlayerCount) {
    bool ret = true;
    FOR_SCRIPTS_RET(PSModuleScript, itr, end, ret)
    if (!itr->second->OnBeforeModifyAttributes(creature, instancePlayerCount)) {
        ret = false;
    }
    return ret;
}

bool PSScriptMgr::OnAfterDefaultMultiplier(Creature *creature, float &defaultMultiplier) {
    bool ret = true;
    FOR_SCRIPTS_RET(PSModuleScript, itr, end, ret)
    if (!itr->second->OnAfterDefaultMultiplier(creature, defaultMultiplier)) {
        ret = false;
    }
    return ret;
}

bool PSScriptMgr::OnBeforeUpdateStats(Creature *creature, uint32 &scaledHealth, float &damageMutliplier) {
    bool ret = true;
    FOR_SCRIPTS_RET(PSModuleScript, itr, end, ret)
    if (!itr->second->OnBeforeUpdateStats(creature, scaledHealth, damageMutliplier)) {
        ret = false;
    }
    return ret;
}

PSModuleScript::PSModuleScript(const char *name)
    : ModuleScript(name)
{
    ScriptRegistry<PSModuleScript>::AddScript(this);
}

class PlayerSettingsCreatureInfo : public DataMap::Base
{
    public:

        PlayerSettingsCreatureInfo() {}
        PlayerSettingsCreatureInfo(uint32 count, float dmg, float hpRate) :
            instancePlayerCount(count), DamageMultiplier(dmg), HealthMultiplier(hpRate) {}
        uint32 instancePlayerCount = 0;
        // This is used to detect creatures that update their entry.
        uint32 entry = 0;
        float DamageMultiplier = 1;
        float HealthMultiplier = 1;
};

class PlayerSettingsMapInfo : public DataMap::Base
{
    public:

        PlayerSettingsMapInfo() {}
        PlayerSettingsMapInfo(uint32 count) : playerCount(count) {}
        uint32 playerCount = 0;
};

static bool enabled;

int GetValidDebugLevel()
{
    int debugLevel = sConfigMgr->GetIntDefault("PlayerSettings.DebugLevel", 2);

    if ((debugLevel < 0) || (debugLevel > 3))
    {
        return 1;
    }

    return debugLevel;
}

class PlayerSettings_WorldScript : public WorldScript
{
    public:
        PlayerSettings_WorldScript()
            : WorldScript("PlayerSettings_WorldScript") {}
        
        void OnBeforeConfigLoad(bool /*reload*/) override
        {
            SetInitialWorldSettings();
        }

        void SetInitialWorldSettings()
        {
            enabled = sConfigMgr->GetBoolDefault("PlayerSettings.Enable", 1);
        }
};

// Add player scripts
class PlayerSettings_PlayerScript : public PlayerScript
{
public:
    PlayerSettings_PlayerScript() : PlayerScript("PlayerSettingsPlayer") { }

    void OnLogin(Player* player) override
    {
        if (enabled)
        {
            ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Player Settings |rmodule.");
        }
    }

    void OnGiveXP(Player *player, uint32 &amount, Unit *victim) override
    {
        if (victim)
        {
            Map *map = player->GetMap();

            if (map->IsDungeon())
            {
                // Ensure that the players always get the same XP, even when
                // entering the dungeon alone.
                uint32 maxPlayerCount = ((InstanceMap*)sMapMgr->FindMap(map->GetId(), map->GetInstanceId()))->GetMaxPlayers();
                uint32 currentPlayerCount = map->GetPlayersCountExceptGMs();
                amount *= (float)currentPlayerCount / maxPlayerCount;
            }
        }
    }
};

// Add all scripts in one
void AddPlayerSettingsScripts()
{
    new PlayerSettings_WorldScript();
    new PlayerSettings_PlayerScript();
}
