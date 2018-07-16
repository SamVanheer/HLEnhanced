#include <type_traits>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "WeaponsConst.h"

#include "../ASCEntBitSet.h"

#include "ASCBaseEntity.h"

/**
*	Registers FixAngleMode.
*	@param engine Script engine.
*/
static void RegisterScriptFixAngleMode( asIScriptEngine& engine )
{
	const char* const pszObjectName = "FixAngleMode";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "FIXANGLE_NO", FIXANGLE_NO );
	engine.RegisterEnumValue( pszObjectName, "FIXANGLE_SET", FIXANGLE_SET );
	engine.RegisterEnumValue( pszObjectName, "FIXANGLE_ADD_AVEL", FIXANGLE_ADD_AVEL );
}

/**
*	Registers MoveType.
*	@param engine Script engine.
*/
static void RegisterScriptMoveType( asIScriptEngine& engine )
{
	const char* const pszObjectName = "MoveType";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_NONE", MOVETYPE_NONE );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_WALK", MOVETYPE_WALK );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_STEP", MOVETYPE_STEP );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_FLY", MOVETYPE_FLY );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_TOSS", MOVETYPE_TOSS );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_PUSH", MOVETYPE_PUSH );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_NOCLIP", MOVETYPE_NOCLIP );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_FLYMISSILE", MOVETYPE_FLYMISSILE );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_BOUNCE", MOVETYPE_BOUNCE );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_BOUNCEMISSILE", MOVETYPE_BOUNCEMISSILE );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_FOLLOW", MOVETYPE_FOLLOW );
	engine.RegisterEnumValue( pszObjectName, "MOVETYPE_PUSHSTEP", MOVETYPE_PUSHSTEP );
}

/**
*	Registers Solid.
*	@param engine Script engine.
*/
static void RegisterScriptSolid( asIScriptEngine& engine )
{
	const char* const pszObjectName = "Solid";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "SOLID_NOT", SOLID_NOT );
	engine.RegisterEnumValue( pszObjectName, "SOLID_TRIGGER", SOLID_TRIGGER );
	engine.RegisterEnumValue( pszObjectName, "SOLID_BBOX", SOLID_BBOX );
	engine.RegisterEnumValue( pszObjectName, "SOLID_SLIDEBOX", SOLID_SLIDEBOX );
	engine.RegisterEnumValue( pszObjectName, "SOLID_BSP", SOLID_BSP );
}

/**
*	Registers RenderMode.
*	@param engine Script engine.
*/
static void RegisterScriptRenderMode( asIScriptEngine& engine )
{
	const char* const pszObjectName = "RenderMode";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "kRenderNormal", kRenderNormal );
	engine.RegisterEnumValue( pszObjectName, "kRenderTransColor", kRenderTransColor );
	engine.RegisterEnumValue( pszObjectName, "kRenderTransTexture", kRenderTransTexture );
	engine.RegisterEnumValue( pszObjectName, "kRenderGlow", kRenderGlow );
	engine.RegisterEnumValue( pszObjectName, "kRenderTransAlpha", kRenderTransAlpha );
	engine.RegisterEnumValue( pszObjectName, "kRenderTransAdd", kRenderTransAdd );
}

/**
*	Registers RenderFX.
*	@param engine Script engine.
*/
static void RegisterScriptRenderFX( asIScriptEngine& engine )
{
	const char* const pszObjectName = "RenderFX";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "kRenderFxNone", kRenderFxNone );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxPulseSlow", kRenderFxPulseSlow );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxPulseFast", kRenderFxPulseFast );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxPulseSlowWide", kRenderFxPulseSlowWide );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxPulseFastWide", kRenderFxPulseFastWide );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxFadeSlow", kRenderFxFadeSlow );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxFadeFast", kRenderFxFadeFast );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxSolidSlow", kRenderFxSolidSlow );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxSolidFast", kRenderFxSolidFast );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxStrobeSlow", kRenderFxStrobeSlow );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxStrobeFast", kRenderFxStrobeFast );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxStrobeFaster", kRenderFxStrobeFaster );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxFlickerSlow", kRenderFxFlickerSlow );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxFlickerFast", kRenderFxFlickerFast );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxNoDissipation", kRenderFxNoDissipation );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxDistort", kRenderFxDistort );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxHologram", kRenderFxHologram );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxDeadPlayer", kRenderFxDeadPlayer );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxExplode", kRenderFxExplode );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxGlowShell", kRenderFxGlowShell );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxClampMinScale", kRenderFxClampMinScale );
	engine.RegisterEnumValue( pszObjectName, "kRenderFxLightMultiplier", kRenderFxLightMultiplier );
}

