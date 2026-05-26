#include "NCPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Common/NCGameplayTags.h"

ANCPlayerCharacter::ANCPlayerCharacter()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;       
	CameraBoom->bUsePawnControlRotation = true; 
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.0f;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void ANCPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ANCPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANCPlayerCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANCPlayerCharacter::Look);
		}
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ANCPlayerCharacter::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ANCPlayerCharacter::StopSprint);
		}
		if (WalkAction)
		{
			EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &ANCPlayerCharacter::ToggleWalk);
		}
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ANCPlayerCharacter::ToggleCrouch);
		}
	}
}

void ANCPlayerCharacter::Server_SetGait_Implementation(FGameplayTag NewGaitTag)
{
	CurrentGaitTag = NewGaitTag;
	ApplyMovementData(CurrentGaitTag);
}

void ANCPlayerCharacter::Server_SetStance_Implementation(FGameplayTag NewStanceTag)
{
	CurrentStanceTag = NewStanceTag;

	if (CurrentStanceTag == NCCharacter::Crouch)
	{
		Crouch();
		ApplyMovementData(CurrentStanceTag);
	}
	else
	{
		UnCrouch();
		ApplyMovementData(CurrentGaitTag);
	}
}

void ANCPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
        
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANCPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
        
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANCPlayerCharacter::StartSprint()
{
	CurrentGaitTag = NCCharacter::Sprint;
	ApplyMovementData(CurrentGaitTag);
	Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::StopSprint()
{
	CurrentGaitTag = NCCharacter::Jog;
	ApplyMovementData(CurrentGaitTag);
	Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::ToggleWalk()
{
	if (CurrentGaitTag == NCCharacter::Walk)
	{
		CurrentGaitTag = NCCharacter::Jog;
	}
	else
	{
		CurrentGaitTag = NCCharacter::Walk;
	}
	ApplyMovementData(CurrentGaitTag);
	Server_SetGait(CurrentGaitTag);
}

void ANCPlayerCharacter::ToggleCrouch()
{
	if (CurrentStanceTag == NCCharacter::Crouch) 
	{
		UnCrouch(); 
		CurrentStanceTag = NCCharacter::Stand;
		ApplyMovementData(CurrentGaitTag);
	}
	else
	{
		Crouch();
		CurrentStanceTag = NCCharacter::Crouch;
		ApplyMovementData(CurrentStanceTag);
	}
	Server_SetStance(CurrentStanceTag);
}
