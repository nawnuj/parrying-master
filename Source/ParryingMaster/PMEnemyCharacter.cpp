#include "PMEnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PMHealthComponent.h"
#include "PMEnemyAIController.h"
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

bool APMEnemyCharacter::IsAttackParryable() const
{
    return bIsAttackParryable;
}

void APMEnemyCharacter::TryAttack(AActor* TargetActor)
{
    if (!bCanAttack || bIsAttacking || !TargetActor)
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
        UGameplayStatics::ApplyDamage(
            HitActor,
            EnemyAttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
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
    bCanAttack = true;

    CurrentAttackTarget.Reset();
}