#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpikeTrap.generated.h"

UCLASS()
class SPARTAPROJECT_API ASpikeTrap : public AActor
{
	GENERATED_BODY()

public:
	ASpikeTrap();
	
	UFUNCTION(BlueprintCallable, Category = "Trap")
	void ActivateTrap();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateSpike(float DeltaTime);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trap", Meta = (AllowPrivateAccess))
	TObjectPtr<UStaticMeshComponent> SpikeMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap", Meta = (AllowPrivateAccess))
	float RiseHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap", Meta = (AllowPrivateAccess))
	float RiseDuration;
	
	bool bIsActivating;
	float CurrentTime;
	
	FVector DefaultRelativeLocation;
};