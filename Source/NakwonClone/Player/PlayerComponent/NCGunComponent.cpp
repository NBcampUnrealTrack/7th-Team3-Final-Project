#include "NCGunComponent.h"
#include "Common/NCGameplayTags.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Gun/NCProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NakwonClone/Player/PlayerCharacter/NCPlayerCharacter.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

UNCGunComponent::UNCGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 평상시 Tick 꺼둠
}

void UNCGunComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UCameraComponent* Cam = FindCamera())
	{
		DefaultFOV = TargetFOV = Cam->FieldOfView;

		DefaultCameraLocation = Cam->GetRelativeLocation();
		TargetCameraLocation = DefaultCameraLocation;
	}
}

void UNCGunComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FullAutoTimerHandle);
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
		World->GetTimerManager().ClearTimer(MuzzleFlashTimerHandle);
		World->GetTimerManager().ClearTimer(ShowMagazineTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UNCGunComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UCameraComponent* Cam = FindCamera();
	if (!Cam) return;

	const float NewFOV = FMath::FInterpTo(
		Cam->FieldOfView,
		TargetFOV,
		DeltaTime,
		ADSInterpSpeed);

	Cam->SetFieldOfView(NewFOV);

	const FVector NewLocation = FMath::VInterpTo(
		Cam->GetRelativeLocation(),
		TargetCameraLocation,
		DeltaTime,
		ADSInterpSpeed);

	Cam->SetRelativeLocation(NewLocation);

	if (!FMath::IsNearlyZero(CurrentRecoilPitch) ||
		!FMath::IsNearlyZero(CurrentRecoilYaw))
	{
		const float RecoverySpeed =
			ActiveGunData ? ActiveGunData->RecoilRecoverySpeed : 5.f;

		ACharacter* Char = Cast<ACharacter>(GetOwner());
		APlayerController* PC =
			Char ? Cast<APlayerController>(Char->GetController()) : nullptr;

		if (PC && Char->IsLocallyControlled())
		{
			const float PitchStep =
				FMath::Min(FMath::Abs(CurrentRecoilPitch),
					RecoverySpeed * DeltaTime);

			const float YawStep =
				FMath::Min(FMath::Abs(CurrentRecoilYaw),
					RecoverySpeed * DeltaTime);

			PC->AddPitchInput(PitchStep);
			PC->AddYawInput(-FMath::Sign(CurrentRecoilYaw) * YawStep);

			CurrentRecoilPitch =
				FMath::Sign(CurrentRecoilPitch) *
				(FMath::Abs(CurrentRecoilPitch) - PitchStep);

			CurrentRecoilYaw =
				FMath::Sign(CurrentRecoilYaw) *
				(FMath::Abs(CurrentRecoilYaw) - YawStep);

			if (FMath::IsNearlyZero(CurrentRecoilPitch, 0.01f))
				CurrentRecoilPitch = 0.f;

			if (FMath::IsNearlyZero(CurrentRecoilYaw, 0.01f))
				CurrentRecoilYaw = 0.f;
		}
	}

	const bool bFOVFinished =
		FMath::IsNearlyEqual(Cam->FieldOfView, TargetFOV, 0.1f);

	const bool bLocationFinished =
		Cam->GetRelativeLocation().Equals(TargetCameraLocation, 0.1f);

	if (bFOVFinished &&
		bLocationFinished &&
		FMath::IsNearlyZero(CurrentRecoilPitch) &&
		FMath::IsNearlyZero(CurrentRecoilYaw))
	{
		SetComponentTickEnabled(false);
	}
}

// ─────────────────────────────────────────────
// 상태 조회

bool UNCGunComponent::IsFiring() const    { return ActiveGunActions.HasTag(NCGun::Action_Firing); }
bool UNCGunComponent::IsReloading() const { return ActiveGunActions.HasTag(NCGun::Action_Reloading); }
bool UNCGunComponent::IsADS() const
{
	return ActiveGunActions.HasTag(NCGun::Action_ADS);
}

