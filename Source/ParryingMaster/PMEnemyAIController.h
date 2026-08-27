#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PMEnemyAIController.generated.h"

UCLASS()
class PARRYINGMASTER_API APMEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    APMEnemyAIController();

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaSeconds) override;

    // 적이 플레이어에게 접근한 뒤 멈출 거리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "AI",
        meta = (ClampMin = "0.0")
    )
    float AcceptanceRadius = 120.0f;

private:
    // 현재 추적 중인 플레이어입니다.
    TWeakObjectPtr<APawn> TargetPawn;
};