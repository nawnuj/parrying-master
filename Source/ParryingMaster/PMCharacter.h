#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "PMCharacter.generated.h"

// 아래 클래스들이 존재한다고 미리 알려주는 선언입니다.
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext; 
class UAnimMontage;
class UPMHealthComponent;
struct FBranchingPointNotifyPayload;

UCLASS()
class PARRYINGMASTER_API APMCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APMCharacter();

protected:
    // 게임이 시작될 때 한 번 실행됩니다.
    virtual void BeginPlay() override;

    // 플레이어 입력과 함수를 연결합니다.
    virtual void SetupPlayerInputComponent(
        UInputComponent* PlayerInputComponent
    ) override;

    /*
     * 카메라를 캐릭터 뒤쪽에 배치하는 막대입니다.
     * 벽에 닿으면 자동으로 짧아져서 카메라가 벽을 뚫지 않게 합니다.
     */
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Camera"
    )
    TObjectPtr<USpringArmComponent> CameraBoom;

    // 플레이어가 실제로 게임 화면을 보는 카메라입니다.
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Camera"
    )
    TObjectPtr<UCameraComponent> FollowCamera;

    // IMC_Default를 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    // IA_Move를 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> MoveAction;

    // 마우스 조작용 IA_MouseLook를 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> MouseLookAction;

    // IA_Jump를 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> JumpAction;

    // IA_Sprint를 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> SprintAction;

    // IA_Dodge를 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> DodgeAction;

    // IA_Attack을 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> AttackAction;

    // 공격할 때 재생할 애니메이션 몽타주입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat"
    )
    TObjectPtr<UAnimMontage> AttackMontage;

    // 피해를 받았을 때 재생할 피격 몽타주입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Hit Reaction"
    )
    TObjectPtr<UAnimMontage> HitReactMontage;

    // 피격 후 행동할 수 없게 되는 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Hit Reaction",
        meta = (ClampMin = "0.05")
    )
    float HitStunDuration = 0.5f;

    // 한 번의 기본 공격으로 주는 피해량입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float AttackDamage = 20.0f;

    // 캐릭터 앞쪽으로 검사할 공격 거리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float AttackRange = 150.0f;

    // 공격 판정으로 사용하는 구체의 크기입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float AttackRadius = 50.0f;

    // 기본 걷기 속도입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Movement",
        meta = (ClampMin = "0.0")
    )
    float WalkSpeed = 350.0f;

    // Shift를 누르고 있을 때의 달리기 속도입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Movement",
        meta = (ClampMin = "0.0")
    )
    float SprintSpeed = 600.0f;

    // 마우스 카메라 감도입니다.
    // BP_PMCharacter의 클래스 디폴트에서 조절할 수 있습니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Camera"
    )
    float MouseSensitivity = 0.3f;

    // 캐릭터가 회피할 때 앞으로 밀려나는 힘입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Dodge",
        meta = (ClampMin = "0.0")
    )
    float DodgeStrength = 700.0f;

    // 회피를 다시 사용할 수 있을 때까지의 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Dodge",
        meta = (ClampMin = "0.0")
    )
    float DodgeCooldown = 0.6f;

    // 플레이어의 체력을 관리하는 컴포넌트입니다.
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Health"
    )
    TObjectPtr<UPMHealthComponent> HealthComponent;


private:
    // WASD 입력을 처리합니다.
    void Move(const FInputActionValue& Value);

    // 마우스 움직임을 처리합니다.
    void Look(const FInputActionValue& Value);

    void StartSprint();
    void StopSprint();

    // 회피 입력을 처리합니다.
    void Dodge();

    // 쿨다운이 끝나면 다시 회피할 수 있게 합니다.
    void ResetDodge();

    // 기본 공격을 시작합니다.
    void Attack();

    // 공격 애니메이션이 끝나면 호출됩니다.
    void ResetAttack();

    // 공격 몽타주의 Notify가 발생했을 때 호출됩니다.
    UFUNCTION()
    void HandleMontageNotifyBegin(
        FName NotifyName,
        const FBranchingPointNotifyPayload& BranchingPointPayload
    );
   
    // 피격 여부를 검사할 수 있도록 점프 입력을 받습니다.
    void StartJump();

    // 진행 중인 공격을 강제로 취소합니다.
    void CancelAttack();

    // 체력 변화를 받아 피해 여부를 판단합니다.
    UFUNCTION()
    void HandleHealthChanged(
        float CurrentHealth,
        float MaxHealth
    );

    // 피격 상태를 시작합니다.
    void StartHitReaction();

    // 피격 경직을 종료합니다.
    void EndHitReaction();

    // 캐릭터 앞쪽을 검사하고 대상에게 피해를 줍니다.
    void PerformAttackTrace();

    // 현재 회피를 사용할 수 있는지 나타냅니다.
    bool bCanDodge = true;

    // 현재 공격 중인지 나타냅니다.
    bool bIsAttacking = false;

    // 사망하거나 피격 중이면 행동할 수 없습니다.
    bool CanPerformAction() const;

    // 회피 쿨다운을 관리하는 타이머입니다.
    FTimerHandle DodgeCooldownTimer;

    // 공격 입력 연속 실행을 방지하는 타이머입니다.
    FTimerHandle AttackTimer;

    // 마지막으로 확인한 체력입니다.
    float PreviousHealth = 0.0f;

    // 현재 피격 경직 상태인지 나타냅니다.
    bool bIsHitReacting = false;

    // 피격 경직 종료 시간을 관리합니다.
    FTimerHandle HitReactTimer;

    // 체력이 0이 되었을 때 호출됩니다.
    UFUNCTION()
    void HandleDeath();
};