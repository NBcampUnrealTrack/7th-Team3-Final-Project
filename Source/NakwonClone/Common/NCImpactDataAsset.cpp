#include "NCImpactDataAsset.h"

FNCImpactFXSet UNCImpactDataAsset::GetImpactFX(TEnumAsByte<EPhysicalSurface> Surface) const
{
	if (const FNCImpactFXSet* Found = SurfaceMap.Find(Surface))
	{
		return *Found;
	}
	return DefaultImpact;
}
