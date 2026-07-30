# Parrying Master

언리얼 엔진과 C++를 이용해 개발 중인 패링 중심의 3인칭 액션 게임입니다.

## 프로젝트 목표

적의 공격을 정확한 타이밍에 패링하고 반격하는 전투 시스템을 구현하는 것을 목표로 합니다.

## 개발 환경

- Unreal Engine 5
- C++
- Blueprint
- Visual Studio 2022
- Git / GitHub

## 현재 구현된 기능

### 플레이어

- [x] 3인칭 캐릭터 이동
- [x] 마우스 카메라 회전
- [x] 점프
- [x] 달리기
- [x] 방향 기반 회피
- [x] 기본 공격 입력 및 애니메이션
- [x] 재사용 가능한 체력 컴포넌트
- [x] 피해 처리 및 사망 이벤트
- [x] 사망 후 플레이어 이동 및 입력 제한
- [x] 플레이어 체력 HUD
- [ ] 피격 및 사망 애니메이션
- [ ] 패링

### 적

- [x] 기본 적 캐릭터 C++ 클래스
- [x] 적 캐릭터 메시와 애니메이션 블루프린트 연결
- [x] 적 체력 컴포넌트 적용
- [x] 플레이어 공격으로 적 피해 처리
- [x] 적 체력이 0일 때 이동 및 충돌 비활성화
- [x] 적 사망 후 자동 제거
- [x] 플레이어 추적 AI
- [x] NavMesh 기반 경로 이동
- [x] 적 이동 및 정지 애니메이션
- [x] 적 공격 거리 판정 및 공격 애니메이션
- [x] 적 공격 타격 시점 Notify
- [x] 적 근접 공격 판정 및 플레이어 피해 적용
- [x] 적 공격 쿨다운
- [ ] 적 피격 및 경직 애니메이션

### 기타

- [x] 테스트용 피해 구역
- [x] 체력과 사망 기능을 가진 테스트 더미
- [x] 기본 공격 판정 및 피해 적용
- [x] 테스트 더미 공격 및 사망 테스트


## 조작 방법

| 동작 | 키 |
|---|---|
| 이동 | W, A, S, D |
| 카메라 | 마우스 |
| 점프 | Space |
| 달리기 | Left Shift |
| 회피 구르기 | Left Ctrl |
| 기본 공격 | 마우스 왼쪽 버튼 |

## 프로젝트 구조

```text
Content/
├─ Animations/
│  ├─ Enemies/
│  │  ├─ ABP_Enemy
│  │  └─ AM_Enemy_Attack_01
│  └─ Player/
├─ Blueprints/
│  ├─ Characters/
│  │  └─ BP_PMCharacter
│  ├─ Enemies/
│  │  ├─ BP_TestDummy
│  │  └─ BP_EnemyCharacter
│  ├─ GameModes/
│  └─ Testing/
│     └─ BP_DamageZone
├─ Input/
├─ Maps/
│  └─ L_TestMovement
└─ UI/
   └─ WBP_HUD

Source/
└─ ParryingMaster/
   ├─ PMCharacter.h
   ├─ PMCharacter.cpp
   ├─ PMEnemyAIController.h
   ├─ PMEnemyAIController.cpp
   ├─ PMEnemyCharacter.h
   ├─ PMEnemyCharacter.cpp
   ├─ PMHealthComponent.h
   └─ PMHealthComponent.cpp

Docs/
├─ DEVLOG.md
├─ TROUBLESHOOTING.md
└─ SCREENSHOTS/

```
## 핵심 클래스 및 블루프린트

| 이름 | 역할 |
|---|---|
| `APMCharacter` | 이동, 카메라, 점프, 달리기, 회피, 공격 입력 및 근접 공격 판정 처리 |
| `UPMHealthComponent` | 플레이어와 적의 최대 체력, 현재 체력, 피해 및 사망 상태 관리 |
| `APMEnemyCharacter` | 적 캐릭터의 체력, 공격 몽타주, 근접 공격 판정, AI Controller 연결 및 사망 처리 |
| `APMEnemyAIController` | 플레이어 탐색, NavMesh 경로 요청, 추적 이동 및 공격 시작 요청 처리 |
| `BP_PMCharacter` | 플레이어 메시, 애니메이션, 입력 에셋 및 HUD 연결 |
| `BP_EnemyCharacter` | 적 메시, 애니메이션, 체력, 충돌, AI 및 공격 설정 |
| `ABP_Enemy` | 적의 실제 이동 속도를 이용한 Idle 및 Walk 애니메이션 전환 |
| `BP_ParryingGameMode` | 기본 플레이어 캐릭터 설정 |
| `AM_Player_Attack_01` | 기본 공격 애니메이션과 타격 시점 Notify 관리 |
| `AM_Enemy_Attack_01` | 적 공격 애니메이션과 타격 시점 Notify 관리 |
| `WBP_HUD` | 플레이어의 현재 체력을 Progress Bar로 표시 |
| `BP_DamageZone` | 플레이어의 체력 및 사망 기능 테스트 |
| `BP_TestDummy` | 플레이어 공격의 판정, 피해 및 사망 처리 테스트 |

## 1주차 완료 결과

플레이어가 테스트 맵에서 이동·점프·달리기·회피·공격할 수 있으며, 공격 애니메이션의 타격 시점에 테스트 더미를 감지하고 피해를 적용할 수 있다.

테스트 더미는 체력이 0이 되면 사망 처리되며, 플레이어는 피해 구역을 통해 체력 감소, HUD 갱신 및 사망 상태를 테스트할 수 있다.

## 2주차 진행 상황

기존 체력 시스템을 재사용하는 기본 적 캐릭터를 구현하고, 플레이어 공격으로 피해를 받아 사망하도록 연결했다.

적 AI Controller와 NavMesh 경로 이동을 추가하여 적이 플레이어를 추적하고 일정 거리에서 멈추도록 구현했다. 적 전용 Animation Blueprint는 실제 이동 속도를 기준으로 Idle과 Walk 상태를 전환한다.

적이 공격 거리 안에서 이동을 멈추고 플레이어 방향으로 회전한 뒤 공격 몽타주를 재생하도록 구현했다. 공격 애니메이션의 타격 시점에는 Pawn 대상 근접 공격 판정을 실행하고 플레이어에게 피해를 적용한다.

현재 플레이어가 사망해도 적이 공격을 계속 시도하므로, 다음 단계에서 플레이어 사망 상태에 따른 적 AI 정지 처리를 추가할 예정이다.

## 개발 기록

프로젝트의 날짜별 구현 내용과 문제 해결 과정을 기록합니다.

- 📘 [개발 일지](Docs/DEVLOG.md)
- 🛠️ [문제 해결 기록](Docs/TROUBLESHOOTING.md)


## 향후 계획

- [ ] 플레이어 피격 및 사망 애니메이션
- [ ] 플레이어 사망 후 레벨 재시작
- [ ] 회피 애니메이션과 무적 시간
- [ ] 공격 애니메이션 및 Root Motion 개선
- [ ] 플레이어 사망 시 적 추적 및 공격 중단
- [ ] 적 공격 패턴 확장
- [ ] 방어 및 패링 입력
- [ ] 패링 성공 판정과 적 경직