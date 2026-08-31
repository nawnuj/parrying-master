#include "PMCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "PMHealthComponent.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

APMCharacter::APMCharacter()
{
    // 지금은 매 프레임 실행할 코드가 없으므로 Tick을 끕니다.
    PrimaryActorTick.bCanEverTick = false;

    /*
     * 캐릭터 몸 자체가 마우스 방향으로 바로 회전하지 않게 합니다.
     * 카메라는 마우스로 움직이되, 캐릭터는 이동 방향을 바라봅니다.
     */
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 캐릭터가 이동하는 방향으로 회전하도록 설정합니다.
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // 캐릭터가 이동 방향으로 돌아가는 속도입니다.
    GetCharacterMovement()->RotationRate =
        FRotator(0.0f, 500.0f, 0.0f);

    // 기본 걷기 속도입니다.
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    // 점프 높이에 영향을 주는 값입니다.
    GetCharacterMovement()->JumpZVelocity = 500.0f;

    // 공중에서 방향을 조절할 수 있는 정도입니다.
    GetCharacterMovement()->AirControl = 0.35f;

    /*
     * CameraBoom을 생성합니다.
     * Spring Arm은 캐릭터와 카메라 사이의 막대 역할을 합니다.
     */
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(
        TEXT("CameraBoom")
    );

    CameraBoom->SetupAttachment(RootComponent);

    // 캐릭터와 카메라 사이의 거리입니다.
    CameraBoom->TargetArmLength = 400.0f;

    // 마우스 입력으로 Spring Arm이 회전하도록 합니다.
    CameraBoom->bUsePawnControlRotation = true;

    // 벽에 카메라가 들어가는 것을 막습니다.
    CameraBoom->bDoCollisionTest = true;

    // 카메라를 생성합니다.
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(
        TEXT("FollowCamera")
    );

    // 카메라를 CameraBoom 끝에 연결합니다.
    FollowCamera->SetupAttachment(
        CameraBoom,
        USpringArmComponent::SocketName
    );

    /*
     * 카메라가 직접 마우스 회전을 적용받는 것은 끕니다.
     * CameraBoom이 회전하므로 카메라는 같이 움직입니다.
     */
    
    FollowCamera->bUsePawnControlRotation = false;

    HealthComponent =
        CreateDefaultSubobject<UPMHealthComponent>(
            TEXT("HealthComponent")
        );
}

void APMCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComponent)
    {
        PreviousHealth =
            HealthComponent->GetCurrentHealth();

        HealthComponent->OnHealthChanged.AddDynamic(
            this,
            &APMCharacter::HandleHealthChanged
        );

        HealthComponent->OnDeath.AddDynamic(
            this,
            &APMCharacter::HandleDeath
        );
    }

    // 현재 캐릭터를 조종하는 PlayerController를 가져옵니다.
    APlayerController* PlayerController =
        Cast<APlayerController>(Controller);

    if (!PlayerController)
    {
        return;
    }

    // 로컬 플레이어 정보를 가져옵니다.
    ULocalPlayer* LocalPlayer =
        PlayerController->GetLocalPlayer();

    if (!LocalPlayer)
    {
        return;
    }

    /*
     * Enhanced Input 시스템을 가져옵니다.
     * 이 시스템에 IMC_Default를 등록해야 입력이 작동합니다.
     */
    UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
        LocalPlayer->GetSubsystem<
        UEnhancedInputLocalPlayerSubsystem
        >();

    if (InputSubsystem && DefaultMappingContext)
    {
        InputSubsystem->AddMappingContext(
            DefaultMappingContext,
            0
        );
    }

    UAnimInstance* AnimInstance =
        GetMesh()->GetAnimInstance();

    if (AnimInstance)
    {
        AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(
            this,
            &APMCharacter::HandleMontageNotifyBegin
        );

        AnimInstance->OnMontageEnded.AddDynamic(
            this,
            &APMCharacter::HandleMontageEnded
        );
    }
}

