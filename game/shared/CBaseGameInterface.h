#ifndef GAME_SHARED_CBASEGAMEINTERFACE_H
#define GAME_SHARED_CBASEGAMEINTERFACE_H

/**
*	Mixin class for shared game interface code.
*/
class CBaseGameInterface
{
public:
	/**
	*	Constructor.
	*/
	CBaseGameInterface() = default;

	/**
	*	Destructor.
	*/
	~CBaseGameInterface() = default;

protected:
	/**
	*	Initializes common code.
	*	@return true on success, false on failure.
	*/
	bool InitializeCommon( CreateInterfaceFn* pFactories, size_t uiNumFactories );

	bool PostInit();

	/**
	*	Shuts down common code.
	*/
	void ShutdownCommon();

private:
	CBaseGameInterface( const CBaseGameInterface& ) = delete;
	CBaseGameInterface& operator=( const CBaseGameInterface& ) = delete;
};

#endif //GAME_SHARED_CBASEGAMEINTERFACE_H