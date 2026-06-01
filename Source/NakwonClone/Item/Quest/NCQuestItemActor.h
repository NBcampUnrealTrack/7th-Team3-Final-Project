#pragma once

#include "CoreMinimal.h"
#include "Item/NCItemActor.h"
#include "NCQuestItemActor.generated.h"

UCLASS(Blueprintable)
class NAKWONCLONE_API ANCQuestItemActor : public ANCItemActor
{
	GENERATED_BODY()

public:
	ANCQuestItemActor();

	/** 아이템 표시 이름 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Quest")
	FText ItemDisplayName;

	/** 아이템 설명 */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Item|Quest")
	FText ItemDescription;

	/** 기본 동작 없음 - 블루프린트 자식에서 오버라이드 */
	virtual void UseItem(class ACharacter* User) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};