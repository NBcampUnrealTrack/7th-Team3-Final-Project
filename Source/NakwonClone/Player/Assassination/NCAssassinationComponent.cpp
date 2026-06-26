#include "NCAssassinationComponent.h"
#include "Camera/CameraActor.h"
#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"

UNCAssassinationComponent::UNCAssassinationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// 헌호수정 - 범위 내 가장 가까운 좀비 반환
AVGMonsterCharacterBase* UNCAssassinationComponent::FindNearestTarget() const
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return nullptr;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	Owner->GetWorld()->OverlapMultiByChannel(
		Overlaps, Owner->GetActorLocation(), FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(AssassinationRange), Params
	);

	AVGMonsterCharacterBase* Nearest = nullptr;
	float NearestDist = MAX_FLT;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AVGMonsterCharacterBase* Monster = Cast<AVGMonsterCharacterBase>(Overlap.GetActor());
		if (!Monster) continue;

		const float Dist = FVector::Dist(Owner->GetActorLocation(), Monster->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			Nearest = Monster;
		}
	}

	return Nearest;
}

// 헌호수정 - 슬로우모션 + 몽타지 (서버 전용)
void UNCAssassinationComponent::StartSlowMo()
{
	if (AssassinationMontage)
	{
		ACharacter* Owner = Cast<ACharacter>(GetOwner());
		if (Owner)
			Owner->PlayAnimMontage(AssassinationMontage);
	}
}

// 헌호수정 - 액션 카메라 스폰 + 블렌드 (로컬 전용)
void UNCAssassinationComponent::StartCamera(AVGMonsterCharacterBase* Target)
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner || !Target) return;

	const FVector CamPos = Target->GetActorLocation()
		+ Target->GetActorRightVector() * 150.f
		+ FVector::UpVector * 80.f
		+ (-Target->GetActorForwardVector()) * 50.f;

	const FRotator CamRot = (Target->GetActorLocation() - CamPos).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AssassinationCamera = Owner->GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CamPos, CamRot, SpawnParams);

	if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
		PC->SetViewTargetWithBlend(AssassinationCamera, 0.3f, EViewTargetBlendFunction::VTBlend_EaseInOut); //헌호수정
}

// 헌호수정 - 슬로우모션 해제 + 카메라 복귀 + 정리
void UNCAssassinationComponent::Finish()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	if (Owner->IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
			PC->SetViewTargetWithBlend(Owner, 0.4f, EViewTargetBlendFunction::VTBlend_EaseInOut); //헌호수정

		FTimerHandle DestroyTimer;
		GetWorld()->GetTimerManager().SetTimer(
			DestroyTimer,
			[this]()
			{
				if (IsValid(AssassinationCamera))
				{
					AssassinationCamera->Destroy();
					AssassinationCamera = nullptr;
				}
			},
			0.5f, false
		);
	}
}

// 헌호수정 - 암살 진입점
void UNCAssassinationComponent::TryAssassinate()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	AVGMonsterCharacterBase* Target = FindNearestTarget();
	if (!Target) return;

	if (Owner->HasAuthority())
		StartSlowMo();

	if (Owner->IsLocallyControlled())
		StartCamera(Target);

	// 좀비 즉사 타이머
	AVGMonsterCharacterBase* TargetRef = Target;
	GetWorld()->GetTimerManager().SetTimer(
		KillTimerHandle,
		[TargetRef]() { if (IsValid(TargetRef)) TargetRef->HandleDead(); },
		0.8f, false
	);

	// 카메라 복귀 타이머
	GetWorld()->GetTimerManager().SetTimer(
		CameraTimerHandle,
		[this]() { Finish(); },
		1.5f, false
	);
}