bool UNCGunComponent::CanFire() const
{
	if (!HasActiveGun())  return false;
	if (IsReloading())    return false;
	if (CurrentAmmo <= 0) return false;

	if (CurrentFireMode == ENCFireMode::SemiAuto && ActiveGunData->FireRate > 0.f && GetWorld())
	{
		const float MinInterval = 1.f / ActiveGunData->FireRate;
		if (GetWorld()->GetTimeSeconds() - LastFireTime < MinInterval)
			return false;
	}

	return true;
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
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FullAutoTimerHandle);
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
		World->GetTimerManager().ClearTimer(ShowMagazineTimerHandle);
		World->GetTimerManager().ClearTimer(MuzzleFlashTimerHandle);
	}

	StopFire();

	bWantsADS = false;
	ActiveGunActions.RemoveTag(NCGun::Action_ADS);
	ActiveGunActions.RemoveTag(NCGun::Action_Reloading);
	ActiveGunActions.RemoveTag(NCGun::Action_Firing);

	if (AActor* Owner = GetOwner())
	{
		if (UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
		{
			ASC->RemoveLooseGameplayTag(NCWeapon::Action_Aiming);
		}
	}

	ActiveGunActions.Reset();

	RestoreFOV();
	DetachGunMesh();

	ActiveGunData = nullptr;
}

float UNCGunComponent::PlayUnequipMontage(const FNCGunData* Data)
{
	if (!Data)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GunUnequip] Data NULL"));
		return 0.f;
	}

	if (Data->UnequipMontage.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GunUnequip] UnequipMontage NULL"));
		return 0.f;
	}

	return PlayGunMontage(Data->UnequipMontage);
}

// ─────────────────────────────────────────────
// 사격

