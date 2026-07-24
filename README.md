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

- [x] 3인칭 캐릭터 이동
- [x] 마우스 카메라 회전
- [x] 점프
- [x] 달리기
- [x] 회피 구르기
- [x] 기본 공격
- [x] 체력 및 피해 시스템
- [ ] 패링
- [ ] 적 AI

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
│  └─ Player/
├─ Blueprints/
│  ├─ Characters/
│  ├─ GameModes/
│  └─ Testing/
├─ Input/
└─ Maps/

Source/
└─ ParryingMaster/
   ├─ PMCharacter.h
   ├─ PMCharacter.cpp
   ├─ PMHealthComponent.h
   └─ PMHealthComponent.cpp

Docs/
├─ DEVLOG.md
├─ TROUBLESHOOTING.md
└─ SCREENSHOTS/


## 핵심 클래스

| 클래스 | 역할 |
|---|---|
| `APMCharacter` | 플레이어 이동, 카메라, 점프, 달리기, 회피 및 공격 입력 처리 |
| `UPMHealthComponent` | 최대 체력, 현재 체력, 피해 및 사망 상태 관리 |
| `BP_PMCharacter` | 플레이어 메시, 애니메이션 및 입력 에셋 연결 |
| `BP_ParryingGameMode` | 기본 플레이어 캐릭터 설정 |
| `BP_DamageZone` | 체력 및 사망 기능을 검사하기 위한 테스트용 피해 구역 |

## 개발 기록

프로젝트의 날짜별 구현 내용과 문제 해결 과정을 기록합니다.

- 📘 [개발 일지](Docs/DEVLOG.md)
- 🛠️ [문제 해결 기록](Docs/TROUBLESHOOTING.md)


## 향후 계획

- [ ] 공격 애니메이션 타격 시점에 공격 판정 추가
- [ ] 테스트용 적 캐릭터에 체력 컴포넌트 적용
- [ ] 한 번의 공격으로 같은 대상에게 한 번만 피해 적용
- [ ] 플레이어 체력 UI 구현
- [ ] 피격 및 사망 애니메이션 연결
- [ ] 회피 애니메이션 개선
- [ ] 패링 판정 구현