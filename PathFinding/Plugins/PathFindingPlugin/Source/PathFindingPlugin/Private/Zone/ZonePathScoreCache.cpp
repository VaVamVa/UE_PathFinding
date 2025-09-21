#include "Zone/ZonePathScoreCache.h"

#include "JsonObjectConverter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "TimerManager.h"

UZonePathScoreCache::UZonePathScoreCache()
{
	bAutoSave = true;
	autoSaveInterval = 300.0f; // 5분/*modify_250921_: 자동 저장 간격을 5분으로 설정*/
}

void UZonePathScoreCache::InitializeZoneCache(const FZoneLevelData& inZoneData)
{
	if (!inZoneData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UZonePathScoreCache::InitializeZoneCache - Invalid ZoneData"));
		return;
	}

	pathGrid.zoneIdentifier = inZoneData.zoneIdentifier;
	GeneratePathGrid(inZoneData);

	// 자동 저장 타이머 설정
	if (bAutoSave && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			autoSaveTimerHandle,
			this,
			&UZonePathScoreCache::AutoSaveCache,
			autoSaveInterval,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("Zone Path Cache initialized for %s"), *inZoneData.zoneIdentifier.ToString());
}

void UZonePathScoreCache::GeneratePathGrid(const FZoneLevelData& inZoneData)
{
	if (!inZoneData.IsValid())
	{
		return;
	}

	pathGrid.InitializeGrid(inZoneData.zoneBounds, inZoneData.pathSettings.gridCellSize);

	PerformCollisionCheck(inZoneData);
	SetupConnectionPoints(inZoneData);

	UE_LOG(LogTemp, Log, TEXT("Path grid generated: %dx%dx%d cells for zone %s"),
		pathGrid.gridDimensions.X, pathGrid.gridDimensions.Y, pathGrid.gridDimensions.Z,
		*inZoneData.zoneIdentifier.ToString());
}

void UZonePathScoreCache::UpdatePathNode(const FIntVector& inGridPosition, float inNewCost, bool bBlocked)
{
	FPathNode node;
	if (pathGrid.GetNodeMutable(inGridPosition, node))
	{
		node.movementCost = inNewCost;
		node.bIsBlocked = bBlocked;

		// 변경된 노드를 다시 배열에 저장
		int32 index = pathGrid.GetNodeIndex(inGridPosition);
		if (index != INDEX_NONE && pathGrid.pathNodes.IsValidIndex(index))
		{
			pathGrid.pathNodes[index] = node;
		}

		pathGrid.lastUpdateTime = FDateTime::Now();
	}
}

bool UZonePathScoreCache::GetPathNode(const FIntVector& inGridPosition, FPathNode& outPathNode) const
{
	return pathGrid.GetNode(inGridPosition, outPathNode);
}

bool UZonePathScoreCache::IsNodeBlocked(const FIntVector& inGridPosition) const
{
	FPathNode node;
	if (GetPathNode(inGridPosition, node))
	{
		return node.bIsBlocked;
	}
	return true;
}

float UZonePathScoreCache::GetMovementCost(const FIntVector& inGridPosition) const
{
	FPathNode node;
	if (GetPathNode(inGridPosition, node))
	{
		return node.movementCost;
	}
	return FLT_MAX;
}

TArray<FIntVector> UZonePathScoreCache::GetNeighborNodes(const FIntVector& inGridPosition, bool bIncludeDiagonals) const
{
	return pathGrid.GetNeighbors(inGridPosition, bIncludeDiagonals);
}

FIntVector UZonePathScoreCache::WorldToGridPosition(const FVector& inWorldPosition) const
{
	return pathGrid.WorldToGrid(inWorldPosition);
}

FVector UZonePathScoreCache::GridToWorldPosition(const FIntVector& inGridPosition) const
{
	return pathGrid.GridToWorld(inGridPosition);
}

void UZonePathScoreCache::ClearCache()
{
	pathGrid = FZonePathGrid();

	if (GetWorld() && autoSaveTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(autoSaveTimerHandle);
	}
}

void UZonePathScoreCache::SaveCacheToFile(const FString& inFilePath)
{
	FString jsonString;
	if (FJsonObjectConverter::UStructToJsonObjectString(pathGrid, jsonString))
	{
		if (!FFileHelper::SaveStringToFile(jsonString, *inFilePath))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save zone path cache to file: %s"), *inFilePath);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Zone path cache saved to: %s"), *inFilePath);
		}
	}
}