void APMCharacter::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent
)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 일반 입력 컴포넌트를 Enhanced Input 컴포넌트로 변환합니다.
    UEnhancedInputComponent* EnhancedInputComponent =
        Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if (!EnhancedInputComponent)
    {
        return;
    }

    // IA_Move 입력이 발생하면 Move()를 실행합니다.
    if (MoveAction)
    {
        EnhancedInputComponent->BindAction(
            MoveAction,
            ETriggerEvent::Triggered,
            this,
            &APMCharacter::Move
        );
    }

    // 마우스를 움직이면 Look()을 실행합니다.
    if (MouseLookAction)
    {
        EnhancedInputComponent->BindAction(
            MouseLookAction,
            ETriggerEvent::Triggered,
            this,
            &APMCharacter::Look
        );
    }

    // Space를 누르기 시작하면 점프합니다.
    if (JumpAction)
    {
        EnhancedInputComponent->BindAction(
            JumpAction,
            ETriggerEvent::Started,
            this,
            &APMCharacter::StartJump
        );

        // Space에서 손을 떼면 점프 입력을 종료합니다.
        EnhancedInputComponent->BindAction(
            JumpAction,
            ETriggerEvent::Completed,
            this,
            &ACharacter::StopJumping
        );
    }

    // Shift를 누르면 달리기를 시작합니다.
    if (SprintAction)
    {
        EnhancedInputComponent->BindAction(
            SprintAction,
            ETriggerEvent::Started,
            this,
            &APMCharacter::StartSprint
        );

        // Shift를 놓으면 걷기 속도로 돌아갑니다.
        EnhancedInputComponent->BindAction(
            SprintAction,
            ETriggerEvent::Completed,
            this,
            &APMCharacter::StopSprint
        );

        /*
         * 플레이 중 창 포커스를 잃는 등의 이유로 입력이 취소된 경우에도
         * 걷기 속도로 돌아갑니다.
         */
        EnhancedInputComponent->BindAction(
            SprintAction,
            ETriggerEvent::Canceled,
            this,
            &APMCharacter::StopSprint
        );
    }

    // Left Ctrl을 누르면 회피를 시도합니다.
    if (DodgeAction)
    {
        EnhancedInputComponent->BindAction(
            DodgeAction,
            ETriggerEvent::Started,
            this,
            &APMCharacter::Dodge
        );
    }

    // 마우스 왼쪽 버튼을 누르면 공격을 시도합니다.
    if (AttackAction)
    {
        EnhancedInputComponent->BindAction(
            AttackAction,
            ETriggerEvent::Started,
            this,
            &APMCharacter::Attack
        );
    }

    // 마우스 오른쪽 버튼을 누르면 패링을 시도합니다.
    if (ParryAction)
    {
        EnhancedInputComponent->BindAction(
            ParryAction,
            ETriggerEvent::Started,
            this,
            &APMCharacter::StartParry
        );
    }
}

void APMCharacter::Move(const FInputActionValue& Value)
{
    if (!CanPerformAction())
    {
        return;
    }

    if (
        bIsParrying
        || bIsParryAnimationPlaying
        )
    {
        return;
    }

    // IA_Move의 Axis2D 값을 가져옵니다.
    const FVector2D MovementValue =
        Value.Get<FVector2D>();

    if (!Controller)
    {
        return;
    }

    /*
     * 현재 카메라가 바라보는 방향을 가져옵니다.
     * 위아래 각도는 이동에 사용하지 않고 좌우 회전값만 사용합니다.
     */
    const FRotator ControlRotation =
        Controller->GetControlRotation();

    const FRotator YawRotation(
        0.0f,
        ControlRotation.Yaw,
        0.0f
    );

    // 카메라 기준 앞쪽 방향을 계산합니다.
    const FVector ForwardDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // 카메라 기준 오른쪽 방향을 계산합니다.
    const FVector RightDirection =
        FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // W와 S 입력입니다.
    AddMovementInput(
        ForwardDirection,
        MovementValue.Y
    );

    // A와 D 입력입니다.
    AddMovementInput(
        RightDirection,
        MovementValue.X
    );
}

void APMCharacter::Look(const FInputActionValue& Value)
{
    // 마우스의 좌우·상하 움직임을 가져옵니다.
    const FVector2D LookValue =
        Value.Get<FVector2D>();

    // 마우스 좌우 움직임으로 카메라를 좌우 회전합니다.
    AddControllerYawInput(
        LookValue.X * MouseSensitivity
    );

    // 마우스 상하 움직임으로 카메라를 위아래 회전합니다.
    AddControllerPitchInput(
        LookValue.Y * MouseSensitivity
    );
}

void APMCharacter::StartJump()
{
    if (!CanPerformAction())
    {
        return;
    }

    if (bIsParrying)
    {
        return;
    }

    Jump();
}

void APMCharacter::StartSprint()
{
    if (!CanPerformAction())
    {
        return;
    }

    if (
        bIsParrying
        || bIsParryAnimationPlaying
        )
    {
        return;
    }

    GetCharacterMovement()->MaxWalkSpeed =
        SprintSpeed;
}

void APMCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APMCharacter::Dodge()
{
    // 피격 또는 사망 상태에서는 회피할 수 없습니다.
    if (!CanPerformAction())
    {
        return;
    }

    // 패링 중에는 회피할 수 없습니다.
    if (bIsParrying)
    {
        return;
    }

    /*
     * 공격 몽타주와 회피 몽타주가 같은 Slot을 사용하므로
     * 공격 중에는 회피를 시작하지 않습니다.
     */
    if (bIsAttacking)
    {
        return;
    }

    // 회피 쿨다운 중에는 다시 회피하지 않습니다.
    if (!bCanDodge)
    {
        return;
    }

    // 공중에서는 회피할 수 없습니다.
    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }

    /*
     * 마지막 이동 입력 방향을 가져옵니다.
     * 이동 중이라면 WASD 입력 방향으로 회피합니다.
     */
    FVector DodgeDirection =
        GetLastMovementInputVector();

    DodgeDirection.Z = 0.0f;

    DodgeDirection =
        DodgeDirection.GetSafeNormal();

    /*
     * 이동 입력이 없다면 캐릭터가 현재
     * 바라보는 방향으로 회피합니다.
     */
    if (DodgeDirection.IsNearlyZero())
    {
        DodgeDirection =
            GetActorForwardVector().GetSafeNormal2D();
    }

    if (DodgeDirection.IsNearlyZero())
    {
        return;
    }

    // 회피 방향으로 캐릭터를 즉시 회전시킵니다.
    const FRotator DodgeRotation(
        0.0f,
        DodgeDirection.Rotation().Yaw,
        0.0f
    );

    SetActorRotation(DodgeRotation);

    // 회피 쿨다운을 시작합니다.
    bCanDodge = false;

    // 회피 몽타주가 재생되는 동안 유지할 상태입니다.
    bIsDodging = true;

    float DodgeMontageDuration = 0.0f;

    if (DodgeMontage)
    {
        DodgeMontageDuration =
            PlayAnimMontage(
                DodgeMontage,
                DodgeMontagePlayRate
            );
    }

    /*
     * 몽타주가 없거나 재생에 실패해도
     * 기존 회피 이동과 무적 기능은 실행합니다.
     */
    if (DodgeMontageDuration <= 0.0f)
    {
        bIsDodging = false;
    }

    StartDodgeMovement(DodgeDirection);

    // 회피 재사용 쿨다운을 시작합니다.
    GetWorldTimerManager().SetTimer(
        DodgeCooldownTimer,
        this,
        &APMCharacter::ResetDodge,
        DodgeCooldown,
        false
    );
}

void APMCharacter::StartDodgeMovement(
    const FVector& DodgeDirection
)
{
    UCharacterMovementComponent* MovementComponent =
        GetCharacterMovement();

    if (!MovementComponent)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(
        DodgeMovementTimer
    );

    SavedGroundFriction =
        MovementComponent->GroundFriction;

    SavedBrakingDecelerationWalking =
        MovementComponent->BrakingDecelerationWalking;

    bIsDodgeMovementActive = true;

    /*
     * 회피 도중 지상 마찰과 감속으로 인해
     * 이동이 즉시 멈추는 것을 방지합니다.
     */
    MovementComponent->GroundFriction = 0.0f;
    MovementComponent->BrakingDecelerationWalking = 0.0f;

    LaunchCharacter(
        DodgeDirection * DodgeStrength,
        true,
        false
    );

    GetWorldTimerManager().SetTimer(
        DodgeMovementTimer,
        this,
        &APMCharacter::EndDodgeMovement,
        DodgeMovementDuration,
        false
    );
}

void APMCharacter::EndDodgeMovement()
{
    if (!bIsDodgeMovementActive)
    {
        return;
    }

    UCharacterMovementComponent* MovementComponent =
        GetCharacterMovement();

    if (MovementComponent)
    {
        MovementComponent->GroundFriction =
            SavedGroundFriction;

        MovementComponent->BrakingDecelerationWalking =
            SavedBrakingDecelerationWalking;

        /*
         * 회피가 끝난 뒤 수평 이동만 멈춥니다.
         * 혹시 공중에 있다면 Z축 속도는 유지합니다.
         */
        FVector CurrentVelocity =
            MovementComponent->Velocity;

        CurrentVelocity.X = 0.0f;
        CurrentVelocity.Y = 0.0f;

        MovementComponent->Velocity =
            CurrentVelocity;
    }

    bIsDodgeMovementActive = false;
}

void APMCharacter::ResetDodge()
{
    bCanDodge = true;
}

void APMCharacter::EndDodge()
{
    EndDodgeMovement();
    EndDodgeInvincibility();

    bIsDodging = false;
}

void APMCharacter::CancelDodge()
{
    if (DodgeMontage)
    {
        StopAnimMontage(DodgeMontage);
    }

    EndDodgeMovement();

    bIsDodging = false;
    EndDodgeInvincibility();
}

