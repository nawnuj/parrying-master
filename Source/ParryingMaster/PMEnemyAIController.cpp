#include "PMEnemyAIController.h"

#include "Kismet/GameplayStatics.h"

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

    if (!TargetPawn.IsValid() || !GetPawn())
    {
        return;
    }

    MoveToActor(
        TargetPawn.Get(),
        AcceptanceRadius,
        true,
        true,
        true,
        nullptr,
        true
    );
}