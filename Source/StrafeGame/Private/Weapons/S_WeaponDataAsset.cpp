#include "Weapons/S_WeaponDataAsset.h"
#include "Weapons/S_Weapon.h" // For TSoftClassPtr<AS_Weapon>

US_WeaponDataAsset::US_WeaponDataAsset()
{
    WeaponDisplayName = FText::FromString(TEXT("Default Weapon"));
    EquipTime = 0.5f;
    UnequipTime = 0.5f;
    AttachmentSocketName = FName("WeaponSocket");
    WeaponMesh_DEPRECATED = nullptr; // Explicitly nulling deprecated property
}

#if WITH_EDITOR
void US_WeaponDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Example: If you change a property, you might want to update another one automatically
    // const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
    // if (PropertyName == GET_MEMBER_NAME_CHECKED(US_WeaponDataAsset, SomeProperty))
    // {
    //     // Do something
    // }
}
#endif

FPrimaryAssetId US_WeaponDataAsset::GetPrimaryAssetId() const
{
    // This returns a unique ID for use with the Asset Manager system.
    // Typically, this is the object path.
    return FPrimaryAssetId(GetPrimaryAssetType(), GetFName());
}