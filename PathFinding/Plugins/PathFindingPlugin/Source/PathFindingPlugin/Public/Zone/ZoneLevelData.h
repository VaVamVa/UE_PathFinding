#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Zone/ZoneTypes.h"
#include "ZoneLevelData.generated.h"

#pragma region Forward_Declare
class UWorld;
class ULevelInstance;
#pragma endregion Forward_Declare

/**
* @Brief Zone spatial boundary definition structure
*
* Defines the 3D boundaries of a zone for collision detection,
* auto-loading triggers, and spatial queries
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FZoneBounds
{
	GENERATED_BODY()

public:
	/** @Brief Center point of the zone in world coordinates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Bounds")
	FVector center;

	/** @Brief Half-extents from center to boundaries on each axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Bounds")
	FVector extent;

	/** @Brief Total height of the zone for vertical boundaries */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Bounds")
	float height;

	/**
	* @Brief Default constructor with reasonable default values
	*/
	FZoneBounds()
		: center(FVector::ZeroVector)
		, extent(FVector(1000.0f, 1000.0f, 1000.0f))
		, height(500.0f)
	{}

	/**
	* @Brief Parameterized constructor for zone bounds creation
	* @inCenter : FVector - Center point in world coordinates
	* @inExtent : FVector - Half-extents on each axis
	* @inHeight : float   - Total zone height (default: 500.0f)
	*/
	FZoneBounds(const FVector& inCenter, const FVector& inExtent, float inHeight = 500.0f)
		: center(inCenter)
		, extent(inExtent)
		, height(inHeight)
	{}

	/**
	* @Brief Generates axis-aligned bounding box from zone bounds
	* @Return : FBox - Bounding box representation of zone boundaries
	*/
	FBox GetBoundingBox() const
	{
		return FBox(center - extent, center + extent);
	}

	/**
	* @Brief Checks if a point is within zone boundaries
	* @inPoint : FVector - World position to test
	* @Return  : bool    - True if point is inside zone bounds
	*/
	bool ContainsPoint(const FVector& inPoint) const
	{
		FBox boundingBox = GetBoundingBox();
		return boundingBox.IsInsideXY(inPoint) &&
			   FMath::Abs(inPoint.Z - center.Z) <= height * 0.5f;
	}
};

/**
* @Brief Pathfinding behavior settings for zone traversal
*
* Configures how pathfinding algorithms behave within a specific zone,
* including movement costs, allowed directions, and grid parameters
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FZonePathSettings
{
	GENERATED_BODY()

public:
	/** @Brief Type of movement allowed in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
	EZonePathType pathType;

	/** @Brief Multiplier for movement cost calculations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float movementCostMultiplier;

	/** @Brief Enable diagonal movement in pathfinding grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
	bool bAllowDiagonalMovement;

	/** @Brief Enable vertical movement for 3D pathfinding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
	bool bAllowVerticalMovement;

	/** @Brief Size of each grid cell for pathfinding discretization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float gridCellSize;

	/** @Brief Actor tags that should block pathfinding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Settings")
	TArray<FName> blockedActorTags;

	/**
	* @Brief Default constructor with standard pathfinding settings
	*/
	FZonePathSettings()
		: pathType(EZonePathType::Ground)
		, movementCostMultiplier(1.0f)
		, bAllowDiagonalMovement(true)
		, bAllowVerticalMovement(false)
		, gridCellSize(100.0f)
	{}
};

/**
* @Brief Connection point for inter-zone pathfinding transitions
*
* Defines entry/exit points between zones for seamless pathfinding
* across multiple zone boundaries
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FZoneConnectionPoint
{
	GENERATED_BODY()

public:
	/** @Brief Target zone identifier for this connection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FZoneIdentifier targetZone;

	/** @Brief Position of connection point in current zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FVector connectionLocation;

	/** @Brief Corresponding position in target zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	FVector targetLocation;

	/** @Brief Cost multiplier for zone transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float transitionCost;

	/** @Brief Allow traversal in both directions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connection")
	bool bBidirectional;

	/**
	* @Brief Default constructor with neutral transition settings
	*/
	FZoneConnectionPoint()
		: connectionLocation(FVector::ZeroVector)
		, targetLocation(FVector::ZeroVector)
		, transitionCost(1.0f)
		, bBidirectional(true)
	{}
};

