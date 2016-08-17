add_sources(
	animation.h
	animation.cpp
	ButtonSounds.h
	ButtonSounds.cpp
	CGlobalState.h
	CGlobalState.cpp
	client.h
	client.cpp
	CMap.h
	CMap.cpp
	CMultiDamage.h
	CMultiDamage.cpp
	CServerGameInterface.h
	CServerGameInterface.cpp
	CServerGameLib.h
	CServerGameLib.cpp
	Decals.h
	Decals.cpp
	Effects.h
	globals.cpp
	SaveRestore.h
	Server.h
	Server.cpp
	ServerEngineOverride.h
	ServerEngineOverride.cpp
	ServerInterface.h
	ServerInterface.cpp
	Skill.h
	Skill.cpp
	SVC.h
	TempEntity.h
	TempEntity.cpp
	UserMessages.h
	UserMessages.cpp
	util.cpp
	voice_gamemgr.h
	voice_gamemgr.cpp
	Weapons.cpp
)

if( USE_ANGELSCRIPT )
	add_subdirectory( Angelscript )
endif()
add_subdirectory( entities )
add_subdirectory( gamerules )
add_subdirectory( nodes )
add_subdirectory( saverestore )
add_subdirectory( sound )