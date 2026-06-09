// Fill out your copyright notice in the Description page of Project Settings.

#include "NCAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "KismetAnimationLibrary.h"

void UNCAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        MovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UNCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerCharacter || !MovementComponent)
    {
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        if (!OwnerCharacter)
        {
            // TryGetPawnOwner 실패 시 스켈레탈 메시 오너에서 직접 가져오기
            if (USkeletalMeshComponent* Mesh = GetSkelMeshComponent())
            {
                OwnerCharacter = Cast<ACharacter>(Mesh->GetOwner());
            }
        }
        if (OwnerCharacter)
        {
            MovementComponent = OwnerCharacter->GetCharacterMovement();
        }
        if (!OwnerCharacter || !MovementComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("[NCAnimInstance] OwnerCharacter 또는 MovementComponent가 null!"));
            return;
        }
    }

    const FVector Velocity = OwnerCharacter->GetVelocity();

    Speed = Velocity.Size2D();
    Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
    bIsInAir = MovementComponent->IsFalling();
    bIsCrouching = MovementComponent->IsCrouching();
    bShouldMove = Speed > 3.f && !MovementComponent->GetCurrentAcceleration().IsNearlyZero();
}