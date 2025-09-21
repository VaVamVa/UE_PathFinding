#include "PathFindingTestPlayerController.h"
#include "PathFindingTestGameMode.h"
#include "Zone/ZoneManager.h"
#include "Zone/ZoneLevelInstanceComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"

APathFindingTestPlayerController::APathFindingTestPlayerController()
{
	bDebugDisplayEnabled = false;
}

void APathFindingTestPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("PathFinding Test Player Controller started"));

	// 초기 안내 메시지
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
			TEXT("PathFinding Test Controls:\nF1 - Toggle Debug\nF2 - Print Zone Info"));
	}
}

void APathFindingTestPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 디버그 토글 키 바인딩 (F1)
	InputComponent->BindAction("ToggleDebug", IE_Pressed, this, &APathFindingTestPlayerController::OnToggleDebug);

	// 존 정보 출력 키 바인딩 (F2)
	InputComponent->BindAction("PrintZoneInfo", IE_Pressed, this, &APathFindingTestPlayerController::OnPrintZoneInfo);
}

UZoneLevelInstanceComponent* APathFindingTestPlayerController::GetCurrentZone() const
{
	if (AZoneManager* zoneManager = GetZoneManager())
	{
		if (APawn* playerPawn = GetPawn())
		{
			return zoneManager->FindZoneContainingPosition(playerPawn->GetActorLocation());
		}
	}
	return nullptr;
}

void APathFindingTestPlayerController::LoadZoneManually(const FZoneIdentifier& inZoneIdentifier)
{
	if (AZoneManager* zoneManager = GetZoneManager())
	{
		zoneManager->LoadZone(inZoneIdentifier);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
				FString::Printf(TEXT("Loading Zone: %s"), *inZoneIdentifier.ToString()));
		}
	}
}

void APathFindingTestPlayerController::ToggleDebugDisplay()
{
	bDebugDisplayEnabled = !bDebugDisplayEnabled;

	if (APathFindingTestGameMode* gameMode = Cast<APathFindingTestGameMode>(GetWorld()->GetAuthGameMode()))
	{
		gameMode->SetDebugMode(bDebugDisplayEnabled);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
			FString::Printf(TEXT("Debug Display: %s"), bDebugDisplayEnabled ? TEXT("ON") : TEXT("OFF")));
	}
}

void APathFindingTestPlayerController::PrintZoneInfo()
{
	UZoneLevelInstanceComponent* currentZone = GetCurrentZone();

	if (currentZone)
	{
		const FZoneLevelData& zoneData = currentZone->GetZoneData();
		FString zoneInfo = FString::Printf(
			TEXT("Current Zone: %s\nType: %s\nLoad State: %s\nPath Cache: %s"),
			*zoneData.zoneIdentifier.ToString(),
			*UEnum::GetValueAsString(zoneData.zoneIdentifier.zoneType),
			*UEnum::GetValueAsString(currentZone->GetZoneLoadState()),
			currentZone->IsPathCacheReady() ? TEXT("Ready") : TEXT("Not Ready")
		);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, zoneInfo);
		}

		UE_LOG(LogTemp, Warning, TEXT("%s"), *zoneInfo);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Not in any zone"));
		}
	}

	// Zone Manager 통계도 출력
	if (AZoneManager* zoneManager = GetZoneManager())
	{
		FString managerInfo = FString::Printf(
			TEXT("Zone Manager Stats:\nTotal Zones: %d\nLoaded Zones: %d"),
			zoneManager->GetTotalZoneCount(),
			zoneManager->GetLoadedZoneCount()
		);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, managerInfo);
		}
	}
}

void APathFindingTestPlayerController::TeleportToLocation(const FVector& inLocation)
{
	if (APawn* playerPawn = GetPawn())
	{
		playerPawn->SetActorLocation(inLocation);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta,
				FString::Printf(TEXT("Teleported to: %s"), *inLocation.ToString()));
		}
	}
}

void APathFindingTestPlayerController::OnToggleDebug()
{
	ToggleDebugDisplay();
}

void APathFindingTestPlayerController::OnPrintZoneInfo()
{
	PrintZoneInfo();
}

AZoneManager* APathFindingTestPlayerController::GetZoneManager() const
{
	if (APathFindingTestGameMode* gameMode = Cast<APathFindingTestGameMode>(GetWorld()->GetAuthGameMode()))
	{
		return gameMode->GetZoneManager();
	}
	return nullptr;
}