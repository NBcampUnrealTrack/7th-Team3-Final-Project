#include "NCKeyGuideLibrary.h"

UTexture2D* UNCKeyGuideLibrary::GetIconForKey(UDataTable* KeyIconTable, FKey Key)
{
    if (!KeyIconTable) return nullptr;

    const FName RowName(*Key.ToString());
    if (FNCKeyIconRow* Row = KeyIconTable->FindRow<FNCKeyIconRow>(RowName, TEXT("GetIconForKey")))
    {
        return Row->Icon;
    }
    return nullptr;
}

bool UNCKeyGuideLibrary::GetTutorialStep(UDataTable* TutorialStepTable, FName StepID, FNCTutorialStepData& OutStepData)
{
    if (!TutorialStepTable) return false;

    if (FNCTutorialStepData* Row = TutorialStepTable->FindRow<FNCTutorialStepData>(StepID, TEXT("GetTutorialStep")))
    {
        OutStepData = *Row;
        return true;
    }
    return false;
}
