#include "Weapons/S_WeaponDataAsset.h"
#include "Weapons/S_Weapon.h" 

US_WeaponDataAsset::US_WeaponDataAsset()
{
    WeaponDisplayName = FText::FromString(TEXT("Default Weapon"));
    EquipTime = 0.5f;
    UnequipTime = 0.5f;
    AttachmentSocketName = FName("WeaponSocket");
    MuzzleFlashSocketName = FName("MuzzleFlashSocket"); // ADDED Default
    MaxAimTraceRange = 10000.0f; // ADDED Default
}

#if WITH_EDITOR
void US_WeaponDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

FPrimaryAssetId US_WeaponDataAsset::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(GetPrimaryAssetType(), GetFName());
}