/**
*	Registers TakeDamageMode.
*	@param engine Script engine.
*/
static void RegisterScriptTakeDamageMode( asIScriptEngine& engine )
{
	const char* const pszObjectName = "TakeDamageMode";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "DAMAGE_NO", DAMAGE_NO );
	engine.RegisterEnumValue( pszObjectName, "DAMAGE_YES", DAMAGE_YES );
	engine.RegisterEnumValue( pszObjectName, "DAMAGE_AIM", DAMAGE_AIM );
}

/**
*	Registers DeadFlag.
*	@param engine Script engine.
*/
static void RegisterScriptDeadFlag( asIScriptEngine& engine )
{
	const char* const pszObjectName = "DeadFlag";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "DEAD_NO", DEAD_NO );
	engine.RegisterEnumValue( pszObjectName, "DEAD_DYING", DEAD_DYING );
	engine.RegisterEnumValue( pszObjectName, "DEAD_DEAD", DEAD_DEAD );
	engine.RegisterEnumValue( pszObjectName, "DEAD_RESPAWNABLE", DEAD_RESPAWNABLE );
	engine.RegisterEnumValue( pszObjectName, "DEAD_DISCARDBODY", DEAD_DISCARDBODY );
}

/**
*	Registers WaterLevel.
*	@param engine Script engine.
*/
static void RegisterScriptWaterLevel( asIScriptEngine& engine )
{
	const char* const pszObjectName = "WaterLevel";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "WATERLEVEL_DRY", WATERLEVEL_DRY );
	engine.RegisterEnumValue( pszObjectName, "WATERLEVEL_FEET", WATERLEVEL_FEET );
	engine.RegisterEnumValue( pszObjectName, "WATERLEVEL_WAIST", WATERLEVEL_WAIST );
	engine.RegisterEnumValue( pszObjectName, "WATERLEVEL_HEAD", WATERLEVEL_HEAD );
}

/**
*	Registers Contents.
*	@param engine Script engine.
*/
static void RegisterScriptContents( asIScriptEngine& engine )
{
	const char* const pszObjectName = "Contents";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "CONTENTS_EMPTY", CONTENTS_EMPTY );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_SOLID", CONTENTS_SOLID );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_WATER", CONTENTS_WATER );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_SLIME", CONTENTS_SLIME );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_LAVA", CONTENTS_LAVA );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_SKY", CONTENTS_SKY );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_ORIGIN", CONTENTS_ORIGIN );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CLIP", CONTENTS_CLIP );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_0", CONTENTS_CURRENT_0 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_90", CONTENTS_CURRENT_90 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_180", CONTENTS_CURRENT_180 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_270", CONTENTS_CURRENT_270 );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_UP", CONTENTS_CURRENT_UP );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_CURRENT_DOWN", CONTENTS_CURRENT_DOWN );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_TRANSLUCENT", CONTENTS_TRANSLUCENT );
	engine.RegisterEnumValue( pszObjectName, "CONTENTS_LADDER", CONTENTS_LADDER );
	engine.RegisterEnumValue( pszObjectName, "CONTENT_FLYFIELD", CONTENT_FLYFIELD );
	engine.RegisterEnumValue( pszObjectName, "CONTENT_GRAVITY_FLYFIELD", CONTENT_GRAVITY_FLYFIELD );
	engine.RegisterEnumValue( pszObjectName, "CONTENT_FOG", CONTENT_FOG );
}

