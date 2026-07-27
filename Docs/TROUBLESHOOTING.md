# 문제 해결 기록

개발 중 발생한 문제와 조사·해결 과정을 기록합니다.

---

## 입력 매핑 오류

### 발생 시점

1주차 2일차 달리기 기능 추가 과정에서 발생했다.

### 증상

- WASD 입력 시 캐릭터 대신 카메라가 회전했다.
- 마우스를 움직여도 카메라가 회전하지 않았다.
- 캐릭터 이동 방향이 정상적으로 계산되지 않았다.

### 조사

C++의 Enhanced Input 바인딩을 확인한 결과 다음 연결은 정상적이었다.

- `IA_Move` → `Move()`
- `IA_MouseLook` → `Look()`
- `IA_Sprint` → `StartSprint()` / `StopSprint()`

따라서 C++ 로직보다 `IMC_Default` 또는 `BP_PMCharacter`의 에셋 설정 문제로 판단했다.

### 원인

`BP_PMCharacter`의 `CameraBoom` 컴포넌트에서
`Use Pawn Control Rotation` 옵션이 비활성화되어 있었다.

이로 인해 마우스로 변경한 Controller 회전값이 CameraBoom에 적용되지 않았다.
반면 CameraBoom은 캐릭터에 부착되어 있기 때문에 캐릭터가 이동 방향으로
회전할 때 함께 회전했고, WASD가 카메라를 회전시키는 것처럼 보였다.

### 해결 방법

`BP_PMCharacter`의 CameraBoom 설정을 다음과 같이 변경했다.

```text
CameraBoom
- Use Pawn Control Rotation: On
- Do Collision Test: On

FollowCamera
- Use Pawn Control Rotation: Off


### 배운 점

- 3인칭 카메라가 마우스 입력을 따라가지 않을 때 `CameraBoom`의 `Use Pawn Control Rotation` 설정을 확인해야 한다.
- 캐릭터 회전, Controller 회전, Spring Arm 회전은 서로 다른 설정으로 관리된다는 것을 이해했다.
- C++ 코드가 정상이어도 블루프린트에서 설정한 값이 C++ 기본값을 덮어쓸 수 있으므로 `BP_PMCharacter`의 컴포넌트 설정을 함께 확인해야 한다.
- 문제 해결 시 C++ 입력 코드, Input Mapping Context, 블루프린트 컴포넌트 설정을 분리해서 확인하면 원인을 좁히기 쉽다.
- 기능 단위로 Git 커밋을 남기면 문제가 발생했을 때 필요한 에셋만 이전 상태로 복구할 수 있다.
- 정확한 원인을 확인하기 전에는 추정과 확정된 사실을 구분해서 기록해야 한다.


---

## 피해 구역이 게임 화면에 보이지 않는 문제

### 발생 시점

1주차 5일차 테스트용 피해 구역 추가 과정에서 발생했다.


### 증상

- `BP_DamageZone`을 레벨에 배치했지만 게임 실행 중 피해 구역의 위치를 확인하기 어려웠다.

### 원인

`Box Collision` 컴포넌트는 충돌 및 Overlap 판정만 담당하며, 게임 화면에 렌더링되는 메시가 아니다.

따라서 에디터에서 선택했을 때는 영역이 보이지만 게임 실행 중에는 기본적으로 보이지 않는다.

### 해결 방법

`BP_DamageZone`에 `DamageZoneMesh`라는 Static Mesh Component를 추가했다.

```text
BP_DamageZone
├─ Box Collision
└─ DamageZoneMesh

### 배운 점 
- Collision Component는 판정을 담당하지만 게임 화면에 자동으로 표시되지 않는다.
- 판정용 컴포넌트와 시각적 표시용 컴포넌트를 분리할 수 있다.
- 테스트용 액터는 기능뿐 아니라 위치와 범위를 쉽게 확인할 수 있도록 시각화하는 것이 좋다.
- 테스트 메시의 충돌을 끄면 실제 판정 컴포넌트와 충돌이 중복되는 것을 방지할 수 있다.

---

## Progress Bar 위치와 크기 설정이 보이지 않는 문제

### 발생 시점
1주차 6일차 체력 HUD를 설정하는 과정에서 발생했다.

### 증상

`WBP_HUD`에서 `PlayerHealthBar`를 선택했지만 위치 X/Y와 크기 X/Y 설정이 표시되지 않았다.

### 원인

`PlayerHealthBar`가 Canvas Panel 없이 위젯의 Root로 직접 배치되어 있었다.

Canvas Panel의 자식이 아니었기 때문에 `Canvas Panel Slot`의 위치와 크기 설정을 사용할 수 없었다.

### 해결 방법

`PlayerHealthBar`를 Canvas Panel로 감싸 계층구조를 다음과 같이 변경했다.

```text
WBP_HUD
└─ Canvas Panel
   └─ PlayerHealthBar

### 배운 점
- 위젯의 위치와 크기 설정은 부모 패널의 Slot 타입에 따라 달라진다.
- 자유로운 위치 배치가 필요할 때는 Canvas Panel을 사용할 수 있다.