void APMCharacter::StartDodgeInvincibility()
{
    GetWorldTimerManager().ClearTimer(
        DodgeInvincibilityTimer
    );

    if (DodgeInvincibilityDuration <= 0.0f)
    {
        bIsDodgeInvincible = false;
        return;
    }

    bIsDodgeInvincible = true;

    GetWorldTimerManager().SetTimer(
        DodgeInvincibilityTimer,
        this,
        &APMCharacter::EndDodgeInvincibility,
        DodgeInvincibilityDuration,
        false
    );
}

void APMCharacter::EndDodgeInvincibility()
{
    GetWorldTimerManager().ClearTimer(
        DodgeInvincibilityTimer
    );

    bIsDodgeInvincible = false;
}

void APMCharacter::Attack()
{
    if (!CanPerformAction())
    {
        return;
    }

    if (bIsParrying)
    {
        return;
    }

    /*
     * 이미 공격 중이라면 새로운 몽타주를 재생하지 않고
     * 다음 공격 입력을 예약합니다.
     */
    if (bIsAttacking)
    {
        if (
            bCanQueueCombo
            && !bComboInputQueued
            && CurrentComboIndex < MaxComboCount
            )
        {
            bComboInputQueued = true;
        }

        return;
    }

    // 공중에서는 콤보를 시작할 수 없습니다.
    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }

    if (!AttackMontage)
    {
        return;
    }

    FaceAttackDirection();
    /*
     * 반격 가능 시간 안에 공격했다면
     * 일반 콤보 대신 반격 공격으로 시작합니다.
     */
    if (CanParryCounter())
    {
        StartParryCounterAttack();
        return;
    }

    StartCombo();
}

void APMCharacter::FaceAttackDirection()
{
    /*
     * 마지막으로 입력된 이동 방향을 가져옵니다.
     * 이동 입력이 있다면 해당 방향으로 공격합니다.
     */
    FVector AttackDirection =
        GetLastMovementInputVector();

    // 수평 방향만 사용합니다.
    AttackDirection.Z = 0.0f;

    AttackDirection =
        AttackDirection.GetSafeNormal();

    /*
     * 이동 입력이 없다면 캐릭터의 현재 방향을
     * 그대로 유지합니다.
     */
    if (AttackDirection.IsNearlyZero())
    {
        return;
    }

    const FRotator AttackRotation(
        0.0f,
        AttackDirection.Rotation().Yaw,
        0.0f
    );

    SetActorRotation(AttackRotation);
}

void APMCharacter::ApplyAttackLunge()
{
    if (AttackLungeStrength <= 0.0f)
    {
        return;
    }

    FVector LungeDirection =
        GetActorForwardVector();

    LungeDirection.Z = 0.0f;

    LungeDirection =
        LungeDirection.GetSafeNormal();

    if (LungeDirection.IsNearlyZero())
    {
        return;
    }

    /*
     * Character Movement에 수평 속도를 추가합니다.
     * true를 사용하여 캐릭터 질량과 관계없이
     * 설정한 값을 속도 변화로 적용합니다.
     */
    GetCharacterMovement()->AddImpulse(
        LungeDirection * AttackLungeStrength,
        true
    );
}

bool APMCharacter::IsParrying() const
{
    return bIsParrying;
}

bool APMCharacter::CanParryAttackFrom(
    const AActor* Attacker
) const
{
    // 현재 패링 중이 아니면 방향과 관계없이 실패합니다.
    if (!bIsParrying)
    {
        return false;
    }

    // 공격자가 없다면 방향을 계산할 수 없습니다.
    if (!Attacker)
    {
        return false;
    }

    /*
     * 높이 차이는 방향 판정에서 제외하고
     * 플레이어에서 공격자로 향하는 수평 방향만 계산합니다.
     */
    FVector DirectionToAttacker =
        Attacker->GetActorLocation()
        - GetActorLocation();

    DirectionToAttacker.Z = 0.0f;

    /*
     * 공격자와 플레이어의 수평 위치가 거의 같다면
     * 유효한 방향을 계산할 수 없습니다.
     */
    if (DirectionToAttacker.IsNearlyZero())
    {
        return false;
    }

    DirectionToAttacker.Normalize();

    // 플레이어의 수평 정면 방향입니다.
    const FVector ForwardDirection =
        GetActorForwardVector().GetSafeNormal2D();

    /*
     *  1.0 : 정면
     *  0.0 : 옆
     * -1.0 : 후방
     */
    const float FacingDot =
        FVector::DotProduct(
            ForwardDirection,
            DirectionToAttacker
        );

    /*
     * ParryFacingAngle은 전체 각도이므로
     * 정면을 중심으로 비교할 때는 절반을 사용합니다.
     */
    const float HalfAngleRadians =
        FMath::DegreesToRadians(
            ParryFacingAngle * 0.5f
        );

    const float MinimumFacingDot =
        FMath::Cos(HalfAngleRadians);

    const bool bIsWithinParryAngle =
        FacingDot >= MinimumFacingDot;

    return bIsWithinParryAngle;
}

