#include "NCQuestItemActor.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Common/NCGameplayTags.h"

ANCQuestItemActor::ANCQuestItemActor()
{
	ItemDisplayName = FText::FromString(TEXT("Unknown Item"));
	ItemDescription = FText::FromString(TEXT(""));
	ItemTypeTag = NCItemTag::Quest;
}

void ANCQuestItemActor::UseItem(ACharacter* User)
{
	Super::UseItem(User);

	UE_LOG(LogTemp, Log, TEXT("[NCQuestItem] %s 사용: %s"),
		User ? *User->GetName() : TEXT("Unknown"),
		*ItemDisplayName.ToString());
}

void ANCQuestItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANCQuestItemActor, ItemDisplayName);
	DOREPLIFETIME(ANCQuestItemActor, ItemDescription);
}