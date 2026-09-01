#include "PMEnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PMHealthComponent.h"
#include "PMEnemyAIController.h"
#include "PMCharacter.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

APMEnemyCharacter::APMEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    HealthComponent = CreateDefaultSubobject<UPMHealthComponent>(
        TEXT("HealthComponent")
    );

    AIControllerClass = APMEnemyAIController::StaticClass();

    AutoPossessAI =
        EAutoPossessAI::PlacedInWorldOrSpawned;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
    GetCharacterMovement()->RotationRate =
        FRotator(0.0f, 360.0f, 0.0f);

    GetCharacterMovement()->MaxWalkSpeed = 250.0f;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void APMEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
    {
        HealthComponent->OnHealthChanged.AddDynamic(
            this,
            &APMEnemyCharacter::HandleHealthChanged
        );

        HealthComponent->OnDeath.AddDynamic(
            this,
            &APMEnemyCharacter::HandleDeath
        );
    }

    UAnimInstance* AnimInstance =
        GetMesh()->GetAnimInstance();

    if (AnimInstance)
    {
        AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(
            this,
            &APMEnemyCharacter::HandleMontageNotifyBegin
        );
    }
}

void APMEnemyCharacter::HandleHealthChanged(
    float CurrentHealth,
    float MaxHealth
)
{
    (void)MaxHealth;

    /*
     * 체력이 0이 된 피해는 일반 피격 반응 대신
     * HandleDeath()에서 처리합니다.
     */
    if (CurrentHealth <= 0.0f)
    {
        return;
    }

    StartHitReaction();
}

void APMEnemyCharacter::StartHitReaction()
{
    if (
        bIsCombatStopped
        || bIsParryStunned
        || bIsHitReacting
        )
    {
        return;
    }

    if (
        !HealthComponent
        || HealthComponent->GetCurrentHealth() <= 0.0f
        )
    {
        return;
    }

    bIsHitReacting = true;

    CloseParryWindow();

    if (EnemyAttackMontage)
    {
        StopAnimMontage(EnemyAttackMontage);
    }

    GetWorldTimerManager().ClearTimer(
        AttackCooldownTimer
    );

    bIsAttacking = false;
    bCanAttack = false;

    CurrentAttackTarget.Reset();

    if (
        APMEnemyAIController* EnemyAIController =
        Cast<APMEnemyAIController>(GetController())
        )
    {
        EnemyAIController->StopMovement();
    }

    GetCharacterMovement()->StopMovementImmediately();

    if (EnemyHitReactMontage)
    {
        PlayAnimMontage(EnemyHitReactMontage);
    }

    GetWorldTimerManager().SetTimer(
        HitReactionTimer,
        this,
        &APMEnemyCharacter::EndHitReaction,
        HitReactionDuration,
        false
    );
}

void APMEnemyCharacter::EndHitReaction()
{
    GetWorldTimerManager().ClearTimer(
        HitReactionTimer
    );

    if (EnemyHitReactMontage)
    {
        StopAnimMontage(EnemyHitReactMontage);
    }

    bIsHitReacting = false;

    if (
        bIsCombatStopped
        || bIsParryStunned
        || (
            HealthComponent
            && HealthComponent->IsDead()
            )
        )
    {
        bCanAttack = false;
        return;
    }

    bCanAttack = true;
}

void APMEnemyCharacter::HandleDeath()
{
    StopCombat();

    if (APMEnemyAIController* EnemyAIController =
        Cast<APMEnemyAIController>(GetController()))
    {
        EnemyAIController->StopMovement();
        EnemyAIController->SetActorTickEnabled(false);
    }

    GetCharacterMovement()->DisableMovement();

    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    GetMesh()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    SetLifeSpan(2.0f);
}

float APMEnemyCharacter::GetAttackRange() const
{
    return EnemyAttackRange;
}

bool APMEnemyCharacter::IsAttackParryable() const
{
    return bIsAttackParryable;
}

bool APMEnemyCharacter::IsParryStunned() const
{
    return bIsParryStunned;
}

bool APMEnemyCharacter::IsHitReacting() const
{
    return bIsHitReacting;
}

