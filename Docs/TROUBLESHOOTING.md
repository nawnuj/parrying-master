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


---

## 플레이어 사망 후에도 적이 공격을 계속하는 문제

### 발생 시점

2주차 4일차 플레이어 사망과 적 AI 동작을 테스트하는 과정에서 발생했다.

### 증상

- 적의 공격으로 플레이어 체력이 0이 된 뒤에도 적이 공격 몽타주를 반복해서 재생했다.
- 플레이어 체력은 0 아래로 감소하지 않았지만 적은 사망한 플레이어를 계속 추적 대상으로 사용했다.

### 조사

플레이어의 `UPMHealthComponent`는 체력이 0이 되면 `bIsDead`를 true로 변경하고 추가 피해를 무시하고 있었다.

하지만 플레이어 Pawn은 맵에 계속 존재했고, 적 AI Controller의 `TargetPawn`도 유효한 상태로 남아 있었다.

```text
플레이어 체력 0
→ HealthComponent 사망 상태
→ 플레이어 Pawn은 계속 존재
→ TargetPawn은 여전히 유효
→ 추적 및 공격 요청 반복

```
### 원인

적 AI Controller는 Target Pawn의 유효성만 확인하고 플레이어의 체력 및 사망 상태는 확인하지 않았다.

Actor 또는 Pawn이 유효하다는 사실만으로 살아 있는 대상이라고 판단했기 때문에 사망 이후에도 기존 AI 로직이 계속 실행되었다.

### 해결 방법

적 AI Controller가 Target Pawn에 연결된 UPMHealthComponent를 찾도록 변경했다.

```text
TargetPawn
→ FindComponentByClass<UPMHealthComponent>()
→ IsDead()

```

플레이어가 사망한 경우 다음 상태를 정리했다.

```text
StopMovement()
→ EnemyCharacter의 StopCombat()
→ TargetPawn.Reset()
→ AI Controller Tick 비활성화
→ return

```

APMEnemyCharacter::StopCombat()은 다음 공격 상태를 한 번에 정리한다.

- 공격 몽타주 중지
- 공격 쿨다운 타이머 제거
- bIsAttacking 해제
- bCanAttack 비활성화
- CurrentAttackTarget 해제

### 배운 점

- Pawn의 유효성과 캐릭터의 생존 상태는 별도로 확인해야 한다.
- 생존 상태의 원본은 체력 컴포넌트 한 곳에서 관리하는 것이 안전하다.
- AI Controller는 판단과 추적을 담당하고 EnemyCharacter는 공격 상태를 직접 정리하도록 역할을 나눌 수 있다.
- 여러 공격 상태를 하나의 StopCombat() 함수로 묶으면 중복 코드와 누락 가능성을 줄일 수 있다.
- 플레이어 부활 기능을 구현할 경우 비활성화한 AI Tick을 다시 활성화해야 한다.

---

## 콤보 입력이 예약되어도 다음 공격이 재생되지 않는 문제

### 발생 시점

2주차 6일차 플레이어 3단 연속 공격을 구현하는 과정에서 발생했다.

### 증상

- 공격 키를 한 번 누르면 1타가 정상적으로 재생되었다.
- `AttackHit`, `ComboWindowOpen`, `ComboWindowClose` Notify가 모두 발생했다.
- 입력 가능 구간에서 공격 키를 눌러도 2타로 연결되지 않았다.
- 출력 로그에서는 입력 예약과 Section 연결 함수가 정상적으로 실행되고 있었다.

```text
Combo Input: Index=1, CanQueue=true
Combo input queued.
TryContinueCombo: Index=1, Queued=true
Connect Section: Attack1 -> Attack2
```

### 원인 

ComboWindowClose Notify가 현재 Section의 끝부분에 너무 가깝게 배치되어 있었다.

다음 Section 연결 요청은 실행되었지만 현재 공격이 종료되는 시점과 지나치게 가까워 실제 몽타주 재생에 연결이 반영되지 않았다.

### 해결 방법 

ComboWindowClose를 Section 끝에서 더 앞쪽으로 이동하여 다음 Section을 연결할 시간을 확보했다.

