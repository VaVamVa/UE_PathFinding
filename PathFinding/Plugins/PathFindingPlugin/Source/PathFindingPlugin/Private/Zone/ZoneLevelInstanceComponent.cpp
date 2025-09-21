#include "Zone/ZoneLevelInstanceComponent.h"
#include "Zone/ZonePathScoreCache.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/LevelStreamingDynamic.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UZoneLevelInstanceComponent::UZoneLevelInstanceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // 1초마다 체크/*modify_250921_: 체크 간격을 1초로 설정*/

	currentLoadState = EZoneLoadState::Unloaded;
	bAutoLoadingEnabled = true;
	bGeneratePathCacheOnLoad = true;
	loadingTimeoutSeconds = 30.0f;
	loadingStartTime = 0.0f;
}

void UZoneLevelInstanceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (zoneData.IsValid())
	{
		SetupPathCache();
		// 캐시된 경로 데이터 로드 시도
		if (compPathScoreCache)
		{
			FString saveDir = FPaths::ProjectSavedDir() / TEXT("PathFindingCache");
			FString fileName = FString::Printf(TEXT("%s_%d.json"),
				*zoneData.zoneIdentifier.zoneName.ToString(),
				zoneData.zoneIdentifier.zoneID);
			FString filePath = saveDir / fileName;

			if (FPaths::FileExists(filePath))
			{
				compPathScoreCache->LoadCacheFromFile(filePath);
			}
		}
	}
}

void UZoneLevelInstanceComponent::EndPlay(const EEndPlayReason::Type inEndPlayReason)
{
	if (IsZoneLoaded())
	{
		// 경로 캐시 저장
		if (compPathScoreCache && compPathScoreCache->IsGridGenerated())
		{
			FString saveDir = FPaths::ProjectSavedDir() / TEXT("PathFindingCache");
			IFileManager::Get().MakeDirectory(*saveDir, true);

			FString fileName = FString::Printf(TEXT("%s_%d.json"),
				*zoneData.zoneIdentifier.zoneName.ToString(),
				zoneData.zoneIdentifier.zoneID);
			FString filePath = saveDir / fileName;

			compPathScoreCache->SaveCacheToFile(filePath);
		}
		UnloadZone();
	}

	// 레벨 스트리밍 정리
	if (compLevelStreamingInstance)
	{
		compLevelStreamingInstance->OnLevelLoaded.RemoveDynamic(this, &UZoneLevelInstanceComponent::OnLevelLoaded);
		compLevelStreamingInstance->OnLevelUnloaded.RemoveDynamic(this, &UZoneLevelInstanceComponent::OnLevelUnloaded);
		compLevelStreamingInstance = nullptr;
	}

	if (GetWorld() && loadingTimeoutHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(loadingTimeoutHandle);
	}

	Super::EndPlay(inEndPlayReason);
}

void UZoneLevelInstanceComponent::TickComponent(float inDeltaTime, ELevelTick inTickType, FActorComponentTickFunction* inThisTickFunction)
{
	Super::TickComponent(inDeltaTime, inTickType, inThisTickFunction);

	// 로딩 타임아웃 체크
	if (currentLoadState == EZoneLoadState::Loading && loadingTimeoutSeconds > 0.0f)
	{
		if (GetWorld()->GetTimeSeconds() - loadingStartTime > loadingTimeoutSeconds)
		{
			UE_LOG(LogTemp, Error, TEXT("Zone loading timeout for: %s"), *zoneData.zoneIdentifier.ToString());
			SetLoadState(EZoneLoadState::Error);
			OnZoneLoadError.Broadcast(zoneData.zoneIdentifier, TEXT("Loading timeout"));
		}
	}
}

void UZoneLevelInstanceComponent::InitializeZone(const FZoneLevelData& inZoneData)
{
	if (!inZoneData.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UZoneLevelInstanceComponent::InitializeZone - Invalid zone data"));
		return;
	}

	// 기존 Zone이 로드되어 있다면 언로드
	if (IsZoneLoaded())
	{
		UnloadZone();
	}

	zoneData = inZoneData;
	SetLoadState(EZoneLoadState::Unloaded);

	SetupPathCache();

	UE_LOG(LogTemp, Log, TEXT("Zone initialized: %s"), *zoneData.zoneIdentifier.ToString());
}

