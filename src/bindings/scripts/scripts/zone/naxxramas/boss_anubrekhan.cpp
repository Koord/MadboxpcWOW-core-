/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Boss_Anubrekhan
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"

#define SAY_GREET           -1533000
#define SAY_AGGRO1          -1533001
#define SAY_AGGRO2          -1533002
#define SAY_AGGRO3          -1533003
#define SAY_TAUNT1          -1533004
#define SAY_TAUNT2          -1533005
#define SAY_TAUNT3          -1533006
#define SAY_TAUNT4          -1533007
#define SAY_SLAY            -1533008

#define SPELL_IMPALE        HEROIC(28783,56090)
#define SPELL_LOCUSTSWARM   HEROIC(28785,54021)

#define SPELL_SELF_SPAWN_5  29105                           //This spawns 5 corpse scarabs ontop of us (most likely the player casts this on death)
#define SPELL_SELF_SPAWN_10 28864                           //This is used by the crypt guards when they die

#define EVENT_IMPALE        1
#define EVENT_LOCUST        2

#define MOB_CRYPT_GUARD     16573

struct TRINITY_DLL_DECL boss_anubrekhanAI : public ScriptedAI
{
    boss_anubrekhanAI(Creature *c) : ScriptedAI(c), summons(me) {}

    bool HasTaunted;
    EventMap events;
    SummonList summons;

    void Reset()
    {
        events.Reset();
        summons.DespawnAll();
        HasTaunted = false;
        DoSpawnCreature(MOB_CRYPT_GUARD, 0, 10, 0, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
        if(HeroicMode)
            DoSpawnCreature(MOB_CRYPT_GUARD, 0, -10, 0, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
    }

    void JustSummoned(Creature *summon)
    {
        summons.Summon(summon);
        DoZoneInCombat(summon);
    }

    void SummonedCreatureDespawn(Creature *summon) {summons.Despawn(summon);}

    void KilledUnit(Unit* victim)
    {
        //Force the player to spawn corpse scarabs via spell
        victim->CastSpell(victim, SPELL_SELF_SPAWN_5, true, NULL, NULL, me->GetGUID());

        if(!(rand()%5))
            DoScriptText(SAY_SLAY, me);
    }

    void EnterCombat(Unit *who)
    {
        switch(rand()%3)
        {
        case 0: DoScriptText(SAY_AGGRO1, m_creature); break;
        case 1: DoScriptText(SAY_AGGRO2, m_creature); break;
        case 2: DoScriptText(SAY_AGGRO3, m_creature); break;
        }

        DoZoneInCombat();

        events.ScheduleEvent(EVENT_IMPALE, 15000, 1);
        events.ScheduleEvent(EVENT_LOCUST, 80000 + rand()%40000, 1);
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!HasTaunted && m_creature->IsWithinDistInMap(who, 60.0f))
        {
            switch(rand()%5)
            {
            case 0: DoScriptText(SAY_GREET, m_creature); break;
            case 1: DoScriptText(SAY_TAUNT1, m_creature); break;
            case 2: DoScriptText(SAY_TAUNT2, m_creature); break;
            case 3: DoScriptText(SAY_TAUNT3, m_creature); break;
            case 4: DoScriptText(SAY_TAUNT4, m_creature); break;
            }
            HasTaunted = true;
        }
        ScriptedAI::MoveInLineOfSight(who);
    }

    void UpdateAI(const uint32 diff)
    {
        if(!UpdateVictim())
            return;

        events.Update(diff);

        while(uint32 eventId = events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_IMPALE:
                    //Cast Impale on a random target
                    //Do NOT cast it when we are afflicted by locust swarm
                    if(!me->HasAura(SPELL_LOCUSTSWARM))
                        if(Unit* target = SelectUnit(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_IMPALE);
                    events.ScheduleEvent(EVENT_IMPALE, 15000, 1);
                    events.DelayEvents(1500, 1);
                    return;
                case EVENT_LOCUST:
                    DoCast(m_creature, SPELL_LOCUSTSWARM);
                    DoSpawnCreature(MOB_CRYPT_GUARD, 5, 5, 0, 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
                    events.ScheduleEvent(EVENT_LOCUST, 90000, 1);
                    return;
            }
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_anubrekhan(Creature *_Creature)
{
    return new boss_anubrekhanAI (_Creature);
}

void AddSC_boss_anubrekhan()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name="boss_anubrekhan";
    newscript->GetAI = &GetAI_boss_anubrekhan;
    newscript->RegisterSelf();
}

