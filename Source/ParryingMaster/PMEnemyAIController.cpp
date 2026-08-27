#include "PMEnemyAIController.h"

#include "Kismet/GameplayStatics.h"
#include "PMEnemyCharacter.h"
#include "GameFramework/Pawn.h"
#include "PMHealthComponent.h"

APMEnemyAIController::APMEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    /*
     * 매 프레임 경로를 다시 요청할 필요는 없으므로
     * 0.25초마다 추적 위치를 갱신합니다.
     */
    PrimaryActorTick.TickInterval = 0.25f;
}

void APMEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

    TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

void APMEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!TargetPawn.IsValid())
    {
        TargetPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    }

    APMEnemyCharacter* EnemyCharacter =
        Cast<APMEnemyCharacter>(GetPawn());

    if (!TargetPawn.IsValid() || !EnemyCharacter)
    {
        return;
    }

    /*
     * 플레이어 Pawn에 연결된 체력 컴포넌트를 찾습니다.
     */
    UPMHealthComponent* TargetHealthComponent =
        TargetPawn->FindComponentByClass<UPMHealthComponent>();

    /*
     * 플레이어가 사망했다면 이동과 공격을 정리하고
     * 더 이상 AI Tick을 실행하지 않습니다.
     */
    if (
        TargetHealthComponent &&
        TargetHealthComponent->IsDead()
        )
    {
        StopMovement();

        EnemyCharacter->StopCombat();

        TargetPawn.Reset();

        SetActorTickEnabled(false);

        return;
    }
    /*
    * 패링 경직 중에는 플레이어 추적과 공격을 모두 중단합니다.
    * 경직 해제는 EnemyCharacter의 ParryStunTimer가 담당합니다.
    */
    if (EnemyCharacter->IsParryStunned())
    {
        StopMovement();
        return;
    }

    const float DistanceToTarget = FVector::Dist2D(
        EnemyCharacter->GetActorLocation(),
        TargetPawn->GetActorLocation()
    );

    const float AttackRange =
        EnemyCharacter->GetAttackRange();

    if (DistanceToTarget <= AttackRange)
    {
        StopMovement();

        EnemyCharacter->TryAttack(
            TargetPawn.Get()
        );

        return;
    }

    MoveToActor(
        TargetPawn.Get(),
        AcceptanceRadius,
        false,
        true,
        true,
        nullptr,
        true
    );
}