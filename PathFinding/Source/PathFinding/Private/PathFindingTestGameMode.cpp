#include "PathFindingTestGameMode.h"
#include "Zone/ZoneManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"

APathFindingTestGameMode::APathFindingTestGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f; // 0.1초마다 체크

	bTrackPlayerPosition = true;
	playerTrackingInterval = 1.0f; // 1초마다 업데이트
	zoneManager = nullptr;
}

void APathFindingTestGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Zone Manager 생성
	CreateZoneManager();

	// 플레이어 위치 추적 시작
	if (bTrackPlayerPosition)
	{
		GetWorldTimerManager().SetTimer(
			playerTrackingTimer,
			this,
			&APathFindingTestGameMode::UpdatePlayerPosition,
			playerTrackingInterval,
			true
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("PathFinding Test Game Mode started"));
}

void APathFindingTestGameMode::Tick(float inDeltaTime)
{
	Super::Tick(inDeltaTime);
}

void APathFindingTestGameMode::SetDebugMode(bool bEnabled)
{
	if (zoneManager)
	{
		zoneManager->SetDebugDisplayEnabled(bEnabled);
	}
}

FVector APathFindingTestGameMode::GetPlayerPosition() const
{
	if (APlayerController* pc = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* playerPawn = pc->GetPawn())
		{
			return playerPawn->GetActorLocation();
		}
	}
	return FVector::ZeroVector;
}

void APathFindingTestGameMode::UpdatePlayerPosition()
{
	if (zoneManager)
	{
		FVector playerPos = GetPlayerPosition();
		zoneManager->SetPlayerPosition(playerPos);
		zoneManager->UpdateAutoLoading(playerPos);
	}
}

void APathFindingTestGameMode::CreateZoneManager()
{
	if (!zoneManager)
	{
		FActorSpawnParameters spawnParams;
		spawnParams.Name = FName("TestZoneManager");
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		zoneManager = GetWorld()->SpawnActor<AZoneManager>(AZoneManager::StaticClass(), spawnParams);

		if (zoneManager)
		{
			// 테스트 데이터 테이블이 설정되어 있다면 초기화
			if (testZoneDataTable)
			{
				zoneManager->InitializeFromDataTable(testZoneDataTable);
			}

			UE_LOG(LogTemp, Warning, TEXT("Zone Manager created for testing"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Zone Manager"));
		}
	}
}