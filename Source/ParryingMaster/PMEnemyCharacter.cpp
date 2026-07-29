#include "PMEnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PMHealthComponent.h"
#include "PMEnemyAIController.h"

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
}

void APMEnemyCharacter::HandleDeath()
{
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