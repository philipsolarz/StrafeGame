#include "Abilities/Weapons/S_WeaponPrimaryAbility.h"
#include "Weapons/S_Weapon.h"
#include "Weapons/S_WeaponDataAsset.h"
#include "Player/S_Character.h"
#include "Player/Attributes/S_AttributeSet.h" // For checking ammo
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h" // If using montages for fire
#include "Animation/AnimMontage.h"
#include "GameFramework/Controller.h"

US_WeaponPrimaryAbility::US_WeaponPrimaryAbility()
{
    // Defaults for a primary fire ability
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted; // Good for responsive shooting
    AbilityInputID = 0; // Conventionally, 0 for primary fire. Set in BP or derived C++ if needed.
    // This should match the InputID used by AS_Character::HandleWeaponEquipped

// Add a generic "Ability.Weapon.PrimaryFire" tag to this ability
    AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.Action.PrimaryFire")));
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

    // Check if weapon is in 'Equipped' state (not equipping/unequipping)
    if (EquippedWeapon->GetCurrentWeaponState() != EWeaponState::Equipped)
    {
        return false;
    }

    // Check Ammo
    if (WeaponData->AmmoAttribute.IsValid())
    {
        if (ASC->GetNumericAttribute(WeaponData->AmmoAttribute) <= 0)
        {
            if (OptionalRelevantTags && WeaponData->OutOfAmmoCueTag.IsValid())
            {
                // OptionalRelevantTags->AddTag(WeaponData->OutOfAmmoCueTag); // This doesn't trigger cue directly
                // Consider sending a gameplay event or having UI react to this failure reason
            }
            // Trigger OutOfAmmo cue directly if that's the desired behavior on CanActivate failure
            if (WeaponData->OutOfAmmoCueTag.IsValid())
            {
                ASC->ExecuteGameplayCue(WeaponData->OutOfAmmoCueTag, ASC->MakeEffectContext());
            }
            return false;
        }
    }
    // Cooldown check is handled by UGameplayAbility::CanActivateAbility using CooldownGameplayEffectClasses or CooldownTags from GameplayEffect
    return true;
}

void US_WeaponPrimaryAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) // This also applies costs and cooldowns if set up correctly
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true); // true for bWasCancelled
        return;
    }

    // Commit Succeeded (cost paid, cooldown applied if configured on ability CDO or via effect)
    // Now perform the actual weapon fire logic
    PerformWeaponFire(Handle, ActorInfo, ActivationInfo);

    // Many simple fire abilities might end immediately after firing one shot.
    // More complex ones (charged, automatic) will have tasks or timers.
    // For a single shot, you might end it here or after a montage.
}

