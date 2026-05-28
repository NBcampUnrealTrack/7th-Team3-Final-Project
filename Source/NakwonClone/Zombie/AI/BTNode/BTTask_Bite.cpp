// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Bite.h"

UBTTask_Bite::UBTTask_Bite()
{
	NodeName = "Bite";
}

EBTNodeResult::Type UBTTask_Bite::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	
	return EBTNodeResult::Succeeded;
}
