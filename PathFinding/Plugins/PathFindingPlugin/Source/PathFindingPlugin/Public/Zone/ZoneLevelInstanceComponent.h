#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "Zone/ZoneTypes.h"
#include "Zone/ZoneLevelData.h"
#include "ZoneLevelInstanceComponent.generated.h"

#pragma region Forward_Declare
class ULevelStreaming;
class UZonePathScoreCache;
class UWorld;
#pragma endregion Forward_Declare

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneLoadStateChanged, const FZoneIdentifier&, inZoneID, EZoneLoadState, inNewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZonePathCacheReady, const FZoneIdentifier&, inZoneID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneLoadError, const FZoneIdentifier&, inZoneID, const FString&, inErrorMessage);

/**
* @Brief Zone level instance management component
*
* Manages individual zone loading, unloading, and path cache generation
* with automatic proximity-based streaming and event notifications
*/
UCLASS(BlueprintType, ClassGroup=(PathFinding), meta=(BlueprintSpawnableComponent))
class PATHFINDINGPLUGIN_API UZoneLevelInstanceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	* @Brief Default constructor with tick configuration
	*/
	UZoneLevelInstanceComponent();

protected:
	/**
	* @Brief Component initialization at game start
	*/
	virtual void BeginPlay() override;

	/**
	* @Brief Component cleanup at game end
	* @inEndPlayReason : EEndPlayReason::Type - Reason for component termination
	*/
	virtual void EndPlay(const EEndPlayReason::Type inEndPlayReason) override;

public:
	/**
	* @Brief Component tick for timeout monitoring
	* @inDeltaTime      : float                     - Time since last tick
	* @inTickType       : ELevelTick                - Type of tick
	* @inThisTickFunction : FActorComponentTickFunction* - Tick function context
	*/
	virtual void TickComponent(float inDeltaTime, ELevelTick inTickType, FActorComponentTickFunction* inThisTickFunction) override;

	/**
	* @Brief Initializes zone with level data configuration
	* @inZoneData : FZoneLevelData - Complete zone configuration
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void InitializeZone(const FZoneLevelData& inZoneData);

	/**
	* @Brief Begins asynchronous zone loading process
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void LoadZone();

	/**
	* @Brief Begins zone unloading process
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void UnloadZone();

	/**
	* @Brief Checks if zone is currently loaded
	* @Return : bool - True if zone is loaded and ready
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	bool IsZoneLoaded() const;

	/**
	* @Brief Gets current zone loading state
	* @Return : EZoneLoadState - Current loading state
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	EZoneLoadState GetZoneLoadState() const;

	/**
	* @Brief Gets read-only zone configuration data
	* @Return : FZoneLevelData - Zone data reference
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	const FZoneLevelData& GetZoneData() const { return zoneData; }

	/**
	* @Brief Gets zone identifier
	* @Return : FZoneIdentifier - Zone identifier reference
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	const FZoneIdentifier& GetZoneIdentifier() const { return zoneData.zoneIdentifier; }

	/**
	* @Brief Gets path score cache instance
	* @Return : UZonePathScoreCache* - Path cache object pointer
	*/
	UFUNCTION(BlueprintCallable, Category = "Path Finding")
	UZonePathScoreCache* GetPathScoreCache() const { return compPathScoreCache; }

	/**
	* @Brief Generates pathfinding cache for loaded zone
	*/
	UFUNCTION(BlueprintCallable, Category = "Path Finding")
	void GeneratePathCache();

	/**
	* @Brief Checks if path cache is ready for pathfinding
	* @Return : bool - True if cache is generated and ready
	*/
	UFUNCTION(BlueprintCallable, Category = "Path Finding")
	bool IsPathCacheReady() const;

	/**
	* @Brief Updates automatic loading based on player position
	* @inPlayerLocation : FVector - Current player world position
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void UpdateAutoLoading(const FVector& inPlayerLocation);

	/**
	* @Brief Enables or disables automatic loading
	* @bEnabled : bool - Enable automatic loading
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	void SetAutoLoadingEnabled(bool bEnabled) { bAutoLoadingEnabled = bEnabled; }

	/**
	* @Brief Checks if automatic loading is enabled
	* @Return : bool - True if auto-loading is active
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	bool IsAutoLoadingEnabled() const { return bAutoLoadingEnabled; }

	/**
	* @Brief Draws debug visualization for zone
	* @bPersistent : bool  - Draw persistent debug info (default: false)
	* @inLifeTime  : float - Debug display duration (default: 1.0f)
	*/
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugInfo(bool bPersistent = false, float inLifeTime = 1.0f) const;

	/**
	* @Brief Checks if world position is within zone bounds
	* @inWorldPosition : FVector - World position to test
	* @Return          : bool    - True if position is in zone
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	bool ContainsWorldPosition(const FVector& inWorldPosition) const;

	/**
	* @Brief Calculates distance from position to zone center
	* @inWorldPosition : FVector - World position to measure from
	* @Return          : float   - Distance to zone center
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Management")
	float GetDistanceToZone(const FVector& inWorldPosition) const;

	// 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Zone Events")
	FOnZoneLoadStateChanged OnZoneLoadStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Zone Events")
	FOnZonePathCacheReady OnZonePathCacheReady;

	UPROPERTY(BlueprintAssignable, Category = "Zone Events")
	FOnZoneLoadError OnZoneLoadError;

protected:
	/** @Brief Complete zone configuration data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Data")
	FZoneLevelData zoneData;

	/** @Brief Level streaming instance for dynamic loading */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Streaming")
	TObjectPtr<ULevelStreaming> compLevelStreamingInstance;

	/** @Brief Path cache component for pathfinding */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Finding")
	TObjectPtr<UZonePathScoreCache> compPathScoreCache;

	/** @Brief Current zone loading state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone State")
	EZoneLoadState currentLoadState;

	/** @Brief Enable automatic proximity-based loading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
	bool bAutoLoadingEnabled;

	/** @Brief Generate path cache automatically on load */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
	bool bGeneratePathCacheOnLoad;

	/** @Brief Maximum time to wait for loading completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Settings")
	float loadingTimeoutSeconds;

	/** @Brief Loading start timestamp for timeout detection */
	float loadingStartTime;

	/** @Brief Timer handle for loading timeout */
	FTimerHandle loadingTimeoutHandle;

	/**
	* @Brief Handles successful level loading completion
	*/
	UFUNCTION()
	void OnLevelLoaded();

	/**
	* @Brief Handles level unloading completion
	*/
	UFUNCTION()
	void OnLevelUnloaded();

	/**
	* @Brief Sets new loading state and broadcasts change
	* @inNewState : EZoneLoadState - New loading state
	*/
	void SetLoadState(EZoneLoadState inNewState);

	/**
	* @Brief Creates level streaming instance for zone
	*/
	void CreateLevelStreamingInstance();

	/**
	* @Brief Sets up path cache component
	*/
	void SetupPathCache();

private:
};