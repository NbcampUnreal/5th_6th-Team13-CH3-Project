
# Smash Run !?

## 프로젝트 소개

### 프로젝트 개요

“Smash Run !?”은 Unreal Engine 5.5 기반의 **물리 충돌형 전투 + AI 시뮬레이션** 프로젝트입니다.
플레이어와 몬스터가 **레벨/속도/크기**를 종합한 공정한 충돌 데미지 공식을 통해 상호작용하고, **히트스톱, 블링크, SFX/VFX** 등 전투 피드백이 통합적으로 동작합니다.

> 본 프로젝트는 단순 HP 감소가 아닌, **물리·속도·규모 기반의 충돌 전투**를 목표로 설계되었습니다.

### 주요 기능 요약

* **충돌 전투 시스템 (UCombatComponent)**

  * 역할 판정(가해자/피해자): 레벨 → 충돌 속도 → 현재 속도 → 포인터 순으로 결정
  * 데미지 공식: `H * (ImpactWeight * ImpactFactor + MassWeight * MassTerm)`

    * ImpactFactor = 현재속도 / 최대속도
    * MassTerm = (공격자 크기 / 방어자 크기) - 1
  * 무적(i-frame) + 깜빡임(Blink) + 히트스톱 + SFX/VFX 일괄 처리
* **피격 인터페이스 (IHitDamageable)**

  * 체력 조회/설정, 사망, 레그돌 전환을 공통 인터페이스로 통합
* **몬스터 기본 클래스 (ABaseMonsterCharacter)**

  * 캡슐 충돌 OnHit에서 전투 컴포넌트 호출
  * 몬스터 태그끼리 상호 충돌 피해 무시, 사망 시 레그돌 전환 및 GameMode로 경험치 보고
* **플레이어(유저 조작) – AHERO_Character**

  * Enhanced Input(LOOK/ACCELERATE/DASH/METEOR/THROW)
  * 스킬: 대시, 투사체 투척(FixedDamageProjectile), 메테오 강하(MeteorAOE 즉사 판정)
* **투사체/광역기**

  * AFixedDamageProjectile: 아군/적군 팀 구분, 고정 피해 적용, 이동 모드(ProjectileMovement/Physics) 지원
  * AMeteorAOE: 반경 내 대상 즉사 + 피드백 호출
* **AI 및 BT 태스크**

  * UBTTaskFindRandomLocation: NavMesh 내 도달 가능한 랜덤 위치
  * UBTTask_RunAway: 타겟 반대 방향 우선, 경로 길이로 우수 후보 선택
  * UBTTask_SetIsRunning / UBTTaskClearInvestigate: 블랙보드 상태 토글/초기화
  * TestAIController: AI 전용 컨트롤러
* **스폰/웨이브/스테이지**

  * ASpawnEnemy + FEnemySpawnRow(DataTable): 확률 기반 스폰
  * ATeam13_GameMode/GameState: 스테이지 시작/종료/게임오버/레벨업 훅 보유
* **오브젝트 풀링**

  * UWorldSubsystem 기반 ObjectPoolManager + IPoolable(인터페이스)
  * 풀 초기화/스폰/반납 지원
* **UI/HUD/메뉴**

  * ATeam13_PlayerController: HUD/메인/가이드/설정/엔딩/크레딧 위젯 관리

### 사용 라이브러리 및 기술

* **Unreal Engine 모듈**: Core, CoreUObject, Engine, InputCore, EnhancedInput, AIModule, GameplayTasks, NavigationSystem, UMG, DeveloperSettings, Niagara
* **언리얼/엔진 기능**: BehaviorTree + Blackboard, NavMesh, UDeveloperSettings(전역 전투 피드백 설정), Niagara VFX, UMG 위젯
* **STL/컨테이너(언리얼 템플릿)**: `TMap`, `TArray`, `TScriptInterface`, `TFunctionRef` 등 광범위 활용

---

## 개발 환경

| 항목  | 내용                      |
| --- | ----------------------- |
| 엔진  | Unreal Engine 5.5       |
| 언어  | C++17 이상                |
| IDE | Visual Studio 2022      |
| 빌드  | Unreal Build Tool (UBT) |
| 대상  | Windows 64-bit          |

