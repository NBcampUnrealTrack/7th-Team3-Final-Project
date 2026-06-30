#include "NCShotgunComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "Common/NCGameplayTags.h"

void UNCShotgunComponent::Reload()
{
	if (IsReloading() || !HasActiveGun()) return;

	const FNCGunData* Data = GetActiveGunData();
	if (!Data) return;

	if (ReserveAmmo <= 0)                  return;
	if (CurrentAmmo >= Data->MagazineSize) return;

	StopFire();
	ActiveGunActions.AddTag(NCGun::Action_Reloading);

	if (!Data->ReloadSound.IsNull())
		UGameplayStatics::PlaySoundAtLocation(this, Data->ReloadSound.LoadSynchronous(), GetOwner()->GetActorLocation());

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	UAnimInstance* AnimInst = Char->GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	UAnimMontage* Montage = Data->ReloadMontage.LoadSynchronous();
	if (!Montage) return;

	// Intro → Shell, Shell은 자기 자신으로 끊김없이 루프
	Char->PlayAnimMontage(Montage, 1.f, TEXT("Intro"));
	AnimInst->Montage_SetNextSection(TEXT("Intro"), TEXT("Shell"), Montage);
	AnimInst->Montage_SetNextSection(TEXT("Shell"), TEXT("Shell"), Montage);

	// Shell 섹션 길이만큼 타이머로 1발씩 추가
	const int32 IntroIdx = Montage->GetSectionIndex(TEXT("Intro"));
	const int32 ShellIdx = Montage->GetSectionIndex(TEXT("Shell"));
	const float IntroDuration = (IntroIdx != INDEX_NONE) ? Montage->GetSectionLength(IntroIdx) : 0.f;
	const float ShellDuration = (ShellIdx != INDEX_NONE) ? Montage->GetSectionLength(ShellIdx) : 0.5f;

	GetWorld()->GetTimerManager().SetTimer(
		ShellTimerHandle,
		this, &UNCShotgunComponent::OnShellInserted,
		ShellDuration, true,
		IntroDuration + ShellDuration); // Intro 끝나고 Shell 1회 재생 후 첫 발동
}

void UNCShotgunComponent::StopReload()
{
	if (!IsReloading()) return;

	GetWorld()->GetTimerManager().ClearTimer(ShellTimerHandle);
	ActiveGunActions.RemoveTag(NCGun::Action_Reloading);

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char) return;

	const FNCGunData* Data = GetActiveGunData();
	if (Data && !Data->ReloadMontage.IsNull())
		Char->StopAnimMontage(Data->ReloadMontage.LoadSynchronous());
}

void UNCShotgunComponent::OnShellInserted()
{
	const FNCGunData* Data = GetActiveGunData();
	if (!Data || !IsReloading()) return;

	const int32 Take = FMath::Min(1, ReserveAmmo);
	CurrentAmmo += Take;
	ReserveAmmo -= Take;
	OnAmmoChanged.Broadcast(CurrentAmmo, ReserveAmmo);

	if (CurrentAmmo >= Data->MagazineSize || ReserveAmmo <= 0)
		StopReload();
}
