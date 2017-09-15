/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#ifndef GAME_SERVER_ENTITIES_NPCS_CBARNEY_H
#define GAME_SERVER_ENTITIES_NPCS_CBARNEY_H

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
#define		BARNEY_AE_DRAW		( 2 )
#define		BARNEY_AE_SHOOT		( 3 )
#define		BARNEY_AE_HOLSTER	( 4 )

#define	BARNEY_BODY_GUNHOLSTERED	0
#define	BARNEY_BODY_GUNDRAWN		1
#define BARNEY_BODY_GUNGONE			2

class CBarney : public CTalkMonster
{
public:
	DECLARE_CLASS( CBarney, CTalkMonster );
	DECLARE_DATADESC();

	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int ISoundMask() override;
	void BarneyFirePistol();
	void AlertSound() override;
	EntityClassification_t GetClassification() override;
	void HandleAnimEvent( AnimEvent_t& event ) override;

	void RunTask( const Task_t* pTask ) override;
	void StartTask( const Task_t* pTask ) override;
	int	ObjectCaps() const override { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	void OnTakeDamage( const CTakeDamageInfo& info ) override;
	bool CheckRangeAttack1( float flDot, float flDist ) override;

	void DeclineFollowing() override;

	// Override these to set behavior
	Schedule_t *GetScheduleOfType( int Type ) override;
	Schedule_t *GetSchedule() override;
	MONSTERSTATE GetIdealState() override;

	void DeathSound() override;
	void PainSound() override;

	void TalkInit();

	void TraceAttack( const CTakeDamageInfo& info, Vector vecDir, TraceResult *ptr ) override;
	void Killed( const CTakeDamageInfo& info, GibAction gibAction ) override;

	bool	m_fGunDrawn;
	float	m_painTime;
	float	m_checkAttackTime;
	bool	m_lastAttackCheck;

	// UNDONE: What is this for?  It isn't used?
	float	m_flPlayerDamage;// how much pain has the player inflicted on me?

	DECLARE_SCHEDULES() override;
};

#endif //GAME_SERVER_ENTITIES_NPCS_CBARNEY_H