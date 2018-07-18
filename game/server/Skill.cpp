/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//=========================================================
// skill.cpp - code for skill level concerns
//=========================================================
#include "extdll.h"
#include "util.h"
#include "Skill.h"
#include "Server.h"
#include "gamerules/GameRules.h"

skilldata_t	gSkillData;

cvar_t* skilldata_t::GetSkillCvar( const char* pszName, const SkillLevel skillLevel )
{
	char szBuffer[ 64 ];

	/*const int iCount = */sprintf( szBuffer, "%s%d", pszName, skillLevel );

	cvar_t* pCvar = CVAR_GET_POINTER( szBuffer );

	if( !pCvar )
	{
		ALERT( at_console, "\n\n** GetSkillCVar No such Cvar %s **\n\n", szBuffer );
		//To prevent having to check if every cvar is null, just return a dummy.
		//This is a problem if it gets modified (it shouldn't), but it's much easier to deal with. - Solokiller
		pCvar = &g_DummyCvar;
	}

	if( pCvar->value <= 0 )
	{
		ALERT( at_console, "\n\n** GetSkillCVar Got a zero for %s **\n\n", szBuffer );
	}

	return pCvar;
}

//=========================================================
// take the name of a cvar, tack a digit for the skill level
// on, and return the value.of that Cvar 
//=========================================================
cvar_t* skilldata_t::GetSkillCvar( const char* pszName ) const
{
	return g_pGameRules->GetSkillCvar( *this, pszName );
}

