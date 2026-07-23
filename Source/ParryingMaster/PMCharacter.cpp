#include "PMCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

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
}

void APMCharacter::BeginPlay()
{
    Super::BeginPlay();

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
            &ACharacter::Jump
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
}

void APMCharacter::Move(const FInputActionValue& Value)
{
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

void APMCharacter::StartSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void APMCharacter::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void APMCharacter::Dodge()
{
    // 아직 쿨다운 중이면 회피하지 않습니다.
    if (!bCanDodge)
    {
        return;
    }

    // 공중에서는 회피하지 못하게 합니다.
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

    // 수평 방향으로만 회피하도록 높이값을 제거합니다.
    DodgeDirection.Z = 0.0f;
    DodgeDirection = DodgeDirection.GetSafeNormal();

    /*
     * 이동 입력이 없는 정지 상태라면
     * 캐릭터가 바라보는 방향으로 회피합니다.
     */
    if (DodgeDirection.IsNearlyZero())
    {
        DodgeDirection = GetActorForwardVector();
        DodgeDirection.Z = 0.0f;
        DodgeDirection.Normalize();
    }

    // 회피 방향으로 캐릭터 몸을 돌립니다.
    const FRotator DodgeRotation(
        0.0f,
        DodgeDirection.Rotation().Yaw,
        0.0f
    );

    SetActorRotation(DodgeRotation);

    // 쿨다운이 끝날 때까지 추가 회피를 막습니다.
    bCanDodge = false;

    // 캐릭터를 회피 방향으로 밀어냅니다.
    LaunchCharacter(
        DodgeDirection * DodgeStrength,
        true,
        false
    );

    // DodgeCooldown초 후 ResetDodge()를 실행합니다.
    GetWorldTimerManager().SetTimer(
        DodgeCooldownTimer,
        this,
        &APMCharacter::ResetDodge,
        DodgeCooldown,
        false
    );
}

void APMCharacter::ResetDodge()
{
    bCanDodge = true;
}

void APMCharacter::Attack()
{
    // 이미 공격 중이면 추가 공격 입력을 무시합니다.
    if (bIsAttacking)
    {
        return;
    }

    // 공중에서는 공격하지 못하게 합니다.
    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }

    // 공격 몽타주가 연결되지 않았다면 실행하지 않습니다.
    if (!AttackMontage)
    {
        return;
    }

    /*
     * 몽타주를 재생하고 재생 시간을 반환받습니다.
     * 재생에 실패하면 0 이하의 값이 반환됩니다.
     */
    const float MontageDuration =
        PlayAnimMontage(AttackMontage);

    if (MontageDuration <= 0.0f)
    {
        return;
    }

    bIsAttacking = true;

    // 몽타주 재생이 끝나면 공격 상태를 해제합니다.
    GetWorldTimerManager().SetTimer(
        AttackTimer,
        this,
        &APMCharacter::ResetAttack,
        MontageDuration,
        false
    );
}

void APMCharacter::ResetAttack()
{
    bIsAttacking = false;
}