bool UZonePathScoreCache::LoadCacheFromFile(const FString& inFilePath)
{
	FString jsonString;
	if (FFileHelper::LoadFileToString(jsonString, *inFilePath))
	{
		if (FJsonObjectConverter::JsonObjectStringToUStruct(jsonString, &pathGrid))
		{
			UE_LOG(LogTemp, Log, TEXT("Zone path cache loaded from: %s"), *inFilePath);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to load zone path cache from file: %s"), *inFilePath);
	return false;
}

void UZonePathScoreCache::PerformCollisionCheck(const FZoneLevelData& inZoneData)
{
	if (!GetWorld())
	{
		return;
	}

	// 충돌 체크를 위한 설정
	FCollisionQueryParams queryParams;
	queryParams.bTraceComplex = false;
	queryParams.bReturnPhysicalMaterial = false;

	int32 blockedNodes = 0;
	const float halfCellSize = pathGrid.cellSize * 0.5f;

	for (int32 i = 0; i < pathGrid.pathNodes.Num(); ++i)
	{
		FPathNode& node = pathGrid.pathNodes[i];

		// 지면 체크를 위한 라인 트레이스
		FVector traceStart = node.worldPosition + FVector(0, 0, 500.0f);
		FVector traceEnd = node.worldPosition - FVector(0, 0, 500.0f);

		FHitResult hitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			hitResult,
			traceStart,
			traceEnd,
			ECC_WorldStatic,
			queryParams
		);

		if (bHit)
		{
			node.worldPosition.Z = hitResult.Location.Z;

			// 블록된 액터 태그 체크
			if (hitResult.GetActor())
			{
				for (const FName& blockedTag : inZoneData.pathSettings.blockedActorTags)
				{
					if (hitResult.GetActor()->ActorHasTag(blockedTag))
					{
						node.bIsBlocked = true;
						blockedNodes++;
						break;
					}
				}
			}

			// 경사도 체크 (너무 가파른 경사는 이동 불가)
			FVector surfaceNormal = hitResult.Normal;
			float slopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(surfaceNormal, FVector::UpVector)));

			if (slopeAngle > 45.0f) // 45도 이상 경사/*modify_250921_: 경사 임계값 설정*/
			{
				node.movementCost *= 2.0f; // 이동 비용 증가
			}
			if (slopeAngle > 60.0f) // 60도 이상은 차단/*modify_250921_: 차단 경사 임계값 설정*/
			{
				node.bIsBlocked = true;
				blockedNodes++;
			}
		}
		else
		{
			// 지면이 없는 경우 공중 노드로 처리
			if (inZoneData.pathSettings.pathType != EZonePathType::Air &&
				inZoneData.pathSettings.pathType != EZonePathType::Mixed)
			{
				node.bIsBlocked = true;
				blockedNodes++;
			}
		}

		// Zone 경계 체크
		if (!inZoneData.zoneBounds.ContainsPoint(node.worldPosition))
		{
			node.bIsBlocked = true;
			blockedNodes++;
		}

		// 이동 비용에 Zone 설정 반영
		node.movementCost *= inZoneData.pathSettings.movementCostMultiplier;
	}

	UE_LOG(LogTemp, Log, TEXT("Collision check completed: %d blocked nodes out of %d total nodes"),
		blockedNodes, pathGrid.pathNodes.Num());
}

void UZonePathScoreCache::SetupConnectionPoints(const FZoneLevelData& inZoneData)
{
	for (const FZoneConnectionPoint& connectionPoint : inZoneData.connectionPoints)
	{
		if (!connectionPoint.targetZone.IsValid())
		{
			continue;
		}

		FIntVector gridPos = pathGrid.WorldToGrid(connectionPoint.connectionLocation);
		FPathNode node;

		if (pathGrid.GetNodeMutable(gridPos, node))
		{
			node.bIsConnectionPoint = true;
			node.connectedZone = connectionPoint.targetZone;
			node.movementCost = connectionPoint.transitionCost;

			// 변경된 노드를 다시 배열에 저장
			int32 index = pathGrid.GetNodeIndex(gridPos);
			if (index != INDEX_NONE && pathGrid.pathNodes.IsValidIndex(index))
			{
				pathGrid.pathNodes[index] = node;
			}

			UE_LOG(LogTemp, Log, TEXT("Connection point set up at %s connecting to %s"),
				*gridPos.ToString(), *connectionPoint.targetZone.ToString());
		}
	}
}

void UZonePathScoreCache::AutoSaveCache()
{
	if (!pathGrid.bIsGenerated)
	{
		return;
	}

	FString saveDir = FPaths::ProjectSavedDir() / TEXT("PathFindingCache");
	IFileManager::Get().MakeDirectory(*saveDir, true);

	FString fileName = FString::Printf(TEXT("%s_%d.json"),
		*pathGrid.zoneIdentifier.zoneName.ToString(),
		pathGrid.zoneIdentifier.zoneID);
	FString filePath = saveDir / fileName;

	SaveCacheToFile(filePath);
}

// FZonePathGrid 구현부
int32 FZonePathGrid::GetNodeIndex(const FIntVector& inGridPos) const
{
	if (inGridPos.X < 0 || inGridPos.X >= gridDimensions.X ||
		inGridPos.Y < 0 || inGridPos.Y >= gridDimensions.Y ||
		inGridPos.Z < 0 || inGridPos.Z >= gridDimensions.Z)
	{
		return INDEX_NONE;
	}

	return inGridPos.X + (inGridPos.Y * gridDimensions.X) + (inGridPos.Z * gridDimensions.X * gridDimensions.Y);
}

