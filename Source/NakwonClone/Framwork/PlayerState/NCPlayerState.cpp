// Fill out your copyright notice in the Description page of Project Settings.


#include "NCPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "NakwonClone/Common/NCGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Common/NCSaveGame.h"
#include "Player/PlayerComponent/NCPlayerInventoryComponent.h"
#include "Inventory/NCStashInventoryComponent.h"

ANCPlayerState::ANCPlayerState()
{
	MaxHP = 100.f;
	CurrentHP = MaxHP;
	MaxStamina = 100.f;
	CurrentStamina = MaxStamina;
	
	LifeStateTag = NCCharacter::Alive;
	
	PlayerInventory = CreateDefaultSubobject<UNCPlayerInventoryComponent>(TEXT("PlayerInventory"));
	StashInventory = CreateDefaultSubobject<UNCStashInventoryComponent>(TEXT("StashInventory"));
}

void ANCPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		LoadInventoryData();
	}
}

void ANCPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANCPlayerState, LifeStateTag);
	DOREPLIFETIME(ANCPlayerState, CurrentHP);
	DOREPLIFETIME(ANCPlayerState, MaxHP);
	DOREPLIFETIME(ANCPlayerState, CurrentStamina);
	DOREPLIFETIME(ANCPlayerState, MaxStamina);
	DOREPLIFETIME(ANCPlayerState, bHost);
	DOREPLIFETIME(ANCPlayerState, TeamIndex);
	DOREPLIFETIME(ANCPlayerState, Credits);
}

void ANCPlayerState::OnRep_LifeStateTag()
{
	OnLifeStateChanged.Broadcast(LifeStateTag);
}

void ANCPlayerState::OnRep_CurrentHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void ANCPlayerState::OnRep_CurrentStemina()
{
	OnStaminaBarChanged.Broadcast(CurrentStamina, MaxStamina);
}

void ANCPlayerState::SaveInventoryData()
{
	if (!HasAuthority()) return;

	UNCSaveGame* SaveGameInstance = Cast<UNCSaveGame>(UGameplayStatics::CreateSaveGameObject(UNCSaveGame::StaticClass()));
	if (!SaveGameInstance) return;

	if (PlayerInventory)
	{
		SaveGameInstance->PlayerInventoryItems    = PlayerInventory->GetItemsArray();
		SaveGameInstance->PlayerEquipmentPresets  = PlayerInventory->GetPresetsArray();
	}
    
	if (StashInventory)
	{
		SaveGameInstance->StashInventoryItems = StashInventory->GetItemsArray();
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, GetInventorySaveSlotName(), 0);
    
	UE_LOG(LogTemp, Log, TEXT("[NCPlayerState] 인벤토리 및 창고 데이터 저장 완료"));
}

void ANCPlayerState::LoadInventoryData()
{
	if (!HasAuthority()) return;

	FString SlotName = GetInventorySaveSlotName();
	int32 UserIndex = 0;

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		UNCSaveGame* LoadGameInstance = Cast<UNCSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
		if (LoadGameInstance)
		{
			if (PlayerInventory && LoadGameInstance->PlayerInventoryItems.Num() > 0)
			{
				PlayerInventory->SetItemsArray(LoadGameInstance->PlayerInventoryItems);
				PlayerInventory->SetPresetsArray(LoadGameInstance->PlayerEquipmentPresets);
			}
            
			if (StashInventory && LoadGameInstance->StashInventoryItems.Num() > 0)
			{
				StashInventory->SetItemsArray(LoadGameInstance->StashInventoryItems);
			}
            
			UE_LOG(LogTemp, Log, TEXT("[NCPlayerState] 세이브 파일 로드 및 가방 데이터 복원 완료"));
			return;
		}
	}

	if (PlayerInventory) PlayerInventory->InitializeInventory();
	if (StashInventory) StashInventory->InitializeInventory();
    
	UE_LOG(LogTemp, Warning, TEXT("[NCPlayerState] 세이브 파일이 없어 신규 인벤토리로 초기화"));
}

FString ANCPlayerState::GetInventorySaveSlotName() const
{
	FString PlayerKey;
	if (GetUniqueId().IsValid())
	{
		PlayerKey = GetUniqueId()->ToString();
	}
	else
	{
		PlayerKey = FString::FromInt(GetPlayerId());
	}
	return FString::Printf(TEXT("InvSave_%s"), *PlayerKey);
}

void ANCPlayerState::OnRep_Credits()
{
	OnCreditsChanged.Broadcast(Credits);
}

void ANCPlayerState::AddCredits(int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}
	Credits = FMath::Clamp(Credits + Amount, 0, MaxCredits);
	OnCreditsChanged.Broadcast(Credits);
}

bool ANCPlayerState::SpendCredits(int32 Amount)
{
	if (!HasAuthority())
	{
		return false;
		
	}
	
	if (Credits < Amount)
	{
		return false;
	}
	
	Credits -= Amount;
	OnCreditsChanged.Broadcast(Credits);
	return true;
}
