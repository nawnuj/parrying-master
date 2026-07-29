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

```

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

```

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

```
### 배운 점
- 위젯의 위치와 크기 설정은 부모 패널의 Slot 타입에 따라 달라진다.
- 자유로운 위치 배치가 필요할 때는 Canvas Panel을 사용할 수 있다.

---

## 적 캐릭터가 공격 Trace에 감지되지 않는 문제

### 발생 시점

2주차 1일차 적 캐릭터 충돌 설정 과정에서 발생했다.

### 증상

- 적 Capsule의 세부 설정에서 예상한 형태의 Visibility = Block 항목을 바로 찾기 어려웠다.
- 기본 Pawn 프리셋에서 Visibility가 Ignore로 설정되어 있었다.

### 원인

플레이어의 근접 공격 판정은 Visibility Trace 채널을 사용한다.

하지만 적 Capsule의 기본 Pawn 콜리전 프리셋에서는 Visibility 응답이 Ignore로 설정되어 있어 공격 Trace가 적을 감지할 수 없었다.

### 해결 방법

BP_EnemyCharacter의 Capsule Component에서 콜리전 프리셋을 Custom으로 변경하고 다음 값을 설정했다.

```text
- Collision Enabled = Query and Physics
- Object Type = Pawn
- Visibility = Block

```

### 배운 점
- Trace가 사용하는 채널과 대상의 Collision Response가 일치해야 한다.
- Visibility가 Ignore이면 해당 채널을 사용하는 Trace에 감지되지 않는다.
- 기본 콜리전 프리셋의 개별 채널을 수정하려면 Custom 프리셋으로 변경해야 한다.
- 충돌 반응 표는 왼쪽부터 Ignore, Overlap, Block 순서로 표시된다.

---

## 적 AI가 Idle 자세로 미끄러지는 문제

### 발생 시점

2주차 2일차 플레이어 추적 AI와 이동 애니메이션을 연결하는 과정에서 발생했다.

### 증상

- 적이 NavMesh 경로를 따라 플레이어를 정상적으로 추적했다.
- 적 캐릭터의 위치와 방향은 정상적으로 변경되었다.
- 이동 중에도 Idle 자세가 유지되어 바닥 위를 미끄러지는 것처럼 보였다.
- 플레이어 캐릭터에서는 동일한 Skeletal Mesh와 Animation Blueprint로 걷기 애니메이션이 정상적으로 재생되었다.

### 조사

`BP_EnemyCharacter`의 애니메이션 설정을 확인한 결과 다음 연결은 정상적이었다.

```text
- Skeletal Mesh Asset = SKM_Manny_Simple
- Animation Mode = Use Animation Blueprint
- Anim Class = ABP_Unarmed

```

적의 Max Walk Speed와 NavMesh 이동도 정상적으로 작동했다.

ABP_Unarmed의 Should Move 계산을 확인한 결과 실제 지면 속도와 현재 가속도를 모두 검사하고 있었다.
Ground Speed > 0.01
and
Current Acceleration != 0
→ Should Move
플레이어 입력 이동은 가속도 조건을 충족하지만 AI의 경로 이동은 동일한 방식으로 입력 가속도를 만들지 않아 Should Move가 false로 유지되었다.

### 원인

플레이어와 적이 동일한 Animation Blueprint를 사용했지만 이동 방식이 서로 달랐다.

플레이어는 이동 입력을 통해 가속도가 발생한 반면, 적은 MoveToActor()의 경로 요청으로 이동했다. 기존 ABP_Unarmed가 플레이어 입력 가속도를 이동 상태 전환 조건으로 사용했기 때문에 적의 실제 속도가 존재해도 Walk 상태로 전환되지 않았다.

### 해결 방법

플레이어용 ABP_Unarmed를 복제하여 적 전용 ABP_Enemy를 생성했다. (Content/Animations/Enemies/ABP_Enemy)

ABP_Enemy의 Should Move 조건에서 현재 가속도 검사를 제거하고, 실제 지면 속도만 검사하도록 변경했다.

변경 전:
Ground Speed > 0.01
and 
Current Acceleration != 0
→ Should Move

변경 후:
Ground Speed > 0.01
→ Should Move

BP_EnemyCharacter의 Animation Class도 다음과 같이 변경했다.

```text
- Animation Mode = Use Animation Blueprint
- Anim Class = ABP_Enemy

```

### 배운 점 

- 같은 Animation Blueprint를 사용해도 플레이어 입력 이동과 AI 경로 이동에서 제공되는 값은 다를 수 있다.
- 실제 위치가 변하는 것과 Animation Blueprint가 이동 상태로 판단하는 것은 별개의 과정이다.
- AI 이동 애니메이션은 입력 가속도보다 실제 속도를 기준으로 판단하는 것이 안전하다.
- 공통 Animation Blueprint를 직접 수정하기보다 용도별 복제본을 만들면 기존 캐릭터에 미치는 영향을 줄일 수 있다.
- 애니메이션 문제를 조사할 때 Mesh, Anim Class, 이동 속도 및 상태 전환 조건을 분리해서 확인해야 한다.