void UNCGunComponent::StartFire()
{
	OnBeforeFire();

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
		if (HasActiveGun() && !IsReloading() && CurrentAmmo <= 0 && !ActiveGunData->EmptyClickSound.IsNull())
			UGameplayStatics::PlaySound2D(this, ActiveGunData->EmptyClickSound.LoadSynchronous());

		StopFire();
		return;
	}

	const FNCGunData* Data = ActiveGunData;
	if (!Data || !Data->ProjectileClass) return;

	LastFireTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastFireTime;

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
	FVector CamLocation = Owner->GetActorLocation();
	FVector CamForward  = Owner->GetActorForwardVector();
	if (UCameraComponent* Cam = FindCamera())
	{
		CamLocation = Cam->GetComponentLocation();
		CamForward  = Cam->GetComponentRotation().Vector();
	}

	const FVector  SpawnLocation = CamLocation;
	const FRotator SpawnRotation = CamForward.Rotation();

	PlayGunMontage(Data->FireMontage);
	// 발사음 재생
	if (!Data->FireSound.IsNull())
	{
		const float FirePitch = FMath::RandRange(1.f - Data->FirePitchVariance, 1.f + Data->FirePitchVariance);
		UGameplayStatics::PlaySoundAtLocation(
			this, Data->FireSound.LoadSynchronous(), SpawnLocation,
			FRotator::ZeroRotator, 1.f, FirePitch);
	}

	if (MuzzleFlashComp)
		MuzzleFlashComp->Activate(true);

	// 총기 메시 자체 애니메이션 재생
	if (EquippedGunSkelMeshComp && !Data->GunFireAnimation.IsNull())
	{
		if (UAnimSequence* GunAnim = Data->GunFireAnimation.LoadSynchronous())
			EquippedGunSkelMeshComp->PlayAnimation(GunAnim, false);
	}

	// 탄피 배출 이펙트 — EjectSocketName 소켓에서 스폰
	// (4ca9f351 "총기 타입별 컴포넌트 분리 리팩토링"에서 누락된 것 복구. 스켈레탈/스태틱 메시 모두 대응)
	{
		UMeshComponent* GunMeshComp = EquippedGunSkelMeshComp
			? static_cast<UMeshComponent*>(EquippedGunSkelMeshComp)
			: static_cast<UMeshComponent*>(EquippedGunMeshComp);

		if (GunMeshComp && GunMeshComp->DoesSocketExist(Data->EjectSocketName))
		{
			const FTransform EjectXform = GunMeshComp->GetSocketTransform(Data->EjectSocketName);

			if (!Data->ShellCasingEffect.IsNull())
			{
				if (UNiagaraSystem* ShellFX = Data->ShellCasingEffect.LoadSynchronous())
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						this, ShellFX, EjectXform.GetLocation(), EjectXform.Rotator());
			}
			else if (!Data->ShellCasingParticle.IsNull())
			{
				if (UParticleSystem* ShellP = Data->ShellCasingParticle.LoadSynchronous())
					UGameplayStatics::SpawnEmitterAtLocation(
						this, ShellP, EjectXform.GetLocation(), EjectXform.Rotator());
			}
		}
	}

	ApplyRecoil(Data);

	const float ShakeScale = IsADS() ? Data->ADSShakeMultiplier : 1.f;

	if (Data->FireShakeClass)
	{
		ACharacter* ShakeChar = Cast<ACharacter>(Owner);
		if (APlayerController* PC = ShakeChar ? Cast<APlayerController>(ShakeChar->GetController()) : nullptr)
			PC->ClientStartCameraShake(Data->FireShakeClass, ShakeScale);
	}

	float MuzzleDistance = 0.f;
	{
		UMeshComponent* MuzzleMeshComp = EquippedGunSkelMeshComp
			? static_cast<UMeshComponent*>(EquippedGunSkelMeshComp)
			: static_cast<UMeshComponent*>(EquippedGunMeshComp);

		if (MuzzleMeshComp && !Data->MuzzleSocketName.IsNone()
			&& MuzzleMeshComp->DoesSocketExist(Data->MuzzleSocketName))
		{
			MuzzleDistance = (MuzzleMeshComp->GetSocketLocation(Data->MuzzleSocketName) - CamLocation).Size();
		}
	}
	const FVector SpreadOrigin = CamLocation + CamForward * MuzzleDistance;

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

		const FVector PelletSpawnLocation = SpreadOrigin;

		ANCProjectile* NCProj = GetWorld()->SpawnActorDeferred<ANCProjectile>(
			Data->ProjectileClass,
			FTransform(PelletRotation, PelletSpawnLocation),
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
			NCProj->HitShakeClass         = Data->HitShakeClass;
			NCProj->HitShakeScale         = ShakeScale;
			if (!Data->TracerEffect.IsNull())
				NCProj->TracerEffect = Data->TracerEffect.LoadSynchronous();
			NCProj->FinishSpawning(FTransform(PelletRotation, PelletSpawnLocation));
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

	if (ReserveAmmo <= 0) return;
	if (CurrentAmmo >= Data->MagazineSize) return;

	StopFire();
	ActiveGunActions.AddTag(NCGun::Action_Reloading);

	if (!Data->ReloadSound.IsNull())
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Data->ReloadSound.LoadSynchronous(),
			GetOwner()->GetActorLocation());
	}

	// 캐릭터 장전 몽타주
	const float MontageLength = PlayGunMontage(Data->ReloadMontage);
	const float ReloadDuration = MontageLength > 0.f ? MontageLength : Data->ReloadTime;

	// 총기 탄창 숨김 + 떨어지는 탄창 생성
	HideGunMagazine();
	DropMagazineMesh();

	// 총기 스켈레탈 메시 자체 장전 애니메이션이 있으면 재생
	if (EquippedGunSkelMeshComp && !Data->GunReloadAnimation.IsNull())
	{
		if (UAnimSequence* ReloadAnim = Data->GunReloadAnimation.LoadSynchronous())
		{
			EquippedGunSkelMeshComp->PlayAnimation(ReloadAnim, false);
		}
	}

	// 장전 끝나면 탄창 다시 보이게 (몽타주 실제 길이 기준)
	GetWorld()->GetTimerManager().SetTimer(
		ShowMagazineTimerHandle,
		this,
		&UNCGunComponent::ShowGunMagazine,
		ReloadDuration,
		false);

	// 장전 완료 처리 (몽타주 실제 길이 기준)
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&UNCGunComponent::OnReloadFinished,
		ReloadDuration,
		false);
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

	if (bWantsADS)
	{
		StartADS();
	}
}