void APMEnemyCharacter::TryAttack(AActor* TargetActor)
{
    if (
        !bCanAttack
        || bIsAttacking
        || bIsHitReacting
        || bIsParryStunned
        || bIsCombatStopped
        || !TargetActor
        )
    {
        return;
    }

    if (HealthComponent && HealthComponent->IsDead())
    {
        return;
    }

    const float DistanceToTarget = FVector::Dist2D(
        GetActorLocation(),
        TargetActor->GetActorLocation()
    );

    if (DistanceToTarget > EnemyAttackRange)
    {
        return;
    }

    if (!EnemyAttackMontage)
    {
        return;
    }

    /*
     * 공격 전에 플레이어 방향으로 회전합니다.
     */
    FVector Direction =
        TargetActor->GetActorLocation() - GetActorLocation();

    Direction.Z = 0.0f;

    if (!Direction.IsNearlyZero())
    {
        const FRotator TargetRotation = Direction.Rotation();

        SetActorRotation(
            FRotator(0.0f, TargetRotation.Yaw, 0.0f)
        );
    }
    /*
    * 이전 공격에서 패링 가능 상태가 남아 있더라도
    * 새로운 공격은 패링 불가능 상태로 시작합니다.
    */
    CloseParryWindow();

    CurrentAttackTarget = TargetActor;

    const float MontageDuration =
        PlayAnimMontage(EnemyAttackMontage);

    if (MontageDuration <= 0.0f)
    {
        CurrentAttackTarget.Reset();
        return;
    }

    bIsAttacking = true;
    bCanAttack = false;

    GetWorldTimerManager().SetTimer(
        AttackCooldownTimer,
        this,
        &APMEnemyCharacter::ResetAttack,
        EnemyAttackCooldown,
        false
    );
}

void APMEnemyCharacter::StopCombat()
{
    // 이후 경직 종료 함수가 전투를 복구하지 못하게 합니다.
    bIsCombatStopped = true;

    CloseParryWindow();

    if (EnemyAttackMontage)
    {
        StopAnimMontage(EnemyAttackMontage);
    }

    if (EnemyHitReactMontage)
    {
        StopAnimMontage(EnemyHitReactMontage);
    }

    if (EnemyParryStunMontage)
    {
        StopAnimMontage(EnemyParryStunMontage);
    }

    GetWorldTimerManager().ClearTimer(
        AttackCooldownTimer
    );

    GetWorldTimerManager().ClearTimer(
        ParryStunTimer
    );

    GetWorldTimerManager().ClearTimer(
        HitReactionTimer
    );

    bIsAttacking = false;
    bCanAttack = false;
    bIsParryStunned = false;
    bIsHitReacting = false;

    CurrentAttackTarget.Reset();
}

void APMEnemyCharacter::HandleMontageNotifyBegin(
    FName NotifyName,
    const FBranchingPointNotifyPayload& BranchingPointPayload
)
{
    (void)BranchingPointPayload;

    // 공격 중이 아닐 때 발생한 Notify는 처리하지 않습니다.
    if (!bIsAttacking)
    {
        return;
    }

    if (NotifyName == FName(TEXT("ParryWindowOpen")))
    {
        OpenParryWindow();
        return;
    }

    if (NotifyName == FName(TEXT("EnemyAttackHit")))
    { 
        PerformAttackTrace();
        return;
    }

    if (NotifyName == FName(TEXT("ParryWindowClose")))
    {
        CloseParryWindow();
    }
}

void APMEnemyCharacter::OpenParryWindow()
{
    // 공격 중이 아니라면 패링 가능 상태를 열지 않습니다.
    if (!bIsAttacking)
    {
        return;
    }

    // 이미 열린 상태라면 중복 처리하지 않습니다.
    if (bIsAttackParryable)
    {
        return;
    }

    bIsAttackParryable = true;
}

void APMEnemyCharacter::CloseParryWindow()
{
    // 이미 닫혀 있다면 중복 처리하지 않습니다.
    if (!bIsAttackParryable)
    {
        return;
    }

    bIsAttackParryable = false;
}