/**
*	Registers entity flags.
*	@param engine Script engine.
*/
static void RegisterScriptEntFlag( asIScriptEngine& engine )
{
	const char* const pszObjectName = "EntFlag";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "FL_FLY", FL_FLY );
	engine.RegisterEnumValue( pszObjectName, "FL_SWIM", FL_SWIM );
	engine.RegisterEnumValue( pszObjectName, "FL_CONVEYOR", FL_CONVEYOR );
	engine.RegisterEnumValue( pszObjectName, "FL_CLIENT", FL_CLIENT );
	engine.RegisterEnumValue( pszObjectName, "FL_INWATER", FL_INWATER );
	engine.RegisterEnumValue( pszObjectName, "FL_MONSTER", FL_MONSTER );
	engine.RegisterEnumValue( pszObjectName, "FL_GODMODE", FL_GODMODE );
	engine.RegisterEnumValue( pszObjectName, "FL_NOTARGET", FL_NOTARGET );
	engine.RegisterEnumValue( pszObjectName, "FL_SKIPLOCALHOST", FL_SKIPLOCALHOST );
	engine.RegisterEnumValue( pszObjectName, "FL_ONGROUND", FL_ONGROUND );
	engine.RegisterEnumValue( pszObjectName, "FL_PARTIALGROUND", FL_PARTIALGROUND );
	engine.RegisterEnumValue( pszObjectName, "FL_WATERJUMP", FL_WATERJUMP );
	engine.RegisterEnumValue( pszObjectName, "FL_FROZEN", FL_FROZEN );
	engine.RegisterEnumValue( pszObjectName, "FL_FAKECLIENT", FL_FAKECLIENT );
	engine.RegisterEnumValue( pszObjectName, "FL_DUCKING", FL_DUCKING );
	engine.RegisterEnumValue( pszObjectName, "FL_FLOAT", FL_FLOAT );
	engine.RegisterEnumValue( pszObjectName, "FL_GRAPHED", FL_GRAPHED );
	engine.RegisterEnumValue( pszObjectName, "FL_IMMUNE_WATER", FL_IMMUNE_WATER );
	engine.RegisterEnumValue( pszObjectName, "FL_IMMUNE_SLIME", FL_IMMUNE_SLIME );
	engine.RegisterEnumValue( pszObjectName, "FL_IMMUNE_LAVA", FL_IMMUNE_LAVA );
	engine.RegisterEnumValue( pszObjectName, "FL_PROXY", FL_PROXY );
	engine.RegisterEnumValue( pszObjectName, "FL_ALWAYSTHINK", FL_ALWAYSTHINK );
	engine.RegisterEnumValue( pszObjectName, "FL_BASEVELOCITY", FL_BASEVELOCITY );
	engine.RegisterEnumValue( pszObjectName, "FL_MONSTERCLIP", FL_MONSTERCLIP );
	engine.RegisterEnumValue( pszObjectName, "FL_ONTRAIN", FL_ONTRAIN );
	engine.RegisterEnumValue( pszObjectName, "FL_WORLDBRUSH", FL_WORLDBRUSH );
	engine.RegisterEnumValue( pszObjectName, "FL_SPECTATOR", FL_SPECTATOR );
	engine.RegisterEnumValue( pszObjectName, "FL_CUSTOMENTITY", FL_CUSTOMENTITY );
	engine.RegisterEnumValue( pszObjectName, "FL_KILLME", FL_KILLME );
	engine.RegisterEnumValue( pszObjectName, "FL_DORMANT", FL_DORMANT );
}

/**
*	Class name for KeyValueData in scripts.
*/
#define AS_KEYVALUEDATA_NAME "KeyValueData"

static std::string KeyValueData_szKeyName( const KeyValueData* pThis )
{
	return pThis->szKeyName;
}

static std::string KeyValueData_szValue( const KeyValueData* pThis )
{
	return pThis->szValue;
}

static bool KeyValueData_get_bHandled( const KeyValueData* pThis )
{
	return pThis->fHandled != 0;
}

static void KeyValueData_set_bHandled( KeyValueData* pThis, const bool bHandled )
{
	pThis->fHandled = bHandled;
}

