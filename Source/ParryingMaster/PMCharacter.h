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

    // 현재 패링 판정이 활성화되어 있는지 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Combat|Parry"
    )
    bool IsParrying() const;

    // 공격자가 현재 패링 가능 각도 안에 있는지 반환합니다.
    bool CanParryAttackFrom(
        const AActor* Attacker
    ) const;

    // 패링 성공을 소비하고 반격 가능 시간을 시작합니다.
    void HandleSuccessfulParry();

    // 현재 패링 반격이 가능한 상태인지 반환합니다.
    UFUNCTION(
        BlueprintPure,
        Category = "Combat|Parry"
    )
    bool CanParryCounter() const;

    // 회피 무적 상태를 확인한 뒤 피해를 처리합니다.
    virtual float TakeDamage(
        float DamageAmount,
        const FDamageEvent& DamageEvent,
        AController* EventInstigator,
        AActor* DamageCauser
    ) override;

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

    // IA_Parry를 연결할 자리입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Input"
    )
    TObjectPtr<UInputAction> ParryAction;

    // 공격할 때 재생할 애니메이션 몽타주입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat"
    )
    TObjectPtr<UAnimMontage> AttackMontage;

    // 공격 단계가 시작될 때 전진하는 힘입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Movement",
        meta = (ClampMin = "0.0")
    )
    float AttackLungeStrength = 250.0f;

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

    // 패링 성공 판정이 활성화되는 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Parry",
        meta = (ClampMin = "0.01")
    )
    float ParryWindowDuration = 0.2f;

    // 패링 입력 후 다시 패링할 수 있을 때까지의 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Parry",
        meta = (ClampMin = "0.01")
    )
    float ParryCooldown = 0.6f;

    // 패링 성공 후 반격할 수 있는 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Parry",
        meta = (ClampMin = "0.01")
    )
    float ParryCounterWindowDuration = 1.0f;


    // 플레이어 정면을 기준으로 패링을 허용하는 전체 각도입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Parry",
        meta = (
            ClampMin = "0.0",
            ClampMax = "360.0"
            )
    )
    float ParryFacingAngle = 120.0f;

    // 한 번의 기본 공격으로 주는 피해량입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (ClampMin = "0.0")
    )
    float AttackDamage = 20.0f;

    // 패링 반격 공격으로 주는 피해량입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Combat|Parry",
        meta = (ClampMin = "0.0")
    )
    float ParryCounterDamage = 40.0f;

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

    // 회피할 때 재생할 애니메이션 몽타주입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Dodge"
    )
    TObjectPtr<UAnimMontage> DodgeMontage;

    // 회피 몽타주의 재생 속도입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Dodge",
        meta = (ClampMin = "0.1")
    )
    float DodgeMontagePlayRate = 1.5f;

    // 실제 회피 이동이 유지되는 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Dodge",
        meta = (ClampMin = "0.01")
    )
    float DodgeMovementDuration = 0.25f;

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

    // 회피 시작 후 피해를 무효화하는 시간입니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Dodge",
        meta = (ClampMin = "0.0")
    )
    float DodgeInvincibilityDuration = 0.25f;

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

    // 회피 방향으로 이동을 시작합니다.
    void StartDodgeMovement(
        const FVector& DodgeDirection
    );

    // 회피 이동을 끝내고 기존 감속 설정을 복구합니다.
    void EndDodgeMovement();

    // 쿨다운이 끝나면 다시 회피할 수 있게 합니다.
    void ResetDodge();

    // 회피 몽타주가 종료되면 회피 상태를 해제합니다.
    void EndDodge();

    // 피격 또는 사망 시 진행 중인 회피를 취소합니다.
    void CancelDodge();

    // 회피 무적 시간을 시작합니다.
    void StartDodgeInvincibility();

    // 회피 무적 시간을 종료합니다.
    void EndDodgeInvincibility();

    // 기본 공격을 시작합니다.
    void Attack();

    // 공격을 시작하기 전에 이동 입력 방향을 바라봅니다.
    void FaceAttackDirection();

    // 캐릭터를 현재 공격 방향으로 짧게 전진시킵니다.
    void ApplyAttackLunge();

    // 패링 입력을 받아 짧은 패링 판정을 시작합니다.
    void StartParry();

    // 활성화된 패링 판정을 종료합니다.
    void EndParry();

    // 패링 쿨다운을 종료하고 다시 사용할 수 있게 합니다.
    void ResetParryCooldown();

    // 패링 성공 후 반격 가능 시간을 시작합니다.
    void StartParryCounterWindow();

    // 패링 반격 가능 시간을 종료합니다.
    void EndParryCounterWindow();

    // 반격 가능 상태를 소비하고 반격 공격을 시작합니다.
    void StartParryCounterAttack();

    // 3단 콤보의 첫 번째 공격을 시작합니다.
    void StartCombo();

    // 예약된 입력이 있으면 다음 공격으로 연결합니다.
    void TryContinueCombo();

    // 콤보 단계에 해당하는 Montage Section 이름을 반환합니다.
    FName GetComboSectionName(
        int32 ComboIndex
    ) const;

    // 모든 콤보 상태를 초기화합니다.
    void ResetCombo();

    // 공격 몽타주의 Notify가 발생했을 때 호출됩니다.
    UFUNCTION()
    void HandleMontageNotifyBegin(
        FName NotifyName,
        const FBranchingPointNotifyPayload& BranchingPointPayload
    );

    // 몽타주가 정상 종료되거나 중단되었을 때 호출됩니다.
    UFUNCTION()
    void HandleMontageEnded(
        UAnimMontage* Montage,
        bool bInterrupted
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

    // 회피 전 지상 마찰 값을 저장합니다.
    float SavedGroundFriction = 0.0f;

    // 회피 전 지상 감속 값을 저장합니다.
    float SavedBrakingDecelerationWalking = 0.0f;

    // 현재 회피 이동 설정이 적용되어 있는지 나타냅니다.
    bool bIsDodgeMovementActive = false;

    // 현재 회피 동작 중인지 나타냅니다.
    bool bIsDodging = false;

    // 현재 회피 무적 상태인지 나타냅니다.
    bool bIsDodgeInvincible = false;

    // 현재 공격 중인지 나타냅니다.
    bool bIsAttacking = false;

    // 현재 패링 성공 판정이 활성화되어 있는지 나타냅니다.
    bool bIsParrying = false;

    // 현재 새로운 패링을 시작할 수 있는지 나타냅니다.
    bool bCanParry = true;

    // 현재 패링 반격을 사용할 수 있는지 나타냅니다.
    bool bCanParryCounter = false;

    // 현재 시작된 공격이 패링 반격 공격인지 나타냅니다.
    bool bIsParryCounterAttacking = false;

    // 회피 이동 종료 시점을 관리합니다.
    FTimerHandle DodgeMovementTimer;

    // 패링 판정 종료 시간을 관리합니다.
    FTimerHandle ParryWindowTimer;

    // 패링 쿨다운 종료 시간을 관리합니다.
    FTimerHandle ParryCooldownTimer;

    // 패링 반격 가능 시간의 종료 시점을 관리합니다.
    FTimerHandle ParryCounterWindowTimer;

    // 현재 실행 중인 콤보 단계입니다.
    // 0은 콤보가 진행 중이지 않은 상태입니다.
    int32 CurrentComboIndex = 0;

    // 현재 다음 공격 입력을 받을 수 있는지 나타냅니다.
    bool bCanQueueCombo = false;

    // 입력 가능 구간에 다음 공격이 예약되었는지 나타냅니다.
    bool bComboInputQueued = false;

    // 플레이어 콤보의 최대 공격 단계입니다.
    static constexpr int32 MaxComboCount = 3;

    // 사망하거나 피격 중이면 행동할 수 없습니다.
    bool CanPerformAction() const;

    // 회피 쿨다운을 관리하는 타이머입니다.
    FTimerHandle DodgeCooldownTimer;

    // 회피 무적 종료 시점을 관리합니다.
    FTimerHandle DodgeInvincibilityTimer;

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