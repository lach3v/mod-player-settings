#ifndef _PLAYER_SETTINGS_MODULE_H_
#define _PLAYER_SETTINGS_MODULE_H_

#include "ScriptMgr.h"
#include "Creature.h"

// Manages registration, loading, and execution of scripts.
class PSScriptMgr
{
    public: /* Initialization */

        static PSScriptMgr* instance();
        // Called at the start of ModifyCreatureAttributes method.  For example,
        // it can be used to add some condition to skip scaling system.
        bool OnBeforeModifyAttributes(Creature *creature, uint32 &instancePlayerCount);
        // Called right after default multiplier has been set, you can use it to
        // change current scaling formula based on number of players or just
        // skip modifications.
        bool OnAfterDefaultMultiplier(Creature *creature, float &defaultMultiplier);
        // Called before change in creature values to tune some values or skip
        // modifications.
        bool OnBeforeUpdateStats(Creature *creature, uint32 &scaledHealth, float &damageMultiplier);
};

#define sPSScriptMgr PSScriptMgr::instance()

/*
 * Dedicated hooks for Player Settings Module which can be used to
 * extend/customize this system.
 */
class PSModuleScript : public ModuleScript
{
    protected:

        PSModuleScript(const char *name);

    public:

        virtual bool OnBeforeModifyAttributes(Creature */*creature*/, uint32 &/*instancePlayerCount*/) { return true; }
        virtual bool OnAfterDefaultMultiplier(Creature */*creature*/, float &/*defaultMultiplier*/) { return true; }
        virtual bool OnBeforeUpdateStats(Creature */*creature*/, uint32 &/*scaledHealth*/, float &/*damageMultiplier*/) { return true; }
};

template class ScriptRegistry<PSModuleScript>;

#endif