#include "ScriptMgr.h"
#include "ScriptMgrMacros.h"
#include "MapManager.h"
#include "Player.h"
#include "Unit.h"
#include "Config.h"
#include "Chat.h"

// DPS count as 1 offensive unit and tanks count as 0.5 offensive units.
// Healers count as 1 defensive unit and tanks count as 0.5 defensive units.

// 5 man: 1 tank, 3 dps, 1 healer = 3.5 offensive units and 1.5 defensive units.
const float Offence5M = 1 / 3.5f, Defence5M = 1 / 1.5f;

// 10 man: 2 tank, 6 dps, 2 healer = 7.0 offensive units and 3.0 defensive units.
const float Offence10M = 1 / 7.0f, Defence10M = 1 / 3.0f;

// 25 man: 3 tank, 17 dps, 5 healer = 18.5 offensive units and 6.5 defensive units.
const float Offence25M = 1 / 18.5f, Defence25M = 1 / 6.5f;

// 40 man: 4 tank, 28 dps, 8 healer = 30.0 offensive units and 10 defensive units.
const float Offence40M = 1 / 30.0f, Defence40M = 10.0f;

class PlayerSettingsCreatureInfo : public DataMap::Base
{
    public:

        PlayerSettingsCreatureInfo() {}
        PlayerSettingsCreatureInfo(uint32 count, float dmg, float hpRate) :
            playerCount(count), DamageMultiplier(dmg), HealthMultiplier(hpRate) {}
        uint32 playerCount = 0;
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

    if ((debugLevel < 0) || (debugLevel > 3)) {
        return 1;
    }

    return debugLevel;
}

class PlayerSettingsWorldScript : public WorldScript
{
    public:
        PlayerSettingsWorldScript()
            : WorldScript("PlayerSettingsWorldScript") {}

        void OnBeforeConfigLoad(bool /*reload*/) override
        {
            SetInitialWorldSettings();
        }

        void SetInitialWorldSettings()
        {
            enabled = sConfigMgr->GetBoolDefault("PlayerSettings.Enable", 1);
        }
};

class PlayerSettingsPlayerScript : public PlayerScript
{
    public:

        PlayerSettingsPlayerScript() : PlayerScript("PlayerSettingsPlayer") { }

        void OnLogin(Player* player) override
        {
            if (enabled) {
                ChatHandler(player->GetSession()).SendSysMessage("This server is running the |cff4CFF00Player Settings |rmodule.");
            }
        }

        void OnGiveXP(Player *player, uint32 &amount, Unit *victim) override
        {
            if (victim) {
                Map *map = player->GetMap();

                if (map->IsDungeon()) {
                    // Ensure that the players always get the same XP, even when
                    // entering the dungeon alone.
                    uint32 maxPlayerCount = ((InstanceMap*)sMapMgr->FindMap(map->GetId(), map->GetInstanceId()))->GetMaxPlayers();
                    uint32 currentPlayerCount = map->GetPlayersCountExceptGMs();
                    amount *= (float)currentPlayerCount / maxPlayerCount;
                }
            }
        }
};

class PlayerSettingsUnitScript : public UnitScript
{
    public:

        PlayerSettingsUnitScript()
            : UnitScript("PlayerSettingsUnitScript", true) {}

        uint32 DealDamage(Unit *AttackerUnit, Unit *playerVictim, uint32 damage, DamageEffectType) override
        {
            return _DealDamage(playerVictim, AttackerUnit, damage);
        }

        void ModifyPeriodicDamageAurasTick(Unit *target, Unit *attacker, uint32 &damage) override
        {
            damage = _DealDamage(target, attacker, damage);
        }

        void ModifySpellDamageTaken(Unit *target, Unit *attacker, int32 &damage) override
        {
            damage = _DealDamage(target, attacker, damage);
        }

        void ModifyMeleeDamage(Unit *target, Unit *attacker, uint32 &damage) override
        {
            damage = _DealDamage(target, attacker, damage);
        }

        void ModifyHealRecieved(Unit *target, Unit *attacker, uint32 &damage) override
        {
            damage = _DealDamage(target, attacker, damage);
        }

        uint32 _DealDamage(Unit *target, Unit *attacker, uint32 damage)
        {
            if (!enabled)
                return damage;

            if (!attacker || attacker->GetTypeId() == TYPEID_PLAYER || !attacker->IsInWorld())
                return damage;

            float damageMultiplier = attacker->CustomData.GetDefault<PlayerSettingsCreatureInfo>("PlayerSettingsCreatureInfo")->DamageMultiplier;

            if (damageMultiplier == 1)
                return damage;

            return damage * damageMultiplier;
        }
};

class PlayerSettingsAllMapScript : public AllMapScript
{
    public:
        PlayerSettingsAllMapScript()
            : AllMapScript("PlayerSettingsAllMapScript") {}

        void OnPlayerEnterAll(Map *map, Player *player)
        {
            if (!enabled)
                return;

            if (player->IsGameMaster())
                return;

            PlayerSettingsMapInfo *mapInfo = map->CustomData.GetDefault<PlayerSettingsMapInfo>("PlayerSettingsMapInfo");
            mapInfo->playerCount = map->GetPlayersCountExceptGMs();

            if (map->GetEntry()->IsDungeon() && player) {
                Map::PlayerList const &playerList = map->GetPlayers();

                if (!playerList.isEmpty()) {
                    for (Map::PlayerList::const_iterator iter = playerList.begin(); iter != playerList.end(); iter++) {
                        if (Player *playerHandle = iter->GetSource()) {
                            ChatHandler chatHandle = ChatHandler(playerHandle->GetSession());
                            chatHandle.PSendSysMessage("|cffFF0000 [PlayerSettings]|r %s has entered the instance. The minions of Hell grow stronger.", player->GetName().c_str());
                        }
                    }
                }
            }
        }

