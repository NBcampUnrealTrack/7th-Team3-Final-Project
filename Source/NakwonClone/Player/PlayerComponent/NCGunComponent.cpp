#include "NCGunComponent.h"
#include "Common/NCGameplayTags.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Gun/NCProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"

UNCGunComponent::UNCGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 평상시 Tick 꺼둠
}

void UNCGunComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UCameraComponent* Cam = FindCamera())
		DefaultFOV = TargetFOV = Cam->FieldOfView;
}

void UNCGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UCameraComponent* Cam = FindCamera();
	if (!Cam) return;

	// FOV 보간
	const float CurrentFOV = Cam->FieldOfView;
	if (FMath::IsNearlyEqual(CurrentFOV, TargetFOV, 0.1f))
		Cam->SetFieldOfView(TargetFOV);
	else
		Cam->SetFieldOfView(FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ADSInterpSpeed));

	// ADS 중 캐릭터 Yaw를 컨트롤러 방향으로 부드럽게 보간
	if (IsADS())
	{
		if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		{
			if (AController* Ctrl = Char->GetController())
			{
				const FRotator ActorRot = Char->GetActorRotation();
				const float TargetYaw   = Ctrl->GetControlRotation().Yaw;
				// 최단 경로로 회전
				const float DeltaYaw = FMath::UnwindDegrees(TargetYaw - ActorRot.Yaw);
				const float NewYaw   = ActorRot.Yaw + FMath::FInterpTo(0.f, DeltaYaw, DeltaTime, ADSInterpSpeed);
				Char->SetActorRotation(FRotator(ActorRot.Pitch, NewYaw, ActorRot.Roll));
			}
		}
	}
	else
	{
		// ADS도 아니고 FOV도 목표 도달이면 Tick 종료
		if (FMath::IsNearlyEqual(CurrentFOV, TargetFOV, 0.1f))
			SetComponentTickEnabled(false);
	}
}

// ─────────────────────────────────────────────
// 상태 조회

bool UNCGunComponent::IsFiring() const    { return ActiveGunActions.HasTag(NCGun::Action_Firing); }
bool UNCGunComponent::IsReloading() const { return ActiveGunActions.HasTag(NCGun::Action_Reloading); }
bool UNCGunComponent::IsADS() const       { return ActiveGunActions.HasTag(NCGun::Action_ADS); }
bool UNCGunComponent::IsSwapping() const  { return ActiveGunActions.HasTag(NCGun::Action_Swapping); }

