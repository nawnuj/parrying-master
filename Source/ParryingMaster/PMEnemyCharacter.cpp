#include "PMEnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PMHealthComponent.h"

APMEnemyCharacter::APMEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    HealthComponent = CreateDefaultSubobject<UPMHealthComponent>(
        TEXT("HealthComponent")
    );

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    GetCharacterMovement()->bOrientRotationToMovement = true;
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
}

void APMEnemyCharacter::HandleDeath()
{
    GetCharacterMovement()->DisableMovement();

    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    GetMesh()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    SetLifeSpan(2.0f);
}