void APMEnemyCharacter::StartParryStun()
{
    // 전투가 영구 중단된 적은 새로운 경직을 시작하지 않습니다.
    if (bIsCombatStopped)
    {
        return;
    }

    // 사망한 적은 패링 경직 상태로 전환하지 않습니다.
    if (
        HealthComponent
        && HealthComponent->IsDead()
        )
    {
        return;
    }

    // 이미 패링 경직 중이라면 중복 실행하지 않습니다.
    if (bIsParryStunned)
    {
        return;
    }

    bIsParryStunned = true;

    /*
     * 패링 경직이 일반 피격 반응보다 우선하므로
     * 진행 중인 일반 피격 상태와 타이머를 종료합니다.
     */
    GetWorldTimerManager().ClearTimer(
        HitReactionTimer
    );

    bIsHitReacting = false;

    if (EnemyHitReactMontage)
    {
        StopAnimMontage(EnemyHitReactMontage);
    }

    // 진행 중인 공격의 패링 가능 구간을 닫습니다.
    CloseParryWindow();

    // 패링된 공격 몽타주를 즉시 중단합니다.
    if (EnemyAttackMontage)
    {
        StopAnimMontage(EnemyAttackMontage);
    }

    /*
     * 기존 공격 쿨다운이 경직 중 공격 상태를
     * 다시 복구하지 못하도록 타이머를 제거합니다.
     */
    GetWorldTimerManager().ClearTimer(
        AttackCooldownTimer
    );

    bIsAttacking = false;
    bCanAttack = false;

    CurrentAttackTarget.Reset();

    // AI Controller가 요청한 이동을 중단합니다.
    if (
        APMEnemyAIController* EnemyAIController =
        Cast<APMEnemyAIController>(GetController())
        )
    {
        EnemyAIController->StopMovement();
    }

    // Character Movement에 남아 있는 속도를 즉시 제거합니다.
    GetCharacterMovement()->StopMovementImmediately();

    // 패링 성공에 대한 적 경직 애니메이션을 재생합니다.
    if (EnemyParryStunMontage)
    {
        PlayAnimMontage(EnemyParryStunMontage);
    }

    // 지정된 시간이 지나면 패링 경직 상태를 종료합니다.
    GetWorldTimerManager().SetTimer(
        ParryStunTimer,
        this,
        &APMEnemyCharacter::EndParryStun,
        ParryStunDuration,
        false
    );
}

void APMEnemyCharacter::EndParryStun()
{
    GetWorldTimerManager().ClearTimer(
        ParryStunTimer
    );

    if (EnemyParryStunMontage)
    {
        StopAnimMontage(EnemyParryStunMontage);
    }
    /*
     * 사망 또는 플레이어 사망으로 전투가 영구 중단됐다면
     * 공격 가능한 상태로 복구하지 않습니다.
     */
    if (bIsCombatStopped)
    {
        bIsParryStunned = false;
        return;
    }

    if (
        HealthComponent
        && HealthComponent->IsDead()
        )
    {
        bIsParryStunned = false;
        return;
    }

    bIsParryStunned = false;
    bCanAttack = true;
}

void APMEnemyCharacter::PerformAttackTrace()
{
    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    const FVector Start =
        GetActorLocation() + GetActorForwardVector() * 50.0f;

    const FVector End =
        GetActorLocation() +
        GetActorForwardVector() * EnemyAttackRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

    FHitResult HitResult;

    const bool bHit = World->SweepSingleByObjectType(
        HitResult,
        Start,
        End,
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(EnemyAttackRadius),
        QueryParams
    );

    AActor* HitActor = HitResult.GetActor();

    if (
        bHit &&
        HitActor &&
        HitActor == CurrentAttackTarget.Get()
        )
    {
        /*
         * 공격 판정에 맞은 실제 Actor가 플레이어 캐릭터인지
         * 확인합니다. 플레이어가 아니라면 nullptr가 반환됩니다.
         */
        APMCharacter* PlayerCharacter =
            Cast<APMCharacter>(HitActor);

        /*
        * 플레이어가 현재 패링 중이면서
        * 공격한 적이 패링 가능 각도 안에 있는지 확인합니다.
        */
        const bool bPlayerCanParryAttack =
            PlayerCharacter
            && PlayerCharacter->CanParryAttackFrom(this);

        const bool bAttackIsParryable =
            IsAttackParryable();

        const bool bParrySucceeded =
            bPlayerCanParryAttack
            && bAttackIsParryable;

        if (bParrySucceeded)
        {
            /*
             * 플레이어의 현재 패링 판정을 소비하고
             * 반격 가능 시간을 시작합니다.
             */
            PlayerCharacter->HandleSuccessfulParry(this);

            // 공격한 적에게 패링 경직을 적용합니다.
            StartParryStun();
        }
        else
        {
            UGameplayStatics::ApplyDamage(
                HitActor,
                EnemyAttackDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }

    DrawDebugLine(
        World,
        Start,
        End,
        FColor::Yellow,
        false,
        0.5f
    );

    DrawDebugSphere(
        World,
        End,
        EnemyAttackRadius,
        16,
        bHit ? FColor::Green : FColor::Red,
        false,
        0.5f
    );
}

void APMEnemyCharacter::ResetAttack()
{
    CloseParryWindow();

    bIsAttacking = false;

    /*
     * 경직 또는 영구 전투 중단 상태에서는
     * 공격 가능한 상태로 복구하지 않습니다.
     */
    if (
        bIsHitReacting
        || bIsParryStunned
        || bIsCombatStopped
        )
    {
        bCanAttack = false;
        CurrentAttackTarget.Reset();
        return;
    }

    bCanAttack = true;

    CurrentAttackTarget.Reset();
}