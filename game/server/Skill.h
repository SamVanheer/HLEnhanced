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
#ifndef GAME_SERVER_SKILL_H
#define GAME_SERVER_SKILL_H
//=========================================================
// skill.h - skill level concerns
//=========================================================

enum SkillLevel
{
	SKILL_FIRST		= 1,

	SKILL_EASY		= SKILL_FIRST,
	SKILL_MEDIUM	= 2,
	SKILL_HARD		= 3,

	SKILL_LAST		= SKILL_HARD
};

struct skilldata_t
{
	/**
	*	@return The current skill level.
	*	@see SkillLevel
	*/
	SkillLevel GetSkillLevel() const { return m_SkillLevel; }

	/**
	*	Sets the current skill level.
	*/
	void SetSkillLevel( const SkillLevel skillLevel )
	{
		m_SkillLevel = skillLevel;
	}

	/**
	*	Gets the cvar pointer for the given skill cvar and skill level.
	*	@return Cvar pointer for the given skill cvar.
	*/
	static cvar_t* GetSkillCvar( const char* pszName, const SkillLevel skillLevel );

	/**
	*	Gets the cvar pointer for the given skill cvar.
	*	@return Cvar pointer for the given skill cvar.
	*/
	cvar_t* GetSkillCvar( const char* pszName ) const;

	/**
	*	Refreshes current skill data.
	*/
	void RefreshSkillData();

private:
	SkillLevel m_SkillLevel; // game skill level

	// Monster Health & Damage
	cvar_t* agruntHealth;
	cvar_t* agruntDmgPunch;

	cvar_t* apacheHealth;

	cvar_t* barneyHealth;

	cvar_t* bigmommaHealthFactor;
	cvar_t* bigmommaDmgSlash;
	cvar_t* bigmommaDmgBlast;
	cvar_t* bigmommaRadiusBlast;

	cvar_t* bullsquidHealth;
	cvar_t* bullsquidDmgBite;
	cvar_t* bullsquidDmgWhip;
	cvar_t* bullsquidDmgSpit;

	cvar_t* gargantuaHealth;
	cvar_t* gargantuaDmgSlash;
	cvar_t* gargantuaDmgFire;
	cvar_t* gargantuaDmgStomp;

	cvar_t* hassassinHealth;

	cvar_t* headcrabHealth;
	cvar_t* headcrabDmgBite;

	cvar_t* hgruntHealth;
	cvar_t* hgruntDmgKick;
	cvar_t* hgruntShotgunPellets;
	cvar_t* hgruntGrenadeSpeed;

	cvar_t* houndeyeHealth;
	cvar_t* houndeyeDmgBlast;

	cvar_t* slaveHealth;
	cvar_t* slaveDmgClaw;
	cvar_t* slaveDmgClawrake;
	cvar_t* slaveDmgZap;

	cvar_t* ichthyosaurHealth;
	cvar_t* ichthyosaurDmgShake;

	cvar_t* leechHealth;
	cvar_t* leechDmgBite;

	cvar_t* controllerHealth;
	cvar_t* controllerDmgZap;
	cvar_t* controllerSpeedBall;
	cvar_t* controllerDmgBall;

	cvar_t* nihilanthHealth;
	cvar_t* nihilanthZap;

	cvar_t* scientistHealth;

	cvar_t* snarkHealth;
	cvar_t* snarkDmgBite;
	cvar_t* snarkDmgPop;

	cvar_t* zombieHealth;
	cvar_t* zombieDmgOneSlash;
	cvar_t* zombieDmgBothSlash;

	cvar_t* turretHealth;
	cvar_t* miniturretHealth;
	cvar_t* sentryHealth;

	// Player Weapons
	cvar_t* plrDmgCrowbar;
	cvar_t* plrDmg9MM;
	cvar_t* plrDmg357;
	cvar_t* plrDmgMP5;
	cvar_t* plrDmgM203Grenade;
	cvar_t* plrDmgBuckshot;
	cvar_t* plrDmgCrossbowClient;
	cvar_t* plrDmgCrossbowMonster;
	cvar_t* plrDmgRPG;
	cvar_t* plrDmgGauss;
	cvar_t* plrDmgEgonNarrow;
	cvar_t* plrDmgEgonWide;
	cvar_t* plrDmgHornet;
	cvar_t* plrDmgHandGrenade;
	cvar_t* plrDmgSatchel;
	cvar_t* plrDmgTripmine;

#ifdef GAME_OPFOR
	cvar_t* plrDmgKnife;
	cvar_t* plrDmgPipewrench;
	cvar_t* plrDmgGrapple;
	cvar_t* plrDmg556;
	cvar_t* plrDmg762;
	cvar_t* plrDmgDeagle;
	cvar_t* plrDmgShockRoachS;
	cvar_t* plrDmgShockRoachM;
	cvar_t* plrDmgDisplacerOther;
	cvar_t* plrRadiusDisplacer;
	cvar_t* plrDmgSpore;
#endif