void US_WeaponPrimaryAbility::PerformWeaponFire(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    // Base implementation. Derived classes will override this.
    // Example: Play a fire montage and then call weapon's ExecuteFire on a gameplay event from montage.
    // Or call ExecuteFire directly.

    AS_Character* Character = GetOwningSCharacter();
    AS_Weapon* Weapon = GetEquippedWeapon();
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();

    if (!Character || !Weapon || !WeaponData)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 1. Get Aiming Data (Start location, Direction)
    FVector FireStartLocation;
    FVector FireDirection;
    AController* Controller = Character->GetController();
    if (Controller)
    {
        Controller->GetPlayerViewPoint(FireStartLocation, FireDirection); // Gets camera view
        FireDirection = Controller->GetControlRotation().Vector(); // Use controller rotation for aim

        // Adjust FireStartLocation to muzzle or a point slightly in front of camera
        // This needs careful handling to avoid shooting through walls or self.
        // A common approach is to trace from camera, then use muzzle for effects.
        FVector MuzzleLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireStartLocation = MuzzleLocation; // Or a trace from camera to find "true" aim point then fire from muzzle towards it

        // More robust: Trace from camera to find target, then adjust FireStartLocation to weapon muzzle
        // and FireDirection from muzzle towards the target point.
        FHitResult CameraTraceHit;
        FVector CamLoc, CamDir;
        Character->GetActorEyesViewPoint(CamLoc, CamDir); // Or Controller->GetPlayerViewPoint()
        CamDir = Controller->GetControlRotation().Vector();

        const float MaxFireDistance = 100000.0f; // Large distance for camera trace
        FVector CamTraceEnd = CamLoc + CamDir * MaxFireDistance;

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(Character);
        QueryParams.AddIgnoredActor(Weapon);

        // Default FireStartLocation to a point in front of the camera if no hit
        FireStartLocation = CamLoc + CamDir * 100.0f; // Default start for effects/projectile
        FireDirection = CamDir;

        if (GetWorld()->LineTraceSingleByChannel(CameraTraceHit, CamLoc, CamTraceEnd, ECC_Visibility, QueryParams))
        {
            FireDirection = (CameraTraceHit.ImpactPoint - MuzzleLocation).GetSafeNormal();
        }
        else {
            FireDirection = (CamTraceEnd - MuzzleLocation).GetSafeNormal();
        }
        FireStartLocation = MuzzleLocation;
    }
    else // AI or no controller? Fallback.
    {
        FireStartLocation = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        FireDirection = Weapon->GetActorForwardVector();
    }


    // 2. Play Fire Montage (Optional, can be part of Weapon's ExecuteFire or a GameplayCue)
    FireMontageTask = PlayWeaponMontage(WeaponData->FireMontage);
    if (FireMontageTask)
    {
        FireMontageTask->OnCompleted.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageCompleted);
        FireMontageTask->OnInterrupted.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        FireMontageTask->OnCancelled.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled);
        // If montage has a GameplayEvent to trigger the actual fire:
        // FireMontageTask->EventReceived.AddDynamic(this, &US_WeaponPrimaryAbility::OnFireEventFromMontage);
        FireMontageTask->ReadyForActivation();
        // Actual call to Weapon->ExecuteFire() might happen on EventReceived or OnCompleted if no event.
    }
    else
    {
        // No montage, fire immediately
        Weapon->ExecuteFire(FireStartLocation, FireDirection, nullptr /*EventData*/, WeaponData->SpreadAngle, WeaponData->MaxRange, WeaponData->ProjectileClass_DEPRECATED); // Example, adapt params
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }

    // 3. Trigger Muzzle Flash Cue
    if (WeaponData->MuzzleFlashCueTag.IsValid() && ActorInfo->AbilitySystemComponent.IsValid())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = Weapon->GetWeaponMeshComponent()->GetSocketLocation(WeaponData->MuzzleFlashSocketName);
        CueParams.Normal = FireDirection; // Or weapon forward vector
        CueParams.Instigator = Character;
        CueParams.EffectCauser = Weapon;
        ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(WeaponData->MuzzleFlashCueTag, CueParams);
    }

    // Specific logic for Hitscan vs Projectile will be in derived primary fire abilities
    // or the weapon's ExecuteFire_Implementation.
    // For now, this base PerformWeaponFire might just end the ability or wait for montage.
    // If FireMontageTask is null (no montage), we should end here.
    if (!FireMontageTask)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
    }
}

void US_WeaponPrimaryAbility::ApplyAmmoCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    if (WeaponData && WeaponData->AmmoCostEffect_Primary && ActorInfo->AbilitySystemComponent.Get())
    {
        FGameplayEffectContextHandle ContextHandle = ActorInfo->AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this); // Ability is source of cost

        FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(WeaponData->AmmoCostEffect_Primary, GetAbilityLevel(), ContextHandle);
        if (SpecHandle.IsValid())
        {
            ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
        }
    }
}

void US_WeaponPrimaryAbility::ApplyAbilityCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    const US_WeaponDataAsset* WeaponData = GetEquippedWeaponData();
    UGameplayEffect* CooldownGE = GetCooldownGameplayEffect(); // From UGameplayAbility

    if (WeaponData && CooldownGE && ActorInfo->AbilitySystemComponent.Get())
    {
        // Check if CooldownTag is set in WeaponDataAsset; if so, CooldownGE should apply this tag.
        // The UGameplayAbility::ApplyCooldown will handle applying the GE that has the CooldownTag.
        // Ensure your Cooldown GE has a GrantedTag that matches WeaponData->CooldownTag_Primary.
    }
    Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo); // This handles applying the CooldownGE
}


void US_WeaponPrimaryAbility::OnFireMontageCompleted()
{
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

void US_WeaponPrimaryAbility::OnFireMontageInterruptedOrCancelled()
{
    EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}