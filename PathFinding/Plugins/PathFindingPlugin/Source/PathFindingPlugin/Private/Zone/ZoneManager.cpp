#include "Zone/ZoneManager.h"
#include "Zone/ZoneLevelInstanceComponent.h"
#include "Zone/ZonePathScoreCache.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

AZoneManager::AZoneManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

	bIsInitialized = false;
	playerPosition = FVector::ZeroVector;
}

void AZoneManager::BeginPlay()
{
	Super::BeginPlay();

	// 기본 데이터 테이블이 설정되어 있다면 자동 초기화
	if (defaultZoneDataTable)
	{
		InitializeFromDataTable(defaultZoneDataTable);
	}

	// 자동 로딩 타이머 설정
	if (settings.bAutoLoadZones)
	{
		GetWorldTimerManager().SetTimer(
			autoLoadTimerHandle,
			[this]() { UpdateAutoLoading(playerPosition); },
			settings.autoLoadCheckInterval,
			true
		);
	}

	// 캐시 저장 타이머 설정
	if (settings.bSaveZoneCachesToDisk)
	{
		GetWorldTimerManager().SetTimer(
			cacheSaveTimerHandle,
			this,
			&AZoneManager::SaveAllZoneCaches,
			settings.cacheSaveInterval,
			true
		);
	}
}

void AZoneManager::EndPlay(const EEndPlayReason::Type inEndPlayReason)
{
	// 모든 타이머 정리
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(autoLoadTimerHandle);
		GetWorldTimerManager().ClearTimer(cacheSaveTimerHandle);
	}

	// Zone 캐시 저장
	if (settings.bSaveZoneCachesToDisk)
	{
		SaveAllZoneCaches();
	}

	// Zone 컴포넌트 정리
	CleanupZoneComponents();

	Super::EndPlay(inEndPlayReason);
}

void AZoneManager::Tick(float inDeltaTime)
{
	Super::Tick(inDeltaTime);

	ProcessLoadingQueue();

	if (settings.bEnableDebugDisplay)
	{
		DrawDebugInfo();
	}
}

void AZoneManager::InitializeFromDataTable(UDataTable* inZoneDataTable)
{
	if (!inZoneDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("AZoneManager::InitializeFromDataTable - Null DataTable"));
		OnZoneManagerInitialized.Broadcast(false);
		return;
	}

	// 기존 Zone들 정리
	UnregisterAllZones();

	TArray<FZoneLevelData*> zoneDataRows;
	inZoneDataTable->GetAllRows<FZoneLevelData>(TEXT("ZoneManager"), zoneDataRows);

	int32 successCount = 0;
	for (FZoneLevelData* zoneData : zoneDataRows)
	{
		if (zoneData && RegisterZone(*zoneData))
		{
			successCount++;
		}
	}

	bIsInitialized = (successCount > 0);

	UE_LOG(LogTemp, Log, TEXT("Zone Manager initialized: %d zones registered from DataTable"), successCount);
	OnZoneManagerInitialized.Broadcast(bIsInitialized);
}

bool AZoneManager::RegisterZone(const FZoneLevelData& inZoneData)
{
	if (!inZoneData.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register invalid zone data"));
		OnZoneRegistered.Broadcast(inZoneData.zoneIdentifier, false);
		return false;
	}

	// 이미 등록된 Zone인지 확인
	if (zoneComponents.Contains(inZoneData.zoneIdentifier))
	{
		UE_LOG(LogTemp, Warning, TEXT("Zone already registered: %s"), *inZoneData.zoneIdentifier.ToString());
		OnZoneRegistered.Broadcast(inZoneData.zoneIdentifier, false);
		return false;
	}

	// Zone 컴포넌트 생성 (Runtime에서 안전한 NewObject 사용)
	UZoneLevelInstanceComponent* zoneComponent = NewObject<UZoneLevelInstanceComponent>(
		this,
		UZoneLevelInstanceComponent::StaticClass(),
		*FString::Printf(TEXT("Zone_%s_%d"), *inZoneData.zoneIdentifier.zoneName.ToString(), inZoneData.zoneIdentifier.zoneID)
	);

	if (!zoneComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create zone component for: %s"), *inZoneData.zoneIdentifier.ToString());
		OnZoneRegistered.Broadcast(inZoneData.zoneIdentifier, false);
		return false;
	}

	// Zone 초기화
	zoneComponent->InitializeZone(inZoneData);

	// 이벤트 바인딩
	zoneComponent->OnZoneLoadStateChanged.AddDynamic(this, &AZoneManager::OnZoneComponentLoadStateChanged);

	// Zone 등록
	zoneComponents.Add(inZoneData.zoneIdentifier, zoneComponent);

	UE_LOG(LogTemp, Log, TEXT("Zone registered: %s"), *inZoneData.zoneIdentifier.ToString());
	OnZoneRegistered.Broadcast(inZoneData.zoneIdentifier, true);

	return true;
}