void UZoneLevelInstanceComponent::LoadZone()
{
	if (!zoneData.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot load zone - invalid zone data"));
		OnZoneLoadError.Broadcast(zoneData.zoneIdentifier, TEXT("Invalid zone data"));
		return;
	}

	if (currentLoadState == EZoneLoadState::Loaded || currentLoadState == EZoneLoadState::Loading)
	{
		return;
	}

	SetLoadState(EZoneLoadState::Loading);
	loadingStartTime = GetWorld()->GetTimeSeconds();

	CreateLevelStreamingInstance();

	if (compLevelStreamingInstance)
	{
		compLevelStreamingInstance->SetShouldBeLoaded(true);
		compLevelStreamingInstance->SetShouldBeVisible(true);

		UE_LOG(LogTemp, Log, TEXT("Started loading zone: %s"), *zoneData.zoneIdentifier.ToString());
	}
	else
	{
		SetLoadState(EZoneLoadState::Error);
		OnZoneLoadError.Broadcast(zoneData.zoneIdentifier, TEXT("Failed to create level streaming instance"));
	}
}

void UZoneLevelInstanceComponent::UnloadZone()
{
	if (currentLoadState == EZoneLoadState::Unloaded || currentLoadState == EZoneLoadState::Unloading)
	{
		return;
	}

	SetLoadState(EZoneLoadState::Unloading);

	if (compLevelStreamingInstance)
	{
		compLevelStreamingInstance->SetShouldBeLoaded(false);
		compLevelStreamingInstance->SetShouldBeVisible(false);
	}

	UE_LOG(LogTemp, Log, TEXT("Started unloading zone: %s"), *zoneData.zoneIdentifier.ToString());
}

bool UZoneLevelInstanceComponent::IsZoneLoaded() const
{
	return currentLoadState == EZoneLoadState::Loaded;
}

EZoneLoadState UZoneLevelInstanceComponent::GetZoneLoadState() const
{
	return currentLoadState;
}

void UZoneLevelInstanceComponent::GeneratePathCache()
{
	if (!IsZoneLoaded())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot generate path cache - zone not loaded"));
		return;
	}

	if (compPathScoreCache)
	{
		compPathScoreCache->GeneratePathGrid(zoneData);
		OnZonePathCacheReady.Broadcast(zoneData.zoneIdentifier);
		UE_LOG(LogTemp, Log, TEXT("Path cache generated for zone: %s"), *zoneData.zoneIdentifier.ToString());
	}
}

bool UZoneLevelInstanceComponent::IsPathCacheReady() const
{
	return compPathScoreCache && compPathScoreCache->IsGridGenerated();
}

void UZoneLevelInstanceComponent::UpdateAutoLoading(const FVector& inPlayerLocation)
{
	if (!bAutoLoadingEnabled || !zoneData.IsValid())
	{
		return;
	}

	bool bShouldLoad = zoneData.ShouldLoad(inPlayerLocation);
	bool bShouldUnload = zoneData.ShouldUnload(inPlayerLocation);

	if (bShouldLoad && !IsZoneLoaded() && currentLoadState != EZoneLoadState::Loading)
	{
		LoadZone();
	}
	else if (bShouldUnload && IsZoneLoaded())
	{
		UnloadZone();
	}
}

