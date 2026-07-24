#include "PMHealthComponent.h"

#include "GameFramework/Actor.h"

UPMHealthComponent::UPMHealthComponent()
{
    // 매 프레임 실행할 필요가 없으므로 Tick을 끕니다.
    PrimaryComponentTick.bCanEverTick = false;
}

void UPMHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    bIsDead = false;

    AActor* Owner = GetOwner();

    if (Owner)
    {
        /*
         * Owner가 피해를 받으면
         * HandleTakeAnyDamage()가 호출되도록 연결합니다.
         */
        Owner->OnTakeAnyDamage.AddDynamic(
            this,
            &UPMHealthComponent::HandleTakeAnyDamage
        );
    }
}

void UPMHealthComponent::HandleTakeAnyDamage(
    AActor* DamagedActor,
    float Damage,
    const UDamageType* DamageType,
    AController* InstigatedBy,
    AActor* DamageCauser
)
{
    // 이미 죽었다면 추가 피해를 무시합니다.
    if (bIsDead)
    {
        return;
    }

    // 0 이하의 피해는 무시합니다.
    if (Damage <= 0.0f)
    {
        return;
    }

    /*
     * 피해량만큼 체력을 감소시킵니다.
     * Clamp를 이용해 체력을 0~MaxHealth 범위로 제한합니다.
     */
    CurrentHealth = FMath::Clamp(
        CurrentHealth - Damage,
        0.0f,
        MaxHealth
    );

    OnHealthChanged.Broadcast(
        CurrentHealth,
        MaxHealth
    );

    UE_LOG(
        LogTemp,
        Log,
        TEXT("%s Health: %.1f / %.1f"),
        *GetOwner()->GetName(),
        CurrentHealth,
        MaxHealth
    );

    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s has died."),
            *GetOwner()->GetName()
        );

        OnDeath.Broadcast();
    }
}

float UPMHealthComponent::GetCurrentHealth() const
{
    return CurrentHealth;
}

float UPMHealthComponent::GetMaxHealth() const
{
    return MaxHealth;
}

float UPMHealthComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f)
    {
        return 0.0f;
    }

    return CurrentHealth / MaxHealth;
}

bool UPMHealthComponent::IsDead() const
{
    return bIsDead;
}