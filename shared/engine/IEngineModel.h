#ifndef SHARED_ENGINE_IENGINEMODEL_H
#define SHARED_ENGINE_IENGINEMODEL_H

#include "interface.h"

#include "com_model.h"

typedef struct edict_s edict_t;
class Vector;

/**
*	Interface to get at engine model data and perform model related operations.
*/
class IEngineModel : public IBaseInterface
{
public:

	/**
	*	Gets the model of the entity at the given index.
	*	@param iEntIndex Entity index.
	*	@return If the index is valid and the entity has a model, returns the model. Otherwise, returns null.
	*/
	virtual model_t* GetModelOfEntity( int iEntIndex ) = 0;

	/**
	*	Gets a model by its index. The index is used for entvars_t::modelindex.
	*	@param iIndex Model index.
	*	@return If the index is valid, returns the model. Otherwise, returns null.
	*/
	virtual model_t* GetModelByIndex( int iIndex ) = 0;

	/**
	*	Gets the index of a model. Unlike enginefuncs_t::pfnModelIndex, this method does not cause a Sys_Error if the model is not precached.
	*	@param pszModelName Name of the model whose index should be returned.
	*	@return If the model is precached, returns its index. Otherwise, returns 0.
	*/
	virtual int GetModelIndex( const char* const pszModelName ) = 0;

	/**
	*	Gets the number of models that have been precached.
	*/
	virtual int GetModelPrecacheCount() = 0;

	/**
	*	Precaches a model. If precaching fails for any reason, the method returns without performing any actions.
	*	No Sys_Error or Host_Error calls are made.
	*	@param pszModelName Name of the model to precache.
	*	@return If the model was precached, the index of the model. Otherwise, returns the index of the error model.
	*/
	virtual int PrecacheModel( const char* const pszModelName ) = 0;

	/**
	*	Sets the model of the given entity. If the model was not precached, uses the error model.
	*	Does nothing on the client side.
	*	@param pEntity Entity whose model should be set.
	*	@param pszModelName Name of the model to set.
	*/
	virtual void SetModel( edict_t* pEntity, const char* const pszModelName ) = 0;

	/**
	*	Sets the model of the given entity. If the index is invalid, uses the error model.
	*	Does nothing on the client side.
	*	@param pEntity Entity whose model should be set.
	*	@param iModelIndex Index of the model to set.
	*/
	virtual void SetModel( edict_t* pEntity, int iModelIndex ) = 0;

	/**
	*	Gets the type of the given model.
	*	@param pModel Model whose type should be retrieved.
	*/
	virtual modtype_t GetModelType( model_t* pModel ) = 0;

	/**
	*	Gets the bounds of the given model.
	*	@param pModel Model whose bounds should be retrieved.
	*	@param[ out ] vecMins Minimum bounds.
	*	@param[ out ] vecMaxs Maximum bounds.
	*/
	virtual void GetModelBounds( model_t* pModel, Vector& vecMins, Vector& vecMaxs ) = 0;
};

#define IENGINEMODEL_NAME "IEngineModelV001"

#endif //SHARED_ENGINE_IENGINEMODEL_H
