#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/ActorComponent.h"
#include "HAL/CriticalSection.h"
#include "Zone/ZoneTypes.h"
#include "Zone/ZoneLevelData.h"
#include "ZonePathScoreCache.generated.h"

#pragma region Forward_Declare
class UDataTable;
class UWorld;
struct FZoneLevelData;
#pragma endregion Forward_Declare

/**
* @Brief Individual pathfinding node data structure
*
* Represents a single node in the pathfinding grid with position,
* cost, and connectivity information for pathfinding algorithms
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FPathNode
{
	GENERATED_BODY()

public:
	/** @Brief World space position of this path node */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	FVector worldPosition;

	/** @Brief Grid coordinates for array indexing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	FIntVector gridPosition;

	/** @Brief Movement cost for traversing this node */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	float movementCost;

	/** @Brief Whether this node blocks movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	bool bIsBlocked;

	/** @Brief Whether this node connects to another zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	bool bIsConnectionPoint;

	/** @Brief Target zone if this is a connection point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Node")
	FZoneIdentifier connectedZone;

	/**
	* @Brief Default constructor with safe default values
	*/
	FPathNode()
		: worldPosition(FVector::ZeroVector)
		, gridPosition(FIntVector::ZeroValue)
		, movementCost(1.0f)
		, bIsBlocked(false)
		, bIsConnectionPoint(false)
	{}

	/**
	* @Brief Parameterized constructor for path node creation
	* @inWorldPos : FVector    - World space position
	* @inGridPos  : FIntVector - Grid coordinates
	* @inCost     : float      - Movement cost (default: 1.0f)
	*/
	FPathNode(const FVector& inWorldPos, const FIntVector& inGridPos, float inCost = 1.0f)
		: worldPosition(inWorldPos)
		, gridPosition(inGridPos)
		, movementCost(inCost)
		, bIsBlocked(false)
		, bIsConnectionPoint(false)
	{}

	/**
	* @Brief Validates node for pathfinding use
	* @Return : bool - True if node is traversable
	*/
	bool IsValid() const
	{
		return !bIsBlocked && movementCost > 0.0f;
	}

	/**
	* @Brief Generates debug string representation
	* @Return : FString - Formatted node information
	*/
	FString ToString() const
	{
		return FString::Printf(TEXT("Node[%d,%d,%d] Cost:%.2f %s"),
			gridPosition.X, gridPosition.Y, gridPosition.Z,
			movementCost,
			bIsBlocked ? TEXT("BLOCKED") : TEXT("OPEN"));
	}
};

/**
* @Brief Zone-specific pathfinding grid data structure
*
* Contains complete pathfinding grid for a single zone including
* nodes, dimensions, and spatial mapping information
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FZonePathGrid
{
	GENERATED_BODY()

public:
	/** @Brief Zone identifier this grid belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Grid")
	FZoneIdentifier zoneIdentifier;

	/** @Brief Grid dimensions in X, Y, Z axes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Grid")
	FIntVector gridDimensions;

	/** @Brief Size of each grid cell in world units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Grid")
	float cellSize;

	/** @Brief World position of grid origin (minimum corner) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Grid")
	FVector gridOrigin;

	/** @Brief Sparse map of path nodes (only stores non-default nodes for memory efficiency) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Grid")
	TMap<FIntVector, FPathNode> pathNodes;

	/** @Brief Whether grid has been successfully generated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Grid")
	bool bIsGenerated;

	/** @Brief Timestamp of last grid update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Grid")
	FDateTime lastUpdateTime;

	/**
	* @Brief Default constructor with empty grid state
	*/
	FZonePathGrid()
		: gridDimensions(FIntVector::ZeroValue)
		, cellSize(100.0f)
		, gridOrigin(FVector::ZeroVector)
		, bIsGenerated(false)
		, lastUpdateTime(FDateTime::Now())
	{}

	/**
	* @Brief Converts 3D grid position to linear array index
	* @inGridPos : FIntVector - Grid coordinates to convert
	* @Return    : int32      - Array index or INDEX_NONE if invalid
	*/
	int32 GetNodeIndex(const FIntVector& inGridPos) const;

	/**
	* @Brief Converts world position to grid coordinates
	* @inWorldPos : FVector    - World space position
	* @Return     : FIntVector - Corresponding grid coordinates
	*/
	FIntVector WorldToGrid(const FVector& inWorldPos) const;

	/**
	* @Brief Converts grid coordinates to world position
	* @inGridPos : FIntVector - Grid coordinates
	* @Return    : FVector    - Corresponding world space position
	*/
	FVector GridToWorld(const FIntVector& inGridPos) const;

	/**
	* @Brief Gets read-only access to path node at grid position
	* @inGridPos : FIntVector  - Grid coordinates
	* @outNode   : FPathNode& - Output parameter for node data
	* @Return    : bool        - True if node exists and is valid
	*/
	bool GetNode(const FIntVector& inGridPos, FPathNode& outNode) const;

	/**
	* @Brief Gets mutable access to path node at grid position
	* @inGridPos : FIntVector  - Grid coordinates
	* @outNode   : FPathNode& - Output parameter for node data
	* @Return    : bool        - True if node exists and is valid
	*/
	bool GetNodeMutable(const FIntVector& inGridPos, FPathNode& outNode);

	/**
	* @Brief Initializes grid with zone bounds and cell size
	* @inZoneBounds : FZoneBounds - Zone spatial boundaries
	* @inCellSize   : float       - Size of each grid cell
	*/
	void InitializeGrid(const FZoneBounds& inZoneBounds, float inCellSize);

	/**
	* @Brief Validates grid position bounds
	* @inGridPos : FIntVector - Grid coordinates to validate
	* @Return    : bool       - True if position is within grid bounds
	*/
	bool IsValidGridPosition(const FIntVector& inGridPos) const;

	/**
	* @Brief Gets neighboring grid positions for pathfinding
	* @inGridPos          : FIntVector - Center grid position
	* @bIncludeDiagonals  : bool       - Include diagonal neighbors (default: true)
	* @Return             : TArray     - Array of valid neighbor positions
	*/
	TArray<FIntVector> GetNeighbors(const FIntVector& inGridPos, bool bIncludeDiagonals = true) const;
};

