#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PhysicalMaterials/PhysicalMaterial.h" // EPhysicalSurface
#include "NCImpactDataAsset.generated.h"

class UNiagaraSystem;
class USoundBase;
class UMaterialInterface;

// 한 표면이 가지는 임팩트 연출 묶음 (파티클 + 소리 + 데칼)
USTRUCT(BlueprintType)
struct FNCImpactFXSet
{
	GENERATED_BODY()

	// 표면에 터질 파티클 (먼지/스파크/파편 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<UNiagaraSystem> ImpactFX = nullptr;

	// 충돌 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<USoundBase> ImpactSound = nullptr;

	// 표면에 남는 탄흔/자국 데칼
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact|Decal")
	TObjectPtr<UMaterialInterface> DecalMaterial = nullptr;

	// 데칼 크기 (X = 투영 깊이, Y·Z = 면 크기)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact|Decal")
	FVector DecalSize = FVector(4.f, 8.f, 8.f);

	// 데칼이 남아있는 시간(초). 0 이하면 영구
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact|Decal")
	float DecalLifeSeconds = 10.f;
};

// 표면(EPhysicalSurface) → 임팩트 연출 매핑 표 (근접/총기 공용)
UCLASS(BlueprintType)
class NAKWONCLONE_API UNCImpactDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// 표면별 연출 (Concrete/Metal/Glass 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	TMap<TEnumAsByte<EPhysicalSurface>, FNCImpactFXSet> SurfaceMap;

	// 매칭되는 표면이 없을 때 사용할 기본 연출 (안전망)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impact")
	FNCImpactFXSet DefaultImpact;

	// 표면으로 연출 묶음 조회 (없으면 DefaultImpact 반환)
	UFUNCTION(BlueprintCallable, Category = "Impact")
	FNCImpactFXSet GetImpactFX(TEnumAsByte<EPhysicalSurface> Surface) const;
};