FIntVector FZonePathGrid::WorldToGrid(const FVector& inWorldPos) const
{
	FVector relativePos = inWorldPos - gridOrigin;
	return FIntVector(
		FMath::FloorToInt(relativePos.X / cellSize),
		FMath::FloorToInt(relativePos.Y / cellSize),
		FMath::FloorToInt(relativePos.Z / cellSize)
	);
}

FVector FZonePathGrid::GridToWorld(const FIntVector& inGridPos) const
{
	return gridOrigin + FVector(
		inGridPos.X * cellSize + cellSize * 0.5f,
		inGridPos.Y * cellSize + cellSize * 0.5f,
		inGridPos.Z * cellSize + cellSize * 0.5f
	);
}

bool FZonePathGrid::GetNode(const FIntVector& inGridPos, FPathNode& outNode) const
{
	int32 index = GetNodeIndex(inGridPos);
	if (index != INDEX_NONE && pathNodes.IsValidIndex(index))
	{
		outNode = pathNodes[index];
		return true;
	}
	return false;
}

bool FZonePathGrid::GetNodeMutable(const FIntVector& inGridPos, FPathNode& outNode)
{
	int32 index = GetNodeIndex(inGridPos);
	if (index != INDEX_NONE && pathNodes.IsValidIndex(index))
	{
		outNode = pathNodes[index];
		return true;
	}
	return false;
}

void FZonePathGrid::InitializeGrid(const FZoneBounds& inZoneBounds, float inCellSize)
{
	cellSize = inCellSize;
	gridOrigin = inZoneBounds.center - inZoneBounds.extent;

	FVector gridSize = inZoneBounds.extent * 2.0f;
	gridDimensions = FIntVector(
		FMath::CeilToInt(gridSize.X / cellSize),
		FMath::CeilToInt(gridSize.Y / cellSize),
		FMath::CeilToInt(inZoneBounds.height / cellSize)
	);

	int32 totalNodes = gridDimensions.X * gridDimensions.Y * gridDimensions.Z;
	pathNodes.Reset(totalNodes);
	pathNodes.SetNum(totalNodes);

	// 노드 초기화
	for (int32 z = 0; z < gridDimensions.Z; ++z)
	{
		for (int32 y = 0; y < gridDimensions.Y; ++y)
		{
			for (int32 x = 0; x < gridDimensions.X; ++x)
			{
				FIntVector gridPos(x, y, z);
				FVector worldPos = GridToWorld(gridPos);
				int32 index = GetNodeIndex(gridPos);

				if (pathNodes.IsValidIndex(index))
				{
					pathNodes[index] = FPathNode(worldPos, gridPos);
				}
			}
		}
	}

	bIsGenerated = true;
	lastUpdateTime = FDateTime::Now();
}

bool FZonePathGrid::IsValidGridPosition(const FIntVector& inGridPos) const
{
	return inGridPos.X >= 0 && inGridPos.X < gridDimensions.X &&
		   inGridPos.Y >= 0 && inGridPos.Y < gridDimensions.Y &&
		   inGridPos.Z >= 0 && inGridPos.Z < gridDimensions.Z;
}

TArray<FIntVector> FZonePathGrid::GetNeighbors(const FIntVector& inGridPos, bool bIncludeDiagonals) const
{
	TArray<FIntVector> neighbors;

	// 직접 인접한 이웃들
	static const TArray<FIntVector> directNeighbors = {
		FIntVector(1, 0, 0), FIntVector(-1, 0, 0),
		FIntVector(0, 1, 0), FIntVector(0, -1, 0),
		FIntVector(0, 0, 1), FIntVector(0, 0, -1)
	};

	// 대각선 이웃들
	static const TArray<FIntVector> diagonalNeighbors = {
		FIntVector(1, 1, 0), FIntVector(1, -1, 0),
		FIntVector(-1, 1, 0), FIntVector(-1, -1, 0),
		FIntVector(1, 0, 1), FIntVector(1, 0, -1),
		FIntVector(-1, 0, 1), FIntVector(-1, 0, -1),
		FIntVector(0, 1, 1), FIntVector(0, 1, -1),
		FIntVector(0, -1, 1), FIntVector(0, -1, -1)
	};

	// 직접 이웃 추가
	for (const FIntVector& offset : directNeighbors)
	{
		FIntVector neighborPos = inGridPos + offset;
		if (IsValidGridPosition(neighborPos))
		{
			neighbors.Add(neighborPos);
		}
	}

	// 대각선 이웃 추가 (옵션)
	if (bIncludeDiagonals)
	{
		for (const FIntVector& offset : diagonalNeighbors)
		{
			FIntVector neighborPos = inGridPos + offset;
			if (IsValidGridPosition(neighborPos))
			{
				neighbors.Add(neighborPos);
			}
		}
	}

	return neighbors;
}