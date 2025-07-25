#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpartaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class UUserWidget;

struct FInputActionValue;

UCLASS()
class SPARTAPROJECT_API ASpartaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpartaCharacter();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* OverheadWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	TSubclassOf<UUserWidget> BuffInfoWidgetClass;

	float NormalSpeed;
	float SprintSpeedMultiplier;
	float SprintSpeed;

	bool bSprinting;

	int32 SpeedDebuffStack;
	float CurrentSpeedMultiplier;

	int32 ReverseControlStack;
	bool bIsControlReversed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	void UpdateOverheadHP();

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void StartJump(const FInputActionValue& value);
	UFUNCTION()
	void StopJump(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void StartSprint(const FInputActionValue& value);
	UFUNCTION()
	void StopSprint(const FInputActionValue& value);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }
	
	UFUNCTION()
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void AddHealth(float Amount);
	void OnDeath();

	UFUNCTION(BlueprintCallable, Category = "Debuff")
	void ApplySpeedDebuff(float Duration, float SpeedMultiplier);

	UFUNCTION(BlueprintCallable, Category = "Debuff")
	void ApplyReverseControlDebuff(float Duration);

	void AddBuffInfoUI(const FString& InBuffName, float InDuration);

	protected:
	UFUNCTION()
	void OnSpeedDebuffEnd();
	UFUNCTION()
	void OnReverseControlDebuffEnd();

	void UpdateCharacterSpeed();
};