bool AZoneManager::UnregisterZone(const FZoneIdentifier& inZoneIdentifier)
{
	TObjectPtr<UZoneLevelInstanceComponent>* zoneComponentPtr = zoneComponents.Find(inZoneIdentifier);
	if (!zoneComponentPtr || !*zoneComponentPtr)
	{
		return false;
	}

	UZoneLevelInstanceComponent* zoneComponent = *zoneComponentPtr;

	// Zone 언로드
	if (zoneComponent->IsZoneLoaded())
	{
		zoneComponent->UnloadZone();
	}

	// 이벤트 언바인딩
	zoneComponent->OnZoneLoadStateChanged.RemoveAll(this);

	// Map에서 제거
	zoneComponents.Remove(inZoneIdentifier);

	// 컴포넌트 제거
	zoneComponent->DestroyComponent();

	UE_LOG(LogTemp, Log, TEXT("Zone unregistered: %s"), *inZoneIdentifier.ToString());
	OnZoneUnregistered.Broadcast(inZoneIdentifier);

	return true;
}

void AZoneManager::UnregisterAllZones()
{
	TArray<FZoneIdentifier> zoneIdentifiers;
	zoneComponents.GetKeys(zoneIdentifiers);

	for (const FZoneIdentifier& zoneID : zoneIdentifiers)
	{
		UnregisterZone(zoneID);
	}

	loadingQueue.Empty();
	currentlyLoading.Empty();
}

UZoneLevelInstanceComponent* AZoneManager::FindZoneByIdentifier(const FZoneIdentifier& inZoneIdentifier) const
{
	TObjectPtr<UZoneLevelInstanceComponent>const* zoneComponentPtr = zoneComponents.Find(inZoneIdentifier);
	return zoneComponentPtr ? *zoneComponentPtr : nullptr;
}

UZoneLevelInstanceComponent* AZoneManager::FindZoneContainingPosition(const FVector& inWorldPosition) const
{
	for (const auto& zonePair : zoneComponents)
	{
		UZoneLevelInstanceComponent* zoneComponent = zonePair.Value;
		if (zoneComponent && zoneComponent->ContainsWorldPosition(inWorldPosition))
		{
			return zoneComponent;
		}
	}

	return nullptr;
}

TArray<UZoneLevelInstanceComponent*> AZoneManager::FindZonesByType(EZoneType inZoneType) const
{
	TArray<UZoneLevelInstanceComponent*> result;

	for (const auto& zonePair : zoneComponents)
	{
		UZoneLevelInstanceComponent* zoneComponent = zonePair.Value;
		if (zoneComponent && zonePair.Key.zoneType == inZoneType)
		{
			result.Add(zoneComponent);
		}
	}

	return result;
}

TArray<UZoneLevelInstanceComponent*> AZoneManager::FindZonesInRadius(const FVector& inCenter, float inRadius) const
{
	TArray<UZoneLevelInstanceComponent*> result;

	for (const auto& zonePair : zoneComponents)
	{
		UZoneLevelInstanceComponent* zoneComponent = zonePair.Value;
		if (zoneComponent && zoneComponent->GetDistanceToZone(inCenter) <= inRadius)
		{
			result.Add(zoneComponent);
		}
	}

	return result;
}

TArray<UZoneLevelInstanceComponent*> AZoneManager::GetAllZones() const
{
	TArray<TObjectPtr<UZoneLevelInstanceComponent>> result;
	zoneComponents.GenerateValueArray(result);
	return result;
}

void AZoneManager::LoadZone(const FZoneIdentifier& inZoneIdentifier)
{
	UZoneLevelInstanceComponent* zoneComponent = FindZoneByIdentifier(inZoneIdentifier);
	if (zoneComponent)
	{
		if (currentlyLoading.Num() < settings.maxConcurrentLoads)
		{
			zoneComponent->LoadZone();
			currentlyLoading.AddUnique(inZoneIdentifier);
		}
		else
		{
			loadingQueue.AddUnique(inZoneIdentifier);
		}
	}
}

void AZoneManager::UpdateAutoLoading(const FVector& inPlayerPosition)
{
	this->playerPosition = inPlayerPosition;

	if (!settings.bAutoLoadZones)
	{
		return;
	}

	for (const auto& zonePair : zoneComponents)
	{
		UZoneLevelInstanceComponent* zoneComponent = zonePair.Value;
		if (zoneComponent && zoneComponent->IsAutoLoadingEnabled())
		{
			zoneComponent->UpdateAutoLoading(inPlayerPosition);
		}
	}
}

