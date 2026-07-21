#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "PMCharacter.generated.h"

// 아래 클래스들이 존재한다고 미리 알려주는 선언입니다.
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UInputMappingContext;

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

    // 마우스 카메라 감도입니다.
    // BP_PMCharacter의 클래스 디폴트에서 조절할 수 있습니다.
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Camera"
    )
    float MouseSensitivity = 0.3f;

private:
    // WASD 입력을 처리합니다.
    void Move(const FInputActionValue& Value);

    // 마우스 움직임을 처리합니다.
    void Look(const FInputActionValue& Value);
};