bool APMCharacter::CanParryCounter() const
{
    return bCanParryCounter;
}

void APMCharacter::HandleSuccessfulParry(
    const AActor* Attacker
)
{
    /*
     * 이미 패링 판정이 소비됐다면 같은 공격에서
     * 성공 처리가 반복되지 않도록 합니다.
     */
    if (!bIsParrying)
    {
        return;
    }

    /*
     * 공격자가 유효하면 플레이어와 공격자의 중간 지점을
     * 패링 성공 피드백 위치로 사용합니다.
     */
    FVector EffectLocation =
        GetActorLocation()
        + GetActorForwardVector() * 75.0f;

    if (Attacker)
    {
        EffectLocation =
            FMath::Lerp(
                GetActorLocation(),
                Attacker->GetActorLocation(),
                0.5f
            );
    }

    // 이펙트가 지면이 아닌 상체 부근에 나타나도록 높입니다.
    EffectLocation.Z += 75.0f;

    /*
     * 현재 패링 판정을 즉시 소비합니다.
     * 기존 패링 쿨다운은 그대로 유지됩니다.
     */
    EndParry();

    // 패링 성공 후 반격 가능 시간을 시작합니다.
    StartParryCounterWindow();

    // Blueprint에서 이펙트와 카메라 흔들림을 재생합니다.
    PlayParrySuccessFeedback(EffectLocation);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Parry Success Feedback Triggered")
    );
}

void APMCharacter::StartParry()
{
    // 피격 또는 사망 상태에서는 패링할 수 없습니다.
    if (!CanPerformAction())
    {
        return;
    }

    // 패링 쿨다운 중에는 다시 시작하지 않습니다.
    if (!bCanParry)
    {
        return;
    }

    // 이미 패링 판정이 활성화되어 있다면 중복 실행하지 않습니다.
    if (bIsParrying)
    {
        return;
    }

    // 공격 중에는 패링으로 전환할 수 없습니다.
    if (bIsAttacking)
    {
        return;
    }

    // 공중에서는 패링할 수 없습니다.
    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }

    // 수평 이동 중에는 패링을 시작할 수 없습니다.
    if (
        GetVelocity().SizeSquared2D()
    > FMath::Square(10.0f)
        )
    {
        return;
    }

    // 달리기 상태에서는 패링을 시작할 수 없습니다.
    if (
        GetCharacterMovement()->MaxWalkSpeed
    > WalkSpeed
        )
    {
        return;
    }

    bIsParrying = true;
    bCanParry = false;

    if (ParryMontage)
    {
        const float MontageDuration =
            PlayAnimMontage(ParryMontage);

        bIsParryAnimationPlaying =
            MontageDuration > 0.0f;

        if (bIsParryAnimationPlaying)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Parry Montage Played")
            );
        }
        else
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Failed to Play Parry Montage")
            );
        }
    }
    else
    {
        bIsParryAnimationPlaying = false;

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Parry Montage Is Not Assigned")
        );
    }


    // 짧은 패링 성공 판정 시간을 시작합니다.
    GetWorldTimerManager().SetTimer(
        ParryWindowTimer,
        this,
        &APMCharacter::EndParry,
        ParryWindowDuration,
        false
    );

    /*
     * 패링 입력 시점부터 쿨다운을 계산합니다.
     * 설정 실수로 쿨다운이 판정 시간보다 짧아지지 않게 보정합니다.
     */
    const float EffectiveCooldown =
        FMath::Max(
            ParryCooldown,
            ParryWindowDuration
        );

    GetWorldTimerManager().SetTimer(
        ParryCooldownTimer,
        this,
        &APMCharacter::ResetParryCooldown,
        EffectiveCooldown,
        false
    );
}

void APMCharacter::EndParry()
{
    if (!bIsParrying)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(
        ParryWindowTimer
    );

    bIsParrying = false;

}

void APMCharacter::ResetParryCooldown()
{
    // 사망했다면 패링 가능 상태로 복구하지 않습니다.
    if (
        HealthComponent
        && HealthComponent->IsDead()
        )
    {
        return;
    }

    bCanParry = true;

}

