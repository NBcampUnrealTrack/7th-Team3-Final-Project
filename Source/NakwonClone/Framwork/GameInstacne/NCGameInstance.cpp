
#include "NCGameInstance.h"

FNCWeaponData* UNCGameInstance::GetWeaponData(FName WeaponID) const
{
	if (!WeaponDataTable || WeaponID.IsNone()) return nullptr;

	return WeaponDataTable->FindRow<FNCWeaponData>(WeaponID, TEXT("GetWeaponData"));
}
