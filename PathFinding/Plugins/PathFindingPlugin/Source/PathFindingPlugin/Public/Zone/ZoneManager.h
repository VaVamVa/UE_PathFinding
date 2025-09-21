#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Zone/ZoneTypes.h"
#include "Zone/ZoneLevelData.h"
#include "ZoneManager.generated.h"

#pragma region Forward_Declare
class UDataTable;
class UZoneLevelInstanceComponent;
struct FZoneLevelData;
struct FZoneConnectionPoint;
#pragma endregion Forward_Declare

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZoneManagerInitialized, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneRegistered, const FZoneIdentifier&, inZoneID, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZoneUnregistered, const FZoneIdentifier&, inZoneID);

/**
* @Brief Zone manager configuration settings
*
* Configurable settings for zone management behavior including
* auto-loading, debug display, and resource management parameters
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FZoneManagerSettings
{
	GENERATED_BODY()

public:
	/** @Brief Enable automatic zone loading based on proximity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Manager Settings")
	bool bAutoLoadZones;

	/** @Brief Enable debug visualization for all zones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Manager Settings")
	bool bEnableDebugDisplay;

	/** @Brief Interval for auto-loading proximity checks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Manager Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float autoLoadCheckInterval;

	/** @Brief Maximum number of zones loading simultaneously */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Manager Settings")
	int32 maxConcurrentLoads;

	/** @Brief Enable automatic cache saving to disk */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Manager Settings")
	bool bSaveZoneCachesToDisk;

	/** @Brief Interval for automatic cache saves */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Manager Settings")
	float cacheSaveInterval;

	/**
	* @Brief Default constructor with standard settings
	*/
	FZoneManagerSettings()
		: bAutoLoadZones(true)
		, bEnableDebugDisplay(false)
		, autoLoadCheckInterval(1.0f)
		, maxConcurrentLoads(3)
		, bSaveZoneCachesToDisk(true)
		, cacheSaveInterval(300.0f)
	{}
};

/**
* @Brief Central zone management actor for world-wide zone coordination
*
* Manages all zones within a world including loading/unloading coordination,
* cross-zone pathfinding, and resource management with DataTable integration
*/
UCLASS(BlueprintType)
class PATHFINDINGPLUGIN_API AZoneManager : public AActor
{
	GENERATED_BODY()

public:
	/**
	* @Brief Default constructor with tick configuration
	*/
	AZoneManager();

protected:
	/**
	* @Brief Actor initialization at game start
	*/
	virtual void BeginPlay() override;

	/**
	* @Brief Actor cleanup at game end
	* @inEndPlayReason : EEndPlayReason::Type - Reason for actor termination
	*/
	virtual void EndPlay(const EEndPlayReason::Type inEndPlayReason) override;

public:
	/**
	* @Brief Actor tick for loading queue processing
	* @inDeltaTime : float - Time since last tick
	*/
	virtual void Tick(float inDeltaTime) override;

