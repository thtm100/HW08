// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SpartaGameState.generated.h"

class ASpawnVolume;
class ACoinItem;
class ASpikeTrap;
class ASpartaPlayerController;
class USpartaGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveNumber);
/**
 * 
 */
UCLASS()
class SPARTAPROJECT_API ASpartaGameState : public AGameState
{
	GENERATED_BODY()

#pragma region Level

public:
	ASpartaGameState();

	virtual void BeginPlay() override;

	FTimerHandle LevelTimerHandle;

	void StartLevel();
	void EndLevel();
	void StartWave();
	void EndWave();
	void OnWaveTimeUp();
	void OnCoinCollected();

	UFUNCTION(BlueprintCallable, Category = "Level")
	void OnGameOver();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;

#pragma endregion

#pragma region Wave

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 MaxWave;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	float WaveDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	TArray<int32> ItemsToSpawnPerWave;

	UPROPERTY(BlueprintAssignable, Category = "Wave")
	FOnWaveStarted OnWaveStarted;

private:
	UPROPERTY()
	TArray<AActor*> CurrentWaveItems;

	void EnableWave2();
	void EnableWave3();
	void SetAllCoinsMove(bool bActive);

	ASpawnVolume* GetSpawnVolume() const;
	ASpartaPlayerController* GetSpartaPlayerController() const;
	USpartaGameInstance* GetSpartaGameInstance() const;
	
#pragma endregion

#pragma region CoinScore

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 SpawnedCoinCount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 CollectedCoinCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Score;

	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScore() const { return Score; }

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Amount);

#pragma endregion

#pragma region SpikeTrap

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave|Hazard")
	TSubclassOf<AActor> SpikeTrapClass;

#pragma endregion

#pragma region HUD

public:
	void UpdateHUD();

	FTimerHandle WaveTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;

#pragma endregion
};