```text
Attack1 종료: 약 30프레임
ComboWindowClose: 약 20~22프레임

Attack2 종료: 약 60프레임
ComboWindowClose: 약 50~52프레임
```

문제 해결 과정에서는 출력 로그를 이용해 다음 단계를 각각 확인했다.

```text
Montage Notify 발생
→ 입력 가능 상태 활성화
→ 다음 공격 입력 예약
→ ComboWindowClose 발생
→ 다음 Section 연결
```

### 배운 점

- 함수가 호출되었다는 사실과 애니메이션 전환이 실제로 적용되었다는 사실은 별도로 확인해야 한다.
- Montage Notify는 현재 Section이 끝나기 전에 충분한 여유를 두고 배치해야 한다.
- 콤보 문제는 입력, 입력 예약, Notify 및 Section 연결 단계를 나누어 조사하면 원인을 찾기 쉽다.
- 디버그 로그는 문제 해결 후 제거하여 출력 로그의 불필요한 반복을 줄이는 것이 좋다.

---

## 3타 발차기에서 다리가 꼬이는 문제

### 발생 시점

2주차 6일차 3단 콤보 애니메이션을 테스트하는 과정에서 발생했다.

### 증상

- 1타와 2타의 주먹 공격은 정상적으로 재생되었다.
- 3타 발차기에서 발이 정상적으로 뻗지 않고 다리가 꼬인 상태로 재생되었다.
- 콤보 단계와 Section 연결 자체는 정상적으로 작동했다.

### 원인

ABP_Unarmed의 AnimGraph에서 공격 몽타주 Slot 뒤에 Control Rig이 연결되어 있었다.

```text
Main States
→ Slot 'DefaultSlot'
→ Control Rig
→ Output Pose
```

공격 몽타주가 발차기 자세를 만든 뒤 Control Rig의 Foot IK가 발을 다시 바닥에 고정하면서 발차기 동작과 충돌했다.

1타와 2타는 상체 중심의 공격이라 문제가 두드러지지 않았지만, 하체 전체를 사용하는 3타 발차기에서는 다리가 꼬이는 현상이 발생했다.

### 해결 방법 

Control Rig이 이동 애니메이션의 발 위치를 먼저 보정하고 공격 몽타주가 마지막에 전신 자세를 적용하도록 노드 순서를 변경했다.

변경 전:

```text
Main States
→ Slot 'DefaultSlot'
→ Control Rig
→ Output Pose
```

변경 후:

```text
Main States
→ Control Rig
→ Slot 'DefaultSlot'
→ Output Pose
```

Is Falling → NOT → Should Do IKTrace 연결은 기존 상태를 유지했다.

### 배운 점

- Animation Blueprint에서는 같은 노드를 사용해도 적용 순서에 따라 최종 자세가 달라진다.
- Foot IK가 전신 공격 몽타주 뒤에 적용되면 발차기와 같은 하체 동작을 덮어쓸 수 있다.
- 상체 공격이 정상이라고 해서 하체까지 몽타주가 정상 적용된 것은 아니다.
- 전신 공격 몽타주는 필요한 경우 Foot IK 이후에 적용해야 한다.
- 애니메이션 문제는 원본 애니메이션, 몽타주 및 Animation Blueprint를 나누어 확인해야 한다.

---

## 회피 거리가 지나치게 길거나 시작할 때만 움직이는 문제

### 발생 시점

4주차 4일차 회피 전용 몽타주와 실제 회피 이동을 구현하는 과정에서 발생했다.

### 증상

- 회피 애니메이션을 재생하면 캐릭터가 지나치게 먼 거리를 빠르게 이동했다.
- Root Motion을 비활성화한 뒤에는 회피 시작 순간에만 조금 이동했다.
- 회피 애니메이션의 나머지 구간에서는 캐릭터가 제자리에서 움직이는 것처럼 보였다.

### 원인

처음에는 원본 애니메이션의 Root Motion 이동과 `LaunchCharacter()`의 코드 이동이 동시에 적용되어 실제 이동 거리가 중복됐다.

Root Motion을 제거한 뒤에는 `LaunchCharacter()`가 수평 속도를 한 번만 적용했으며, 캐릭터가 지면에 있기 때문에 `GroundFriction`과 `BrakingDecelerationWalking`이 속도를 빠르게 감소시켰다.