	/**
	* @Brief Initializes zone manager from DataTable configuration
	* @inZoneDataTable : UDataTable* - DataTable containing zone definitions
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void InitializeFromDataTable(UDataTable* inZoneDataTable);

	/**
	* @Brief Registers new zone with manager
	* @inZoneData : FZoneLevelData - Complete zone configuration
	* @Return     : bool           - True if registration succeeded
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	bool RegisterZone(const FZoneLevelData& inZoneData);

	/**
	* @Brief Unregisters zone from manager
	* @inZoneIdentifier : FZoneIdentifier - Zone to unregister
	* @Return           : bool            - True if unregistration succeeded
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	bool UnregisterZone(const FZoneIdentifier& inZoneIdentifier);

	/**
	* @Brief Unregisters all managed zones
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void UnregisterAllZones();

	/**
	* @Brief Finds zone component by identifier
	* @inZoneIdentifier : FZoneIdentifier              - Zone to search for
	* @Return           : UZoneLevelInstanceComponent* - Zone component or nullptr
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	UZoneLevelInstanceComponent* FindZoneByIdentifier(const FZoneIdentifier& inZoneIdentifier) const;

	/**
	* @Brief Finds zone containing specific world position
	* @inWorldPosition : FVector                       - Position to test
	* @Return          : UZoneLevelInstanceComponent*  - Containing zone or nullptr
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	UZoneLevelInstanceComponent* FindZoneContainingPosition(const FVector& inWorldPosition) const;

	/**
	* @Brief Finds all zones of specific type
	* @inZoneType : EZoneType                             - Zone type to search for
	* @Return     : TArray<UZoneLevelInstanceComponent*> - Array of matching zones
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	TArray<UZoneLevelInstanceComponent*> FindZonesByType(EZoneType inZoneType) const;

	/**
	* @Brief Finds zones within radius of position
	* @inCenter : FVector                                 - Center position for search
	* @inRadius : float                                   - Search radius
	* @Return   : TArray<UZoneLevelInstanceComponent*>   - Array of zones in radius
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	TArray<UZoneLevelInstanceComponent*> FindZonesInRadius(const FVector& inCenter, float inRadius) const;

	/**
	* @Brief Gets all managed zone components
	* @Return : TArray<UZoneLevelInstanceComponent*> - Array of all zones
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	TArray<UZoneLevelInstanceComponent*> GetAllZones() const;

	/**
	* @Brief Requests zone loading with queue management
	* @inZoneIdentifier : FZoneIdentifier - Zone to load
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void LoadZone(const FZoneIdentifier& inZoneIdentifier);

	/**
	* @Brief Updates automatic loading based on player position
	* @inPlayerPosition : FVector - Current player world position
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void UpdateAutoLoading(const FVector& inPlayerPosition);

	/**
	* @Brief Sets current player position for auto-loading
	* @inPosition : FVector - Player world position
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void SetPlayerPosition(const FVector& inPosition) { playerPosition = inPosition; }

	/**
	* @Brief Gets current tracked player position
	* @Return : FVector - Player world position
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	FVector GetPlayerPosition() const { return playerPosition; }

	/**
	* @Brief Finds connections between two zones
	* @inFromZone : FZoneIdentifier               - Source zone
	* @inToZone   : FZoneIdentifier               - Target zone
	* @Return     : TArray<FZoneConnectionPoint> - Array of connection points
	*/
	UFUNCTION(BlueprintCallable, Category = "Path Finding")
	TArray<FZoneConnectionPoint> FindConnectionsBetweenZones(const FZoneIdentifier& inFromZone, const FZoneIdentifier& inToZone) const;

	/**
	* @Brief Gets total number of managed zones
	* @Return : int32 - Total zone count
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	int32 GetTotalZoneCount() const { return zoneComponents.Num(); }

	/**
	* @Brief Gets number of currently loaded zones
	* @Return : int32 - Loaded zone count
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	int32 GetLoadedZoneCount() const;

	/**
	* @Brief Updates zone manager settings
	* @inNewSettings : FZoneManagerSettings - New configuration settings
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void SetZoneManagerSettings(const FZoneManagerSettings& inNewSettings);

	/**
	* @Brief Gets current zone manager settings
	* @Return : FZoneManagerSettings - Current settings reference
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	const FZoneManagerSettings& GetZoneManagerSettings() const { return settings; }

	/**
	* @Brief Enables or disables debug display
	* @bEnabled : bool - Enable debug visualization
	*/
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDebugDisplayEnabled(bool bEnabled);

	/**
	* @Brief Draws debug information for all zones
	*/
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugInfo() const;

	// 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Zone Manager Events")
	FOnZoneManagerInitialized OnZoneManagerInitialized;

	UPROPERTY(BlueprintAssignable, Category = "Zone Manager Events")
	FOnZoneRegistered OnZoneRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Zone Manager Events")
	FOnZoneUnregistered OnZoneUnregistered;

	/** @Brief Default zone data table for automatic initialization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Data")
	TObjectPtr<UDataTable> defaultZoneDataTable;

protected:
	/** @Brief Map of zone identifiers to their components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Components")
	TMap<FZoneIdentifier, TObjectPtr<UZoneLevelInstanceComponent>> zoneComponents;

	/** @Brief Current zone manager configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FZoneManagerSettings settings;

	/** @Brief Tracked player position for auto-loading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Tracking")
	FVector playerPosition;

	/** @Brief Timer handles for automatic operations */
	FTimerHandle autoLoadTimerHandle;
	FTimerHandle cacheSaveTimerHandle;

	/** @Brief Loading queue management */
	TArray<FZoneIdentifier> loadingQueue;
	TArray<FZoneIdentifier> currentlyLoading;

	/** @Brief Manager initialization state */
	bool bIsInitialized;

	/**
	* @Brief Processes zone loading queue with concurrency limits
	*/
	void ProcessLoadingQueue();

	/**
	* @Brief Handles zone component load state changes
	* @inZoneID   : FZoneIdentifier - Zone that changed state
	* @inNewState : EZoneLoadState  - New loading state
	*/
	UFUNCTION()
	void OnZoneComponentLoadStateChanged(const FZoneIdentifier& inZoneID, EZoneLoadState inNewState);

	/**
	* @Brief Saves all zone caches to disk
	*/
	void SaveAllZoneCaches();

	/**
	* @Brief Cleans up all zone components
	*/
	void CleanupZoneComponents();
};