// ─────────────────────────────────────────────
// ADS

void UNCGunComponent::StartADS()
{
	if (!HasActiveGun())
	{
		return;
	}

	bWantsADS = true;
	ActiveGunActions.AddTag(NCGun::Action_ADS);

	if (AActor* Owner = GetOwner())
	{
		if (UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
		{
			ASC->AddLooseGameplayTag(NCWeapon::Action_Aiming);
		}
	}

	ApplyADSFOV();
}

void UNCGunComponent::StopADS()
{
	bWantsADS = false;
	ActiveGunActions.RemoveTag(NCGun::Action_ADS);

	if (AActor* Owner = GetOwner())
	{
		if (UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>())
		{
			ASC->RemoveLooseGameplayTag(NCWeapon::Action_Aiming);
		}
	}

	RestoreFOV();
}

void UNCGunComponent::ApplyADSFOV()
{
	if (!ActiveGunData)
		return;

	TargetFOV = DefaultFOV * ActiveGunData->ADSFOVMultiplier;

	TargetCameraLocation = ADSCameraLocation;

	SetComponentTickEnabled(true);
}

void UNCGunComponent::RestoreFOV()
{
	TargetFOV = DefaultFOV;

	TargetCameraLocation = DefaultCameraLocation;

	SetComponentTickEnabled(true);
}

UCameraComponent* UNCGunComponent::FindCamera()
{
	if (CachedCamera)
	{
		return CachedCamera;
	}

	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
	{
		CachedCamera = Char->FindComponentByClass<UCameraComponent>();
	}

	return CachedCamera;
}

// ─────────────────────────────────────────────
// 발사 모드 전환

void UNCGunComponent::ToggleFireMode()
{
	if (!ActiveGunData || !ActiveGunData->bCanToggleFireMode) return;

	if (IsFiring()) return;

	CurrentFireMode = (CurrentFireMode == ENCFireMode::SemiAuto)
		? ENCFireMode::FullAuto
		: ENCFireMode::SemiAuto;

	if (!ActiveGunData->ToggleFireModeSound.IsNull())
		UGameplayStatics::PlaySound2D(this, ActiveGunData->ToggleFireModeSound.LoadSynchronous());

	OnFireModeChanged.Broadcast(CurrentFireMode);
}

// ─────────────────────────────────────────────
// 몽타주 재생

float UNCGunComponent::PlayGunMontage(const TSoftObjectPtr<UAnimMontage>& MontageSoft)
{
	if (MontageSoft.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GunMontage] MontageSoft NULL"));
		return 0.f;
	}

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GunMontage] Owner Character NULL"));
		return 0.f;
	}

	UAnimMontage* Montage = MontageSoft.LoadSynchronous();
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GunMontage] Load Failed"));
		return 0.f;
	}

	const float Length = Char->PlayAnimMontage(Montage);

	return Length;
}

// ─────────────────────────────────────────────
// 총기 메시 부착 / 해제

