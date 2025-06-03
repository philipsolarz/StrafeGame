// Source/StrafeGame/Private/Weapons/S_WeaponDataAsset.cpp
#include "Weapons/S_WeaponDataAsset.h"
#include "Weapons/S_Weapon.h" 
#include "UI/ViewModels/S_WeaponViewModel.h" // Include base ViewModel

US_WeaponDataAsset::US_WeaponDataAsset()
{
    WeaponDisplayName = FText::FromString(TEXT("Default Weapon"));
    EquipTime = 0.5f;
    UnequipTime = 0.5f;
    AttachmentSocketName = FName("WeaponSocket");
    MuzzleFlashSocketName = FName("MuzzleFlashSocket");
    MaxAimTraceRange = 10000.0f;
    WeaponViewModelClass = US_WeaponViewModel::StaticClass(); // Default to base ViewModel
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