void UZoneLevelInstanceComponent::DrawDebugInfo(bool bPersistent, float inLifeTime) const
{
	if (!GetWorld() || !zoneData.IsValid())
	{
		return;
	}

	FColor zoneColor = FColor::Green;
	switch (currentLoadState)
	{
	case EZoneLoadState::Unloaded:
		zoneColor = FColor::Silver;
		break;
	case EZoneLoadState::Loading:
		zoneColor = FColor::Yellow;
		break;
	case EZoneLoadState::Loaded:
		zoneColor = FColor::Green;
		break;
	case EZoneLoadState::Unloading:
		zoneColor = FColor::Orange;
		break;
	case EZoneLoadState::Error:
		zoneColor = FColor::Red;
		break;
	}

	// Zone 경계 박스 그리기
	FBox zoneBounds = zoneData.zoneBounds.GetBoundingBox();
	DrawDebugBox(GetWorld(), zoneBounds.GetCenter(), zoneBounds.GetExtent(), zoneColor, bPersistent, inLifeTime, 0, 2.0f);

	// Zone 정보 텍스트
	FVector textLocation = zoneData.zoneBounds.center + FVector(0, 0, zoneData.zoneBounds.height * 0.6f);
	FString debugText = FString::Printf(TEXT("%s\n%s\nCache: %s"),
		*zoneData.zoneIdentifier.ToString(),
		*UEnum::GetValueAsString(currentLoadState),
		IsPathCacheReady() ? TEXT("Ready") : TEXT("Not Ready"));

	DrawDebugString(GetWorld(), textLocation, debugText, nullptr, zoneColor, inLifeTime);

	// Connection Points 그리기
	for (const FZoneConnectionPoint& connection : zoneData.connectionPoints)
	{
		DrawDebugSphere(GetWorld(), connection.connectionLocation, 50.0f, 12, FColor::Blue, bPersistent, inLifeTime);
		DrawDebugLine(GetWorld(), connection.connectionLocation, connection.targetLocation, FColor::Cyan, bPersistent, inLifeTime, 0, 3.0f);
	}
}

bool UZoneLevelInstanceComponent::ContainsWorldPosition(const FVector& inWorldPosition) const
{
	return zoneData.zoneBounds.ContainsPoint(inWorldPosition);
}

float UZoneLevelInstanceComponent::GetDistanceToZone(const FVector& inWorldPosition) const
{
	return FVector::Dist(inWorldPosition, zoneData.zoneBounds.center);
}

void UZoneLevelInstanceComponent::OnLevelLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("Zone level loaded: %s"), *zoneData.zoneIdentifier.ToString());
	SetLoadState(EZoneLoadState::Loaded);

	if (bGeneratePathCacheOnLoad)
	{
		GeneratePathCache();
	}
}

void UZoneLevelInstanceComponent::OnLevelUnloaded()
{
	UE_LOG(LogTemp, Log, TEXT("Zone level unloaded: %s"), *zoneData.zoneIdentifier.ToString());
	SetLoadState(EZoneLoadState::Unloaded);
}

void UZoneLevelInstanceComponent::SetLoadState(EZoneLoadState inNewState)
{
	if (currentLoadState != inNewState)
	{
		EZoneLoadState oldState = currentLoadState;
		currentLoadState = inNewState;

		OnZoneLoadStateChanged.Broadcast(zoneData.zoneIdentifier, inNewState);

		UE_LOG(LogTemp, Log, TEXT("Zone %s state changed: %s -> %s"),
			*zoneData.zoneIdentifier.ToString(),
			*UEnum::GetValueAsString(oldState),
			*UEnum::GetValueAsString(inNewState));
	}
}

void UZoneLevelInstanceComponent::CreateLevelStreamingInstance()
{
	if (!GetWorld() || !zoneData.IsValid() || compLevelStreamingInstance)
	{
		return;
	}

	FString levelPath = zoneData.levelAsset.GetLongPackageName();
	if (levelPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid level asset path for zone: %s"), *zoneData.zoneIdentifier.ToString());
		return;
	}

	// Dynamic Level Streaming 인스턴스 생성
	bool bOutSuccess = false;
	compLevelStreamingInstance = ULevelStreamingDynamic::LoadLevelInstance(
		GetWorld(),
		levelPath,
		zoneData.levelTransform.GetLocation(),
		zoneData.levelTransform.GetRotation().Rotator(),
		bOutSuccess // 즉시 로드하지 않음/*modify_250921_: 즉시 로드 비활성화로 설정*/
	);

	if (compLevelStreamingInstance)
	{
		// 델리게이트 바인딩
		compLevelStreamingInstance->OnLevelLoaded.AddDynamic(this, &UZoneLevelInstanceComponent::OnLevelLoaded);
		compLevelStreamingInstance->OnLevelUnloaded.AddDynamic(this, &UZoneLevelInstanceComponent::OnLevelUnloaded);

		UE_LOG(LogTemp, Log, TEXT("Level streaming instance created for zone: %s"), *zoneData.zoneIdentifier.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create level streaming instance for zone: %s"), *zoneData.zoneIdentifier.ToString());
	}
}

void UZoneLevelInstanceComponent::SetupPathCache()
{
	if (!compPathScoreCache)
	{
		compPathScoreCache = NewObject<UZonePathScoreCache>(this);
	}
}