/**
*	Registers KeyValueData.
*	szClassName is not exposed, it can be retrieved through other means.
*	fHandled becomes bHandled, and is a boolean.
*	@param engine Script engine.
*/
static void RegisterScriptKeyValueData( asIScriptEngine& engine )
{
	const char* const pszObjectName = AS_KEYVALUEDATA_NAME;

	engine.RegisterObjectType( pszObjectName, 0, asOBJ_REF | asOBJ_NOCOUNT );

	engine.RegisterObjectMethod(
		pszObjectName, "string get_szKeyName() const",
		asFUNCTION( KeyValueData_szKeyName ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string get_szValue() const",
		asFUNCTION( KeyValueData_szValue ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "bool get_bHandled() const",
		asFUNCTION( KeyValueData_get_bHandled ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void set_bHandled(const bool bHandled)",
		asFUNCTION( KeyValueData_set_bHandled ), asCALL_CDECL_OBJFIRST );
}

/**
*	Class name for FCapability in scripts.
*/
#define AS_FCAPABILITY_NAME "FCapability"

static void RegisterScriptFCapability( asIScriptEngine& engine )
{
	const char* const pszObjectName = AS_FCAPABILITY_NAME;

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "FCAP_CUSTOMSAVE", FCAP_CUSTOMSAVE );
	engine.RegisterEnumValue( pszObjectName, "FCAP_ACROSS_TRANSITION", FCAP_ACROSS_TRANSITION );
	engine.RegisterEnumValue( pszObjectName, "FCAP_MUST_SPAWN", FCAP_MUST_SPAWN );
	engine.RegisterEnumValue( pszObjectName, "FCAP_DONT_SAVE", FCAP_DONT_SAVE );
	engine.RegisterEnumValue( pszObjectName, "FCAP_IMPULSE_USE", FCAP_IMPULSE_USE );
	engine.RegisterEnumValue( pszObjectName, "FCAP_CONTINUOUS_USE", FCAP_CONTINUOUS_USE );
	engine.RegisterEnumValue( pszObjectName, "FCAP_ONOFF_USE", FCAP_ONOFF_USE );
	engine.RegisterEnumValue( pszObjectName, "FCAP_DIRECTIONAL_USE", FCAP_DIRECTIONAL_USE );
	engine.RegisterEnumValue( pszObjectName, "FCAP_MASTER", FCAP_MASTER );
	engine.RegisterEnumValue( pszObjectName, "FCAP_FORCE_TRANSITION", FCAP_FORCE_TRANSITION );
}

/**
*	Class name for USE_TYPE in scripts.
*/
#define AS_USE_TYPE_NAME "USE_TYPE"

static void RegisterScriptUSE_TYPE( asIScriptEngine& engine )
{
	const char* const pszObjectName = AS_USE_TYPE_NAME;

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "USE_OFF", USE_OFF );
	engine.RegisterEnumValue( pszObjectName, "USE_ON", USE_ON );
	engine.RegisterEnumValue( pszObjectName, "USE_SET", USE_SET );
	engine.RegisterEnumValue( pszObjectName, "USE_TOGGLE", USE_TOGGLE );
}

/**
*	Class name for BloodColor in scripts.
*/
#define AS_BLOODCOLOR_NAME "BloodColor"

static void RegisterScriptBloodColor( asIScriptEngine& engine )
{
	const char* const pszObjectName = AS_BLOODCOLOR_NAME;

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "DONT_BLEED", DONT_BLEED );
	engine.RegisterEnumValue( pszObjectName, "BLOOD_COLOR_RED", BLOOD_COLOR_RED );
	engine.RegisterEnumValue( pszObjectName, "BLOOD_COLOR_YELLOW", BLOOD_COLOR_YELLOW );
	engine.RegisterEnumValue( pszObjectName, "BLOOD_COLOR_GREEN", BLOOD_COLOR_GREEN );
}

static void DefaultConstructCTakeDamageInfo( CTakeDamageInfo* pMemory )
{
	new( pMemory ) CTakeDamageInfo();
}

static void CopyConstructCTakeDamageInfo( CTakeDamageInfo* pMemory, const CTakeDamageInfo& other )
{
	new( pMemory ) CTakeDamageInfo( other );
}

static void CompleteConstructCTakeDamageInfo( CTakeDamageInfo* pMemory, CBaseEntity* pInflictor, CBaseEntity* pAttacker, const float flDamage, const int iBitsDamageTypes )
{
	new( pMemory ) CTakeDamageInfo( pInflictor, pAttacker, flDamage, iBitsDamageTypes );
}

