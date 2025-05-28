#include "Abilities/Weapons/S_WeaponPrimaryAbility.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h" // Base DA
#include "Weapons/S_HitscanWeaponDataAsset.h" // For specific params if needed
#include "Weapons/S_ProjectileWeaponDataAsset.h" // For specific params if needed
#include "Player/S_Character.h"
#include "Player/Attributes/S_AttributeSet.h" 
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h" 
#include "Animation/AnimMontage.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectTypes.h" 

US_WeaponPrimaryAbility::US_WeaponPrimaryAbility()
{
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    AbilityInputID = 0;
    AssetTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Action.PrimaryFire"))); // CORRECTED
}

bool US_WeaponPrimaryAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
    {
        return false;
    }

    const AS_Character* Character = GetOwningSCharacter();
    const AS_Weapon* EquippedWeapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

    if (!Character || !EquippedWeapon || !WeaponData || !ASC)
    {
        return false;
    }

    if (EquippedWeapon->GetCurrentWeaponState() != EWeaponState::Equipped)
    {
        return false;
    }

    if (WeaponData->AmmoAttribute.IsValid())
    {
        if (ASC->GetNumericAttribute(WeaponData->AmmoAttribute) <= 0)
        {
            if (WeaponData->OutOfAmmoCueTag.IsValid())
            {
                ASC->ExecuteGameplayCue(WeaponData->OutOfAmmoCueTag, ASC->MakeEffectContext());
            }
            return false;
        }
    }
    return true;
}

void US_WeaponPrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    PerformWeaponFire(Handle, ActorInfo, ActivationInfo);
}

void US_WeaponPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    AS_Character* Character = GetOwningSCharacter();
    AS_Weapon* Weapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData(); // This is US_WeaponDataAsset

    if (!Character || !Weapon || !WeaponData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector FireStartLocation;
    FRotator CamRot; // CORRECTED
    FVector CamAimDir;

    AController* Controller = Character->GetController();
    if (Controller)
    {
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        FVector CamLoc;
        Controller->GetPlayerViewPoint(CamLoc, CamRot); // CORRECTED
        CamAimDir = CamRot.Vector();

        const float MaxTraceDist = WeaponData->MaxAimTraceRange > 0.f ? WeaponData->MaxAimTraceRange : 100000.0f; // CORRECTED
        FVector CamTraceEnd = CamLoc + CamAimDir * MaxTraceDist;

        FHitResult CameraTraceHit;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        QueryParams.AddIgnoredActor(Weapon);

        FireStartLocation = MuzzleLocation;
        if (GetWorld()->LineTraceSingleByChannel(CameraTraceHit, CamLoc, CamTraceEnd, ECC_Visibility, QueryParams))
        {
            FireDirection = (CameraTraceHit.ImpactPoint - MuzzleLocation).GetSafeNormal();
        }
        else {
            FireDirection = (CamTraceEnd - MuzzleLocation).GetSafeNormal();
        }
    }
    else
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        FireDirection = Weapon->GetActorForwardVector();
    }

    FireMontageTask = PlayWeaponMontage(WeaponData->FireMontage);
    if (FireMontageTask)
    {
        FireMontageTask->OnCompleted.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageCompleted);
        FireMontageTask->OnInterrupted.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->OnCancelled.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->ReadyForActivation();
        // Derived abilities will call ExecuteFire. This base version might not, or only if no montage.
    }
    else // No montage, fire directly if this base class is supposed to
    {
        // This is an example for a generic fire. Derived classes MUST provide their specific parameters.
        float Spread = 0.f;
        float Range = 0.f;
        TSubclassOf<AS_Projectile> ProjClass = nullptr;

        if (const US_HitscanWeaponDataAsset* HitscanData = Cast<US_HitscanWeaponDataAsset>(WeaponData))
        {
            Spread = HitscanData->SpreadAngle;
            Range = HitscanData->MaxRange;
        }
        else if (const US_ProjectileWeaponDataAsset* ProjData = Cast<US_ProjectileWeaponDataAsset>(WeaponData))
        {
            ProjClass = ProjData->ProjectileClass;
            // Range for projectile is determined by lifespan/speed
        }

        const FGameplayEventData* AbilityTriggerData = GetAbilityTriggerData(); // CORRECTED: UGameplayAbility method
        Weapon->ExecuteFire(
            FireStartLocation,
            FireDirection,
            AbilityTriggerData ? *AbilityTriggerData : FGameplayEventData(),
            Spread,
            Range,
            ProjClass
        );
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (WeaponData->MuzzleFlashCueTag.IsValid() && ASC)
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName); // CORRECTED
        CueParams.Normal = FireDirection;
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Weapon;
        ASC->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams);
    }

    if (!FireMontageTask) // If there was no montage, we already ended, but to be safe if logic changes
    {
        // EndAbility(Handle, ActorInfo, ActivationInfo, false, false); // Already handled
    }
}

void US_WeaponPrimaryAbility::ApplyAmmoCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (WeaponData && WeaponData->AmmoCostEffect_Primary && ASC)
    {
        FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
        ContextHandle.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(WeaponData->AmmoCostEffect_Primary, GetAbilityLevel(), ContextHandle);
        if (SpecHandle.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
    }
}

void US_WeaponPrimaryAbility::ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}


void US_WeaponPrimaryAbility::OnFireMontageCompleted()
{
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled()
{
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}