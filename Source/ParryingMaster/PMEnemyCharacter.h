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
   
    // 진행 중인 공격과 전투 상태를 정리합니다.
    void StopCombat();

    // 현재 적의 공격이 패링 가능한 상태인지 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Combat|Parry"
    )
    bool IsAttackParryable() const;

    // 현재 적이 패링으로 인해 경직 중인지 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Combat|Parry"
    )
    bool IsParryStunned() const;

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

    // 패링 성공 후 적이 행동할 수 없는 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Parry",
        meta = (ClampMin = "0.05")
    )
    float ParryStunDuration = 1.0f;

private:
    UFUNCTION()
    void HandleDeath();

    UFUNCTION()
    void HandleMontageNotifyBegin(
        FName NotifyName,
        const FBranchingPointNotifyPayload& BranchingPointPayload
    );

    // 현재 적 공격의 패링 가능 구간을 시작합니다.
    void OpenParryWindow();

    // 현재 적 공격의 패링 가능 구간을 종료합니다.
    void CloseParryWindow();

    // 패링 성공으로 인한 적 경직을 시작합니다.
    void StartParryStun();

    // 패링 경직을 종료하고 전투 행동을 복구합니다.
    void EndParryStun();

    void PerformAttackTrace();
    void ResetAttack();

    bool bIsAttacking = false;
    bool bCanAttack = true;

    // 현재 적 공격이 패링 가능한 구간인지 나타냅니다.
    bool bIsAttackParryable = false;

    // 현재 적이 패링으로 인해 경직 중인지 나타냅니다.
    bool bIsParryStunned = false;

    // 사망 또는 플레이어 사망으로 전투가 영구 중단됐는지 나타냅니다.
    bool bIsCombatStopped = false;

    TWeakObjectPtr<AActor> CurrentAttackTarget;

    FTimerHandle AttackCooldownTimer;

    // 패링 경직 종료 시점을 관리합니다.
    FTimerHandle ParryStunTimer;
};