static void SimpleConstructCTakeDamageInfo( CTakeDamageInfo* pMemory, CBaseEntity* pAttacker, const float flDamage, const int iBitsDamageTypes )
{
	new( pMemory ) CTakeDamageInfo( pAttacker, flDamage, iBitsDamageTypes );
}

static void DestructCTakeDamageInfo( CTakeDamageInfo* pMemory )
{
	pMemory->~CTakeDamageInfo();
}

/**
*	Registers CTakeDamageInfo.
*	@param engine Script engine.
*/
static void RegisterScriptCTakeDamageInfo( asIScriptEngine& engine )
{
	const char* const pszObjectName = "CTakeDamageInfo";

	engine.RegisterObjectType( pszObjectName, sizeof( CTakeDamageInfo ), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void CTakeDamageInfo()",
		asFUNCTION( DefaultConstructCTakeDamageInfo ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void CTakeDamageInfo(const CTakeDamageInfo& in other)",
		asFUNCTION( CopyConstructCTakeDamageInfo ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void CTakeDamageInfo(CBaseEntity@ pInflictor, CBaseEntity@ pAttacker, const float flDamage, const int iBitsDamageTypes)",
		asFUNCTION( CompleteConstructCTakeDamageInfo ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void CTakeDamageInfo(CBaseEntity@ pAttacker, const float flDamage, const int iBitsDamageTypes)",
		asFUNCTION( SimpleConstructCTakeDamageInfo ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void CTakeDamageInfo()",
		asFUNCTION( DestructCTakeDamageInfo ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetInflictor() const",
		asMETHOD( CTakeDamageInfo, GetInflictor ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetInflictor(CBaseEntity@ pInflictor)",
		asMETHOD( CTakeDamageInfo, SetInflictor ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ GetAttacker() const",
		asMETHOD( CTakeDamageInfo, GetAttacker ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetAttacker(CBaseEntity@ pAttacker)",
		asMETHOD( CTakeDamageInfo, SetAttacker ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float GetDamage() const",
		asMETHOD( CTakeDamageInfo, GetDamage ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "float& GetMutableDamage()",
		asMETHOD( CTakeDamageInfo, GetMutableDamage ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetDamage(const float flDamage)",
		asMETHOD( CTakeDamageInfo, SetDamage ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int GetDamageTypes() const",
		asMETHOD( CTakeDamageInfo, GetDamageTypes ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "int& GetMutableDamageTypes()",
		asMETHOD( CTakeDamageInfo, GetMutableDamageTypes ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "void SetDamageTypes(const int iBitsDamageTypes)",
		asMETHOD( CTakeDamageInfo, SetDamageTypes ), asCALL_THISCALL );
}

static void ConstructTraceResult( TraceResult* pMemory )
{
	new( pMemory ) TraceResult;

	memset( pMemory, 0, sizeof( TraceResult ) );
}

static void CopyConstructTraceResult( TraceResult* pMemory, const TraceResult& tr )
{
	new( pMemory ) TraceResult( tr );
}

static void DestructTraceResult( TraceResult* pMemory )
{
	//Nothing.
}

static TraceResult& TraceResult_Assign( TraceResult* pThis, const TraceResult& tr )
{
	*pThis = tr;

	return *pThis;
}

static CBaseEntity* TraceResult_get_pHit( const TraceResult* pThis )
{
	if( pThis->pHit )
		return CBaseEntity::Instance( pThis->pHit );

	return nullptr;
}

static void TraceResult_set_pHit( TraceResult* pThis, CBaseEntity* pEntity )
{
	pThis->pHit = pEntity ? pEntity->edict() : nullptr;
}

/**
*	Registers TraceResult.
*	@param engine Script engine.
*/
void RegisterScriptTraceResult( asIScriptEngine& engine )
{
	const char* const pszObjectName = "TraceResult";

	engine.RegisterObjectType( pszObjectName, 
		sizeof( TraceResult ), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void TraceResult()",
		asFUNCTION( ConstructTraceResult ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void TraceResult(const TraceResult& in tr)",
		asFUNCTION( CopyConstructTraceResult ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void TraceResult()",
		asFUNCTION( DestructTraceResult ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "TraceResult& opAssign(const TraceResult& in tr)", 
		asFUNCTION( TraceResult_Assign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectProperty(
		pszObjectName, "int fAllSolid", 
		asOFFSET( TraceResult, fAllSolid ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int fStartSolid", 
		asOFFSET( TraceResult, fStartSolid ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int fInOpen", 
		asOFFSET( TraceResult, fInOpen ) );

	engine.RegisterObjectProperty(
		pszObjectName, "int fInWater", 
		asOFFSET( TraceResult, fInWater ) );

	engine.RegisterObjectProperty(
		pszObjectName, "float flFraction", 
		asOFFSET( TraceResult, flFraction ) );

	engine.RegisterObjectProperty(
		pszObjectName, "Vector vecEndPos", 
		asOFFSET( TraceResult, vecEndPos ) );

	engine.RegisterObjectProperty(
		pszObjectName, "float flPlaneDist", 
		asOFFSET( TraceResult, flPlaneDist ) );

	engine.RegisterObjectProperty(
		pszObjectName, "Vector vecPlaneNormal",
		asOFFSET( TraceResult, vecPlaneNormal ) );

	engine.RegisterObjectMethod(
		pszObjectName, "CBaseEntity@ get_pHit() const",
		asFUNCTION( TraceResult_get_pHit ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "void set_pHit(CBaseEntity@ pEntity)",
		asFUNCTION( TraceResult_set_pHit ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectProperty(
		pszObjectName, "int iHitgroup", 
		asOFFSET( TraceResult, iHitgroup ) );
}

static void RegisterScriptGibAction( asIScriptEngine& engine )
{
	const char* const pszObjectName = "GibAction";

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "GIB_NORMAL", GIB_NORMAL );
	engine.RegisterEnumValue( pszObjectName, "GIB_NEVER", GIB_NEVER );
	engine.RegisterEnumValue( pszObjectName, "GIB_ALWAYS", GIB_ALWAYS );
}

static void RegisterScriptBullet( asIScriptEngine& engine )
{
	const char* const pszObjectName = "Bullet";

	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( pszObjectName );

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "NONE", BULLET_NONE );
	engine.RegisterEnumValue( pszObjectName, "PLAYER_9MM", BULLET_PLAYER_9MM );
	engine.RegisterEnumValue( pszObjectName, "PLAYER_MP5", BULLET_PLAYER_MP5 );
	engine.RegisterEnumValue( pszObjectName, "PLAYER_357", BULLET_PLAYER_357 );
	engine.RegisterEnumValue( pszObjectName, "PLAYER_BUCKSHOT", BULLET_PLAYER_BUCKSHOT );
	engine.RegisterEnumValue( pszObjectName, "PLAYER_CROWBAR", BULLET_PLAYER_CROWBAR );

	engine.RegisterEnumValue( pszObjectName, "MONSTER_9MM", BULLET_MONSTER_9MM );
	engine.RegisterEnumValue( pszObjectName, "MONSTER_MP5", BULLET_MONSTER_MP5 );
	engine.RegisterEnumValue( pszObjectName, "MONSTER_12MM", BULLET_MONSTER_12MM );

#ifdef GAME_OPFOR
	engine.RegisterEnumValue( pszObjectName, "PLAYER_556", BULLET_PLAYER_556 );
	engine.RegisterEnumValue( pszObjectName, "PLAYER_762", BULLET_PLAYER_762 );
	engine.RegisterEnumValue( pszObjectName, "PLAYER_DEAGLE", BULLET_PLAYER_DEAGLE );
#endif

	engine.SetDefaultNamespace( szOldNS.c_str() );
}

static void RegisterScriptDmg( asIScriptEngine& engine )
{
	const char* const pszObjectName = "Dmg";

	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( pszObjectName );

	engine.RegisterEnum( pszObjectName );

	engine.RegisterEnumValue( pszObjectName, "GENERIC", DMG_GENERIC );
	engine.RegisterEnumValue( pszObjectName, "CRUSH", DMG_CRUSH );
	engine.RegisterEnumValue( pszObjectName, "BULLET", DMG_BULLET );
	engine.RegisterEnumValue( pszObjectName, "SLASH", DMG_SLASH );
	engine.RegisterEnumValue( pszObjectName, "BURN", DMG_BURN );
	engine.RegisterEnumValue( pszObjectName, "FREEZE", DMG_FREEZE );
	engine.RegisterEnumValue( pszObjectName, "FALL", DMG_FALL );
	engine.RegisterEnumValue( pszObjectName, "BLAST", DMG_BLAST );
	engine.RegisterEnumValue( pszObjectName, "CLUB", DMG_CLUB );
	engine.RegisterEnumValue( pszObjectName, "SHOCK", DMG_SHOCK );
	engine.RegisterEnumValue( pszObjectName, "SONIC", DMG_SONIC );
	engine.RegisterEnumValue( pszObjectName, "ENERGYBEAM", DMG_ENERGYBEAM );
	engine.RegisterEnumValue( pszObjectName, "NEVERGIB", DMG_NEVERGIB );
	engine.RegisterEnumValue( pszObjectName, "ALWAYSGIB", DMG_ALWAYSGIB );
	engine.RegisterEnumValue( pszObjectName, "DROWN", DMG_DROWN );
	engine.RegisterEnumValue( pszObjectName, "TIMEBASED", DMG_TIMEBASED );
	engine.RegisterEnumValue( pszObjectName, "PARALYZE", DMG_PARALYZE );
	engine.RegisterEnumValue( pszObjectName, "NERVEGAS", DMG_NERVEGAS );
	engine.RegisterEnumValue( pszObjectName, "POISON", DMG_POISON );
	engine.RegisterEnumValue( pszObjectName, "RADIATION", DMG_RADIATION );
	engine.RegisterEnumValue( pszObjectName, "DROWNRECOVER", DMG_DROWNRECOVER );
	engine.RegisterEnumValue( pszObjectName, "ACID", DMG_ACID );
	engine.RegisterEnumValue( pszObjectName, "SLOWBURN", DMG_SLOWBURN );
	engine.RegisterEnumValue( pszObjectName, "SLOWFREEZE", DMG_SLOWFREEZE );
	engine.RegisterEnumValue( pszObjectName, "MORTAR", DMG_MORTAR );
	engine.RegisterEnumValue( pszObjectName, "GIB_CORPSE", DMG_GIB_CORPSE );
	engine.RegisterEnumValue( pszObjectName, "SHOWNHUD", DMG_SHOWNHUD );

	engine.SetDefaultNamespace( szOldNS.c_str() );
}

static const char* const g_pszEntities[] = 
{
	AS_CBASEENTITY_NAME,
	"CBaseMonster",		//TODO: register fully elsewhere - Solokiller
	"CBasePlayer",
	"CPathTrack",
};

void RegisterScriptEntityDependencies( asIScriptEngine& engine )
{
	//Forward declare all entity types for use in the API.
	for( auto pszName : g_pszEntities )
	{
		engine.RegisterObjectType( pszName, 0, asOBJ_REF | asOBJ_NOCOUNT );
	}

	static_assert( std::is_same<uint32_t, EntityClassification_t>::value, "Update EntityClassification_t for Angelscript" );

	engine.RegisterTypedef( "EntityClassification_t", "uint32" );

	RegisterScriptFixAngleMode( engine );
	RegisterScriptMoveType( engine );
	RegisterScriptSolid( engine );
	RegisterScriptRenderMode( engine );
	RegisterScriptRenderFX( engine );
	RegisterScriptTakeDamageMode( engine );
	RegisterScriptDeadFlag( engine );
	RegisterScriptWaterLevel( engine );
	RegisterScriptContents( engine );
	RegisterScriptEntFlag( engine );
	RegisterScriptKeyValueData( engine );
	RegisterScriptFCapability( engine );
	RegisterScriptUSE_TYPE( engine );
	RegisterScriptBloodColor( engine );
	RegisterScriptCTakeDamageInfo( engine );
	RegisterScriptTraceResult( engine );
	RegisterScriptGibAction( engine );
	RegisterScriptBullet( engine );
	RegisterScriptDmg( engine );

	RegisterScriptCEntBitSet( engine );
}

std::string CBaseEntity_GetClassname( const CBaseEntity* pThis )
{
	return pThis->GetClassname();
}

bool CBaseEntity_ClassnameIs( const CBaseEntity* pThis, const std::string& szClassname )
{
	return pThis->ClassnameIs( szClassname.c_str() );
}

std::string CBaseEntity_GetGlobalName( const CBaseEntity* pThis )
{
	return pThis->GetGlobalName();
}

void CBaseEntity_SetGlobalName( CBaseEntity* pThis, const std::string& szGlobalName )
{
	pThis->SetGlobalName( ALLOC_STRING( szGlobalName.c_str() ) );
}

std::string CBaseEntity_GetTargetname( const CBaseEntity* pThis )
{
	return pThis->GetTargetname();
}

void CBaseEntity_SetTargetname( CBaseEntity* pThis, const std::string& szTargetname )
{
	pThis->SetTargetname( ALLOC_STRING( szTargetname.c_str() ) );
}

std::string CBaseEntity_GetTarget( const CBaseEntity* pThis )
{
	return pThis->GetTarget();
}

void CBaseEntity_SetTarget( CBaseEntity* pThis, const std::string& szTarget )
{
	pThis->SetTarget( ALLOC_STRING( szTarget.c_str() ) );
}

std::string CBaseEntity_GetNetName( const CBaseEntity* pThis )
{
	return pThis->GetNetName();
}

void CBaseEntity_SetNetName( CBaseEntity* pThis, const std::string& szNetName )
{
	pThis->SetNetName( ALLOC_STRING( szNetName.c_str() ) );
}

std::string CBaseEntity_GetModelName( const CBaseEntity* pThis )
{
	return pThis->GetModelName();
}

void CBaseEntity_SetModel( CBaseEntity* pThis, const std::string& szModelName )
{
	pThis->SetModel( ALLOC_STRING( szModelName.c_str() ) );
}

#ifdef SERVER_DLL
std::string CBaseEntity_GetViewModelName( const CBaseEntity* pThis )
{
	return pThis->GetViewModelName();
}

void CBaseEntity_SetViewModelName( CBaseEntity* pThis, const std::string& szViewModelName )
{
	pThis->SetViewModelName( ALLOC_STRING( szViewModelName.c_str() ) );
}
#endif

std::string CBaseEntity_GetWeaponModelName( const CBaseEntity* pThis )
{
	return pThis->GetWeaponModelName();
}

void CBaseEntity_SetWeaponModelName( CBaseEntity* pThis, const std::string& szWeaponModelName )
{
	pThis->SetWeaponModelName( ALLOC_STRING( szWeaponModelName.c_str() ) );
}

std::string CBaseEntity_GetMessage( const CBaseEntity* pThis )
{
	return pThis->GetMessage();
}

void CBaseEntity_SetMessage( CBaseEntity* pThis, const std::string& szMessage )
{
	pThis->SetMessage( ALLOC_STRING( szMessage.c_str() ) );
}

bool CBaseEntity_HasTarget( const CBaseEntity* pThis, const std::string& szTarget )
{
	return pThis->HasTarget( szTarget.c_str() );
}

std::string CBaseEntity_TeamID( const CBaseEntity* pThis )
{
	return pThis->TeamID();
}

std::string CBaseEntity_GetNoise( const CBaseEntity* pThis )
{
	return pThis->GetNoise();
}

void CBaseEntity_SetNoise( CBaseEntity* pThis, const std::string& szNoise )
{
	pThis->SetNoise( ALLOC_STRING( szNoise.c_str() ) );
}

std::string CBaseEntity_GetNoise1( const CBaseEntity* pThis )
{
	return pThis->GetNoise1();
}

void CBaseEntity_SetNoise1( CBaseEntity* pThis, const std::string& szNoise )
{
	pThis->SetNoise1( ALLOC_STRING( szNoise.c_str() ) );
}

std::string CBaseEntity_GetNoise2( const CBaseEntity* pThis )
{
	return pThis->GetNoise2();
}

void CBaseEntity_SetNoise2( CBaseEntity* pThis, const std::string& szNoise )
{
	pThis->SetNoise2( ALLOC_STRING( szNoise.c_str() ) );
}

std::string CBaseEntity_GetNoise3( const CBaseEntity* pThis )
{
	return pThis->GetNoise3();
}

void CBaseEntity_SetNoise3( CBaseEntity* pThis, const std::string& szNoise )
{
	pThis->SetNoise3( ALLOC_STRING( szNoise.c_str() ) );
}
