#include "ANCLootBoxActor.h"
#include "Components/StaticMeshComponent.h"
#include "NakwonClone/Inventory/NCInventoryBaseComponent.h"

AANCLootBoxActor::AANCLootBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	RootComponent = BoxMesh;

	LootInventory = CreateDefaultSubobject<UNCInventoryBaseComponent>(TEXT("LootInventory"));
	LootInventory->SetIsReplicated(true);
}

void AANCLootBoxActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		GenerateLoot();
	}
}

void AANCLootBoxActor::Interact(AActor* Interactor)
{
}

void AANCLootBoxActor::GenerateLoot()
{
	for (const TPair<FName, int32>& FixedItem : FixedLootItems)
	{
		// TODO: LootInventory->Initialize or AddItem 로직 처리
	}
}

void AANCLootBoxActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor) return;

	if (!HasAuthority())
	{
		// TODO: 화면에 파밍 상자 인벤토리 UI(UMG) 위젯 띄우기
	}
    
	if (HasAuthority())
	{
		// TODO: 여러 명이 동시에 한 상자를 열지 못하게 사용 중 상태 걸기
		// TODO: 상호작용한 클라이언트의 Controller를 찾아 UI를 열라는 Client RPC 전송
		UE_LOG(LogTemp, Log, TEXT("서버: %s 가 상자를 열었습니다."), *Interactor->GetName());
	}
}

bool AANCLootBoxActor::CanInteract_Implementation(AActor* Interactor)
{
	return true;
}

void AANCLootBoxActor::ToggleHighlight_Implementation(bool bHighlight)
{
	if (BoxMesh)
	{
		BoxMesh->SetRenderCustomDepth(bHighlight);
	}
}