/**
* @Brief Zone-specific path score caching system
*
* Manages pathfinding grid generation, caching, and persistence
* for a single zone with collision detection and file I/O support
*/
UCLASS(BlueprintType, ClassGroup=(PathFinding))
class PATHFINDINGPLUGIN_API UZonePathScoreCache : public UObject
{
	GENERATED_BODY()

public:
	/**
	* @Brief Default constructor with auto-save configuration
	*/
	UZonePathScoreCache();

	/**
	* @Brief Initializes zone cache with zone data
	* @inZoneData : FZoneLevelData - Zone configuration data
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	void InitializeZoneCache(const FZoneLevelData& inZoneData);

	/**
	* @Brief Generates pathfinding grid for zone
	* @inZoneData : FZoneLevelData - Zone configuration data
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	void GeneratePathGrid(const FZoneLevelData& inZoneData);

	/**
	* @Brief Updates individual path node properties
	* @inGridPosition : FIntVector - Grid coordinates of node
	* @inNewCost      : float      - New movement cost
	* @bBlocked       : bool       - Whether node should be blocked (default: false)
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	void UpdatePathNode(const FIntVector& inGridPosition, float inNewCost, bool bBlocked = false);

	/**
	* @Brief Gets read-only access to path node
	* @inGridPosition : FIntVector  - Grid coordinates
	* @outPathNode    : FPathNode& - Output parameter for node data
	* @Return         : bool        - True if node exists and is valid
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	bool GetPathNode(const FIntVector& inGridPosition, FPathNode& outPathNode) const;

	/**
	* @Brief Checks if node is blocked for pathfinding
	* @inGridPosition : FIntVector - Grid coordinates
	* @Return         : bool       - True if node blocks movement
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	bool IsNodeBlocked(const FIntVector& inGridPosition) const;

	/**
	* @Brief Gets movement cost for specific grid position
	* @inGridPosition : FIntVector - Grid coordinates
	* @Return         : float      - Movement cost or FLT_MAX if invalid
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	float GetMovementCost(const FIntVector& inGridPosition) const;

	/**
	* @Brief Gets neighboring nodes for pathfinding algorithms
	* @inGridPosition     : FIntVector - Center grid position
	* @bIncludeDiagonals  : bool       - Include diagonal neighbors (default: true)
	* @Return             : TArray     - Array of neighbor grid positions
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	TArray<FIntVector> GetNeighborNodes(const FIntVector& inGridPosition, bool bIncludeDiagonals = true) const;

	/**
	* @Brief Converts world position to grid coordinates
	* @inWorldPosition : FVector    - World space position
	* @Return          : FIntVector - Corresponding grid coordinates
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	FIntVector WorldToGridPosition(const FVector& inWorldPosition) const;

	/**
	* @Brief Converts grid coordinates to world position
	* @inGridPosition : FIntVector - Grid coordinates
	* @Return         : FVector    - Corresponding world space position
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	FVector GridToWorldPosition(const FIntVector& inGridPosition) const;

	/**
	* @Brief Clears all cached path data
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	void ClearCache();

	/**
	* @Brief Checks if pathfinding grid is ready for use
	* @Return : bool - True if grid has been generated
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	bool IsGridGenerated() const { return pathGrid.bIsGenerated; }

	/**
	* @Brief Gets zone identifier for this cache
	* @Return : FZoneIdentifier - Zone identifier reference
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	const FZoneIdentifier& GetZoneIdentifier() const { return pathGrid.zoneIdentifier; }

	/**
	* @Brief Gets timestamp of last cache update
	* @Return : FDateTime - Last update time
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	FDateTime GetLastUpdateTime() const { return pathGrid.lastUpdateTime; }

	/**
	* @Brief Saves cache data to file for persistence
	* @inFilePath : FString - File path for cache storage
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	void SaveCacheToFile(const FString& inFilePath);

	/**
	* @Brief Loads cache data from file
	* @inFilePath : FString - File path for cache loading
	* @Return     : bool    - True if loading succeeded
	*/
	UFUNCTION(BlueprintCallable, Category = "Zone Path Cache")
	bool LoadCacheFromFile(const FString& inFilePath);

protected:
	/** @Brief Main pathfinding grid data structure */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zone Path Cache")
	FZonePathGrid pathGrid;

	/** @Brief Enable automatic cache saving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Path Cache")
	bool bAutoSave;

	/** @Brief Interval for automatic cache saves in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone Path Cache")
	float autoSaveInterval;

	/** @Brief Timer handle for auto-save functionality */
	FTimerHandle autoSaveTimerHandle;

	/** @Brief Thread safety mutex for PathGrid operations */
	mutable FCriticalSection pathGridMutex;

	/**
	* @Brief Performs collision checking for pathfinding grid
	* @inZoneData : FZoneLevelData - Zone configuration for collision rules
	*/
	void PerformCollisionCheck(const FZoneLevelData& inZoneData);

	/**
	* @Brief Sets up inter-zone connection points
	* @inZoneData : FZoneLevelData - Zone data with connection information
	*/
	void SetupConnectionPoints(const FZoneLevelData& inZoneData);

	/**
	* @Brief Performs automatic cache saving
	*/
	void AutoSaveCache();
};