void APMCharacter::StartParryCounterWindow()
{
    /*
     * 혹시 기존 반격 타이머가 남아 있다면 제거하고
     * 새로운 반격 가능 시간을 시작합니다.
     */
    GetWorldTimerManager().ClearTimer(
        ParryCounterWindowTimer
    );

    bCanParryCounter = true;

    GetWorldTimerManager().SetTimer(
        ParryCounterWindowTimer,
        this,
        &APMCharacter::EndParryCounterWindow,
        ParryCounterWindowDuration,
        false
    );
}

void APMCharacter::EndParryCounterWindow()
{
    GetWorldTimerManager().ClearTimer(
        ParryCounterWindowTimer
    );

    bCanParryCounter = false;
}

void APMCharacter::StartParryCounterAttack()
{
    if (!bCanParryCounter)
    {
        return;
    }

    /*
     * 공격 버튼을 누른 시점에 반격 가능 상태를 소비합니다.
     * 타이머도 함께 제거되므로 반격 기회가 다시 종료되지 않습니다.
     */
    EndParryCounterWindow();

    /*
     * 실제 피해 판정은 AttackHit Notify에서 실행되므로
     * 이번 공격이 반격이라는 정보를 그때까지 보관합니다.
     */
    bIsParryCounterAttacking = true;

    /*
     * 아직 반격 전용 몽타주가 없으므로
     * 기존 콤보의 Attack1을 사용합니다.
     */
    StartCombo();
}

void APMCharacter::StartCombo()
{
    CurrentComboIndex = 1;
    bIsAttacking = true;
    bCanQueueCombo = false;
    bComboInputQueued = false;

    const FName StartSection =
        GetComboSectionName(CurrentComboIndex);

    const float MontageDuration =
        PlayAnimMontage(
            AttackMontage,
            1.0f,
            StartSection
        );

    if (MontageDuration <= 0.0f)
    {
        ResetCombo();
        return;
    }

    ApplyAttackLunge();
}

void APMCharacter::HandleMontageNotifyBegin(
    FName NotifyName,
    const FBranchingPointNotifyPayload& BranchingPointPayload
)
{
    (void)BranchingPointPayload;

    /*
    * 회피 몽타주 Notify는 공격 상태 확인보다 먼저 처리합니다.
    * 회피 중에는 bIsAttacking이 false이기 때문입니다.
    */
    if (bIsDodging)
    {
        if (
            NotifyName
            == TEXT("DodgeInvincibilityStart")
            )
        {
            StartDodgeInvincibility();

            return;
        }

        if (
            NotifyName
            == TEXT("DodgeInvincibilityEnd")
            )
        {
            EndDodgeInvincibility();

            return;
        }
    }
    
    if (!bIsAttacking)
    {
        return;
    }

    if (NotifyName == TEXT("AttackHit"))
    {
        PerformAttackTrace();
        return;
    }

    if (NotifyName == TEXT("ComboWindowOpen"))
    {
        if (CurrentComboIndex < MaxComboCount)
        {
            bCanQueueCombo = true;
        }

        return;
    }

    if (NotifyName == TEXT("ComboWindowClose"))
    {
        TryContinueCombo();
    }
}

void APMCharacter::HandleMontageEnded(
    UAnimMontage* Montage,
    bool bInterrupted
)
{
    (void)bInterrupted;

    if (Montage == ParryMontage)
    {
        bIsParryAnimationPlaying = false;
        return;
    }

    // 회피 몽타주가 끝나면 회피 동작 상태를 종료합니다.
    if (Montage == DodgeMontage)
    {
        EndDodge();
        return;
    }

    // 공격 몽타주가 아니라면 처리하지 않습니다.
    if (Montage != AttackMontage)
    {
        return;
    }

    ResetCombo();
}

