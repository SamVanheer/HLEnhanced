add_sources(
	cbase.h
	CBaseGameInterface.h
	CBaseGameInterface.cpp
	cdll_dll.h
	CReplacementCache.h
	CReplacementCache.cpp
	CReplacementMap.h
	CReplacementMap.cpp
	CTakeDamageInfo.h
	CWeaponInfo.h
	CWeaponInfo.cpp
	CWeaponInfoCache.h
	CWeaponInfoCache.cpp
	Damage.h
	DefaultClassifications.cpp
	DefaultClassifications.h
	DelayLoad.cpp
	DelayLoad.h
	enginecallback.h
	EntityClasses.cpp
	EntityClasses.h
	extdll.h
	GameConstants.h
	HudColors.h
	Relationship.cpp
	Relationship.h
	ScriptEvent.h
	shared_game_utils.h
	shared_game_utils.cpp
	util.h
	voice_common.h
	Weapons.h
	Weapons_Shared.cpp
	WeaponsConst.h
	WeaponsConst.cpp
	Weather.h
)

if( USE_ANGELSCRIPT )
	add_subdirectory( Angelscript )
endif()
add_subdirectory( entities )
add_subdirectory( materials )
add_subdirectory( studio )