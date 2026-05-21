#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NCBaseCharacter.generated.h"

UCLASS()
class NAKWONCLONE_API ANCBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANCBaseCharacter();

protected:
	virtual void BeginPlay() override;
};
