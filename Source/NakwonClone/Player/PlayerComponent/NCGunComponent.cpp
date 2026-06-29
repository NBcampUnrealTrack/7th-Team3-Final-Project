#include "NCGunComponent.h"
#include "Common/NCGameplayTags.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
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
	{
		Cam->SetFieldOfView(TargetFOV);
		SetComponentTickEnabled(false);
		return;
	}

	Cam->SetFieldOfView(FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ADSInterpSpeed));

	// 헌호수정 - 반동 복귀
	if (!FMath::IsNearlyZero(CurrentRecoilPitch) || !FMath::IsNearlyZero(CurrentRecoilYaw))
	{
		const float RecoverySpeed = ActiveGunData ? ActiveGunData->RecoilRecoverySpeed : 5.f;

		ACharacter* Char = Cast<ACharacter>(GetOwner());
		APlayerController* PC = Char ? Cast<APlayerController>(Char->GetController()) : nullptr;
		if (PC && Char->IsLocallyControlled())
		{
			const float PitchStep = FMath::Min(FMath::Abs(CurrentRecoilPitch), RecoverySpeed * DeltaTime);
			const float YawStep   = FMath::Min(FMath::Abs(CurrentRecoilYaw),   RecoverySpeed * DeltaTime);

			PC->AddPitchInput(PitchStep);   // 위로 밀었던 만큼 복귀
			PC->AddYawInput(-FMath::Sign(CurrentRecoilYaw) * YawStep);

			CurrentRecoilPitch = FMath::Sign(CurrentRecoilPitch) * (FMath::Abs(CurrentRecoilPitch) - PitchStep);
			CurrentRecoilYaw   = FMath::Sign(CurrentRecoilYaw)   * (FMath::Abs(CurrentRecoilYaw)   - YawStep);

			if (FMath::IsNearlyZero(CurrentRecoilPitch, 0.01f)) CurrentRecoilPitch = 0.f;
			if (FMath::IsNearlyZero(CurrentRecoilYaw,   0.01f)) CurrentRecoilYaw   = 0.f;
		}
	}
}

// ─────────────────────────────────────────────
// 상태 조회

bool UNCGunComponent::IsFiring() const    { return ActiveGunActions.HasTag(NCGun::Action_Firing); }
bool UNCGunComponent::IsReloading() const { return ActiveGunActions.HasTag(NCGun::Action_Reloading); }
bool UNCGunComponent::IsADS() const       { return ActiveGunActions.HasTag(NCGun::Action_ADS); }

bool UNCGunComponent::CanFire() const
{
	if (!HasActiveGun())  return false;
	if (IsReloading())    return false;
	return CurrentAmmo > 0;
}

// ─────────────────────────────────────────────
// 활성화 / 비활성화

void UNCGunComponent::ActivateGun(const FNCGunData* InGunData, int32 InCurrentAmmo, int32 InReserveAmmo)
{
	ActiveGunData = InGunData;
	CurrentAmmo   = InCurrentAmmo;
	ReserveAmmo   = InReserveAmmo;

	if (ActiveGunData)
	{
		CurrentFireMode = ActiveGunData->DefaultFireMode;
		OnFireModeChanged.Broadcast(CurrentFireMode);
		AttachGunMesh(ActiveGunData);
		PlayGunMontage(ActiveGunData->EquipMontage);
	}
}

