// Source/StrafeGame/Public/Weapons/S_Weapon.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "S_Weapon.generated.h"

// Forward Declarations
class USkeletalMeshComponent;
class US_WeaponDataAsset;
class AS_Character;
class AS_Projectile;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Equipping UMETA(DisplayName = "Equipping"),
    Equipped UMETA(DisplayName = "Equipped"),
    Unequipping UMETA(DisplayName = "Unequipping")
};

UCLASS(Abstract, Blueprintable, Config = Game)
class STRAFEGAME_API AS_Weapon : public AActor
{
    GENERATED_BODY()

public:
    AS_Weapon();

    //~ Begin AActor Interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    //~ End AActor Interface

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponConfig")
    TObjectPtr<US_WeaponDataAsset> WeaponData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> WeaponMesh;

    virtual void Equip(AS_Character* NewOwner);
    virtual void Unequip();
    virtual void StartUnequipEffects();

    UFUNCTION(BlueprintPure, Category = "WeaponState")
    bool IsEquipped() const;

    UFUNCTION(BlueprintPure, Category = "WeaponState")
    EWeaponState GetCurrentWeaponState() const { return CurrentWeaponState; }

    virtual void SetOwnerCharacter(AS_Character* NewOwnerCharacter);

    UFUNCTION(BlueprintPure, Category = "WeaponConfig")
    US_WeaponDataAsset* GetWeaponData() const { return WeaponData; }

    UFUNCTION(BlueprintPure, Category = "Components")
    USkeletalMeshComponent* GetWeaponMeshComponent() const { return WeaponMesh; }

    /**
     * Called by GameplayAbilities to perform the primary firing logic.
     * @param FireStartLocation The starting point for the trace or projectile.
     * @param FireDirection The normalized direction of the fire.
     * @param EventData Optional FGameplayEventData passed from the ability.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Firing")
    void ExecutePrimaryFire(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData);
    virtual void ExecutePrimaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData);

    /**
     * Called by GameplayAbilities to perform the secondary firing logic.
     * @param FireStartLocation The starting point for the trace or projectile.
     * @param FireDirection The normalized direction of the fire.
     * @param EventData Optional FGameplayEventData passed from the ability.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|Firing")
    void ExecuteSecondaryFire(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData);
    virtual void ExecuteSecondaryFire_Implementation(const FVector& FireStartLocation, const FVector& FireDirection, const FGameplayEventData& EventData);

protected:
    UPROPERTY(Transient, ReplicatedUsing = OnRep_OwnerCharacter)
    TObjectPtr<AS_Character> OwnerCharacter;

    UFUNCTION()
    virtual void OnRep_OwnerCharacter();

    UPROPERTY(Transient, ReplicatedUsing = OnRep_WeaponState)
    EWeaponState CurrentWeaponState;

    UFUNCTION()
    virtual void OnRep_WeaponState();

    virtual void SetWeaponState(EWeaponState NewState);

    UPROPERTY(EditDefaultsOnly, Category = "WeaponConfig|Attachment")
    FName AttachSocketName;

    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Effects", meta = (DisplayName = "OnEquippedEffects"))
    void K2_OnEquipped();

    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Effects", meta = (DisplayName = "OnUnequippedEffects"))
    void K2_OnUnequipped();

    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon|Effects", meta = (DisplayName = "OnStartUnequipEffects"))
    void K2_OnStartUnequipEffects();
};