### 빌드 / 실행 방법

1. UE 5.5에서 `.uproject` 열기
2. `Team13_Project` 모듈 빌드
3. Content 내 테스트 맵 (StartMenu) 실행
4. PIE에서 플레이어 조작 및 몬스터 충돌/전투 시뮬레이션

### 의존/플러그인

* **필수 모듈**: EnhancedInput, AIModule, GameplayTasks, NavigationSystem, UMG, DeveloperSettings, Niagara
* **BehaviorTree/Blackboard**: AI 편집기에서 블랙보드 키(`TargetActor`, `EscapeLocation`, `IsRunning`, `IsInvestigating`)가 코드와 일치하는지 확인

---

## 프로젝트 구조

```
Source/Team13_Project
 ├─ Public
 │  ├─ CombatComponent.h              // 전투/피드백 핵심(데미지 공식, 무적/블링크/히트스톱, SFX/VFX)
 │  ├─ HitDamageable.h                // 공통 피격 인터페이스
 │  ├─ BaseMonsterCharacter.h         // 몬스터 기본(충돌시 Combat 호출, 사망/레그돌)
 │  ├─ HERO_Character.h               // 플레이어(스킬/메테오/투척/대시, IHitDamageable)
 │  ├─ FixedDamageProjectile.h        // 고정 피해 투사체(팀/이동 모드)
 │  ├─ MeteorAOE.h                    // 메테오 즉사 AOE
 │  ├─ CombatFeedbackSettings.h       // 전역 피드백 설정(UDeveloperSettings)
 │  ├─ BTTaskFindRandomLocation.h     // AI: 랜덤 목적지
 │  ├─ BTTask_RunAway.h               // AI: 도주 경로 탐색
 │  ├─ BTTask_SetIsRunning.h          // AI: 상태 토글
 │  ├─ BTTaskClearInvestigate.h       // AI: 탐색 상태 해제
 │  ├─ TestAIController.h             // AI 컨트롤러
 │  ├─ SpawnEnemy.h / EnemySpawnRow.h // 스폰 볼륨 + DataTable 행
 │  ├─ ObjectPoolManager.h            // 월드 서브시스템 풀 매니저
 │  ├─ Poolable.h                     // 풀 인터페이스
 │  ├─ Team13_GameMode.h              // 킬/레벨업/스테이지 제어
 │  ├─ Team13_GameState.h             // 스테이지 시간/게임오버/HUD 갱신
 │  └─ Team13_PlayerController.h      // HUD/메뉴 위젯 관리
 └─ Private
    └─ 각 헤더의 구현부(.cpp)
```

---

## 핵심 로직 / 클래스 설명

### 1) UCombatComponent

* **역할 판정(DecideRoles)**: 레벨 우선 → 충돌 속도 → 현재 속도 → 포인터 주소
* **충돌 데미지(ApplyImpactDamage)**

  * 동레벨에서 방어자 속도가 더 빠르면 데미지 무효(공정성 보정)
  * 사망 시 레그돌 전환 + 임펄스 적용

* **고정 데미지(ApplyFixedDamage / ApplyFixedDamage_BP)**: 투사체/트랩 등
* **즉사(ApplyInstantKill)**: 특수 스킬(메테오 등)
* **피드백(ApplyCollisionFeedbackForDefender)**

  * 히트스톱, 넉백/런치, 지상마찰/감속 임시 튜닝, SFX/VFX 재생
* **무적/블링크**

  * `StartInvincibility` → 타이머로 깜빡임 토글, 종료 시 `StopInvincibility`
  * 머티리얼 스칼라 파라미터(`HitBlink` 기본값)로 깜빡임 표시

### 2) ABaseMonsterCharacter

* **OnCapsuleHit**에서 상대가 몬스터(태그/클래스)면 전투 처리 생략, 아니면 Combat 호출
* **OnDead** → GameMode에 경험치 보고, 이동 비활성, 컨트롤러 분리
* **EnableRagdollAndImpulse** → 메시 물리/프로파일 전환, 캡슐 충돌 비활성

