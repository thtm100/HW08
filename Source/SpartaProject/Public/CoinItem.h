#pragma once

#include "CoreMinimal.h"
#include "BaseItem.h"
#include "CoinItem.generated.h"

UCLASS()
class SPARTAPROJECT_API ACoinItem : public ABaseItem
{
	GENERATED_BODY()

public:
	ACoinItem();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void ActivateItem(AActor* Activator) override;

public:
	void SetWave3MoveActive(bool bActive);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
	float SpinSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
	float MoveDistance;

	float Direction;
	FVector StartLocation;
	
	bool bMovingMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coin")
	int32 PointValue;

private:
	void UpdateSpin(float DeltaTime);
	
	void Move(float DeltaTime);
};
