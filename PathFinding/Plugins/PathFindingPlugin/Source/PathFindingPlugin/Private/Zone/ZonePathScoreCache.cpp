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
	FScopeLock Lock(&pathGridMutex);

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
	FScopeLock Lock(&pathGridMutex);

	if (!pathGrid.IsValidGridPosition(inGridPosition))
	{
		return;
	}

	// 스파스 구조에서 노드 찾거나 생성
	FPathNode* nodePtr = pathGrid.pathNodes.Find(inGridPosition);
	if (!nodePtr)
	{
		// 새 노드 생성
		FVector worldPos = pathGrid.GridToWorld(inGridPosition);
		FPathNode newNode(worldPos, inGridPosition);
		nodePtr = &pathGrid.pathNodes.Add(inGridPosition, newNode);
	}

	nodePtr->movementCost = inNewCost;
	nodePtr->bIsBlocked = bBlocked;
	pathGrid.lastUpdateTime = FDateTime::Now();
}

bool UZonePathScoreCache::GetPathNode(const FIntVector& inGridPosition, FPathNode& outPathNode) const
{
	FScopeLock Lock(&pathGridMutex);
	return pathGrid.GetNode(inGridPosition, outPathNode);
}

bool UZonePathScoreCache::IsNodeBlocked(const FIntVector& inGridPosition) const
{
	FScopeLock Lock(&pathGridMutex);

	if (!pathGrid.IsValidGridPosition(inGridPosition))
	{
		return true;
	}

	if (const FPathNode* nodePtr = pathGrid.pathNodes.Find(inGridPosition))
	{
		return nodePtr->bIsBlocked;
	}

	// 스파스 그리드에서 저장되지 않은 기본 노드는 차단되지 않음
	return false;
}

