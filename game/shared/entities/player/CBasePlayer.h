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
#ifndef PLAYER_H
#define PLAYER_H

#include "Color.h"
#include "HudColors.h"

#include "materials/MaterialsConst.h"

#include "entities/weapons/CAmmoTypes.h"

struct WeaponHUDSprite;

#ifdef GAME_OPFOR
class CRope;
#endif

/**
*	Player PHYSICS FLAGS bits
*/
enum PhysicsFlag
{
	PFLAG_ONLADDER		= 1 << 0,
	PFLAG_ONSWING		= 1 << 0,
	PFLAG_ONTRAIN		= 1 << 1,
	PFLAG_ONBARNACLE	= 1 << 2,

	/**
	*	In the process of ducking, but totally squatted yet.
	*/
	PFLAG_DUCKING		= 1 << 3,

	/**
	*	Using a continuous entity.
	*/
	PFLAG_USING			= 1 << 4,

	/**
	*	Player is locked in stationary cam mode. Spectators can move, observers can't.
	*/
	PFLAG_OBSERVER		= 1 << 5,

	/**
	*	On a rope.
	*/
	PFLAG_ONROPE		= 1 << 6,
};

enum TrainFlag
{
	TRAIN_ACTIVE	= 0x80,
	TRAIN_NEW		= 0xc0,
	TRAIN_OFF		= 0x00,
	TRAIN_NEUTRAL	= 0x01,
	TRAIN_SLOW		= 0x02,
	TRAIN_MEDIUM	= 0x03,
	TRAIN_FAST		= 0x04,
	TRAIN_BACK		= 0x05,
};

//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------
#define CSUITPLAYLIST	4		// max of 4 suit sentences queued up at any time

enum SuitUpdateType
{
	SUIT_GROUP		= 1,
	SUIT_SENTENCE	= 0
};

enum
{
	SUIT_REPEAT_OK		= 0,
	SUIT_NEXT_IN_30SEC	= 30,
	SUIT_NEXT_IN_1MIN	= 60,
	SUIT_NEXT_IN_5MIN	= 300,
	SUIT_NEXT_IN_10MIN	= 600,
	SUIT_NEXT_IN_30MIN	= 1800,
	SUIT_NEXT_IN_1HOUR	= 3600,
};

#define CSUITNOREPEAT		32

#define TEAM_NAME_LENGTH	16

enum PLAYER_ANIM
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
#ifdef GAME_OPFOR
	PLAYER_GRAPPLE,
#endif
};

#define MAX_ID_RANGE 2048
#define SBAR_STRING_SIZE 128

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END,
};

#define CHAT_INTERVAL 1.0f

/**
*	How many seconds the player can stay connected without having sent weapon validation. - Solokiller
*/
#define WEAPON_VALIDATION_GRACE_TIME 10

int TrainSpeed( int iSpeed, int iMax );

class CBasePlayer : public CBaseMonster
{
public:
	DECLARE_CLASS( CBasePlayer, CBaseMonster );
	DECLARE_DATADESC();

	CBasePlayer()
	{
	}

private:
	//Have we sent init data to the client? - Solokiller
	bool m_bSentInitData = false;
	
public:
	// Spectator camera
	void	Observer_FindNextPlayer( bool bReverse );
	void	Observer_HandleButtons();
	void	Observer_SetMode( int iMode );
	void	Observer_CheckTarget();
	void	Observer_CheckProperties();
	EHANDLE	m_hObserverTarget;
	float	m_flNextObserverInput;
	int		m_iObserverWeapon;	// weapon of current tracked target
	int		m_iObserverLastMode;// last used observer mode

	bool	IsObserver() const { return pev->iuser1 != 0; }

	int GetObserverMode() const { return pev->iuser1; }

private:
	void InternalSetObserverMode( const int iMode )
	{
		pev->iuser1 = iMode;
	}

public:

	int GetObserverTargetIndex()
	{
		return pev->iuser2;
	}

	void SetObserverTargetIndex( const int iIndex )
	{
		pev->iuser2 = iIndex;
	}

	void StartDeathCam();
	void StartObserver( Vector vecPosition, Vector vecViewAngle );

	int					random_seed;    // See that is shared between client & server for shared weapons code

	int					m_iPlayerSound;// the index of the sound list slot reserved for this player
	int					m_iTargetVolume;// ideal sound volume. 
	int					m_iWeaponVolume;// how loud the player's weapon is right now.
	int					m_iExtraSoundTypes;// additional classification for this weapon's sound
	int					m_iWeaponFlash;// brightness of the weapon flash
	float				m_flStopExtraSoundTime;
	
