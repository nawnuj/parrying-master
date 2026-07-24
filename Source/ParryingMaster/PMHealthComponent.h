#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PMHealthComponent.generated.h"

class AController;
class AActor;
class UDamageType;

/*
 * 체력이 변경될 때 실행되는 이벤트입니다.
 * 현재 체력과 최대 체력을 블루프린트에 전달합니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnHealthChangedSignature,
    float,
    CurrentHealth,
    float,
    MaxHealth
);

// 체력이 0이 되었을 때 실행되는 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
    FOnDeathSignature
);

UCLASS(
    ClassGroup = (Custom),
    meta = (BlueprintSpawnableComponent)
)
class PARRYINGMASTER_API UPMHealthComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UPMHealthComponent();

    // 현재 체력을 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Health"
    )
    float GetCurrentHealth() const;

    // 최대 체력을 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Health"
    )
    float GetMaxHealth() const;

    // 0~1 사이의 체력 비율을 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Health"
    )
    float GetHealthPercent() const;

    // 현재 사망 상태인지 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Health"
    )
    bool IsDead() const;

    // 체력이 변할 때 블루프린트에도 알립니다.
    UPROPERTY(
        BlueprintAssignable,
        Category = "Health"
    )
    FOnHealthChangedSignature OnHealthChanged;

    // 사망했을 때 블루프린트에도 알립니다.
    UPROPERTY(
        BlueprintAssignable,
        Category = "Health"
    )
    FOnDeathSignature OnDeath;

protected:
    virtual void BeginPlay() override;

    // 에디터에서 조절할 수 있는 최대 체력입니다.
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Health",
        meta = (ClampMin = "1.0")
    )
    float MaxHealth = 100.0f;

    // 게임 중 사용하는 현재 체력입니다.
    UPROPERTY(
        VisibleInstanceOnly,
        BlueprintReadOnly,
        Category = "Health"
    )
    float CurrentHealth = 0.0f;

private:
    /*
     * 이 컴포넌트를 가진 액터가 피해를 받으면
     * 언리얼의 OnTakeAnyDamage 이벤트를 통해 호출됩니다.
     */
    UFUNCTION()
    void HandleTakeAnyDamage(
        AActor* DamagedActor,
        float Damage,
        const UDamageType* DamageType,
        AController* InstigatedBy,
        AActor* DamageCauser
    );

    bool bIsDead = false;
};