	// weapons shared by monsters
	cvar_t* monDmg9MM;
	cvar_t* monDmgMP5;
	cvar_t* monDmg12MM;
	cvar_t* monDmgHornet;

	// health/suit charge
	cvar_t* suitchargerCapacity;
	cvar_t* batteryCapacity;
	cvar_t* healthchargerCapacity;
	cvar_t* healthkitCapacity;
	cvar_t* scientistHeal;

	// monster damage adj
	cvar_t* monHead;
	cvar_t* monChest;
	cvar_t* monStomach;
	cvar_t* monLeg;
	cvar_t* monArm;

	// player damage adj
	cvar_t* plrHead;
	cvar_t* plrChest;
	cvar_t* plrStomach;
	cvar_t* plrLeg;
	cvar_t* plrArm;

public:
// Monster Health & Damage
	float GetAGruntHealth() const
	{
		return agruntHealth->value;
	}

	float GetAGruntDmgPunch() const
	{
		return agruntDmgPunch->value;
	}

	float GetApacheHealth() const
	{
		return apacheHealth->value;
	}

	float GetBarneyHealth() const
	{
		return barneyHealth->value;
	}

	/**
	*	Multiply each node's health by this
	*/
	float GetBigMommaHealthFactor() const
	{
		return bigmommaHealthFactor->value;
	}

	/**
	*	melee attack damage
	*/
	float GetBigMommaDmgSlash() const
	{
		return bigmommaDmgSlash->value;
	}

	/**
	*	mortar attack damage
	*/
	float GetBigMommaDmgBlast() const
	{
		return bigmommaDmgBlast->value;
	}

	/**
	*	mortar attack radius
	*/
	float GetBigMommaRadiusBlast() const
	{
		return bigmommaRadiusBlast->value;
	}

	float GetBullsquidHealth() const
	{
		return bullsquidHealth->value;
	}

	float GetBullsquidDmgBite() const
	{
		return bullsquidDmgBite->value;
	}

	float GetBullsquidDmgWhip() const
	{
		return bullsquidDmgWhip->value;
	}

	float GetBullsquidDmgSpit() const
	{
		return bullsquidDmgSpit->value;
	}

	float GetGargantuaHealth() const
	{
		return gargantuaHealth->value;
	}

	float GetGargantuaDmgSlash() const
	{
		return gargantuaDmgSlash->value;
	}

	float GetGargantuaDmgFire() const
	{
		return gargantuaDmgFire->value;
	}

	float GetGargantuaDmgStomp() const
	{
		return gargantuaDmgStomp->value;
	}

	float GetHAssassinHealth() const
	{
		return hassassinHealth->value;
	}

	float GetHeadcrabHealth() const
	{
		return headcrabHealth->value;
	}

	float GetHeadcrabDmgBite() const
	{
		return headcrabDmgBite->value;
	}

	float GetHGruntHealth() const
	{
		return hgruntHealth->value;
	}

	float GetHGruntDmgKick() const
	{
		return hgruntDmgKick->value;
	}

	float GetHGruntShotgunPellets() const
	{
		return hgruntShotgunPellets->value;
	}

	float GetHGruntGrenadeSpeed() const
	{
		return hgruntGrenadeSpeed->value;
	}

	float GetHoundeyeHealth() const
	{
		return houndeyeHealth->value;
	}

	float GetHoundeyeDmgBlast() const
	{
		return houndeyeDmgBlast->value;
	}

	float GetSlaveHealth() const
	{
		return slaveHealth->value;
	}

	float GetSlaveDmgClaw() const
	{
		return slaveDmgClaw->value;
	}

	float GetSlaveDmgClawrake() const
	{
		return slaveDmgClawrake->value;
	}

	float GetSlaveDmgZap() const
	{
		return slaveDmgZap->value;
	}

	float GetIchthyosaurHealth() const
	{
		return ichthyosaurHealth->value;
	}

	float GetIchthyosaurDmgShake() const
	{
		return ichthyosaurDmgShake->value;
	}

	float GetLeechHealth() const
	{
		return leechHealth->value;
	}

	float GetLeechDmgBite() const
	{
		return leechDmgBite->value;
	}

	float GetControllerHealth() const
	{
		return controllerHealth->value;
	}

	float GetControllerDmgZap() const
	{
		return controllerDmgZap->value;
	}

	float GetControllerSpeedBall() const
	{
		return controllerSpeedBall->value;
	}

	float GetControllerDmgBall() const
	{
		return controllerDmgBall->value;
	}

	float GetNihilanthHealth() const
	{
		return nihilanthHealth->value;
	}

	float GetNihilanthZap() const
	{
		return nihilanthZap->value;
	}

	float GetScientistHealth() const
	{
		return scientistHealth->value;
	}

	float GetSnarkHealth() const
	{
		return snarkHealth->value;
	}

	float GetSnarkDmgBite() const
	{
		return snarkDmgBite->value;
	}

	float GetSnarkDmgPop() const
	{
		return snarkDmgPop->value;
	}