### 3) AHERO_Character

* **입력**: Look/Accelerate/Dash/Meteor/Throw 매핑
* **스킬**

  * **Dash**: 쿨다운, 상태 머신
  * **Throw**: `AFixedDamageProjectile` 스폰, 초기 방향/속도 세팅
  * **Meteor**: 상승→표적 강하→착지 시 `AMeteorAOE` 스폰(반경 즉사 + 피드백 브로드캐스트)
* **Landed** → 메테오 강하 종료/AOE 생성 처리 포함

### 4) 투사체 & 광역기

* **AFixedDamageProjectile**

  * 팀(ETeam) 구분, 수명, 히트 시 `Combat.ApplyFixedDamage` 호출
  * 이동 방식 2종: ProjectileMovement / Physics(Impulse)
* **AMeteorAOE**

  * `SphereOverlapActors`로 반경 내 Pawn 대상 수집 → `ApplyInstantKill` 호출
  * 피드백을 위해 가짜 Hit 정보 생성 후 `ApplyCollisionFeedbackForDefender` 호출

### 5) AI / Behavior Tree

* **UBTTaskFindRandomLocation**: NavMesh에서 도달 가능한 랜덤 위치 반환
* **UBTTask_RunAway**: 타겟 반대 방향 직선 후보 → 각도 보정 후보 → 경로 길이 최적 후보 선택
* **상태 토글 태스크**: `UBTTask_SetIsRunning`, `UBTTaskClearInvestigate`

### 6) 스폰/스테이지/게임 흐름

* **ASpawnEnemy + FEnemySpawnRow**: DataTable 기반 확률 스폰
* **ATeam13_GameMode**: `MonsterKilled`, `PlayerLevelUp`, `StartGameStage`, `CompleteGame`
* **ATeam13_GameState**: 스테이지 시간/게임오버/Stage 시작·종료·HUD 갱신

### 7) 오브젝트 풀링

* **ObjectPoolManager(월드 서브시스템)**: 클래스 키로 풀을 보관, 스폰/반납 관리
* **IPoolable**: 스폰/반납 훅 제공(블루프린트 구현 가능)

### 8) UI / 컨트롤러

* **ATeam13_PlayerController**: HUD/Guide/Setting/End/Credit 위젯 인스턴스화 및 표시/제거/전환 API 제공



## 주의사항

* **블랙보드 키 이름**은 BT 태스크 코드와 동일하게 맞춰야 합니다.
  예: `"TargetActor"`, `"EscapeLocation"`, `"IsRunning"`, `"IsInvestigating"`.
* **Niagara/Sound 리소스**는 `UCombatFeedbackSettings` 또는 `UCombatComponent`에 지정되어 있어야 정상 재생됩니다.
* **레그돌 전환** 시 캡슐 충돌이 비활성화되므로, 사후 충돌 이벤트를 기대하면 안 됩니다.
* **Projectile Physics 모드** 사용 시 초기 임펄스/콜리전 설정을 점검하십시오(자체 반동/중력 영향 주의).

---

## 빌드 설정 참고 (Team13_Project.Build.cs)

* `PublicDependencyModuleNames`:
  `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `AIModule`, `GameplayTasks`, `NavigationSystem`, `UMG`, `DeveloperSettings`, `Niagara`
  → 플러그인/모듈 누락으로 인한 빌드 실패를 예방하려면, 프로젝트 플러그인 활성 상태를 확인하세요.

---

## 추후 확장 아이디어

* **스테이지/웨이브 데이터테이블화**: 난이도 곡선, 스폰 테이블, 보상 곡선 외부화
* **콤보/상태이상 시스템**: 경직/에어본/다운/기절 등 상태머신 연동
* **풀링 확대**: 투사체/이펙트 전반을 풀로 전환하여 GC/스폰 오버헤드 감소
* **테스트 자동화**: 충돌/데미지/무적 관련 유닛/통합 테스트 추가

## 오류

* **머트리얼이 깜빡이지 않아서 무적만 작용됨 머트리얼 작동 안함**