float UZonePathScoreCache::GetMovementCost(const FIntVector& inGridPosition) const
{
	FScopeLock Lock(&pathGridMutex);

	if (!pathGrid.IsValidGridPosition(inGridPosition))
	{
		return FLT_MAX;
	}

	if (const FPathNode* nodePtr = pathGrid.pathNodes.Find(inGridPosition))
	{
		return nodePtr->movementCost;
	}

	// 스파스 그리드에서 저장되지 않은 기본 노드의 기본 비용
	return 1.0f;
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
	int32 totalCheckedNodes = 0;

	// 스파스 그리드: 샘플링을 통해 필요한 노드만 검사하여 메모리 절약
	const int32 sampleStep = FMath::Max(1, pathGrid.gridDimensions.X / 50); // 적응적 샘플링

	for (int32 z = 0; z < pathGrid.gridDimensions.Z; z += sampleStep)
	{
		for (int32 y = 0; y < pathGrid.gridDimensions.Y; y += sampleStep)
		{
			for (int32 x = 0; x < pathGrid.gridDimensions.X; x += sampleStep)
			{
				FIntVector gridPos(x, y, z);
				FVector worldPos = pathGrid.GridToWorld(gridPos);
				totalCheckedNodes++;

				// 지면 체크를 위한 라인 트레이스
				FVector traceStart = worldPos + FVector(0, 0, 500.0f);
				FVector traceEnd = worldPos - FVector(0, 0, 500.0f);

				FHitResult hitResult;
				bool bHit = GetWorld()->LineTraceSingleByChannel(
					hitResult,
					traceStart,
					traceEnd,
					ECC_WorldStatic,
					queryParams
				);

				// 충돌 또는 특별한 상황이 있는 경우에만 노드를 스파스 맵에 저장
				bool bShouldStoreNode = false;
				FPathNode newNode(worldPos, gridPos);

				if (bHit)
				{
					newNode.worldPosition.Z = hitResult.Location.Z;

					// 블록된 액터 태그 체크
					if (hitResult.GetActor())
					{
						for (const FName& blockedTag : inZoneData.pathSettings.blockedActorTags)
						{
							if (hitResult.GetActor()->ActorHasTag(blockedTag))
							{
								newNode.bIsBlocked = true;
								blockedNodes++;
								bShouldStoreNode = true;
								break;
							}
						}
					}

					// 경사도 체크
					FVector surfaceNormal = hitResult.Normal;
					float slopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(surfaceNormal, FVector::UpVector)));

					if (slopeAngle > 45.0f)
					{
						newNode.movementCost *= 2.0f;
						bShouldStoreNode = true;
					}
					if (slopeAngle > 60.0f)
					{
						newNode.bIsBlocked = true;
						blockedNodes++;
						bShouldStoreNode = true;
					}
				}
				else
				{
					// 지면이 없는 경우
					if (inZoneData.pathSettings.pathType != EZonePathType::Air &&
						inZoneData.pathSettings.pathType != EZonePathType::Mixed)
					{
						newNode.bIsBlocked = true;
						blockedNodes++;
						bShouldStoreNode = true;
					}
				}

				// Zone 경계 체크
				if (!inZoneData.zoneBounds.ContainsPoint(newNode.worldPosition))
				{
					newNode.bIsBlocked = true;
					blockedNodes++;
					bShouldStoreNode = true;
				}

				// 기본값이 아닌 경우에만 스파스 맵에 저장
				if (bShouldStoreNode || newNode.movementCost != 1.0f)
				{
					newNode.movementCost *= inZoneData.pathSettings.movementCostMultiplier;
					pathGrid.pathNodes.Add(gridPos, newNode);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Sparse collision check completed: %d blocked nodes, %d stored nodes out of %d checked (%d total possible)"),
		blockedNodes, pathGrid.pathNodes.Num(), totalCheckedNodes,
		pathGrid.gridDimensions.X * pathGrid.gridDimensions.Y * pathGrid.gridDimensions.Z);
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

		// 스파스 구조에서 연결점 노드 생성 또는 업데이트
		FPathNode* nodePtr = pathGrid.pathNodes.Find(gridPos);
		if (!nodePtr)
		{
			// 새 연결점 노드 생성
			FVector worldPos = pathGrid.GridToWorld(gridPos);
			FPathNode newNode(worldPos, gridPos);
			nodePtr = &pathGrid.pathNodes.Add(gridPos, newNode);
		}

		nodePtr->bIsConnectionPoint = true;
		nodePtr->connectedZone = connectionPoint.targetZone;
		nodePtr->movementCost = connectionPoint.transitionCost;

		UE_LOG(LogTemp, Log, TEXT("Connection point set up at %s connecting to %s"),
			*gridPos.ToString(), *connectionPoint.targetZone.ToString());
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
	if (!IsValidGridPosition(inGridPos))
	{
		return false;
	}

	if (const FPathNode* nodePtr = pathNodes.Find(inGridPos))
	{
		outNode = *nodePtr;
		return true;
	}

	// 기본 노드 반환 (스파스 그리드에서 저장되지 않은 기본값)
	FVector worldPos = GridToWorld(inGridPos);
	outNode = FPathNode(worldPos, inGridPos);
	return true;
}

bool FZonePathGrid::GetNodeMutable(const FIntVector& inGridPos, FPathNode& outNode)
{
	if (!IsValidGridPosition(inGridPos))
	{
		return false;
	}

	if (FPathNode* nodePtr = pathNodes.Find(inGridPos))
	{
		outNode = *nodePtr;
		return true;
	}

	// 기본 노드 생성 및 반환
	FVector worldPos = GridToWorld(inGridPos);
	outNode = FPathNode(worldPos, inGridPos);
	pathNodes.Add(inGridPos, outNode);
	return true;
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

	// 스파스 그리드 초기화 - 기본 노드들은 필요할 때만 생성
	pathNodes.Empty();
	pathNodes.Reserve(FMath::Min(1000, gridDimensions.X * gridDimensions.Y)); // 예상 활성 노드 수

	bIsGenerated = true;
	lastUpdateTime = FDateTime::Now();

	UE_LOG(LogTemp, Log, TEXT("Sparse grid initialized with capacity for %d nodes (potential %d total)"),
		pathNodes.GetAllocatedSize(), gridDimensions.X * gridDimensions.Y * gridDimensions.Z);
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