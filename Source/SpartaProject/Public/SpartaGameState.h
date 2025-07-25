// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SpartaGameState.generated.h"

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
	void OnLevelTimeUp();
	void EndLevel();

	void OnCoinCollected();

	UFUNCTION(BlueprintCallable, Category = "Level")
	void OnGameOver();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	float LevelDuration;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;

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

#pragma region HUD

public:
	void UpdateHUD();

	FTimerHandle HUDUpdateTimerHandle;

#pragma endregion
};