/**
* @Brief Comprehensive zone level data structure for DataTable integration
*
* Complete zone definition including identity, level references,
* pathfinding settings, and connection information
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FZoneLevelData : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** @Brief Unique identifier for this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Identity")
	FZoneIdentifier zoneIdentifier;

	/** @Brief Localized display name for UI purposes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Identity")
	FText displayName;

	/** @Brief Detailed description of zone purpose and characteristics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Identity", meta = (MultiLine = true))
	FText description;

	/** @Brief Reference to level asset for streaming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Instance")
	TSoftObjectPtr<UWorld> levelAsset;

	/** @Brief World transform for level instance placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Instance")
	FTransform levelTransform;

	/** @Brief Spatial boundaries for zone collision detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Properties")
	FZoneBounds zoneBounds;

	/** @Brief Pathfinding behavior configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Finding")
	FZonePathSettings pathSettings;

	/** @Brief Inter-zone connection points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Connections")
	TArray<FZoneConnectionPoint> connectionPoints;

	/** @Brief Current loading state of zone level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Properties")
	EZoneLoadState loadState;

	/** @Brief Loading priority for resource management */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Properties")
	int32 priority;

	/** @Brief Enable automatic loading based on proximity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Properties")
	bool bAutoLoad;

	/** @Brief Prevent automatic unloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Properties")
	bool bPersistent;

	/** @Brief Distance threshold for automatic loading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Properties")
	float loadDistance;

	/** @Brief Distance threshold for automatic unloading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Properties")
	float unloadDistance;

	/** @Brief Enable debug visualization for this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo;

	/**
	* @Brief Default constructor with standard zone settings
	*/
	FZoneLevelData()
		: levelTransform(FTransform::Identity)
		, loadState(EZoneLoadState::Unloaded)
		, priority(0)
		, bAutoLoad(true)
		, bPersistent(false)
		, loadDistance(2000.0f)
		, unloadDistance(3000.0f)
		, bShowDebugInfo(false)
	{}

	/**
	* @Brief Validates zone data completeness and integrity
	* @Return : bool - True if zone data is valid for use
	*/
	bool IsValid() const
	{
		return zoneIdentifier.IsValid() && !levelAsset.IsNull();
	}

	/**
	* @Brief Generates debug string representation of zone data
	* @Return : FString - Formatted debug information
	*/
	FString GetDebugString() const
	{
		return FString::Printf(TEXT("Zone: %s | Level: %s | State: %s"),
			*zoneIdentifier.ToString(),
			levelAsset.IsValid() ? *levelAsset.GetAssetName() : TEXT("Invalid"),
			*UEnum::GetValueAsString(loadState));
	}

	/**
	* @Brief Determines if zone should be loaded based on player position
	* @inPlayerLocation : FVector - Current player world position
	* @Return           : bool    - True if zone should be loaded
	*/
	bool ShouldLoad(const FVector& inPlayerLocation) const
	{
		if (!bAutoLoad || loadState == EZoneLoadState::Loaded || loadState == EZoneLoadState::Loading)
		{
			return false;
		}

		float distanceToZone = FVector::Dist(inPlayerLocation, zoneBounds.center);
		return distanceToZone <= loadDistance;
	}

	/**
	* @Brief Determines if zone should be unloaded based on player position
	* @inPlayerLocation : FVector - Current player world position
	* @Return           : bool    - True if zone should be unloaded
	*/
	bool ShouldUnload(const FVector& inPlayerLocation) const
	{
		if (!bAutoLoad || bPersistent || loadState != EZoneLoadState::Loaded)
		{
			return false;
		}

		float distanceToZone = FVector::Dist(inPlayerLocation, zoneBounds.center);
		return distanceToZone > unloadDistance;
	}
};