TArray<FZoneConnectionPoint> AZoneManager::FindConnectionsBetweenZones(const FZoneIdentifier& inFromZone, const FZoneIdentifier& inToZone) const
{
	TArray<FZoneConnectionPoint> connections;

	UZoneLevelInstanceComponent* fromZoneComponent = FindZoneByIdentifier(inFromZone);
	if (!fromZoneComponent)
	{
		return connections;
	}

	const FZoneLevelData& fromZoneData = fromZoneComponent->GetZoneData();
	for (const FZoneConnectionPoint& connection : fromZoneData.connectionPoints)
	{
		if (connection.targetZone == inToZone)
		{
			connections.Add(connection);
		}
	}

	return connections;
}

int32 AZoneManager::GetLoadedZoneCount() const
{
	int32 count = 0;
	for (const auto& zonePair : zoneComponents)
	{
		if (zonePair.Value && zonePair.Value->IsZoneLoaded())
		{
			count++;
		}
	}
	return count;
}

void AZoneManager::SetZoneManagerSettings(const FZoneManagerSettings& inNewSettings)
{
	settings = inNewSettings;

	// 타이머 업데이트
	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(autoLoadTimerHandle);
		GetWorldTimerManager().ClearTimer(cacheSaveTimerHandle);

		if (settings.bAutoLoadZones)
		{
			GetWorldTimerManager().SetTimer(
				autoLoadTimerHandle,
				[this]() { UpdateAutoLoading(playerPosition); },
				settings.autoLoadCheckInterval,
				true
			);
		}

		if (settings.bSaveZoneCachesToDisk)
		{
			GetWorldTimerManager().SetTimer(
				cacheSaveTimerHandle,
				this,
				&AZoneManager::SaveAllZoneCaches,
				settings.cacheSaveInterval,
				true
			);
		}
	}
}

void AZoneManager::SetDebugDisplayEnabled(bool bEnabled)
{
	settings.bEnableDebugDisplay = bEnabled;
}

void AZoneManager::DrawDebugInfo() const
{
	if (!GetWorld())
	{
		return;
	}

	for (const auto& zonePair : zoneComponents)
	{
		UZoneLevelInstanceComponent* zoneComponent = zonePair.Value;
		if (zoneComponent)
		{
			zoneComponent->DrawDebugInfo(false, 0.1f);
		}
	}

	// Zone Manager 정보 표시
	FVector textLocation = GetActorLocation() + FVector(0, 0, 500.0f);
	FString statsText = FString::Printf(TEXT("ZoneManager: %d total, %d loaded"),
		GetTotalZoneCount(), GetLoadedZoneCount());

	DrawDebugString(GetWorld(), textLocation, statsText, nullptr, FColor::White, 0.1f);
}

void AZoneManager::ProcessLoadingQueue()
{
	while (loadingQueue.Num() > 0 && currentlyLoading.Num() < settings.maxConcurrentLoads)
	{
		FZoneIdentifier zoneID = loadingQueue[0];
		loadingQueue.RemoveAt(0);

		UZoneLevelInstanceComponent* zoneComponent = FindZoneByIdentifier(zoneID);
		if (zoneComponent && !zoneComponent->IsZoneLoaded())
		{
			zoneComponent->LoadZone();
			currentlyLoading.Add(zoneID);
		}
	}
}

void AZoneManager::OnZoneComponentLoadStateChanged(const FZoneIdentifier& inZoneID, EZoneLoadState inNewState)
{
	if (inNewState == EZoneLoadState::Loaded || inNewState == EZoneLoadState::Unloaded || inNewState == EZoneLoadState::Error)
	{
		currentlyLoading.Remove(inZoneID);
	}

	UE_LOG(LogTemp, Log, TEXT("Zone %s state changed to %s"),
		*inZoneID.ToString(), *UEnum::GetValueAsString(inNewState));
}

void AZoneManager::SaveAllZoneCaches()
{
	for (const auto& zonePair : zoneComponents)
	{
		UZoneLevelInstanceComponent* zoneComponent = zonePair.Value;
		if (zoneComponent && zoneComponent->IsPathCacheReady())
		{
			UZonePathScoreCache* cache = zoneComponent->GetPathScoreCache();
			if (cache)
			{
				FString saveDir = FPaths::ProjectSavedDir() / TEXT("PathFindingCache");
				FString fileName = FString::Printf(TEXT("%s_%d.json"),
					*zonePair.Key.zoneName.ToString(), zonePair.Key.zoneID);
				FString filePath = saveDir / fileName;

				cache->SaveCacheToFile(filePath);
			}
		}
	}
}

void AZoneManager::CleanupZoneComponents()
{
	for (const auto& zonePair : zoneComponents)
	{
		UZoneLevelInstanceComponent* zoneComponent = zonePair.Value;
		if (zoneComponent)
		{
			zoneComponent->OnZoneLoadStateChanged.RemoveAll(this);
		}
	}

	zoneComponents.Empty();
}