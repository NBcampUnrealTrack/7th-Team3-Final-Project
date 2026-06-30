#include "NCAssassinationComponent.h"
#include "Camera/CameraActor.h"
#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "NakwonClone/Zombie/ZombieCharacter/Base/VGMonsterCharacterBase.h"
#include "NakwonClone/Player/PlayerAnimation/NCCombatComponent.h"
#include "NakwonClone/Player/PlayerData/NCWeaponData.h"

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

// 헌호수정 - 암살 몽타지 재생 (서버 전용, DT에서 읽음)
void UNCAssassinationComponent::StartSlowMo()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	UNCCombatComponent* Combat = Owner->FindComponentByClass<UNCCombatComponent>();
	if (!Combat) return;

	const FNCWeaponData* WeaponData = Combat->GetEquippedWeaponData();
	if (!WeaponData) return;

	UAnimMontage* Montage = WeaponData->AssassinationMontage.LoadSynchronous();
	if (Montage)
		Owner->PlayAnimMontage(Montage);
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

	// 헌호수정 - 현재 무기 DT 데이터 읽기
	UNCCombatComponent* Combat = Owner->FindComponentByClass<UNCCombatComponent>();
	const FNCWeaponData* WeaponData = Combat ? Combat->GetEquippedWeaponData() : nullptr;

	if (Owner->HasAuthority())
	{
		StartSlowMo();

		// 헌호수정 - 좀비 리액션 몽타지 전달
		UAnimMontage* VictimMontage = WeaponData ? WeaponData->AssassinationVictimMontage.LoadSynchronous() : nullptr;
		Target->BeginAssassinationVictim(Owner, VictimMontage);
	}

	if (Owner->IsLocallyControlled())
		StartCamera(Target);

	// 헌호수정 - 즉사 타이머: DT 값 사용 (없으면 기본 0.8f)
	const float KillTime = WeaponData ? WeaponData->AssassinationKillTime : 0.8f;
	AVGMonsterCharacterBase* TargetRef = Target;
	GetWorld()->GetTimerManager().SetTimer(
		KillTimerHandle,
		[TargetRef]() { if (IsValid(TargetRef)) TargetRef->HandleDead(); },
		KillTime, false
	);

	// 카메라 복귀 타이머
	GetWorld()->GetTimerManager().SetTimer(
		CameraTimerHandle,
		[this]() { Finish(); },
		1.5f, false
	);
}