void skilldata_t::RefreshSkillData()
{
	int	iSkill = ( int ) CVAR_GET_FLOAT( "skill" );

	if( iSkill < SKILL_FIRST )
	{
		iSkill = SKILL_FIRST;
	}
	else if( iSkill > SKILL_LAST )
	{
		iSkill = SKILL_LAST;
	}

	SetSkillLevel( static_cast<SkillLevel>( iSkill ) );

	ALERT( at_console, "\nGAME SKILL LEVEL:%d\n", iSkill );

	//Agrunt		
	agruntHealth = GetSkillCvar( "sk_agrunt_health" );
	agruntDmgPunch = GetSkillCvar( "sk_agrunt_dmg_punch" );

	// Apache 
	apacheHealth = GetSkillCvar( "sk_apache_health" );

	// Barney
	barneyHealth = GetSkillCvar( "sk_barney_health" );

	// Big Momma
	bigmommaHealthFactor = GetSkillCvar( "sk_bigmomma_health_factor" );
	bigmommaDmgSlash = GetSkillCvar( "sk_bigmomma_dmg_slash" );
	bigmommaDmgBlast = GetSkillCvar( "sk_bigmomma_dmg_blast" );
	bigmommaRadiusBlast = GetSkillCvar( "sk_bigmomma_radius_blast" );

	// Bullsquid
	bullsquidHealth = GetSkillCvar( "sk_bullsquid_health" );
	bullsquidDmgBite = GetSkillCvar( "sk_bullsquid_dmg_bite" );
	bullsquidDmgWhip = GetSkillCvar( "sk_bullsquid_dmg_whip" );
	bullsquidDmgSpit = GetSkillCvar( "sk_bullsquid_dmg_spit" );

	// Gargantua
	gargantuaHealth = GetSkillCvar( "sk_gargantua_health" );
	gargantuaDmgSlash = GetSkillCvar( "sk_gargantua_dmg_slash" );
	gargantuaDmgFire = GetSkillCvar( "sk_gargantua_dmg_fire" );
	gargantuaDmgStomp = GetSkillCvar( "sk_gargantua_dmg_stomp" );

	// Hassassin
	hassassinHealth = GetSkillCvar( "sk_hassassin_health" );

	// Headcrab
	headcrabHealth = GetSkillCvar( "sk_headcrab_health" );
	headcrabDmgBite = GetSkillCvar( "sk_headcrab_dmg_bite" );

	// Hgrunt 
	hgruntHealth = GetSkillCvar( "sk_hgrunt_health" );
	hgruntDmgKick = GetSkillCvar( "sk_hgrunt_kick" );
	hgruntShotgunPellets = GetSkillCvar( "sk_hgrunt_pellets" );
	hgruntGrenadeSpeed = GetSkillCvar( "sk_hgrunt_gspeed" );

	// Houndeye
	houndeyeHealth = GetSkillCvar( "sk_houndeye_health" );
	houndeyeDmgBlast = GetSkillCvar( "sk_houndeye_dmg_blast" );

	// ISlave
	slaveHealth = GetSkillCvar( "sk_islave_health" );
	slaveDmgClaw = GetSkillCvar( "sk_islave_dmg_claw" );
	slaveDmgClawrake = GetSkillCvar( "sk_islave_dmg_clawrake" );
	slaveDmgZap = GetSkillCvar( "sk_islave_dmg_zap" );

	// Icthyosaur
	ichthyosaurHealth = GetSkillCvar( "sk_ichthyosaur_health" );
	ichthyosaurDmgShake = GetSkillCvar( "sk_ichthyosaur_shake" );

	// Leech
	leechHealth = GetSkillCvar( "sk_leech_health" );

	leechDmgBite = GetSkillCvar( "sk_leech_dmg_bite" );

	// Controller
	controllerHealth = GetSkillCvar( "sk_controller_health" );
	controllerDmgZap = GetSkillCvar( "sk_controller_dmgzap" );
	controllerSpeedBall = GetSkillCvar( "sk_controller_speedball" );
	controllerDmgBall = GetSkillCvar( "sk_controller_dmgball" );

	// Nihilanth
	nihilanthHealth = GetSkillCvar( "sk_nihilanth_health" );
	nihilanthZap = GetSkillCvar( "sk_nihilanth_zap" );

	// Scientist
	scientistHealth = GetSkillCvar( "sk_scientist_health" );

	// Snark
	snarkHealth = GetSkillCvar( "sk_snark_health" );
	snarkDmgBite = GetSkillCvar( "sk_snark_dmg_bite" );
	snarkDmgPop = GetSkillCvar( "sk_snark_dmg_pop" );

	// Zombie
	zombieHealth = GetSkillCvar( "sk_zombie_health" );
	zombieDmgOneSlash = GetSkillCvar( "sk_zombie_dmg_one_slash" );
	zombieDmgBothSlash = GetSkillCvar( "sk_zombie_dmg_both_slash" );

	//Turret
	turretHealth = GetSkillCvar( "sk_turret_health" );

	// MiniTurret
	miniturretHealth = GetSkillCvar( "sk_miniturret_health" );

	// Sentry Turret
	sentryHealth = GetSkillCvar( "sk_sentry_health" );

	// PLAYER WEAPONS

	// Crowbar whack
	plrDmgCrowbar = GetSkillCvar( "sk_plr_crowbar" );

	// Glock Round
	plrDmg9MM = GetSkillCvar( "sk_plr_9mm_bullet" );

	// 357 Round
	plrDmg357 = GetSkillCvar( "sk_plr_357_bullet" );

	// MP5 Round
	plrDmgMP5 = GetSkillCvar( "sk_plr_9mmAR_bullet" );

	// M203 grenade
	plrDmgM203Grenade = GetSkillCvar( "sk_plr_9mmAR_grenade" );

	// Shotgun buckshot
	plrDmgBuckshot = GetSkillCvar( "sk_plr_buckshot" );

	// Crossbow
	plrDmgCrossbowClient = GetSkillCvar( "sk_plr_xbow_bolt_client" );
	plrDmgCrossbowMonster = GetSkillCvar( "sk_plr_xbow_bolt_monster" );

	// RPG
	plrDmgRPG = GetSkillCvar( "sk_plr_rpg" );

	// Gauss gun
	plrDmgGauss = GetSkillCvar( "sk_plr_gauss" );

	// Egon Gun
	plrDmgEgonNarrow = GetSkillCvar( "sk_plr_egon_narrow" );
	plrDmgEgonWide = GetSkillCvar( "sk_plr_egon_wide" );

	// Hand Grendade
	plrDmgHandGrenade = GetSkillCvar( "sk_plr_hand_grenade" );

	// PLAYER HORNET
	plrDmgHornet = GetSkillCvar( "sk_plr_hornet_dmg" );

	// Satchel Charge
	plrDmgSatchel = GetSkillCvar( "sk_plr_satchel" );

	// Tripmine
	plrDmgTripmine = GetSkillCvar( "sk_plr_tripmine" );

#ifdef GAME_OPFOR
	// Knife whack
	plrDmgKnife = GetSkillCvar( "sk_plr_knife" );

	// Pipewrench whack
	plrDmgPipewrench = GetSkillCvar( "sk_plr_pipewrench" );

	// Grapple
	plrDmgGrapple = GetSkillCvar( "sk_plr_grapple" );

	// 556 Round
	plrDmg556 = GetSkillCvar( "sk_plr_556_bullet" );

	// 762 Round
	plrDmg762 = GetSkillCvar( "sk_plr_762_bullet" );

	// Desert Eagle
	plrDmgDeagle = GetSkillCvar( "sk_plr_eagle" );

	// Shock Rifle
	plrDmgShockRoachS = GetSkillCvar( "sk_plr_shockroachs" );
	plrDmgShockRoachM = GetSkillCvar( "sk_plr_shockroachm" );

	// Displacer Cannon
	plrDmgDisplacerOther = GetSkillCvar( "sk_plr_displacer_other" );
	plrRadiusDisplacer = GetSkillCvar( "sk_plr_displacer_radius" );

	// Spore
	plrDmgSpore = GetSkillCvar( "sk_plr_spore" );
#endif

	// MONSTER WEAPONS
	monDmg12MM = GetSkillCvar( "sk_12mm_bullet" );
	monDmgMP5 = GetSkillCvar( "sk_9mmAR_bullet" );
	monDmg9MM = GetSkillCvar( "sk_9mm_bullet" );

	// MONSTER HORNET
	monDmgHornet = GetSkillCvar( "sk_hornet_dmg" );

	// HEALTH/CHARGE
	suitchargerCapacity = GetSkillCvar( "sk_suitcharger" );
	batteryCapacity = GetSkillCvar( "sk_battery" );
	healthchargerCapacity = GetSkillCvar( "sk_healthcharger" );
	healthkitCapacity = GetSkillCvar( "sk_healthkit" );
	scientistHeal = GetSkillCvar( "sk_scientist_heal" );

	// monster damage adj
	monHead = GetSkillCvar( "sk_monster_head" );
	monChest = GetSkillCvar( "sk_monster_chest" );
	monStomach = GetSkillCvar( "sk_monster_stomach" );
	monLeg = GetSkillCvar( "sk_monster_leg" );
	monArm = GetSkillCvar( "sk_monster_arm" );

	// player damage adj
	plrHead = GetSkillCvar( "sk_player_head" );
	plrChest = GetSkillCvar( "sk_player_chest" );
	plrStomach = GetSkillCvar( "sk_player_stomach" );
	plrLeg = GetSkillCvar( "sk_player_leg" );
	plrArm = GetSkillCvar( "sk_player_arm" );
}