void UNCGunComponent::DeactivateGun()
{
	GetWorld()->GetTimerManager().ClearTimer(FullAutoTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
	ActiveGunActions.Reset();
	RestoreFOV();
	DetachGunMesh();
	ActiveGunData = nullptr;
}

void UNCGunComponent::PlayUnequipMontage(const FNCGunData* Data)
{
	if (Data) PlayGunMontage(Data->UnequipMontage);
}

// ─────────────────────────────────────────────
// 사격

void UNCGunComponent::StartFire()
{
	// 탄약 없을 때 빈 총 클릭음
	if (HasActiveGun() && !IsReloading() && CurrentAmmo <= 0)
	{
		if (!ActiveGunData->EmptyClickSound.IsNull())
			UGameplayStatics::PlaySound2D(this, ActiveGunData->EmptyClickSound.LoadSynchronous());
		return;
	}

	if (!CanFire()) return;

	ActiveGunActions.AddTag(NCGun::Action_Firing);
	FireOnce();

	if (CurrentFireMode == ENCFireMode::FullAuto)
	{
		const float Interval = (ActiveGunData->FireRate > 0.f) ? (1.f / ActiveGunData->FireRate) : 0.1f;
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

	const FNCGunData* Data = ActiveGunData;
	if (!Data || !Data->ProjectileClass) return;

	--CurrentAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, ReserveAmmo);

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// 헌호수정 - 발사 시 카메라 방향으로 캐릭터 즉시 회전
	if (ACharacter* RotChar = Cast<ACharacter>(Owner))
	{
		if (APlayerController* RotPC = Cast<APlayerController>(RotChar->GetController()))
		{
			FRotator ControlRot = RotPC->GetControlRotation();
			Owner->SetActorRotation(FRotator(0.f, ControlRot.Yaw, 0.f));
		}
	}
	// 기본 발사 방향: 카메라 전방 (조준 기준)
	FVector  SpawnLocation = Owner->GetActorLocation();
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
		MuzzleFlashComp->Activate(true);

	ApplyRecoil(Data);

	if (Data->FireShakeClass)
	{
		ACharacter* ShakeChar = Cast<ACharacter>(Owner);
		if (APlayerController* PC = ShakeChar ? Cast<APlayerController>(ShakeChar->GetController()) : nullptr)
			PC->ClientStartCameraShake(Data->FireShakeClass);
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

// 헌호수정 - 반동 입력 적용
void UNCGunComponent::ApplyRecoil(const FNCGunData* Data)
{
	if (!Data) return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* PC = Char ? Cast<APlayerController>(Char->GetController()) : nullptr;
	if (!PC || !Char->IsLocallyControlled()) return;

	const float PitchAmount = Data->RecoilPitch;
	const float YawAmount   = FMath::RandRange(-Data->RecoilYaw, Data->RecoilYaw);

	PC->AddPitchInput(-PitchAmount); // 위로 밀기 (Pitch 음수 = 위)
	PC->AddYawInput(YawAmount);

	CurrentRecoilPitch += PitchAmount;
	CurrentRecoilYaw   += YawAmount;

	SetComponentTickEnabled(true); // 복귀를 위해 Tick 활성화
}

// ─────────────────────────────────────────────
// 재장전

void UNCGunComponent::Reload()
{
	if (IsReloading() || !HasActiveGun()) return;

	const FNCGunData* Data = ActiveGunData;
	if (!Data) return;

	if (ReserveAmmo <= 0)                  return;
	if (CurrentAmmo >= Data->MagazineSize) return;

	StopFire();
	ActiveGunActions.AddTag(NCGun::Action_Reloading);

	PlayGunMontage(Data->ReloadMontage);

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

	const FNCGunData* Data = ActiveGunData;
	if (!Data) return;

	const int32 Needed = Data->MagazineSize - CurrentAmmo;
	const int32 Take   = FMath::Min(Needed, ReserveAmmo);
	CurrentAmmo += Take;
	ReserveAmmo -= Take;

	OnAmmoChanged.Broadcast(CurrentAmmo, ReserveAmmo);
}

// ─────────────────────────────────────────────
// ADS

void UNCGunComponent::StartADS()
{
	if (!HasActiveGun()) return;

	ActiveGunActions.AddTag(NCGun::Action_ADS);

	if (AActor* Owner = GetOwner())
	{
		if (UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
			ASC->AddLooseGameplayTag(NCWeapon::Action_Aiming);
	}

	ApplyADSFOV();

	if (ActiveGunData) PlayGunMontage(ActiveGunData->ADSInMontage);
}

void UNCGunComponent::StopADS()
{
	ActiveGunActions.RemoveTag(NCGun::Action_ADS);

	if (AActor* Owner = GetOwner())
	{
		if (UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
			ASC->RemoveLooseGameplayTag(NCWeapon::Action_Aiming);
	}

	RestoreFOV();

	if (ActiveGunData) PlayGunMontage(ActiveGunData->ADSOutMontage);
}

void UNCGunComponent::ApplyADSFOV()
{
	if (!ActiveGunData) return;
	TargetFOV = DefaultFOV * ActiveGunData->ADSFOVMultiplier;
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
	if (!ActiveGunData || !ActiveGunData->bCanToggleFireMode) return;

	ActiveGunActions.AddTag(NCGun::Action_ToggleFireMode);

	CurrentFireMode = (CurrentFireMode == ENCFireMode::SemiAuto)
		? ENCFireMode::FullAuto
		: ENCFireMode::SemiAuto;

	OnFireModeChanged.Broadcast(CurrentFireMode);

	ActiveGunActions.RemoveTag(NCGun::Action_ToggleFireMode);
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