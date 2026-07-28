#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PMEnemyCharacter.generated.h"

class UPMHealthComponent;

UCLASS()
class PARRYINGMASTER_API APMEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APMEnemyCharacter();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Components"
    )
    TObjectPtr<UPMHealthComponent> HealthComponent;

private:
    UFUNCTION()
    void HandleDeath();
};