	float				m_flFlashLightTime;	// Time until next battery draw/Recharge
	int					m_iFlashBattery;		// Flashlight Battery Draw

	int					m_afButtonLast;
	int					m_afButtonPressed;
	int					m_afButtonReleased;
	
	EHANDLE				m_SndLast;				// last sound entity to modify player room type
	float				m_flSndRoomtype;		// last roomtype set by sound entity
	float				m_flSndRange;			// dist from player to sound entity

	float				m_flFallVelocity;
	
	int					m_rgItems[MAX_ITEMS];

	unsigned int		m_afPhysicsFlags;	// physics flags - set when 'normal' physics should be revisited or overriden
	float				m_fNextSuicideTime; // the time after which the player can next use the suicide command


// these are time-sensitive things that we keep track of
	float				m_flTimeStepSound;	// when the last stepping sound was made
	float				m_flTimeWeaponIdle; // when to play another weapon idle animation.
	float				m_flSwimTime;		// how long player has been underwater
	float				m_flDuckTime;		// how long we've been ducking
	float				m_flWallJumpTime;	// how long until next walljump

	float				m_flSuitUpdate;					// when to play next suit update
	int					m_rgSuitPlayList[CSUITPLAYLIST];// next sentencenum to play for suit update
	int					m_iSuitPlayNext;				// next sentence slot for queue storage;
	int					m_rgiSuitNoRepeat[CSUITNOREPEAT];		// suit sentence no repeat list
	float				m_rgflSuitNoRepeatTime[CSUITNOREPEAT];	// how long to wait before allowing repeat
	int					m_lastDamageAmount;		// Last damage taken
	float				m_tbdPrev;				// Time-based damage timer

	float				m_flgeigerRange;		// range to nearest radiation source
	float				m_flgeigerDelay;		// delay per update of range msg to client
	int					m_igeigerRangePrev;
	int					m_iStepLeft;			// alternate left/right foot stepping sound
	char				m_szTextureName[CBTEXTURENAMEMAX];	// current texture name we're standing on
	char				m_chTextureType;		// current texture type

	int					m_idrowndmg;			// track drowning damage taken
	int					m_idrownrestored;		// track drowning damage restored

	int					m_bitsHUDDamage;		// Damage bits for the current fame. These get sent to 
												// the hude via the DAMAGE message
	bool				m_fInitHUD;				// True when deferred HUD restart msg needs to be sent
	bool				m_fGameHUDInitialized;
	int					m_iTrain;				// Train control position
	bool				m_fWeapon;				// Set this to false to force a reset of the current weapon HUD info

	EHANDLE				m_pTank;				// the tank which the player is currently controlling,  NULL if no tank
	float				m_fDeadTime;			// the time at which the player died  (used in PlayerDeathThink())

	bool			m_fNoPlayerSound;	// a debugging feature. Player makes no sound if this is true. 
	bool			m_fLongJump; // does this player have the longjump module?

	int			m_iUpdateTime;		// stores the number of frame ticks before sending HUD update messages
	int			m_iClientHealth;	// the health currently known by the client.  If this changes, send a new
	int			m_iClientBattery;	// the Battery currently known by the client.  If this changes, send a new
	int			m_iHideHUD;		// the players hud weapon info is to be hidden
	int			m_iClientHideHUD;
	int			m_iFOV;			// field of view
	int			m_iClientFOV;	// client's known FOV

	//TODO: correct these names - Solokiller
	// usable player items 
	CBasePlayerWeapon* m_rgpPlayerItems[ MAX_WEAPON_SLOTS ];
	CBasePlayerWeapon* m_pActiveItem;
	CBasePlayerWeapon* m_pClientActiveItem;  // client version of the active item
	CBasePlayerWeapon* m_pLastItem;
	// shared ammo slots
	int	m_rgAmmo[ CAmmoTypes::MAX_AMMO_TYPES ];

	/**
	*	On the client, this is the latest ammo value for every type. Not the predicted value. - Solokiller
	*/
	int	m_rgAmmoLast[ CAmmoTypes::MAX_AMMO_TYPES ];

	Vector				m_vecAutoAim;
	bool				m_fOnTarget;
	int					m_iDeaths;
	float				m_iRespawnFrames;	// used in PlayerDeathThink() to make sure players can always respawn

	int m_lastx, m_lasty;  // These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames;// Custom clan logo frames for this player
	float	m_flNextDecalTime;// next time this player can spray a decal

