#include "SpartaCharacter.h"
#include "SpartaPlayerController.h"
#include "SpartaGameState.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Blueprint/UserWidget.h"
#include "BuffInfo.h"

ASpartaCharacter::ASpartaCharacter()
{
 	PrimaryActorTick.bCanEverTick = false;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(GetMesh());
	OverheadWidget->SetWidgetSpace(EWidgetSpace::Screen);

	NormalSpeed = 600.0f;
	SprintSpeedMultiplier = 1.5f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;

	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	MaxHealth = 100.0f;
	Health = MaxHealth;

	bSprinting = false;
	SpeedDebuffStack = 0;
	CurrentSpeedMultiplier = 1.0f;
	ReverseControlStack = 0;
	bIsControlReversed = false;
}


void ASpartaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = 
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASpartaPlayerController* PlayerController = 
			Cast<ASpartaPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInputComponent->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Move
				);
			}
			if (PlayerController->JumpAction)
			{
				EnhancedInputComponent->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartJump
				);

				EnhancedInputComponent->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopJump
				);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInputComponent->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::Look
				);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInputComponent->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Triggered,
					this,
					&ASpartaCharacter::StartSprint
				);

				EnhancedInputComponent->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&ASpartaCharacter::StopSprint
				);
			}
		}
	}
}

void ASpartaCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateOverheadHP();
}

void ASpartaCharacter::UpdateOverheadHP()
{
	if (!OverheadWidget) { return; }

	UUserWidget* OverheadWidgetInstance = OverheadWidget->GetUserWidgetObject();
	if (OverheadWidgetInstance) { return; }

	if (UProgressBar* HPBar =
		Cast<UProgressBar>(OverheadWidgetInstance->GetWidgetFromName(TEXT("HealthBar"))))
	{
		const float HPPercent = (MaxHealth > 0.f) ? Health / MaxHealth : 0.f;
		HPBar->SetPercent(HPPercent);

		if (HPPercent < 0.3f)
		{
			HPBar->SetFillColorAndOpacity(FLinearColor::Red);
		}
	}

	if (UTextBlock* HPText = 
		Cast<UTextBlock>(OverheadWidgetInstance->GetWidgetFromName(
		TEXT("OverHeadHP"))))
	{
		HPText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f / %.0f"), Health, MaxHealth)));
	}
}

void ASpartaCharacter::Move(const FInputActionValue& value) // value will be FVector2D
{
	if (!Controller) return;

	FVector2D MoveInput = value.Get<FVector2D>();

	if (bIsControlReversed)
	{
		MoveInput *= -1.f;
	}
	
	if (!FMath::IsNearlyZero(MoveInput.X)) // float value issue
	{
		AddMovementInput(GetActorForwardVector(), MoveInput.X);
	}
	if (!FMath::IsNearlyZero(MoveInput.Y))
	{
		AddMovementInput(GetActorRightVector(), MoveInput.Y);
	}
}

void ASpartaCharacter::StartJump(const FInputActionValue& value) // value will be boolean
{
	if (value.Get<bool>())
	{
		Jump();
	}
}

void ASpartaCharacter::StopJump(const FInputActionValue& value)
{
	if (!value.Get<bool>())
	{
		StopJumping();
	}
}

void ASpartaCharacter::Look(const FInputActionValue& value) // value will be FVector2D
{
	FVector2D LookInput = value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void ASpartaCharacter::StartSprint(const FInputActionValue& value) // value will be boolean
{
	bSprinting = true;
	UpdateCharacterSpeed();
}

void ASpartaCharacter::StopSprint(const FInputActionValue& value)
{
	bSprinting = false;
	UpdateCharacterSpeed();
}

float ASpartaCharacter::TakeDamage(
	float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);
	UpdateOverheadHP();

	if (Health < KINDA_SMALL_NUMBER)
	{
		OnDeath();
	}

	return ActualDamage;
}

void ASpartaCharacter::AddHealth(float Amount)
{
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	UpdateOverheadHP();
}

void ASpartaCharacter::OnDeath()
{
	ASpartaGameState* SpartaGameState = GetWorld() ?
		GetWorld()->GetGameState<ASpartaGameState>() : nullptr;
	if (SpartaGameState)
	{
		SpartaGameState->OnGameOver();
	}
}

void ASpartaCharacter::ApplySpeedDebuff(float Duration, float SpeedMultiplier)
{
	SpeedDebuffStack++;

	if (SpeedMultiplier < CurrentSpeedMultiplier)
	{
		CurrentSpeedMultiplier = SpeedMultiplier;
	}
	UpdateCharacterSpeed();

	FTimerHandle TempTimerHandle;
	GetWorldTimerManager().SetTimer(
		TempTimerHandle,
		this,
		&ThisClass::OnSpeedDebuffEnd,
		Duration,
		false
	);

	AddBuffInfoUI(TEXT("Speed Down"), Duration);
}

void ASpartaCharacter::ApplyReverseControlDebuff(float Duration)
{
	ReverseControlStack++;

	if (!bIsControlReversed)
	{
		bIsControlReversed = true;
	}

	FTimerHandle TempTimerHandle;
	GetWorldTimerManager().SetTimer(
		TempTimerHandle,
		this,
		&ThisClass::OnReverseControlDebuffEnd,
		Duration,
		false
	);

	AddBuffInfoUI(TEXT("Reverse Control"), Duration);
}

void ASpartaCharacter::AddBuffInfoUI(const FString& InBuffName, float InDuration)
{
	if (ASpartaPlayerController* PC = Cast<ASpartaPlayerController>(GetController()))
	{
		UBuffInfo* ExistingBuff = nullptr;

		if (UUserWidget* HUDWidget = PC->GetHUDWidget())
		{
			UVerticalBox* BuffInfoStack =
				Cast<UVerticalBox>(HUDWidget->GetWidgetFromName(TEXT("BuffInfoStack")));

			if (!BuffInfoStack) { return; }

			for (UWidget* Child : BuffInfoStack->GetAllChildren())
			{
				UBuffInfo* BuffWidget = Cast<UBuffInfo>(Child);

				if (BuffWidget && BuffWidget->GetBuffName() == InBuffName)
				{
					ExistingBuff = BuffWidget;
					break;
				}
			}

			if (ExistingBuff)
			{
				ExistingBuff->ExtendBuffDuration(InDuration);
			}
			else
			{
				UBuffInfo* BuffInfoInstance =
					CreateWidget<UBuffInfo>(PC, BuffInfoWidgetClass);

				if (BuffInfoInstance)
				{
					BuffInfoStack->AddChild(BuffInfoInstance);
					BuffInfoInstance->InitializeBuffInfo(InBuffName, InDuration);
				}
			}
		}
	}
}

void ASpartaCharacter::OnSpeedDebuffEnd()
{
	SpeedDebuffStack = FMath::Max(0, SpeedDebuffStack - 1);
	if (SpeedDebuffStack == 0)
	{
		CurrentSpeedMultiplier = 1.0f;
		UpdateCharacterSpeed();
	}
}

void ASpartaCharacter::OnReverseControlDebuffEnd()
{
	ReverseControlStack = FMath::Max(0, ReverseControlStack - 1);
	if (ReverseControlStack == 0)
	{
		bIsControlReversed = false;
	}
}

void ASpartaCharacter::UpdateCharacterSpeed()
{
	const float BaseSpeed = bSprinting ? SprintSpeed : NormalSpeed;
	GetCharacterMovement()->MaxWalkSpeed = BaseSpeed * CurrentSpeedMultiplier;
}

