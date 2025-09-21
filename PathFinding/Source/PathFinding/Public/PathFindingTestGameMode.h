#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Zone/ZoneManager.h"
#include "PathFindingTestGameMode.generated.h"

#pragma region Forward_Declare
class AZoneManager;
class UDataTable;
#pragma endregion Forward_Declare

/**
* @Brief Test game mode for PathFinding plugin validation
*
* Sets up a controlled testing environment for zone-based pathfinding
* with automatic zone manager initialization and player tracking
*/
UCLASS(BlueprintType)
class PATHFINDING_API APathFindingTestGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/**
	* @Brief Default constructor with test configuration
	*/
	APathFindingTestGameMode();

protected:
	/**
	* @Brief Game mode initialization
	*/
	virtual void BeginPlay() override;

public:
	/**
	* @Brief Game mode tick for player position tracking
	* @inDeltaTime : float - Time since last tick
	*/
	virtual void Tick(float inDeltaTime) override;

	/**
	* @Brief Gets zone manager instance
	* @Return : AZoneManager* - Zone manager reference or nullptr
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	AZoneManager* GetZoneManager() const { return zoneManager; }

	/**
	* @Brief Enables or disables debug visualization
	* @bEnabled : bool - Enable debug display
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	void SetDebugMode(bool bEnabled);

	/**
	* @Brief Gets current player position for zone tracking
	* @Return : FVector - Player world position
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	FVector GetPlayerPosition() const;

protected:
	/** @Brief Zone manager instance for testing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PathFinding Test")
	TObjectPtr<AZoneManager> zoneManager;

	/** @Brief Test zone data table reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFinding Test")
	TObjectPtr<UDataTable> testZoneDataTable;

	/** @Brief Enable automatic player position tracking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFinding Test")
	bool bTrackPlayerPosition;

	/** @Brief Player position update interval */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PathFinding Test")
	float playerTrackingInterval;

	/** @Brief Timer handle for player tracking */
	FTimerHandle playerTrackingTimer;

	/**
	* @Brief Updates player position in zone manager
	*/
	void UpdatePlayerPosition();

	/**
	* @Brief Creates and spawns zone manager
	*/
	void CreateZoneManager();
};