	float GetZombieHealth() const
	{
		return zombieHealth->value;
	}

	float GetZombieDmgOneSlash() const
	{
		return zombieDmgOneSlash->value;
	}

	float GetZombieDmgBothSlash() const
	{
		return zombieDmgBothSlash->value;
	}

	float GetTurretHealth() const
	{
		return turretHealth->value;
	}

	float GetMiniTurretHealth() const
	{
		return miniturretHealth->value;
	}

	float GetSentryHealth() const
	{
		return sentryHealth->value;
	}

// Player Weapons
	float GetPlrDmgCrowbar() const
	{
		return plrDmgCrowbar->value;
	}

	float GetDmg9MM() const
	{
		return plrDmg9MM->value;
	}

	float GetPlrDmg357() const
	{
		return plrDmg357->value;
	}

	float GetPlrDmgMP5() const
	{
		return plrDmgMP5->value;
	}

	float GetPlrDmgM203Grenade() const
	{
		return plrDmgM203Grenade->value;
	}

	float GetPlrDmgBuckshot() const
	{
		return plrDmgBuckshot->value;
	}

	float GetPlrDmgCrossbowClient() const
	{
		return plrDmgCrossbowClient->value;
	}

	float GetPlrDmgCrossbowMonster() const
	{
		return plrDmgCrossbowMonster->value;
	}

	float GetPlrDmgRPG() const
	{
		return plrDmgRPG->value;
	}

	float GetPlrDmgGauss() const
	{
		return plrDmgGauss->value;
	}

	float GetPlrDmgEgonNarrow() const
	{
		return plrDmgEgonNarrow->value;
	}

	float GetPlrDmgEgonWide() const
	{
		return plrDmgEgonWide->value;
	}

	float GetPlrDmgHornet() const
	{
		return plrDmgHornet->value;
	}

	float GetPlrDmgHandGrenade() const
	{
		return plrDmgHandGrenade->value;
	}

	float GetPlrDmgSatchel() const
	{
		return plrDmgSatchel->value;
	}

	float GetPlrDmgTripmine() const
	{
		return plrDmgTripmine->value;
	}

#ifdef GAME_OPFOR
	float GetPlrDmgKnife() const
	{
		return plrDmgKnife->value;
	}

	float GetPlrDmgPipewrench() const
	{
		return plrDmgPipewrench->value;
	}

	float GetPlrDmgGrapple() const
	{
		return plrDmgGrapple->value;
	}

	float GetPlrDmg556() const
	{
		return plrDmg556->value;
	}

	float GetPlrDmg762() const
	{
		return plrDmg762->value;
	}

	float GetPlrDmgDeagle() const
	{
		return plrDmgDeagle->value;
	}

	/**
	*	Singleplayer damage value.
	*/
	float GetPlrDmgShockRoachS() const
	{
		return plrDmgShockRoachS->value;
	}

	/**
	*	Multiplayer damage value.
	*/
	float GetPlrDmgShockRoachM() const
	{
		return plrDmgShockRoachM->value;
	}

	float GetPlrDmgDisplacerOther() const
	{
		return plrDmgDisplacerOther->value;
	}

	float GetPlrRadiusDisplacer() const
	{
		return plrRadiusDisplacer->value;
	}

	float GetPlrDmgSpore() const
	{
		return plrDmgSpore->value;
	}
#endif
	
// weapons shared by monsters
	float GetMonDmg9MM() const
	{
		return monDmg9MM->value;
	}

	float GetMonDmgMP5() const
	{
		return monDmgMP5->value;
	}

	float GetMonDmg12MM() const
	{
		return monDmg12MM->value;
	}

	float GetMonDmgHornet() const
	{
		return monDmgHornet->value;
	}

// health/suit charge
	float GetSuitChargerCapacity() const
	{
		return suitchargerCapacity->value;
	}

	float GetBatteryCapacity() const
	{
		return batteryCapacity->value;
	}

	float GetHealthChargerCapacity() const
	{
		return healthchargerCapacity->value;
	}

	float GetHealthKitCapacity() const
	{
		return healthkitCapacity->value;
	}

	float GetScientistHeal() const
	{
		return scientistHeal->value;
	}

// monster damage adj
	float GetMonHead() const
	{
		return monHead->value;
	}

	float GetMonChest() const
	{
		return monChest->value;
	}

	float GetMonStomach() const
	{
		return monStomach->value;
	}

	float GetMonLeg() const
	{
		return monLeg->value;
	}

	float GetMonArm() const
	{
		return monArm->value;
	}

// player damage adj
	float GetPlrHead() const
	{
		return plrHead->value;
	}

	float GetPlrChest() const
	{
		return plrChest->value;
	}

	float GetPlrStomach() const
	{
		return plrStomach->value;
	}

	float GetPlrLeg() const
	{
		return plrLeg->value;
	}

	float GetPlrArm() const
	{
		return plrArm->value;
	}
};

extern DLL_GLOBAL skilldata_t gSkillData;

#endif //GAME_SERVER_SKILL_H