bool UNCGunComponent::CanFire() const
{
	if (!HasActiveGun()) return false;
	if (IsReloading())   return false;
	if (IsSwapping())    return false;

	const FNCGunSlotData& SlotData = (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
	return SlotData.CurrentAmmo > 0;
}
// ─────────────────────────────────────────────
// 탄약 / 총기 정보 getter

int32 UNCGunComponent::GetCurrentAmmo() const
{
	if (!HasActiveGun()) return 0;
	return (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot.CurrentAmmo : SecondarySlot.CurrentAmmo;
}

int32 UNCGunComponent::GetReserveAmmo() const
{
	if (!HasActiveGun()) return 0;
	return (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot.ReserveAmmo : SecondarySlot.ReserveAmmo;
}

FName UNCGunComponent::GetActiveGunID() const
{
	if (ActiveSlot == ENCGunSlot::Primary)   return PrimarySlot.GunID;
	if (ActiveSlot == ENCGunSlot::Secondary) return SecondarySlot.GunID;
	return NAME_None;
}

FName UNCGunComponent::GetOccupantGunID(FName ForGunID) const
{
	const FNCGunData* Data = FindGunData(ForGunID);
	if (!Data) return NAME_None;

	if (Data->SlotType == ENCGunSlot::Primary)   return PrimarySlot.GunID;
	if (Data->SlotType == ENCGunSlot::Secondary)  return SecondarySlot.GunID;
	return NAME_None;
}

// ─────────────────────────────────────────────
// 장착 / 해제

bool UNCGunComponent::EquipGun(FName GunID)
{
	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	FNCGunSlotData& Slot = (Data->SlotType == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
	Slot.GunID       = GunID;
	Slot.CurrentAmmo = Data->MagazineSize;
	Slot.ReserveAmmo = Data->MaxReserveAmmo;

	if (ActiveSlot == Data->SlotType)
	{
		CurrentFireMode = Data->DefaultFireMode;
		OnFireModeChanged.Broadcast(CurrentFireMode);
		AttachGunMesh(Data);
	}

	OnGunEquipped.Broadcast(GunID);
	return true;
}

bool UNCGunComponent::EquipGunWithAmmo(FName GunID, int32 CurrentAmmo, int32 ReserveAmmo)
{
	const FNCGunData* Data = FindGunData(GunID);
	if (!Data) return false;

	FNCGunSlotData& Slot = (Data->SlotType == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
	Slot.GunID       = GunID;
	Slot.CurrentAmmo = FMath::Clamp(CurrentAmmo, 0, Data->MagazineSize);
	Slot.ReserveAmmo = FMath::Clamp(ReserveAmmo, 0, Data->MaxReserveAmmo);

	if (ActiveSlot == Data->SlotType)
	{
		CurrentFireMode = Data->DefaultFireMode;
		OnFireModeChanged.Broadcast(CurrentFireMode);
		AttachGunMesh(Data);
	}

	OnGunEquipped.Broadcast(GunID);
	return true;
}

void UNCGunComponent::UnequipGun(ENCGunSlot Slot)
{
	if (Slot == ENCGunSlot::Primary)   PrimarySlot.Clear();
	if (Slot == ENCGunSlot::Secondary) SecondarySlot.Clear();

	if (ActiveSlot == Slot)
	{
		GetWorld()->GetTimerManager().ClearTimer(FullAutoTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(SwapTimerHandle);
		ActiveSlot = ENCGunSlot::None;
		ActiveGunActions.Reset();
		RestoreFOV();
		DetachGunMesh();
		OnGunUnequipped.Broadcast();
	}
}

void UNCGunComponent::SelectSlot(ENCGunSlot Slot)
{
	if (ActiveSlot == Slot) return;
	if (IsSwapping()) return;

	StopFire();
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	RestoreFOV();

	if (const FNCGunData* CurrentData = GetActiveGunData())
		PlayGunMontage(CurrentData->UnequipMontage);

	// 태그 정리 후 스왑
	ActiveGunActions.Reset();
	ActiveGunActions.AddTag(NCGun::Action_Swapping);

	// SwapDelay 후 실제 슬롯 전환
	GetWorld()->GetTimerManager().SetTimer(
		SwapTimerHandle,
		FTimerDelegate::CreateUObject(this, &UNCGunComponent::OnSwapFinished, Slot),
		SwapDelay, false);
}

void UNCGunComponent::OnSwapFinished(ENCGunSlot TargetSlot)
{
	ActiveGunActions.RemoveTag(NCGun::Action_Swapping);
	ActiveSlot = TargetSlot;

	const FNCGunData* Data = GetActiveGunData();
	if (Data)
	{
		CurrentFireMode = Data->DefaultFireMode;
		OnFireModeChanged.Broadcast(CurrentFireMode);
		AttachGunMesh(Data);
		PlayGunMontage(Data->EquipMontage);

		const FNCGunSlotData& SlotData = GetActiveSlotData();
		OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);
		OnGunEquipped.Broadcast(SlotData.GunID);
	}
	else
	{
		DetachGunMesh();
		OnGunUnequipped.Broadcast();
	}

	OnSwapCompleted.Broadcast(TargetSlot);
	if (Data)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("GunTypeTag = %s"),
			*Data->GunTypeTag.ToString());
	}
}

// ─────────────────────────────────────────────
// 사격

void UNCGunComponent::StartFire()
{
	// 탄약 없을 때 빈 총 클릭음
	if (HasActiveGun() && !IsReloading() && !IsSwapping())
	{
		const FNCGunSlotData& SlotData = (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
		if (SlotData.CurrentAmmo <= 0)
		{
			const FNCGunData* Data = GetActiveGunData();
			if (Data && !Data->EmptyClickSound.IsNull())
				UGameplayStatics::PlaySound2D(this, Data->EmptyClickSound.LoadSynchronous());
			return;
		}
	}

	if (!CanFire()) return;

	ActiveGunActions.AddTag(NCGun::Action_Firing);
	FireOnce();

	if (CurrentFireMode == ENCFireMode::FullAuto)
	{
		const FNCGunData* Data = GetActiveGunData();
		if (!Data) return;

		const float Interval = (Data->FireRate > 0.f) ? (1.f / Data->FireRate) : 0.1f;
		GetWorld()->GetTimerManager().SetTimer(
			FullAutoTimerHandle,
			this, &UNCGunComponent::FireOnce,
			Interval, true);
	}
	else
	{
		ActiveGunActions.RemoveTag(NCGun::Action_Firing);
	}
}

void UNCGunComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FullAutoTimerHandle);
	ActiveGunActions.RemoveTag(NCGun::Action_Firing);
}

void UNCGunComponent::FireOnce()
{
	if (!CanFire())
	{
		StopFire();
		return;
	}

	const FNCGunData* Data = GetActiveGunData();
	if (!Data || !Data->ProjectileClass) return;

	FNCGunSlotData& SlotData = GetActiveSlotData();
	--SlotData.CurrentAmmo;
	OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector SpawnLocation = Owner->GetActorLocation();
	FRotator SpawnRotation = Owner->GetActorRotation();

	FVector CamLocation = SpawnLocation;
	FVector CamForward  = Owner->GetActorForwardVector();
	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		if (UCameraComponent* Cam = Char->FindComponentByClass<UCameraComponent>())
		{
			CamLocation = Cam->GetComponentLocation();
			CamForward  = Cam->GetComponentRotation().Vector();
		}
	}

	FVector AimPoint = CamLocation + CamForward * Data->MaxRange;
	FHitResult AimHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	if (GetWorld()->LineTraceSingleByChannel(AimHit, CamLocation, AimPoint, ECC_Visibility, Params))
		AimPoint = AimHit.ImpactPoint;

	if (EquippedGunMeshComp && !Data->MuzzleSocketName.IsNone()
		&& EquippedGunMeshComp->DoesSocketExist(Data->MuzzleSocketName))
	{
		SpawnLocation = EquippedGunMeshComp->GetSocketLocation(Data->MuzzleSocketName);
		const FVector ToAim = AimPoint - SpawnLocation;
		SpawnRotation = ToAim.SizeSquared() > (10.f * 10.f) ? ToAim.Rotation() : CamForward.Rotation();
	}
	else
	{
		SpawnLocation = CamLocation;
		SpawnRotation = CamForward.Rotation();
	}


	PlayGunMontage(Data->FireMontage);

	// 발사음 재생
	if (!Data->FireSound.IsNull())
		UGameplayStatics::PlaySoundAtLocation(this, Data->FireSound.LoadSynchronous(), SpawnLocation);

	if (MuzzleFlashComp)
	{
		GetWorld()->GetTimerManager().ClearTimer(MuzzleFlashTimerHandle);
		MuzzleFlashComp->Activate(true);
		GetWorld()->GetTimerManager().SetTimer(MuzzleFlashTimerHandle, [this]()
		{
			if (MuzzleFlashComp) MuzzleFlashComp->Deactivate();
		}, 0.08f, false);
	}
	else if (EquippedGunMeshComp && !Data->MuzzleFlashParticle.IsNull())
		UGameplayStatics::SpawnEmitterAttached(
			Data->MuzzleFlashParticle.LoadSynchronous(), EquippedGunMeshComp, Data->MuzzleSocketName,
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);

	// 탄피 이펙트 (메시 소켓 기준)
	if (EquippedGunMeshComp)
	{
		if (!Data->ShellCasingEffect.IsNull())
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				Data->ShellCasingEffect.LoadSynchronous(), EquippedGunMeshComp, Data->EjectSocketName,
				FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		else if (!Data->ShellCasingParticle.IsNull())
			UGameplayStatics::SpawnEmitterAttached(
				Data->ShellCasingParticle.LoadSynchronous(), EquippedGunMeshComp, Data->EjectSocketName,
				FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);
	}

	const int32 PelletCount = FMath::Max(1, Data->NumPellets);
	for (int32 i = 0; i < PelletCount; ++i)
	{
		FRotator PelletRotation = SpawnRotation;
		if (Data->SpreadAngle > 0.f)
		{
			const float Spread = Data->SpreadAngle * (IsADS() ? Data->ADSSpreadMultiplier : 1.f);
			PelletRotation.Yaw   += FMath::RandRange(-Spread, Spread);
			PelletRotation.Pitch += FMath::RandRange(-Spread, Spread);
		}

		// 탄도 디버그 라인 — ADS: 파란색 / 힙파이어: 빨간색 (총구 소켓 발사 전환 후 재테스트)
		// DrawDebugLine(GetWorld(), SpawnLocation,
		// 	SpawnLocation + PelletRotation.Vector() * Data->MaxRange,
		// 	IsADS() ? FColor::Blue : FColor::Red,
		// 	false, 3.f, 0, 1.f);

		ANCProjectile* NCProj = GetWorld()->SpawnActorDeferred<ANCProjectile>(
			Data->ProjectileClass,
			FTransform(PelletRotation, SpawnLocation),
			Owner, Cast<APawn>(Owner),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (NCProj)
		{
			NCProj->Damage                = Data->Damage;
			NCProj->MaxRange              = Data->MaxRange;
			NCProj->ProjectileSpeed       = Data->ProjectileSpeed;
			NCProj->ImpactFleshEffect     = Data->ImpactFleshEffect.Get();
			NCProj->ImpactSurfaceEffect   = Data->ImpactSurfaceEffect.Get();
			NCProj->ImpactFleshParticle   = Data->ImpactFleshParticle.Get();
			NCProj->ImpactSurfaceParticle = Data->ImpactSurfaceParticle.Get();
			if (!Data->TracerEffect.IsNull())
				NCProj->TracerEffect = Data->TracerEffect.LoadSynchronous();
			NCProj->FinishSpawning(FTransform(PelletRotation, SpawnLocation));
		}
	}
}
// ─────────────────────────────────────────────
// 재장전

void UNCGunComponent::Reload()
{
	if (IsReloading() || IsSwapping() || !HasActiveGun()) return;

	FNCGunSlotData& SlotData = GetActiveSlotData();
	const FNCGunData* Data   = GetActiveGunData();
	if (!Data) return;

	if (SlotData.ReserveAmmo <= 0)                  return;
	if (SlotData.CurrentAmmo >= Data->MagazineSize) return;

	StopFire();
	ActiveGunActions.AddTag(NCGun::Action_Reloading);

	PlayGunMontage(Data->ReloadMontage);

	// 재장전음 재생
	if (!Data->ReloadSound.IsNull())
		UGameplayStatics::PlaySoundAtLocation(this, Data->ReloadSound.LoadSynchronous(), GetOwner()->GetActorLocation());

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this, &UNCGunComponent::OnReloadFinished,
		Data->ReloadTime, false);
}

void UNCGunComponent::OnReloadFinished()
{
	ActiveGunActions.RemoveTag(NCGun::Action_Reloading);

	FNCGunSlotData& SlotData = GetActiveSlotData();
	const FNCGunData* Data   = GetActiveGunData();
	if (!Data) return;

	const int32 Needed = Data->MagazineSize - SlotData.CurrentAmmo;
	const int32 Take   = FMath::Min(Needed, SlotData.ReserveAmmo);
	SlotData.CurrentAmmo += Take;
	SlotData.ReserveAmmo -= Take;

	OnAmmoChanged.Broadcast(SlotData.CurrentAmmo, SlotData.ReserveAmmo);
}

// ─────────────────────────────────────────────
// ADS

void UNCGunComponent::StartADS()
{
	if (!HasActiveGun() || IsSwapping()) return;

	ActiveGunActions.AddTag(NCGun::Action_ADS);

	if (AActor* Owner = GetOwner())
	{
		if (UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
		{
			ASC->AddLooseGameplayTag(NCWeapon::Action_Aiming);
		}
	}

	ApplyADSFOV();

	// ADS 중에는 이동 방향 회전 끄기
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		Char->GetCharacterMovement()->bOrientRotationToMovement = false;
	}

	const FNCGunData* Data = GetActiveGunData();
	if (Data) PlayGunMontage(Data->ADSInMontage);
}

void UNCGunComponent::StopADS()
{
	ActiveGunActions.RemoveTag(NCGun::Action_ADS);

	if (AActor* Owner = GetOwner())
	{
		if (UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
		{
			ASC->RemoveLooseGameplayTag(NCWeapon::Action_Aiming);
		}
	}

	// ADS 해제 시 이동 방향 회전 복귀
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		Char->GetCharacterMovement()->bOrientRotationToMovement = true;
	}

	RestoreFOV();

	const FNCGunData* Data = GetActiveGunData();
	if (Data) PlayGunMontage(Data->ADSOutMontage);
}

void UNCGunComponent::ApplyADSFOV()
{
	const FNCGunData* Data = GetActiveGunData();
	if (!Data) return;

	TargetFOV = DefaultFOV * Data->ADSFOVMultiplier;
	SetComponentTickEnabled(true);
}

void UNCGunComponent::RestoreFOV()
{
	TargetFOV = DefaultFOV;
	SetComponentTickEnabled(true);
}

UCameraComponent* UNCGunComponent::FindCamera() const
{
	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		return Char->FindComponentByClass<UCameraComponent>();
	return nullptr;
}

// ─────────────────────────────────────────────
// 발사 모드 전환

void UNCGunComponent::ToggleFireMode()
{
	const FNCGunData* Data = GetActiveGunData();
	if (!Data || !Data->bCanToggleFireMode) return;

	ActiveGunActions.AddTag(NCGun::Action_ToggleFireMode);

	CurrentFireMode = (CurrentFireMode == ENCFireMode::SemiAuto)
		? ENCFireMode::FullAuto
		: ENCFireMode::SemiAuto;

	OnFireModeChanged.Broadcast(CurrentFireMode);

	ActiveGunActions.RemoveTag(NCGun::Action_ToggleFireMode);
}

// ─────────────────────────────────────────────
// 헬퍼

const FNCGunData* UNCGunComponent::GetActiveGunData() const
{
	if (ActiveSlot == ENCGunSlot::Primary)   return FindGunData(PrimarySlot.GunID);
	if (ActiveSlot == ENCGunSlot::Secondary) return FindGunData(SecondarySlot.GunID);
	return nullptr;
}

FNCGunSlotData& UNCGunComponent::GetActiveSlotData()
{
	return (ActiveSlot == ENCGunSlot::Primary) ? PrimarySlot : SecondarySlot;
}

const FNCGunData* UNCGunComponent::FindGunData(FName GunID) const
{
	if (!GunDataTable || GunID.IsNone()) return nullptr;
	return GunDataTable->FindRow<FNCGunData>(GunID, TEXT("NCGunComponent"));
}

// ─────────────────────────────────────────────
// 몽타주 재생

void UNCGunComponent::PlayGunMontage(const TSoftObjectPtr<UAnimMontage>& MontageSoft)
{
	if (MontageSoft.IsNull()) return;
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;
	if (UAnimMontage* Montage = MontageSoft.LoadSynchronous())
		Char->PlayAnimMontage(Montage);
}

// ─────────────────────────────────────────────
// 총기 메시 부착 / 해제

void UNCGunComponent::AttachGunMesh(const FNCGunData* Data)
{
	DetachGunMesh();
	if (!Data || Data->GunMesh.IsNull()) return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	UStaticMesh* Mesh = Data->GunMesh.LoadSynchronous();
	if (!Mesh) return;

	EquippedGunMeshComp = NewObject<UStaticMeshComponent>(Char);
	EquippedGunMeshComp->SetStaticMesh(Mesh);
	EquippedGunMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedGunMeshComp->RegisterComponent();

	// TODO: 찬우님이 스켈레톤에 총기 전용 소켓 추가하면 DT_GunData HandSocketName에 입력
	//       소켓 미존재 시 루트 본에 부착됨 (임시 확인용)
	EquippedGunMeshComp->AttachToComponent(
		Char->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		Data->HandSocketName);

	if (!Data->MuzzleFlashEffect.IsNull())
	{
		MuzzleFlashComp = NewObject<UNiagaraComponent>(Char);
		MuzzleFlashComp->SetAsset(Data->MuzzleFlashEffect.LoadSynchronous());
		MuzzleFlashComp->SetAutoActivate(false);
		MuzzleFlashComp->RegisterComponent();
		MuzzleFlashComp->AttachToComponent(
			EquippedGunMeshComp,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			Data->MuzzleSocketName);
		MuzzleFlashComp->OnSystemFinished.AddDynamic(this, &UNCGunComponent::OnMuzzleFlashFinished);
	}
}

void UNCGunComponent::OnMuzzleFlashFinished(UNiagaraComponent* /*PSystem*/)
{
	if (MuzzleFlashComp)
		MuzzleFlashComp->Deactivate();
}

void UNCGunComponent::DetachGunMesh()
{
	if (MuzzleFlashComp)
	{
		MuzzleFlashComp->DestroyComponent();
		MuzzleFlashComp = nullptr;
	}
	if (EquippedGunMeshComp)
	{
		EquippedGunMeshComp->DestroyComponent();
		EquippedGunMeshComp = nullptr;
	}
}
