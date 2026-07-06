// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RotateToFaceTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NakwonClone/Zombie/AI/AIController/Base/VGMonsterAIControllerBase.h"

UBTTask_RotateToFaceTarget::UBTTask_RotateToFaceTarget()
{
	NodeName = "Rotate To Face Target";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RotateToFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Pawn || !BB) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey));
	if (!Target) return EBTNodeResult::Failed;

	const FVector Dir = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal2D();
	const float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Pawn->GetActorForwardVector(), Dir)));
	return (AngleDiff <= Precision) ? EBTNodeResult::Succeeded : EBTNodeResult::InProgress;
}

void UBTTask_RotateToFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Pawn || !BB) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AVGMonsterAIControllerBase::TargetActorKey));
	if (!Target) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	const FVector Dir = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal2D();
	const FRotator TargetRot = Dir.Rotation();
	const FRotator NewRot = FMath::RInterpConstantTo(Pawn->GetActorRotation(), TargetRot, DeltaSeconds, RotationSpeed);
	Pawn->SetActorRotation(NewRot);

	const float AngleDiff = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Pawn->GetActorForwardVector(), Dir)));
	if (AngleDiff <= Precision)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