void UNCGunComponent::AttachGunMesh(const FNCGunData* Data)
{
	DetachGunMesh();
	if (!Data) return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	USceneComponent* AttachedMeshComp = nullptr;

	if (!Data->GunSkeletalMesh.IsNull())
	{
		USkeletalMesh* SkelMesh = Data->GunSkeletalMesh.LoadSynchronous();
		if (SkelMesh)
		{
			EquippedGunSkelMeshComp = NewObject<USkeletalMeshComponent>(Char);
			EquippedGunSkelMeshComp->SetSkeletalMesh(SkelMesh);
			EquippedGunSkelMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			EquippedGunSkelMeshComp->RegisterComponent();
			EquippedGunSkelMeshComp->AttachToComponent(
				Char->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				Data->HandSocketName);
			AttachedMeshComp = EquippedGunSkelMeshComp;
		}
	}
	else if (!Data->GunMesh.IsNull())
	{
		UStaticMesh* Mesh = Data->GunMesh.LoadSynchronous();
		if (Mesh)
		{
			EquippedGunMeshComp = NewObject<UStaticMeshComponent>(Char);
			EquippedGunMeshComp->SetStaticMesh(Mesh);
			EquippedGunMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			EquippedGunMeshComp->RegisterComponent();
			EquippedGunMeshComp->AttachToComponent(
				Char->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				Data->HandSocketName);
			AttachedMeshComp = EquippedGunMeshComp;
		}
	}

	if (AttachedMeshComp && !Data->MuzzleFlashEffect.IsNull())
	{
		MuzzleFlashComp = NewObject<UNiagaraComponent>(Char);
		MuzzleFlashComp->SetAsset(Data->MuzzleFlashEffect.LoadSynchronous());
		MuzzleFlashComp->SetAutoActivate(false);
		MuzzleFlashComp->RegisterComponent();
		MuzzleFlashComp->AttachToComponent(
			AttachedMeshComp,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			Data->MuzzleSocketName);
		MuzzleFlashComp->SetWorldScale3D(FVector(Data->MuzzleFlashScale));
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
	if (EquippedGunSkelMeshComp)
	{
		EquippedGunSkelMeshComp->DestroyComponent();
		EquippedGunSkelMeshComp = nullptr;
	}
}

void UNCGunComponent::DropMagazineMesh()
{
	if (!ActiveGunData || ActiveGunData->MagazineDropMesh.IsNull())
		return;

	if (!EquippedGunSkelMeshComp)
		return;

	UStaticMesh* MagMesh = ActiveGunData->MagazineDropMesh.LoadSynchronous();
	if (!MagMesh)
		return;

	FVector SpawnLocation = EquippedGunSkelMeshComp->GetComponentLocation();
	FRotator SpawnRotation = EquippedGunSkelMeshComp->GetComponentRotation();

	const FName MagazineBoneName = TEXT("Magazine_joint");

	if (EquippedGunSkelMeshComp->GetBoneIndex(MagazineBoneName) != INDEX_NONE)
	{
		SpawnLocation = EquippedGunSkelMeshComp->GetBoneLocation(MagazineBoneName);
		SpawnRotation = EquippedGunSkelMeshComp->GetBoneQuaternion(MagazineBoneName).Rotator();
	}

	UStaticMeshComponent* MagComp = NewObject<UStaticMeshComponent>(GetOwner());
	if (!MagComp)
		return;

	MagComp->SetStaticMesh(MagMesh);
	MagComp->RegisterComponent();
	MagComp->SetWorldLocationAndRotation(SpawnLocation, SpawnRotation);
	MagComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MagComp->SetSimulatePhysics(true);

	MagComp->AddImpulse(
		FVector(0.f, 0.f, -80.f),
		NAME_None,
		true);

	FTimerHandle DestroyTimer;
	GetWorld()->GetTimerManager().SetTimer(
		DestroyTimer,
		[MagComp]()
		{
			if (MagComp)
			{
				MagComp->DestroyComponent();
			}
		},
		5.f,
		false);
}

void UNCGunComponent::HideGunMagazine()
{
	if (!EquippedGunSkelMeshComp)
		return;

	EquippedGunSkelMeshComp->HideBoneByName(TEXT("Magazine_joint"), EPhysBodyOp::PBO_None);
	EquippedGunSkelMeshComp->HideBoneByName(TEXT("Bullets_joint"), EPhysBodyOp::PBO_None);
}

void UNCGunComponent::ShowGunMagazine()
{
	if (!EquippedGunSkelMeshComp)
		return;

	EquippedGunSkelMeshComp->UnHideBoneByName(TEXT("Magazine_joint"));
	EquippedGunSkelMeshComp->UnHideBoneByName(TEXT("Bullets_joint"));
}