```text
Root Motion 활성화
+ LaunchCharacter()
→ 애니메이션 이동과 코드 이동 중복
→ 지나치게 긴 회피 거리

Root Motion 비활성화
+ 기존 지상 마찰 유지
→ LaunchCharacter() 직후 빠르게 감속
→ 회피 시작 부분에서만 이동
```

### 해결 방법

원본 `MM_Dash`를 `A_Player_Dodge`로 복제하고 다음과 같이 설정했다.

```text
Enable Root Motion: 비활성화
Force Root Lock: 활성화
```

`A_Player_Dodge`는 `AM_Player_Dodge`의 Montage Track에 배치하여 제자리 회피 자세만 재생하도록 했다.

실제 이동은 `StartDodgeMovement()`에서 담당하며, 회피 이동 중 지상 마찰과 보행 감속을 일시적으로 0으로 변경했다. 0.25초가 지나면 `EndDodgeMovement()`에서 수평 속도를 제거하고 기존 설정을 복구했다.

### 배운 점 

- Root Motion 이동과 코드 기반 이동을 함께 사용하면 이동 거리가 중복될 수 있다.
- LaunchCharacter()의 Strength는 이동 거리가 아니라 적용할 속도의 크기다.
- 지상 캐릭터의 이동 결과는 Ground Friction과 Braking Deceleration의 영향을 받는다.
- 회피 속도와 회피 이동 시간은 별도로 관리해야 체감을 조절하기 쉽다.
- 애니메이션은 자세를 담당하고 코드는 실제 위치를 담당하도록 역할을 분리할 수 있다.
- 임시로 변경한 Character Movement 설정은 회피 종료, 피격 및 사망 시 반드시 복구해야 한다.

---

## 사망 몽타주가 끝난 뒤 Idle 자세로 돌아오는 문제

### 발생 시점

4주차 6일차 플레이어 사망 몽타주를 구현하는 과정에서 발생했다.

### 증상

- 플레이어 체력이 0이 되면 사망 몽타주가 정상적으로 재생됐다.
- 사망 애니메이션의 재생 시간이 짧아 곧바로 재생이 끝났다.
- 사망 몽타주가 끝나면 쓰러진 자세를 유지하지 않고 기본 Idle 자세로 돌아왔다.
- 입력과 이동은 비활성화되어 있지만 캐릭터 외형만 다시 서 있는 상태가 됐다.

### 원인

`AM_Player_Death`의 자동 Blend Out이 활성화되어 있었다.

사망 애니메이션이 끝나면 몽타주가 자동으로 종료되고, `DefaultSlot`의 출력이 Animation Blueprint의 이동 State Machine으로 돌아가면서 Idle 자세가 다시 적용됐다.

```text
사망 몽타주 종료
→ 자동 Blend Out
→ DefaultSlot 종료
→ 이동 State Machine 출력
→ Idle 자세 복귀
```

### 해결 방법

AM_Player_Death의 Asset Details에서 다음 설정을 변경했다.

```text
Enable Auto Blend Out: 비활성화
Loop: 비활성화
```
반복 재생 없이 마지막 프레임까지 한 번 재생한 뒤 몽타주가 자동으로 빠져나오지 않도록 구성했다.

```text
사망 몽타주 재생
→ 마지막 사망 프레임 도달
→ 자동 Blend Out 차단
→ 마지막 사망 자세 유지
```

### 배운 점

- 캐릭터 이동과 입력을 비활성화하는 것만으로 애니메이션 자세가 유지되는 것은 아니다.
- Montage가 Blend Out되면 Animation Blueprint의 기본 State Machine 출력으로 돌아간다.
- 사망처럼 마지막 자세를 유지해야 하는 몽타주는 자동 Blend Out 설정을 확인해야 한다.
- 사망 애니메이션은 반복 재생하지 않고 마지막 프레임에서 유지하는 방식으로 처리할 수 있다.
- 몽타주 재생 순서는 기존 공격, 회피 및 피격 몽타주를 모두 중단한 이후여야 한다.

---

## 적이 일정 거리에서 멈춘 채 공격하지 않는 문제

