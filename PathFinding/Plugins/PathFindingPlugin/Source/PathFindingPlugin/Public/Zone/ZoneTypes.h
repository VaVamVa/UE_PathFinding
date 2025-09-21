#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "ZoneTypes.generated.h"

/**
* @Brief Zone classification enumeration for different area types
*
* Defines various zone types that can exist within a game world,
* each with different gameplay characteristics and pathfinding behaviors
*/
UENUM(BlueprintType)
enum class EZoneType : uint8
{
	None			UMETA(DisplayName = "None"),
	Neutral			UMETA(DisplayName = "Neutral Zone"),
	War				UMETA(DisplayName = "War Zone"),
	Farming			UMETA(DisplayName = "Farming Zone"),
	Trading			UMETA(DisplayName = "Trading Zone"),
	Dungeon			UMETA(DisplayName = "Dungeon Zone"),
	SafeHouse		UMETA(DisplayName = "Safe House Zone"),
	PvP				UMETA(DisplayName = "PvP Zone"),
	Custom			UMETA(DisplayName = "Custom Zone")
};

/**
* @Brief Path traversal type enumeration for movement capabilities
*
* Specifies the type of movement allowed within a zone,
* affecting pathfinding algorithm selection and cost calculation
*/
UENUM(BlueprintType)
enum class EZonePathType : uint8
{
	Ground			UMETA(DisplayName = "Ground Path"),
	Air				UMETA(DisplayName = "Air Path"),
	Water			UMETA(DisplayName = "Water Path"),
	Underground		UMETA(DisplayName = "Underground Path"),
	Mixed			UMETA(DisplayName = "Mixed Path")
};

/**
* @Brief Zone loading state enumeration for level streaming management
*
* Tracks the current loading state of a zone's level instance,
* used for proper resource management and loading synchronization
*/
UENUM(BlueprintType)
enum class EZoneLoadState : uint8
{
	Unloaded		UMETA(DisplayName = "Unloaded"),
	Loading			UMETA(DisplayName = "Loading"),
	Loaded			UMETA(DisplayName = "Loaded"),
	Unloading		UMETA(DisplayName = "Unloading"),
	Error			UMETA(DisplayName = "Error")
};

/**
* @Brief Zone identification structure for unique zone referencing
*
* Provides a comprehensive identification system for zones,
* combining name, type, and ID for robust zone management
*/
USTRUCT(BlueprintType)
struct PATHFINDINGPLUGIN_API FZoneIdentifier
{
	GENERATED_BODY()

public:
	/** @Brief Human-readable zone name for identification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	FName zoneName;

	/** @Brief Zone type classification for gameplay behavior */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	EZoneType zoneType;

	/** @Brief Unique numerical identifier for zone instances */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
	int32 zoneID;

	/**
	* @Brief Default constructor initializing to invalid state
	*/
	FZoneIdentifier()
		: zoneName(NAME_None)
		, zoneType(EZoneType::None)
		, zoneID(-1)
	{}

	/**
	* @Brief Parameterized constructor for zone identifier creation
	* @inZoneName  : FName     - Human-readable zone name
	* @inZoneType  : EZoneType - Zone classification type
	* @inZoneID    : int32     - Unique numerical identifier
	*/
	FZoneIdentifier(const FName& inZoneName, EZoneType inZoneType, int32 inZoneID)
		: zoneName(inZoneName)
		, zoneType(inZoneType)
		, zoneID(inZoneID)
	{}

	/**
	* @Brief Validates zone identifier completeness
	* @Return : bool - True if all required fields are properly set
	*/
	bool IsValid() const
	{
		return zoneName != NAME_None && zoneType != EZoneType::None && zoneID >= 0;
	}

	/**
	* @Brief Equality comparison operator for zone identifiers
	* @inOther : FZoneIdentifier - Other zone identifier to compare
	* @Return  : bool            - True if identifiers match completely
	*/
	bool operator==(const FZoneIdentifier& inOther) const
	{
		return zoneName == inOther.zoneName && zoneType == inOther.zoneType && zoneID == inOther.zoneID;
	}

	/**
	* @Brief Inequality comparison operator for zone identifiers
	* @inOther : FZoneIdentifier - Other zone identifier to compare
	* @Return  : bool            - True if identifiers differ
	*/
	bool operator!=(const FZoneIdentifier& inOther) const
	{
		return !(*this == inOther);
	}

	/**
	* @Brief Generates human-readable string representation
	* @Return : FString - Formatted string containing zone information
	*/
	FString ToString() const
	{
		return FString::Printf(TEXT("[%s_%d]%s"),
			*UEnum::GetValueAsString(zoneType), zoneID, *zoneName.ToString());
	}
};

/**
* @Brief Hash function for FZoneIdentifier to enable use in hash-based containers
* @inZoneID : FZoneIdentifier - Zone identifier to hash
* @Return   : uint32          - Hash value for the zone identifier
*/
FORCEINLINE uint32 GetTypeHash(const FZoneIdentifier& inZoneID)
{
	return HashCombine(
		HashCombine(GetTypeHash(inZoneID.zoneName), GetTypeHash(inZoneID.zoneType)),
		GetTypeHash(inZoneID.zoneID)
	);
}