	char m_szTeamName[TEAM_NAME_LENGTH];

	/**
	*	If i'm currently looking through a camera, this is the camera. - Solokiller
	*	Not save/restored because the engine doesn't save off the view entity variable.
	*/
	EHANDLE m_hCamera;

public:
	//CBaseEntity overrides

	// Player is moved across the transition by other means
	int ObjectCaps() const override { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	EntityClassification_t GetClassification() override;

	void Precache() override;

	void InitialSpawn();

	void Spawn() override;
	
	Vector GetGunPosition() override;

	void TraceAttack( const CTakeDamageInfo& info, Vector vecDir, TraceResult& tr ) override;

	void OnTakeDamage( const CTakeDamageInfo& info ) override;

	void Killed( const CTakeDamageInfo& info, GibAction gibAction ) override;

	Vector BodyTarget( const Vector &posSrc ) const override { return Center() + GetViewOffset() * RANDOM_FLOAT( 0.5, 1.1 ); }
	bool IsAlive() const override { return ( GetDeadFlag() == DEAD_NO) && GetHealth() > 0; }
	bool ShouldFadeOnDeath() const override { return false; }

	/**
	*	Spectators should return false for this, they aren't "players" as far as game logic is concerned.
	*/
	bool IsPlayer() const override { return true; }	

	/**
	*	Bots should return false for this, they can't receive NET messages.
	*	Spectators should return true for this.
	*/
	bool IsNetClient() const override { return true; }

	const char *TeamID() const override;

	bool Restore( CRestore &restore ) override;

	bool BarnacleVictimGrabbed( CBaseEntity* pBarnacle ) override;
	void BarnacleVictimBitten( CBaseEntity* pBarnacle ) override;
	void BarnacleVictimReleased() override;

	int Illumination() const override;

	//Game logic

	void PlayerUse();

	virtual void Jump();
	virtual void Duck();

	void Pain();

	void DeathSound() override;

	void AddPoints( int score, const bool bAllowNegativeScore );
	void AddPointsToTeam( int score, const bool bAllowNegativeScore );
	void EnableControl( const bool fControl );

	void SetAnimation( PLAYER_ANIM playerAnim );

	/**
	*	@return true if the player is attached to a ladder.
	*/
	bool IsOnLadder() const;
	bool FlashlightIsOn() const;
	void FlashlightTurnOn();
	void FlashlightTurnOff();

	/**
	*	Plays a vox sentence for this player.
	*	Taken from TFC.
	*	@param pszSentence to play.
	*/
	void ClientHearVox( const char* const pszSentence );

	//Weapons

	void RenewItems();
	void PackDeadPlayerItems();
	void RemoveAllItems( const bool removeSuit );
	bool SwitchWeapon( CBasePlayerWeapon *pWeapon );

	/**
	*	@return The weapon animation type.
	*/
	const char* GetWeaponAnimType() const { return m_szAnimExtension; }

	/**
	*	Sets the weapon animation type.
	*	This sets the extension to use when looking up player animations.
	*	@param pszExtension Extension to use.
	*/
	void SetWeaponAnimType( const char* const pszExtension );

private:
	char m_szAnimExtension[ 32 ];

public:

	bool AddPlayerItem( CBasePlayerWeapon *pItem );
	bool RemovePlayerItem( CBasePlayerWeapon *pItem );
	void DropPlayerItem( char *pszItemName );
	bool HasPlayerItem( CBasePlayerWeapon *pCheckItem ) const;
	bool HasNamedPlayerItem( const char *pszItemName ) const;
	bool HasWeapons() const;// do I have ANY weapons?

	/**
	*	Gets a weapon by bucket and position.
	*/
	CBasePlayerWeapon* GetWeapon( int iBucket, int iPosition );

	/**
	*	Counts the amount of ammo that the player has of the given ammo type.
	*	Accesses the latest data on the client, not the currently predicted value. - Solokiller
	*/
	int CountAmmo( const int iID ) const;

	/**
	*	@return Whether the player has any ammo for the given weapon.
	*/
	bool HasAmmo( CBasePlayerWeapon* pWeapon ) const;

	/**
	*	Gets the first weapon in a given bucket, searching by position set in the weapon info file.
	*/
	CBasePlayerWeapon* GetFirstPos( int iBucket );

	/**
	*	Gets the next weapon in a given bucket, starting at a previous weapon's position.
	*/
	CBasePlayerWeapon* GetNextActivePos( int iBucket, int iPosition );

#ifdef CLIENT_DLL
	/**
	*	Helper function to return an Ammo pointer from id.
	*/
	static const WeaponHUDSprite* GetAmmoPicFromWeapon( AmmoID_t iAmmoId );
#endif

	void SelectPrevItem( int iItem );
	void SelectNextItem( int iItem );
	void SelectLastItem();
	void SelectItem( const char *pstr );
	void ItemPreFrame();
	void ItemPostFrame();
	void GiveNamedItem( const char *szName );

	int GiveAmmo( int iAmount, const char *szName );
	void SendAmmoUpdate();

	static int GetAmmoIndex( const char *psz );

	/**
	*	Gets the amount of ammo of a given type that the player has.
	*	@param pszName Name of the ammo type.
	*/
	int GetAmmoCount( const char* const pszName ) const;

	/**
	*	Gets the amount of ammo of a given type that the player has.
	*	@param ammoID ID of the ammo type.
	*/
	int GetAmmoCountByID( const AmmoID_t ammoID ) const;

	/**
	*	Sets the amount of ammo of a given type the player has.
	*	@param pszName Name of the ammo type.
	*	@param iCount Amount of ammo the player will have.
	*/
	void SetAmmoCount( const char* const pszName, const int iCount );

	/**
	*	Sets the amount of ammo of a given type the player has.
	*	@param ammoID ID of the ammo type.
	*	@param iCount Amount of ammo the player will have.
	*/
	void SetAmmoCountByID( const AmmoID_t ammoID, const int iCount );

	void ResetAutoaim();
	Vector GetAutoaimVector( float flDelta );
	Vector GetAutoaimVectorFromPoint( const Vector& vecSrc, float flDelta );
	Vector AutoaimDeflection( const Vector &vecSrc, float flDist, float flDelta );

	//Networking

	enum class ConnectState
	{
		NOT_CONNECTED = 0,
		CONNECTED,
		DISCONNECTING
	};

private:
	ConnectState m_ConnectState = ConnectState::NOT_CONNECTED;

	float m_flConnectTime = 0;
	float m_flDisconnectTime = 0;

	//In singleplayer, restore will get the wrong time value since that's restored too late.
	//This tells the game to reset the value later on. - Solokiller
	bool m_bNeedsNewConnectTime = false;
	bool m_bWeaponValidationReceived = false;

public:

	/**
	*	@return The connection state.
	*/
	ConnectState GetConnectState() const { return m_ConnectState; }

	/**
	*	Sets the connection state.
	*/
	void SetConnectState( const ConnectState state )
	{
		m_ConnectState = state;
	}

	/**
	*	@return Whether the client is connected.
	*/
	bool IsConnected() const { return m_ConnectState == ConnectState::CONNECTED; }

	/**
	*	@return The time at which the client connected.
	*/
	float GetConnectTime() const { return m_flConnectTime; }

	/**
	*	Sets the time at which the client connected.
	*/
	void SetConnectTime( const float flConnectTime )
	{
		m_flConnectTime = flConnectTime;
	}

	/**
	*	@return The time at which the client disconnected.
	*/
	float GetDisconnectTime() const { return m_flDisconnectTime; }

	/**
	*	Sets the time at which the client connected.
	*/
	void SetDisconnectTime( const float flDisconnectTime )
	{
		m_flDisconnectTime = flDisconnectTime;
	}

	/**
	*	@return Whether the weapon & ammo validation message was received.
	*/
	bool WeaponValidationReceived() const { return m_bWeaponValidationReceived; }

	void SetWeaponValidationReceived( const bool bWeaponValidationReceived )
	{
		m_bWeaponValidationReceived = bWeaponValidationReceived;
	}

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void UpdateClientData();

	void ForceClientDllUpdate();  // Forces all client .dll specific data to be resent to client.

	/**
	*	Sends weather data to the client.
	*/
	void SendWeatherUpdate();

	//Frame logic

	virtual void PreThink();
	virtual void PostThink();

	//virtual void Think();

	void PlayerDeathThink();

	void UpdatePlayerSound();

	void WaterMove();

	virtual void ImpulseCommands();
	void CheatImpulseCommands( int iImpulse );

	void CheckSuitUpdate();
	void SetSuitUpdate( const char* const pszName, const SuitUpdateType updateType, int iNoRepeat );
	void UpdateGeigerCounter();

	void CheckTimeBasedDamage();

	//Misc

	void DeathMessage( CBaseEntity* pKiller );

	void SetCustomDecalFrames( int nFrames );
	int GetCustomDecalFrames();

	bool IsOnRope() const { return ( m_afPhysicsFlags & PFLAG_ONROPE ) != 0; }

	void SetOnRopeState( bool bOnRope )
	{
		if( bOnRope )
			m_afPhysicsFlags |= PFLAG_ONROPE;
		else
			m_afPhysicsFlags &= ~PFLAG_ONROPE;
	}

#ifdef GAME_OPFOR
	CRope* GetRope() { return m_pRope; }

	void SetRope( CRope* pRope )
	{
		m_pRope = pRope;
	}

	void SetIsClimbing( const bool bIsClimbing )
	{
		m_bIsClimbing = bIsClimbing;
	}

	const Vector& GetDisplacerReturn() const { return m_vecDisplacerReturn; }

	void SetDisplacerReturn( const Vector& vecDisplacerReturn )
	{
		m_vecDisplacerReturn = vecDisplacerReturn;
	}

	float GetDisplacerSndRoomtype() const { return m_flDisplacerSndRoomtype; }

	void SetDisplacerSndRoomtype( float flDisplacerSndRoomtype )
	{
		m_flDisplacerSndRoomtype = flDisplacerSndRoomtype;
	}

private:
	CRope* m_pRope;

	Vector m_vecDisplacerReturn;

	float m_flDisplacerSndRoomtype;
#else
private:
#endif
	
	float m_flLastClimbTime = 0;
	bool m_bIsClimbing = false;

public:

	//TODO: still used? - Solokiller
	float m_flStartCharge;
	float m_flAmmoStartCharge;
	float m_flPlayAftershock;
	float m_flNextAmmoBurn;// while charging, when to absorb another unit of player's ammo?
	
	//Player ID
	void InitStatusBar();
	void UpdateStatusBar();
	int m_izSBarState[ SBAR_END ];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[ SBAR_STRING_SIZE ];
	char m_SbarString1[ SBAR_STRING_SIZE ];
	
	float m_flNextChatTime;

	/**
	*	@return The primary HUD color.
	*/
	const Color& GetPrimaryHudColor() const { return m_HudColors.m_PrimaryColor; }

	/**
	*	Sets the primary HUD color.
	*	@param color Color to set.
	*/
	void SetPrimaryHudColor( const Color& color )
	{
		m_HudColors.m_PrimaryColor = color;

		m_bUseCustomHudColors = true;

		m_flLastHudColorChangeTime = gpGlobals->time;
	}

	/**
	*	@return The empty / nearly empty HUD color.
	*/
	const Color& GetEmptyItemHudColor() const { return m_HudColors.m_EmptyItemColor; }

	/**
	*	Sets the empty / nearly empty HUD color.
	*	@param color Color to set.
	*/
	void SetEmptyItemHudColor( const Color& color )
	{
		m_HudColors.m_EmptyItemColor = color;

		m_bUseCustomHudColors = true;

		m_flLastHudColorChangeTime = gpGlobals->time;
	}

	/**
	*	@return The ammo bar HUD color.
	*/
	const Color& GetAmmoBarHudColor() const { return m_HudColors.m_AmmoBarColor; }

	/**
	*	Sets the ammo bar HUD color.
	*	@param color Color to set.
	*/
	void SetAmmoBarHudColor( const Color& color )
	{
		m_HudColors.m_AmmoBarColor = color;

		m_bUseCustomHudColors = true;

		m_flLastHudColorChangeTime = gpGlobals->time;
	}

	/**
	*	Sets all hud colors.
	*/
	void SetHudColors( const CHudColors& colors )
	{
		m_HudColors = colors;

		m_bUseCustomHudColors = true;

		m_flLastHudColorChangeTime = gpGlobals->time;
	}

	/**
	*	Resets Hud colors to their defaults.
	*/
	void ResetHudColors()
	{
		m_bUseCustomHudColors = false;
		//Tells CMap to resend Hud colors using its settings.
		m_flLastHudColorChangeTime = gpGlobals->time;

		m_HudColors.Reset();
	}

	bool UsesCustomHudColors() const { return m_bUseCustomHudColors; }

	float GetLastHudColorChangeTime() const { return m_flLastHudColorChangeTime; }

	const CHudColors& GetHudColors() const { return m_HudColors; }

private:
	bool m_bUseCustomHudColors = false;

	float m_flLastHudColorChangeTime = 0;

	CHudColors m_HudColors;
};

extern int	gmsgHudText;
extern bool gInitHUD;

#endif // PLAYER_H
