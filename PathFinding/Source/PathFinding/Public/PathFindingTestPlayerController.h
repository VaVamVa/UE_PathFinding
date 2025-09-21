#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Zone/ZoneTypes.h"
#include "PathFindingTestPlayerController.generated.h"

#pragma region Forward_Declare
class AZoneManager;
class UZoneLevelInstanceComponent;
#pragma endregion Forward_Declare

/**
* @Brief Test player controller for PathFinding plugin interaction
*
* Provides debug commands and zone interaction capabilities
* for testing PathFinding plugin functionality
*/
UCLASS(BlueprintType)
class PATHFINDING_API APathFindingTestPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/**
	* @Brief Default constructor
	*/
	APathFindingTestPlayerController();

protected:
	/**
	* @Brief Player controller initialization
	*/
	virtual void BeginPlay() override;

	/**
	* @Brief Sets up input bindings
	*/
	virtual void SetupInputComponent() override;

public:
	/**
	* @Brief Gets current zone containing player
	* @Return : UZoneLevelInstanceComponent* - Current zone or nullptr
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	UZoneLevelInstanceComponent* GetCurrentZone() const;

	/**
	* @Brief Manually triggers zone loading for testing
	* @inZoneIdentifier : FZoneIdentifier - Zone to load
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	void LoadZoneManually(const FZoneIdentifier& inZoneIdentifier);

	/**
	* @Brief Toggles debug visualization
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	void ToggleDebugDisplay();

	/**
	* @Brief Prints current zone information to screen
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	void PrintZoneInfo();

	/**
	* @Brief Teleports player to specific world position
	* @inLocation : FVector - Target world position
	*/
	UFUNCTION(BlueprintCallable, Category = "PathFinding Test")
	void TeleportToLocation(const FVector& inLocation);

protected:
	/** @Brief Current debug display state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PathFinding Test")
	bool bDebugDisplayEnabled;

	/**
	* @Brief Input handler for debug toggle
	*/
	void OnToggleDebug();

	/**
	* @Brief Input handler for zone info display
	*/
	void OnPrintZoneInfo();

	/**
	* @Brief Gets zone manager from game mode
	* @Return : AZoneManager* - Zone manager reference or nullptr
	*/
	AZoneManager* GetZoneManager() const;
};