        void OnPlayerLeaveAll(Map *map, Player *player)
        {
            if (!enabled)
                return;

            if (player->IsGameMaster())
                return;

            PlayerSettingsMapInfo *mapInfo = map->CustomData.GetDefault<PlayerSettingsMapInfo>("PlayerSettingsMapInfo");

            if (map->GetEntry() && map->GetEntry()->IsDungeon()) {
                bool PlayerSettingsCheck = false;
                Map::PlayerList const &playerList = map->GetPlayers();

                for (Map::PlayerList::const_iterator iter = playerList.begin(); iter != playerList.end(); iter++) {
                    if (Player *player = iter->GetSource())
                    {
                        if (player->IsInCombat())
                            PlayerSettingsCheck = true;
                    }
                }

                if (PlayerSettingsCheck) {
                    for (Map::PlayerList::const_iterator iter = playerList.begin(); iter != playerList.end(); iter++)
                    {
                        if (Player *player = iter->GetSource())
                        {
                            player->GetSession()->SendNotification("|cffFF0000 [PlayerSettings]|r %s has left during a battle. Instance elastic lock.", player->GetName().c_str());
                        }
                    }
                } else {

                }
            }

            if (map->GetEntry()->IsDungeon() && player) {
                Map::PlayerList const &playerList = map->GetPlayers();

                if (!playerList.isEmpty()) {
                    for (Map::PlayerList::const_iterator iter = playerList.begin(); iter != playerList.end(); iter++) {
                        if (Player *playerHandle = iter->GetSource()) {
                            ChatHandler chatHandle = ChatHandler(playerHandle->GetSession());
                            chatHandle.PSendSysMessage("|cffFF0000 [PlayerSettings]|r %s has left the instance. The minions of Hell grow weaker.", player->GetName().c_str());
                        }
                    }
                }
            }
        }
};

class PlayerSettingsAllCreatureScript : public AllCreatureScript
{
    public:
        PlayerSettingsAllCreatureScript()
            : AllCreatureScript("PlayerSettingsAllCreatureScript") {}

        void OnAllCreatureUpdate(Creature *creature, uint32 /*diff*/) override
        {
            if (!enabled)
                return;

            ModifyCreatureAttributes(creature);
        }

        void ModifyCreatureAttributes(Creature *creature)
        {
            if (!creature || !creature->GetMap())
                return;

            if (!creature->GetMap()->IsDungeon() && !creature->GetMap()->IsBattleground())
                return;

            if (((creature->IsHunterPet() || creature->IsPet() || creature->IsSummon()) && creature->IsControlledByPlayer()))
                return;

            PlayerSettingsMapInfo *mapInfo = creature->GetMap()->CustomData.GetDefault<PlayerSettingsMapInfo>("PlayerSettingsMapInfo");
            CreatureTemplate const *creatureTemplate = creature->GetCreatureTemplate();
            InstanceMap *instanceMap = ((InstanceMap*)sMapMgr->FindMap(creature->GetMapId(), creature->GetInstanceId()));
            uint32 maxPlayers = instanceMap->GetMaxPlayers();
            PlayerSettingsCreatureInfo *creatureInfo = creature->CustomData.GetDefault<PlayerSettingsCreatureInfo>("PlayerSettingsCreatureInfo");

            if (!creature->IsAlive())
                return;

            creatureInfo->playerCount = mapInfo->playerCount;

            if (!creatureInfo->playerCount)
                return;

            creatureInfo->entry = creature->GetEntry();

            CreatureBaseStats const *stats = sObjectMgr->GetCreatureBaseStats(creature->getLevel(), creatureTemplate->unit_class);
            uint32 baseHealth = stats->GenerateHealth(creatureTemplate);
            uint32 scaledHealth = 0;

            float offence = 1.0f;
            float defence = 1.0f;
            if (creatureInfo->playerCount < maxPlayers) {
                if (instanceMap->IsRaid()) {
                    switch (instanceMap->GetMaxPlayers()) {
                        case 10:
                            offence = Offence10M;
                            defence = Defence10M;
                            break;
                        case 25:
                            offence = Offence25M;
                            defence = Defence25M;
                            break;
                        default:
                            offence = Offence40M;
                            defence = Defence40M;
                            break;
                    }
                } else {
                    offence = Offence5M;
                    defence = Defence5M;
                }
            }

            creatureInfo->HealthMultiplier = offence * (1 + 0.5f * (creatureInfo->playerCount - 1));
            creatureInfo->DamageMultiplier = defence * (1 + 0.0625f * (creatureInfo->playerCount - 1));

            scaledHealth = round(((float)baseHealth * creatureInfo->HealthMultiplier) + 1.0f);

            uint32 previousMaxHealth = creature->GetMaxHealth();
            uint32 previousHealth = creature->GetHealth();

            creature->SetCreateHealth(scaledHealth);
            creature->SetMaxHealth(scaledHealth);
            creature->ResetPlayerDamageReq();
            creature->SetModifierValue(UNIT_MOD_HEALTH, BASE_VALUE, (float)scaledHealth);

            uint32 scaledCurrentHealth = previousHealth && previousMaxHealth ? float(scaledHealth) / float(previousMaxHealth) * float(previousHealth) : 0;

            creature->SetHealth(scaledCurrentHealth);
            creature->UpdateAllStats();
        }
};

void AddPlayerSettingsScripts()
{
    new PlayerSettingsWorldScript();
    new PlayerSettingsPlayerScript();
    new PlayerSettingsUnitScript();
    new PlayerSettingsAllMapScript();
    new PlayerSettingsAllCreatureScript();
}
