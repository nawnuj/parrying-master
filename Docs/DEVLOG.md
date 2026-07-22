# 개발 일지

Parrying Master의 기능 구현 과정과 테스트 결과를 기록합니다.

---

## 1주차 1일차 — 기본 이동과 카메라

### 목표

3인칭 캐릭터의 기본 조작을 구현한다.

### 구현 내용

- `PMCharacter` C++ 클래스 구성
- Enhanced Input 연결
- WASD 이동 구현
- 마우스 카메라 회전 구현
- 점프 구현
- Spring Arm과 Follow Camera 구성
- 마우스 감도 변수 추가
- 테스트 맵 `L_TestMovement` 생성

### 사용한 주요 에셋

- `IA_Move`
- `IA_MouseLook`
- `IA_Jump`
- `IMC_Default`
- `BP_PMCharacter`
- `BP_ParryingGameMode`

### 테스트 결과

- WASD 이동 정상
- 카메라 기준 이동 방향 계산 정상
- 마우스 카메라 회전 정상
- 점프 정상
- GameMode를 통한 플레이어 생성 정상

### 배운 점

- Input Action은 행동을 정의하고 Input Mapping Context는 실제 키를 연결한다.
- C++ 클래스는 기능을 구현하고 블루프린트는 입력 에셋과 캐릭터 리소스를 연결한다.
- `UPROPERTY`를 사용하면 C++ 변수를 블루프린트에서 조절할 수 있다.

---

## 1주차 2일차 — 달리기

### 목표

왼쪽 Shift를 누르는 동안 캐릭터가 달리도록 구현한다.

### 구현 내용

- `IA_Sprint` 생성
- `IMC_Default`에 Left Shift 매핑
- `WalkSpeed`와 `SprintSpeed` 추가
- `StartSprint()` 구현
- `StopSprint()` 구현
- 입력 취소 시 걷기 속도로 복귀하도록 처리

### 설정값

| 항목 | 값 |
|---|---:|
| Walk Speed | 350 |
| Sprint Speed | 600 |
| Camera Arm Length | 400 |
| Mouse Sensitivity | 0.3 |

### 테스트 결과

- Shift를 누르는 동안 달리기 정상
- Shift를 놓으면 걷기 속도로 복귀
- 입력을 반복해도 속도가 누적되지 않음
- 이동, 점프, 카메라와 함께 정상 작동

### 발생한 문제

달리기 입력 추가 이후 WASD가 카메라를 회전시키고 마우스 카메라 입력이 작동하지 않는 문제가 발생했다.

자세한 해결 과정은 다음 문서에 기록했다.

- [입력 매핑 오류 해결](TROUBLESHOOTING.md#입력-매핑-오류)

---

## 1주차 3일차 — 회피 구르기

### 목표

왼쪽 Ctrl을 누르면 캐릭터가 이동 방향으로 회피하고, 쿨다운을 통해 연속 사용을 제한하도록 구현한다.

### 구현 내용

- `IA_Dodge` 생성
- `IMC_Default`에 Left Ctrl 매핑
- `DodgeStrength`와 `DodgeCooldown` 추가
- `Dodge()`와 `ResetDodge()` 구현
- 이동 입력에 따른 회피 방향 계산
- 정지 상태에서는 캐릭터 정면으로 회피하도록 처리
- 공중 회피 및 연속 사용 제한
- 타이머를 이용한 회피 쿨다운 구현

### 설정값

| 항목 | 값 |
|---|---:|
| Dodge Key | Left Ctrl |
| Dodge Strength | 900 |
| Dodge Cooldown | 0.6초 |

### 테스트 결과

- 이동 중 회피하면 현재 이동 입력 방향으로 회피한다.
- 정지 상태에서 회피하면 캐릭터 정면으로 회피한다.
- 점프 또는 낙하 중에는 회피할 수 없다.
- 회피 직후 연속으로 회피할 수 없다.
- 쿨다운이 끝나면 다시 회피할 수 있다.
- 기존 이동, 카메라, 점프, 달리기가 정상적으로 작동한다.

### 현재 한계 및 향후 개선

- 아직 회피 애니메이션이 없어 캐릭터가 미끄러지는 것처럼 보인다.
- 현재 이동은 `LaunchCharacter()`를 사용한다.
- 이후 회피 애니메이션 몽타주를 연결할 예정이다.
- 애니메이션에 Root Motion을 적용할 경우 기존 이동 방식을 조정할 예정이다.
- 적의 공격 시스템을 구현한 뒤 회피 무적 시간을 추가할 예정이다.
- 스태미나 시스템 구현 후 회피 시 스태미나가 감소하도록 확장할 예정이다.