void APMCharacter::PerformAttackTrace()
{
    /*
     * 이번 공격 판정이 패링 반격인지 먼저 저장합니다.
     * 이후 상태를 즉시 소비하여 다음 콤보 타격에는
     * 반격 피해가 반복되지 않게 합니다.
     */
    const bool bWasParryCounterAttack =
        bIsParryCounterAttacking;

    bIsParryCounterAttacking = false;

    const float DamageToApply =
        bWasParryCounterAttack
        ? ParryCounterDamage
        : AttackDamage;

    const FVector ForwardDirection =
        GetActorForwardVector();

    /*
     * 캐릭터 중심보다 약간 앞쪽과 위쪽에서
     * 공격 범위 검사를 시작합니다.
     */
    const FVector TraceStart =
        GetActorLocation()
        + FVector(0.0f, 0.0f, 50.0f)
        + ForwardDirection * 50.0f;

    const FVector TraceEnd =
        TraceStart
        + ForwardDirection * AttackRange;

    TArray<FHitResult> HitResults;

    FCollisionQueryParams QueryParams;

    // 자기 자신은 공격 판정에서 제외합니다.
    QueryParams.AddIgnoredActor(this);

    const FCollisionShape AttackShape =
        FCollisionShape::MakeSphere(AttackRadius);

    /*
     * TraceStart에서 TraceEnd까지 구체를 이동시키며
     * 충돌한 모든 액터를 찾습니다.
     */
    const bool bHit =
        GetWorld()->SweepMultiByChannel(
            HitResults,
            TraceStart,
            TraceEnd,
            FQuat::Identity,
            ECC_Visibility,
            AttackShape,
            QueryParams
        );

    /*
     * 하나의 액터가 여러 컴포넌트로 감지되더라도
     * 피해는 한 번만 주기 위해 Set을 사용합니다.
     */
    TSet<AActor*> DamagedActors;

    if (bHit)
    {
        for (const FHitResult& HitResult : HitResults)
        {
            AActor* HitActor =
                HitResult.GetActor();

            if (!HitActor)
            {
                continue;
            }

            if (DamagedActors.Contains(HitActor))
            {
                continue;
            }

            /*
             * HealthComponent가 없는 벽이나 바닥에는
             * 피해를 적용하지 않습니다.
             */
            UPMHealthComponent* TargetHealth =
                HitActor->FindComponentByClass<
                UPMHealthComponent
                >();

            if (!TargetHealth)
            {
                continue;
            }

            DamagedActors.Add(HitActor);

            UGameplayStatics::ApplyDamage(
                HitActor,
                DamageToApply,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }

    // 공격 판정의 방향과 끝 지점을 1초간 표시합니다.
    DrawDebugLine(
        GetWorld(),
        TraceStart,
        TraceEnd,
        FColor::Yellow,
        false,
        1.0f,
        0,
        2.0f
    );

    DrawDebugSphere(
        GetWorld(),
        TraceEnd,
        AttackRadius,
        16,
        bHit ? FColor::Green : FColor::Red,
        false,
        1.0f
    );
}

bool APMCharacter::CanPerformAction() const
{
    if (bIsHitReacting)
    {
        return false;
    }

    if (bIsDodging)
    {
        return false;
    }

    if (
        HealthComponent
        && HealthComponent->IsDead()
        )
    {
        return false;
    }

    return true;
}

void APMCharacter::CancelAttack()
{
    if (AttackMontage)
    {
        StopAnimMontage(AttackMontage);
    }

    ResetCombo();
}

float APMCharacter::TakeDamage(
    float DamageAmount,
    const FDamageEvent& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser
)
{
    if (bIsDodgeInvincible)
    {
        return 0.0f;
    }

    return Super::TakeDamage(
        DamageAmount,
        DamageEvent,
        EventInstigator,
        DamageCauser
    );
}

void APMCharacter::HandleHealthChanged(
    float CurrentHealth,
    float MaxHealth
)
{
    (void)MaxHealth;

    const bool bTookDamage =
        CurrentHealth < PreviousHealth;

    PreviousHealth = CurrentHealth;

    // 체력이 0이면 곧 OnDeath가 방송되므로
    // 일반 피격 반응을 실행하지 않습니다.
    if (CurrentHealth <= 0.0f)
    {
        return;
    }

    if (!bTookDamage)
    {
        return;
    }

    // 추가 피해는 적용되지만 피격 반응은
    // 처음부터 다시 시작하지 않습니다.
    if (bIsHitReacting)
    {
        return;
    }

    StartHitReaction();
}

void APMCharacter::StartHitReaction()
{
    if (bIsHitReacting)
    {
        return;
    }

    bIsHitReacting = true;

    EndParry();

    StopSprint();
    StopJumping();
    CancelAttack();
    CancelDodge();

    GetCharacterMovement()->StopMovementImmediately();

    if (HitReactMontage)
    {
        PlayAnimMontage(HitReactMontage);
    }

    GetWorldTimerManager().SetTimer(
        HitReactTimer,
        this,
        &APMCharacter::EndHitReaction,
        HitStunDuration,
        false
    );
}

void APMCharacter::EndHitReaction()
{
    // 타이머가 끝나기 전에 사망했다면
    // 행동 가능한 상태로 복구하지 않습니다.
    if (
        HealthComponent
        && HealthComponent->IsDead()
        )
    {
        return;
    }

    bIsHitReacting = false;
}

void APMCharacter::HandleDeath()
{
    /*
     * 사망 이전에 실행 중이던 모든 타이머를 정리합니다.
     */
    GetWorldTimerManager().ClearTimer(
        HitReactTimer
    );

    GetWorldTimerManager().ClearTimer(
        DodgeCooldownTimer
    );

    GetWorldTimerManager().ClearTimer(
        DodgeMovementTimer
    );

    GetWorldTimerManager().ClearTimer(
        DodgeInvincibilityTimer
    );

    GetWorldTimerManager().ClearTimer(
        ParryWindowTimer
    );

    GetWorldTimerManager().ClearTimer(
        ParryCooldownTimer
    );

    GetWorldTimerManager().ClearTimer(
        ParryCounterWindowTimer
    );

    GetWorldTimerManager().ClearTimer(
        DeathRestartTimer
    );

    /*
     * 사망 후 기존 행동 상태가 남지 않도록 초기화합니다.
     */
    bIsHitReacting = false;
    bCanDodge = false;
    bIsDodging = false;
    bIsParrying = false;
    bCanParry = false;
    bCanParryCounter = false;
    bIsParryCounterAttacking = false;
    bIsDodgeInvincible = false;
    bIsParryAnimationPlaying = false;

    /*
     * 현재 실행 중인 행동과 몽타주를 정리합니다.
     */
    StopSprint();
    StopJumping();
    CancelAttack();
    CancelDodge();

    if (HitReactMontage)
    {
        StopAnimMontage(HitReactMontage);
    }

    /*
     * 사망한 플레이어의 이동을 즉시 중단합니다.
     */
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    /*
     * 다른 몽타주를 모두 정리한 뒤
     * 사망 몽타주를 재생합니다.
     */
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    /*
     * 사망 후 플레이어 입력을 비활성화합니다.
     */
    APlayerController* PlayerController =
        Cast<APlayerController>(Controller);

    if (PlayerController)
    {
        DisableInput(PlayerController);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Player died. Restarting level in %.1f seconds."),
        DeathRestartDelay
    );

    /*
     * 대기 시간이 0 이하라면 즉시 현재 레벨을 다시 엽니다.
     */
    if (DeathRestartDelay <= 0.0f)
    {
        RestartCurrentLevel();
        return;
    }

    /*
     * 지정된 시간이 지나면 현재 레벨을 다시 시작합니다.
     */
    GetWorldTimerManager().SetTimer(
        DeathRestartTimer,
        this,
        &APMCharacter::RestartCurrentLevel,
        DeathRestartDelay,
        false
    );
}

void APMCharacter::RestartCurrentLevel()
{
    const FString CurrentLevelName =
        UGameplayStatics::GetCurrentLevelName(
            this,
            true
        );

    if (CurrentLevelName.IsEmpty())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Failed to restart level: Level name is empty.")
        );

        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Restarting Level: %s"),
        *CurrentLevelName
    );

    UGameplayStatics::OpenLevel(
        this,
        FName(*CurrentLevelName)
    );
}