### 발생 시점

4주차 7일차 사망 후 레벨 재시작 기능을 통합 테스트하는 과정에서 발견했다.

### 증상

- 적이 플레이어를 추적하다가 일정 거리에서 이동을 멈췄다.
- 이동을 멈춘 뒤에도 공격 몽타주가 재생되지 않았다.
- 처음에는 플레이어가 적에게 등을 돌렸을 때 발생하는 것처럼 보였다.
- Acceptance Radius를 낮추면 문제가 발생하는 위치만 달라지고 현상은 계속됐다.
- 공격 Trace 이전에 문제가 발생했으므로 공격 애니메이션 자체가 시작되지 않았다.

### 조사 결과

AI Controller의 공격 시작 조건은 플레이어의 방향을 사용하지 않았다.

```text
FVector::Dist2D(
    EnemyLocation,
    PlayerLocation
)
```

TryAttack()에서도 공격을 시작하기 전에 적이 플레이어 방향으로 직접 회전하고 있었다. 따라서 플레이어가 앞이나 뒤를 바라보는 것은 공격 시작 여부와 관계가 없었다.
실제 문제는 AI Controller와 Enemy Character가 서로 다른 거리 설정값을 공격 판단에 사용하던 구조였다.

```text
APMEnemyAIController
→ AcceptanceRadius 기준으로 이동 정지 및 공격 요청

APMEnemyCharacter
→ EnemyAttackRange 기준으로 실제 공격 허용
```

### 원인

AcceptanceRadius는 원래 NavMesh 이동 요청의 도착 허용 오차이지만 공격 시작 거리로도 사용되고 있었다.
AI가 이동을 멈춘 뒤 TryAttack()이 다시 EnemyAttackRange를 검사하기 때문에 두 값 또는 실제 BP 설정이 어긋나면 이동은 멈췄지만 공격은 시작하지 않는 데드존이 발생할 수 있었다.

```text
AI 이동 정지
→ TryAttack() 요청
→ 실제 공격 거리 검사 실패
→ 공격 몽타주 재생 실패
→ 적이 같은 위치에 정지
```

### 해결 방법

APMEnemyCharacter에 실제 공격 거리를 반환하는 함수를 추가했다.

```text
float APMEnemyCharacter::GetAttackRange() const
{
    return EnemyAttackRange;
}
```

AI Controller의 공격 시작 조건도 동일한 공격 거리를 사용하도록 변경했다.

```text
const float AttackRange =
    EnemyCharacter->GetAttackRange();

if (DistanceToTarget <= AttackRange)
{
    StopMovement();

    EnemyCharacter->TryAttack(
        TargetPawn.Get()
    );

    return;
}
```

AcceptanceRadius는 MoveToActor()의 이동 허용 오차로만 유지했다.

```text
EnemyAttackRange
→ AI 공격 요청과 실제 공격 허용 기준

AcceptanceRadius
→ NavMesh 이동 요청의 도착 허용 오차
```

### 테스트 결과

- 플레이어가 적을 바라볼 때 정상적으로 공격한다.
- 플레이어가 적에게 등을 돌려도 동일하게 공격한다.
- 적이 공격 범위 안에 들어오면 이동을 멈추고 공격 몽타주를 재생한다.
- 공격 거리 경계에서 적이 멈춘 채 대기하는 현상이 해결됐다.
- 기존 공격 Trace와 피해 적용이 정상적으로 유지된다.
- 기존 패링 가능 구간과 패링 경직이 정상적으로 유지된다.

### 배운 점

- 동시에 사용되는 거리 설정값은 이름이 비슷하더라도 책임을 명확하게 구분해야 한다.
- NavMesh의 Acceptance Radius는 전투 공격 범위와 같은 개념이 아니다.
- 같은 공격 가능 여부를 여러 클래스가 판단한다면 하나의 설정값을 공통으로 사용해야 한다.
- 플레이어의 방향과 함께 발생한 현상이라도 실제 조건문에서 방향을 사용하는지 먼저 확인해야 한다.
- 애니메이션이 시작되지 않는 문제는 공격 Trace보다 앞선 AI 요청 및 상태 조건부터 확인해야 한다.