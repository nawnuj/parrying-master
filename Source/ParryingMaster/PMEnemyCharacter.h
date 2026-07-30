#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PMEnemyCharacter.generated.h"

class UPMHealthComponent;
class UAnimMontage;
struct FBranchingPointNotifyPayload;

UCLASS()
class PARRYINGMASTER_API APMEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APMEnemyCharacter();
    // AI Controller가 공격 거리 안에서 호출합니다.
    void TryAttack(AActor* TargetActor);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Components"
    )
    TObjectPtr<UPMHealthComponent> HealthComponent;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat"
    )
    TObjectPtr<UAnimMontage> EnemyAttackMontage;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float EnemyAttackDamage = 15.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float EnemyAttackRange = 180.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float EnemyAttackRadius = 45.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float EnemyAttackCooldown = 1.5f;

private:
    UFUNCTION()
    void HandleDeath();

    UFUNCTION()
    void HandleMontageNotifyBegin(
        FName NotifyName,
        const FBranchingPointNotifyPayload& BranchingPointPayload
    );

    void PerformAttackTrace();
    void ResetAttack();

    bool bIsAttacking = false;
    bool bCanAttack = true;

    TWeakObjectPtr<AActor> CurrentAttackTarget;

    FTimerHandle AttackCooldownTimer;
};