FName APMCharacter::GetComboSectionName(
    int32 ComboIndex
) const
{
    switch (ComboIndex)
    {
    case 1:
        return FName(TEXT("Attack1"));

    case 2:
        return FName(TEXT("Attack2"));

    case 3:
        return FName(TEXT("Attack3"));

    default:
        return NAME_None;
    }
}

void APMCharacter::TryContinueCombo()
{
    bCanQueueCombo = false;

    if (!bComboInputQueued)
    {
        return;
    }

    if (CurrentComboIndex >= MaxComboCount)
    {
        bComboInputQueued = false;
        return;
    }

    UAnimInstance* AnimInstance =
        GetMesh()->GetAnimInstance();

    if (!AnimInstance || !AttackMontage)
    {
        ResetCombo();
        return;
    }

    const FName CurrentSection =
        GetComboSectionName(CurrentComboIndex);

    const FName NextSection =
        GetComboSectionName(CurrentComboIndex + 1);

    if (
        CurrentSection.IsNone()
        || NextSection.IsNone()
        )
    {
        ResetCombo();
        return;
    }
    /*
    * 다음 콤보 단계로 넘어가기 전에
    * 현재 이동 입력 방향을 다시 바라봅니다.
    */
    FaceAttackDirection();

    /*
     * 현재 Section이 끝난 뒤 다음 Section으로
     * 이어지도록 연결합니다.
     */
    AnimInstance->Montage_SetNextSection(
        CurrentSection,
        NextSection,
        AttackMontage
    );

    ++CurrentComboIndex;

    ApplyAttackLunge();

    // 사용한 예약 입력을 제거합니다.
    bComboInputQueued = false;
}

void APMCharacter::ResetCombo()
{
    bIsAttacking = false;
    bIsParryCounterAttacking = false;
    CurrentComboIndex = 0;
    bCanQueueCombo = false;
    bComboInputQueued = false;
}


