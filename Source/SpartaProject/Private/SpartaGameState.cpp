#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "SpikeTrap.h"

ASpartaGameState::ASpartaGameState()
	: Score(0),
	SpawnedCoinCount(0),
	CollectedCoinCount(0),
	CurrentLevelIndex(0),
	MaxLevel(3),
	CurrentWaveIndex(0),
	MaxWave(3),
	WaveDuration(20.f),
	ItemsToSpawnPerWave({20, 30, 40}),
	SpikeTrapClass(nullptr)
{
}

void ASpartaGameState::BeginPlay()
{
	Super::BeginPlay();

	StartLevel();

	GetWorldTimerManager().SetTimer(
		HUDUpdateTimerHandle,
		this,
		&ThisClass::UpdateHUD,
		0.1f,
		true
	);
}

void ASpartaGameState::StartLevel()
{
	if (ASpartaPlayerController* SpartaPlayerController = GetSpartaPlayerController())
	{
		SpartaPlayerController->ShowGameHUD();
	}

	if (USpartaGameInstance* SpartaGameInstance = GetSpartaGameInstance())
	{
		CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
	}

	CurrentWaveIndex = 0;
	StartWave();
}

void ASpartaGameState::EndLevel()
{
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	if (USpartaGameInstance* SpartaGameInstance = GetSpartaGameInstance())
	{
		AddScore(Score);
		CurrentLevelIndex++;
		SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;

		if (CurrentLevelIndex >= MaxLevel)
		{
			OnGameOver();
			return;
		}

		if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
		{
			UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
		}
		else
		{
			OnGameOver();
		}
	}
}

void ASpartaGameState::StartWave()
{
	OnWaveStarted.Broadcast(CurrentWaveIndex);

	SpawnedCoinCount = 0;
	CollectedCoinCount = 0;

	for (AActor* Item : CurrentWaveItems)
	{
		if (Item && Item->IsValidLowLevelFast())
		{
			Item->Destroy();
		}
	}
	CurrentWaveItems.Empty();

	int32 ItemToSpawn = (ItemsToSpawnPerWave.IsValidIndex(CurrentWaveIndex)) ? ItemsToSpawnPerWave[CurrentWaveIndex] : 20;

	if (ASpawnVolume* SpawnVolume = GetSpawnVolume())
	{
		for (int32 i = 0; i < ItemToSpawn; ++i)
		{
			if (AActor* SpawnedActor = SpawnVolume->SpawnRandomItem())
			{
				if (SpawnedActor->IsA(ACoinItem::StaticClass()))
				{
					SpawnedCoinCount++;
				}

				CurrentWaveItems.Add(SpawnedActor);
			}
		}
	}

	if (CurrentWaveIndex == 1)
	{
		EnableWave2();
	}
	else if (CurrentWaveIndex == 2)
	{
		EnableWave3();
	}

	GetWorldTimerManager().SetTimer(
		WaveTimerHandle,
		this,
		&ThisClass::OnWaveTimeUp,
		WaveDuration,
		false
	);
}

void ASpartaGameState::EndWave()
{
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);

	if (CurrentWaveIndex == 2)
	{
		SetAllCoinsMove(false);
	}

	++CurrentWaveIndex;
	if (CurrentWaveIndex >= MaxWave)
	{
		EndLevel();
	}
	else
	{
		StartWave();
	}
}

void ASpartaGameState::OnWaveTimeUp()
{
	EndWave();
}

void ASpartaGameState::OnCoinCollected()
{
	CollectedCoinCount++;

	if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
	{
		EndWave();
	}
}

void ASpartaGameState::OnGameOver()
{
	if (ASpartaPlayerController* SpartaPlayerController = GetSpartaPlayerController())
	{
		SpartaPlayerController->ShowMainMenu(true);
		SpartaPlayerController->SetPause(true);
	}
}

void ASpartaGameState::EnableWave2()
{
	const FString Msg = TEXT("Wave 2: Spawning and activating 5 Spike Traps!");

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Msg);
	}

	if (ASpawnVolume* SpawnVolume = GetSpawnVolume())
	{
		if (SpikeTrapClass)
		{
			for (int32 i = 0; i < 15; ++i)
			{
				if (AActor* SpawnedActor = SpawnVolume->SpawnItem(SpikeTrapClass))
				{
					if (ASpikeTrap* SpikeTrap = Cast<ASpikeTrap>(SpawnedActor))
					{
						SpikeTrap->ActivateTrap();
					}
				}
			}
		}
	}
}

void ASpartaGameState::EnableWave3()
{
	const FString Msg = TEXT("Wave 3: Spawning coins that orbit around");

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Msg);
	}

	SetAllCoinsMove(true);
}

void ASpartaGameState::SetAllCoinsMove(bool bActive)
{
	for (AActor* CoinActor : CurrentWaveItems)
	{
		if (ACoinItem* Coin = Cast<ACoinItem>(CoinActor))
		{
			Coin->SetWave3MoveActive(bActive);
		}
	}
}

ASpawnVolume* ASpartaGameState::GetSpawnVolume() const
{
	TArray<AActor*> FoundVolumes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
	return (FoundVolumes.Num() > 0) ? Cast<ASpawnVolume>(FoundVolumes[0]) : nullptr;
}

ASpartaPlayerController* ASpartaGameState::GetSpartaPlayerController() const
{
	return Cast<ASpartaPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

USpartaGameInstance* ASpartaGameState::GetSpartaGameInstance() const
{
	return Cast<USpartaGameInstance>(GetGameInstance());
}

void ASpartaGameState::AddScore(int32 Amount)
{
	if (USpartaGameInstance* SpartaGameInstance = GetSpartaGameInstance())
	{
		SpartaGameInstance->AddToScore(Amount);
	}
}

void ASpartaGameState::UpdateHUD()
{
	if (ASpartaPlayerController* SpartaPlayerController = GetSpartaPlayerController())
	{
		if (UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget())
		{
			if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
			{
				float RemainingTime = GetWorldTimerManager().GetTimerRemaining(WaveTimerHandle);
				TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
			}

			if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
			{
				if (USpartaGameInstance* SpartaGameInstance = GetSpartaGameInstance())
				{
					ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
				}
			}

			if (UTextBlock* LevelText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
			{
				LevelText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d"), CurrentLevelIndex + 1)));
			}

			if (UTextBlock* WaveText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Wave"))))
			{
				WaveText->SetText(FText::FromString(FString::Printf(TEXT("Wave: %d / %d"), CurrentWaveIndex + 1, MaxWave)));
			}
		}
	}
}


