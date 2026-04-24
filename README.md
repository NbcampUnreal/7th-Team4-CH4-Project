# 🧙 SagoMagic

> **분명 우린 최강이었는데 너무 약해져버렸다!?**  
> : 전설의 마법사들이 전생했더니 잡몹보다 약해져버려 레벨 1부터 시작해서 다시 전설을 만드려고 합니다!!


<div align="center">
  <img src="https://img1.daumcdn.net/thumb/R1280x0/?scode=mtistory2&fname=https%3A%2F%2Fblog.kakaocdn.net%2Fdna%2FnOzpS%2FdJMcajaRGY0%2FAAAAAAAAAAAAAAAAAAAAAPQ_SpT2vjRYEk7STW5mdkF1kchorJ71eyTWMmVClb19%2Fimg.jpg%3Fcredential%3DyqXZFxpELC7KVnFOS48ylbz2pIh7yKj8%26expires%3D1777561199%26allow_ip%3D%26allow_referer%3D%26signature%3DzZnYw0Y5wkoTwYpaZ3YOb5N4s%252BE%253D" width="100%">

  <br>
  <br>
  
  ![Unreal Engine 5](https://img.shields.io/badge/Unreal%20Engine-5.6.1-gray?style=flat-square&logo=unrealengine&logoColor=black)
  ![C++](https://img.shields.io/badge/C++-17-00599C?style=flat-square&logo=c%2B%2B&logoColor=white)
  ![Platform](https://img.shields.io/badge/Platform-PC-EF9421?style=flat-square)
  ![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)

  [🎥**시연 영상**](https://youtu.be/11NvUDFDwIE?si=7EqfNG1Lt5ieRw9K) | 
  [📄**기능 기획서(필수)**](https://www.notion.so/teamsparta/ver-2-3362dc3ef5148013811ee4bd9c28205d) |
  [📄**기능 기획서(도전)**](https://www.notion.so/teamsparta/ver-2-3392dc3ef51480faa997d5f22175e76c) |
  <br>
  [📊**발표 자료**](https://www.canva.com/design/DAHHiVhB08M/icFDoNHq8O5WApR6Pd9RGg/edit) | 
  [📦**게임 패키징(서버)**](https://drive.google.com/file/d/1mJyKENK7O3nlUiQSO09mg-pw7SmfpErd/view?usp=drive_link) |
  [📦**게임 패키징(클라)**](https://drive.google.com/file/d/1m9pm-QaL-CIlj_-8AV06nEPv75UZtqIR/view?usp=drive_link) |
  [💻**저장소**](https://github.com/NbcampUnreal/7th-Team4-CH4-Project)

</div>


<br>

## 1. 프로젝트 개요

| 항목 | 내용 |
|---|---|
| **장르** | Co-op 탑다운 타워디펜스 |
| **플레이어** | 최대 4인 협동 |
| **목표** | 몬스터 웨이브를 막으며 베이스캠프를 수호하라! |
| **엔진** | Unreal Engine 5.6 |
| **언어** | C++, Blueprint |
| **네트워크** | Dedicated Server, Seamless Travel |
| **핵심 기술** | GAS (Gameplay Ability System), 그리드 건축, 격자형 인벤토리 |

<br>

## 2. 게임 루프

```
[타이틀] → [로비 - 4인 접속/커스터마이징] → [정비 단계] → [웨이브 전투]
                                                   ↑              ↓
                                                   └──── 반복 ────┘
                                                         ↓
                                                    [결과 화면] → [로비 귀환]
```

| 단계 | 상태명 | 주요 행동 |
|---|---|---|
| 로비 | `LobbyState` | 참가, 레디, 닉네임·무기·스킬 커스터마이징 |
| 정비 단계 | `BuildState` | 울타리 건설, 스킬/젬 세팅, 베이스캠프 수리 |
| 웨이브 전투 | `CombatState` | 몬스터 교전, 막타 골드 수집 |
| 게임 종료 | `ResultState` | 승리/패배 결과 화면 → 로비 자동 귀환 |

### 승패 조건

| 조건 | 결과 |
|---|---|
| **승리** | 마지막 웨이브 전 몬스터 처치 + 베이스캠프 HP > 0 |
| **패배** | 베이스캠프 HP ≤ 0 |

> 플레이어 전원 사망해도 베이스캠프 HP가 남아있으면 자동 부활 후 게임 계속 진행

<br>

## 3. 웨이브 시스템

### 3-1. 웨이브 타임라인

| 구간 | 제한 시간 |
|---|---|
| 정비 단계 | 20초 |
| Wave 전투 | DT_Wave에서 웨이브별 설정 |

### 3-2. 웨이브 플로우

```
BuildState::Enter()
  └─ WaveManagerSubsystem::PreSpawnForWave()   // 몬스터 미리 스폰 (Hidden 상태)
     ├─ GameState::SetAssetsToLoad()           // 클라이언트 DataAsset 비동기 로드 신호
     ├─ 서버 로드 완료 → bServerReady = true
     ├─ 클라이언트 로드 완료 → ReadyClientCount++
     └─ 전원 준비 완료 → OnReadyForCombat → CombatState 진입

CombatState::Enter()
  └─ WaveManagerSubsystem::StartWave()        // 숨겨진 몬스터 활성화 (SpawnInterval 간격)
     ├─ 전 몬스터 처치 → CheckWaveCleared() → GM::OnWaveCleared
     └─ 제한 시간 초과 → SelfKillAllAliveMonsters() → BaseCamp 피해 → 다음 웨이브
```

### 3-3. 타임오버 패널티

전투 제한 시간 초과 시 잔존 몬스터가 자폭(`SelfKill`)하여 베이스캠프에 마리당 고정 데미지를 입힙니다.

<br>

## 4. 베이스캠프 (ASMBaseCampActor)

- GAS + `USMBaseCampAttributeSet`으로 HP 관리, 클라이언트에 `OnRep_Health`로 복제
- 몬스터만 공격 가능 (아군 태그 시스템으로 플레이어 스킬 피격 방지)
- 상호작용(F키) → 골드 소모로 수리 (서버 권한 검증 후 처리)
- HP가 0이 되면 `ASMGameMode::OnBaseCampDestroyed()` 호출 → `ResultState` 진입 (패배)
- HP 변화 시 `GameplayMessageSubsystem(UI.Event.BaseCamp)` 방송 → HUD 실시간 갱신

<br>

## 5. 플레이어 사망 및 부활

```
HP ≤ 0 감지 (Server, AttributeSet::PostGameplayEffectExecute)
└─ bIsDead = true → OnRep_IsDead → HandleDeath()
   ├─ 이동 즉시 정지 + MovementMode 비활성화
   ├─ 캡슐 충돌 제거 + 래그돌 활성화
   ├─ GameMode::OnPlayerDead() 호출
   │   ├─ ClientRPC_ShowDeathUI(RespawnTime)
   │   ├─ SpectatorTimer (3초 후 언포세스)
   │   └─ RespawnTimer (10초 후 부활)
   └─ 부활 시: PlayerState::ResetForRespawn() → HP 전량 회복 → 새 폰 스폰
```

| 항목 | 값 |
|---|---|
| 관전 전환 | 사망 3초 후 |
| 부활 대기 | 사망 10초 후 자동 부활 |
| 부활 위치 | PlayerStart 랜덤 선택 |
| 인벤토리 | 유지 (드롭 없음) |

<br>

## 6. 조작 시스템

| 구분 | 입력 | 행동 |
|---|---|---|
| 이동 | WASD | 8방향 이동 |
| 공격 | 마우스 좌클릭 | 스킬 발동 (커서 방향 조준) |
| 상호작용 | F | 아이템 픽업, 베이스캠프 수리 |
| 스킬 전환 | 1 / 2 | 퀵슬롯 A/B 전환 |
| 인벤토리 | Tab | 인벤토리 열기/닫기 |
| 아이템 이동 | 좌클릭 드래그 | 격자 내 이동 |
| 아이템 회전 | R | 90도씩 회전 |
| 건축 모드 | B | 건축 모드 On/Off (토글) |
| 펜스 배치 | 좌클릭 × 2 | 시작점 → 끝점 지정으로 경로 배치 |
| 건물 교체 | 마우스 휠 | 이전/다음 건물 선택 |
| 편집 모드 | V | 편집 모드 On/Off (토글) |
| 박스 선택 | 좌클릭 드래그 | 다중 건물 선택 |
| 건물 이동 | 선택 후 좌클릭 홀드 | 드래그 이동 (클릭으로 확정) |
| 건물 삭제 | Delete | 선택 건물 즉시 삭제 |

<br>

## 7. 인벤토리 시스템

Diablo / Path of Exile 방식의 **격자형(Tetris-style) 인벤토리**

```
USMInventoryComponent (PlayerState 소속, COND_OwnerOnly 복제)
├── MainInventory (15×15 격자)
│   ├── 스킬 아이템 — 내부 인벤토리 보유
│   └── 젬 아이템  — 스킬 내부에 장착 가능
├── SkillInternalContainers[]
│   ├── 동일 스킬 장착 → 레벨업
│   └── 젬 장착 → 스탯 강화 / 동작 분기 변화
└── QuickSlots (2슬롯)
    └── 장착 시 GAS에 어빌리티 자동 등록/해제
```

### 7-1. 아이템 형태 (비트마스크 기반)

각 아이템은 `USMGridShapeFragment`에 정의된 비트마스크(`FSMGridMaskData`)로 격자 상의 점유 형태를 표현합니다. 회전(0°/90°/180°/270°)을 지원하며, 배치 시 충돌 감지도 비트마스크 연산으로 처리됩니다.

### 7-2. 스킬 레벨업

스킬 내부 인벤토리에 **동일 이름의 빈 스킬**을 배치하면 레벨이 증가합니다.

| 레벨 | 조건 |
|---|---|
| Lv.1 → Lv.2 | 동일 스킬 1개 내부 장착 |
| Lv.2 → Lv.3 | 동일 스킬 2개 내부 장착 (최대 레벨은 DT_Skill 기준) |

> 레벨업 소재로 사용되는 스킬 아이콘 내부에는 젬 장착 불가

### 7-2. Fragment 패턴

아이템 정의(`USMItemDefinition`)는 Fragment를 조합해 기능을 구성합니다.

| Fragment | 역할 |
|---|---|
| `USMGridShapeFragment` | 점유 형태 비트마스크 |
| `USMAbilityFragment` | 연결 어빌리티 클래스 및 태그 |
| `USMGemModifierFragment` | 젬 효과 종류·수치·장착 조건 태그 |
| `USMInternalInventoryFragment` | 스킬 내부 인벤토리 마스크 및 규칙 |
| `USMSkillProgressionFragment` | 레벨업 규칙 및 최대 레벨 |
| `USMDropRuleFragment` | 드랍/삭제 가능 여부 |
| `USMDisplayInfoFragment` | 표시 이름, 설명, 강조 색상 |
| `USMWorldVisualFragment` | 월드 드랍 메시 및 머티리얼 |

<br>

## 8. 스킬 시스템

### 8-1. GAS 스킬 실행 흐름

```
마우스 좌클릭 → TryActivateAbilitiesByTag()
  → GA_SkillBase::ActivateAbility()
    ├─ LoadActiveSkillSummary() — 인벤토리에서 FSMCompiledSkillSummary 로드
    │   (최종 데미지·쿨타임·범위·지속시간·업그레이드 태그 미리 계산)
    ├─ CommitAbility() → GE_SkillCooldown 적용 (SetByCaller Duration)
    ├─ 몽타주 재생 → AN_SendEvent(AnimNotify) → GameplayEvent 전달
    └─ OnFireEventReceived()
        ├─ [클라이언트] 마우스 좌표 수집 → ServerSetReplicatedTargetData
        └─ [서버] OnTargetDataReadyCallback() → OnSkillEffect() → 실제 피해 적용
```

### 8-2. 구현 스킬 목록

| 스킬 | 클래스 | 기본 동작 | 젬 업그레이드 |
|---|---|---|---|
| **Projectile** | `UGA_Projectile` | 단발 투사체 발사 | Multishot(3발 동시), Homing(유도 추적) |
| **LineTrace (빔)** | `UGA_LineTrace` | 지속 빔 공격 (틱 데미지) | Penetrate(관통), Chain(체인 번개) |
| **SkillField (장판)** | `UGA_SkillField` | 범위 장판 지속 데미지 | Pull(끌어당기기), Slow(이동속도 감소) |
| **ApplyInstantDamage** | `UGA_ApplyInstantDamage` | 범위 즉발 피해 | InstantMulti(다중 동시 타겟), SeparateMulti(연속 낙뢰) |
| **Explosion** | `UGA_Explosion` | 지점 지정 충전 후 폭발 | Duration Gem(충전 시간 단축) |
| **SkillTurret** | `UGA_SkillTurret` | 자동 조준 포탑 설치 | Splash(범위 피해), Barrage(연사) |

### 8-3. 쿨타임 시스템

- `GE_SkillCooldown` (HasDuration) + `SetByCaller(Data.Cooldown)`으로 동적 쿨타임 주입
- GA의 `ActivationBlockedTags`에 `Cooldown.Skill.*` 태그 등록 → 쿨타임 중 자동 차단
- 쿨타임 GE의 `GrantedTags`로 태그가 동적 부여/제거되어 `USMSkillCooldownWidget`이 실시간 갱신

<br>

## 9. 젬 시스템

스킬 내부 인벤토리에 장착해 스탯과 동작 메커니즘을 변화시키는 아이템

### 9-1. 젬 효과 종류

| 젬 종류 | 효과 |
|---|---|
| `Effect` | 스킬 공격력 % 증가 |
| `RangeOrArea` | 사거리 또는 범위 % 증가 |
| `Cooldown` | 쿨타임 % 감소 |
| `TickInterval` | 데미지 적용 간격 % 감소 |
| `Duration` | 지속 시간 % 증가 (Explosion은 충전 시간 단축) |

### 9-2. 젬 태그 호환 조건

```
USMGemModifierFragment
├── RequiredAllTargetTags — 스킬이 모든 태그를 보유해야 장착 가능 (AND)
├── RequiredAnyTargetTags — 스킬이 태그 중 하나 이상을 보유해야 장착 가능 (OR)
└── BlockedTargetTags     — 스킬이 하나라도 해당 태그를 보유하면 장착 불가
```

태그를 아무것도 설정하지 않으면 모든 스킬에 자유 장착 가능합니다.

### 9-3. 스킬 요약 계산 (`FSMCompiledSkillSummary`)

스킬 내부에 장착된 모든 젬을 순회하여 최종 수치를 미리 계산·캐싱합니다.  
어빌리티는 매 발동마다 이 캐시를 읽어 수치를 적용합니다.

```
BuildSkillSummary()
  ├─ DT_Skill 기반 베이스 수치 + 레벨 수치 합산
  ├─ 내부 스킬 수 → CurrentLevel 계산
  └─ 내부 젬 순회
      ├─ Effect    → FinalDamage *= (1 + %)
      ├─ RangeArea → FinalRange *= (1 + %)
      ├─ Cooldown  → FinalCooldown *= (1 - %)
      ├─ Duration  → FinalDuration *= (1 + %)
      ├─ TickInterval → FinalTickInterval *= (1 - %)
      └─ GrantedBehaviorTags → BehaviorTags에 병합 (업그레이드 동작 분기용)
```

<br>

## 10. 건축 시스템

### 10-1. 그리드 매니저 (ASMGridManager)

```
ASMGridManager
├── GridData[]         — 1D 배열 (Index = Y × Width + X)
│   ├── bIsOccupied    — 점유 여부 (클라이언트 복제)
│   ├── BuildingType   — 건물 타입 (클라이언트 복제)
│   ├── OwnerId        — 시공자 ID (클라이언트 복제)
│   └── PlacedActor    — 서버 전용, 복제 제외
├── BakeGridHeights()  — Landscape Z값 사전 베이크 (StartPlay 시 1회)
├── FindPath()         — A* 알고리즘 (방향 전환 패널티 포함, 직선 경로 우선)
├── PlaceBuilding()    — 서버 전용
└── ClearCell()        — 서버 전용
```

### 10-2. 건축 모드 흐름

```
B키 → 건축 모드 진입 (BuildIMC 추가, Tick 활성화)
  ├─ 마우스 이동: UpdateGhostTransform()
  │   ├─ 첫 클릭 전: 단일 고스트 프리뷰
  │   └─ 첫 클릭 후: A*로 시작점~커서까지 경로 전체 고스트 시각화
  │       └─ 코너 자동 감지 (GetEffectiveCornerInfo) → ConvertToCornerPreview()
  ├─ 고스트 머티리얼: 설치 가능(녹색) / 불가(빨간색) 실시간 반영
  ├─ 1차 좌클릭: 시작점 고정 (FenceStartGrid)
  └─ 2차 좌클릭: ServerRPC_RequestPlaceBuilding()
      └─ GA_BuildPlace::ActivateAbility() [ServerOnly]
          ├─ 셀 점유 검증
          ├─ ApplyBuildCost() — GE로 골드 차감
          └─ SpawnAndRegister() — 건물 스폰 + GridData 등록
```

### 10-3. 코너 자동 처리

펜스 경로의 방향이 꺾이는 지점을 자동으로 감지하여 코너 메시로 교체하고 적절한 Yaw를 계산합니다. 경로의 끝점에서도 인접 점유 셀을 검사해 접합부 코너를 자동 처리합니다.

### 10-4. 편집 모드 (SMEditModeComponent)

| 동작 | 처리 방식 |
|---|---|
| 박스 드래그 선택 | 스크린 좌표 기반 ProjectWorldLocationToScreen으로 건물 포함 여부 판단 |
| 건물 이동 (드래그) | 클라이언트 로컬 이동 + `ServerRPC_PreviewMove` → `MulticastRPC_PreviewMove`로 타 클라이언트 동기화 |
| 이동 확정 | `ServerRPC_MoveBuildings` — 원자적 검증 후 전체 성공 또는 전체 원위치 처리 |
| 이동 중 충돌 | Pawn 충돌 채널 비활성화 (`MulticastRPC_SetBuildCollision`) |
| 삭제 | `ServerRPC_DeleteBuildings` → `ClearCellsByActor` + `Destroy()` |

<br>

## 11. 몬스터 AI

### 11-1. AI 구조

```
ASMMonsterAIController
├── BehaviorTree + Blackboard
│   ├── TargetActor   — 항상 BaseCamp (이동 목표)
│   └── IsAttacking   — 공격 대상 존재 여부
└── SetTimer(AttackCooldown) → UpdateTargetAndTryAttack()
    ├─ FindBuildingInRange()   — 1순위: 공격 범위 내 파괴 가능 건물
    ├─ FindNearestPlayerInRange() — 2순위: 범위 내 살아있는 플레이어
    └─ BaseCamp (범위 내)      — 3순위: 베이스캠프
```

### 11-2. 타겟팅 우선순위

| 우선순위 | 타겟 | 조건 |
|---|---|---|
| 1순위 | 건물 (울타리/포탑) | 공격 범위 내, 파괴 가능하고 이동 중이 아닌 건물 |
| 2순위 | 플레이어 | 공격 범위 내, HP > 0인 플레이어 |
| 3순위 | 베이스캠프 | 공격 범위 내, HP > 0인 BaseCamp |

이동 목표는 항상 **가장 가까운 살아있는 BaseCamp**로 유지됩니다.

### 11-3. 몬스터 종류

| 타입 | 공격 방식 | 어빌리티 |
|---|---|---|
| 근거리 (Monkey) | 근접 SphereTrace → `UGA_MonsterAttackBase` | 애니메이션 Notify 타이밍에 타격 판정 |
| 원거리 (Squid) | 투사체 발사 → `UGA_MonsterRangedAttack` | `ASMMonsterProjectile` 스폰 |
| 엘리트 (Elite) | 근접 강화형 | 강화된 스탯 |

### 11-4. 몬스터 사전 스폰 (PreSpawn)

정비 단계에 서버 + 모든 클라이언트의 DataAsset 로드가 완료된 후 몬스터를 숨긴 채로 미리 스폰합니다.  
전투 시작 시 `SpawnInterval` 간격으로 순차 활성화하여 실제 스폰 부하를 분산시킵니다.

### 11-5. 스킬/젬 드롭

- `DropChance`(기본 20%) 확률로 드롭 발생
- `DT_DropTable` 가중치 기반 랜덤 1개 선택 → `ASMBaseItemDropActor` 스폰
- 상호작용(F키)으로 인벤토리에 추가 (공간 없으면 알림 표시 후 미획득)

<br>

## 12. 네트워크 아키텍처

```
[Dedicated Server]
├── ASMGameMode         — 게임 흐름 제어 (웨이브, 리스폰, 결과, 씸리스 트래블)
├── ASMGameState        — 클라이언트 상태 복제 (GameState, 타이머, 에셋 로드 목록)
├── ASMPlayerState      — 플레이어별 ASC, 인벤토리, 커스터마이징 데이터
├── ASMPlayerController — RPC 허브 (UI 지시, 인벤토리 조작, 건축 검증)
├── ASMGridManager      — GridData 복제 (DOREPLIFETIME)
└── USMWaveManagerSubsystem — 서버 전용, 스폰 관리

[Client]
├── ASMPlayerCharacter  — 캐릭터 이동, 스킬 발동, 모드 전환
├── USMInventoryComponent — OwnerOnly 복제 (소유 클라이언트만 수신)
└── UI (UMG + GMS)     — HUD, 인벤토리, 알림, 웨이브 정보
```

### 12-1. 복제 전략

| 대상 | 복제 모드 | 특이사항 |
|---|---|---|
| 플레이어 ASC | `Full` | PlayerState 소속 |
| 몬스터 ASC | `Minimal` | 클라이언트 태그 동기화 |
| 건물 ASC | `Minimal` | HP 등 어트리뷰트만 복제 |
| GridData | `DOREPLIFETIME` | `PlacedActor` 제외 |
| 인벤토리 | `COND_OwnerOnly` | 소유 플레이어에게만 |
| 건물 액터 | `bReplicates = true` | 서버 스폰, 자동 복제 |

### 12-2. 클라이언트 예측 & 서버 검증

```
스킬 발동 (LocalPredicted)
  ├─ 클라이언트: 즉시 코스메틱 (GameplayCue 예측 발동)
  ├─ 마우스 좌표 → ServerSetReplicatedTargetData
  └─ 서버: OnTargetDataReadyCallback → 실제 피해 적용

건축 배치 (ServerOnly)
  ├─ 클라이언트: 고스트 프리뷰만 로컬 표시
  └─ ServerRPC → GA_BuildPlace [ServerOnly] → 검증 후 스폰
```

### 12-3. 보안

- **이속 핵 방어**: 서버에서 1초마다 `MaxWalkSpeed` 검증 및 강제 복구 (허용 오차 110%)
- **건축 검증**: 클라이언트 고스트는 프리뷰 전용, 실제 배치는 서버에서 이중 검증
- **인벤토리 조작**: 모든 Add/Move/Drop이 서버에서 Authority 체크 후 처리

<br>

## 13. 데이터 시스템

### 13-1. DataTable 목록

| 테이블 | 관리 클래스 | 내용 |
|---|---|---|
| `DT_Monster` | `USMSyncDataManager` | 몬스터 스탯, DataAsset 소프트 참조 |
| `DT_Wave` | `USMSyncDataManager` | 웨이브별 스폰 리스트, 제한시간 |
| `DT_Skill` | `USMSyncDataManager` | 스킬 기본 수치, 레벨별 증가량, 업그레이드 태그 |
| `DT_Building` | `USMSyncDataManager` | 건물 비용, GridSize, 클래스 참조 |
| `DT_DropTable` | `USMSyncDataManager` | 아이템 드롭 가중치 |
| `DT_Sound` | `USMSoundManager` | 사운드 ID, 에셋, 카테고리 |

### 13-2. 데이터 매니저 구분

| 매니저 | 생성 조건 | 역할 |
|---|---|---|
| `USMSyncDataManager` | 서버 전용, L_Play + L_Lobby | DataTable 동기 캐싱 |
| `USMAsyncDataManager` | 서버+클라, L_Play | PrimaryDataAsset 비동기 로드/캐시 |

### 13-3. 몬스터 DataAsset 로드 흐름

```
PreSpawnForWave()
  ├─ DT_Monster에서 MonsterDataAsset 경로 수집
  ├─ AssetManager로 PrimaryAssetId 조회
  ├─ GameState::SetAssetsToLoad() → 클라이언트에 로드 목록 복제
  ├─ 서버: AM::LoadAssetsByIDWithBundles({"Server"})
  └─ 클라이언트: OnRep_AssetsToLoad() → AM::LoadAssetsByIDWithBundles({"Client"})
      └─ 완료 시: ServerNotifyClientLoadComplete() → 전원 준비 확인
```

<br>

## 14. 사운드 시스템 (USMSoundManager)

GameInstance 서브시스템으로 게임 전체에서 단일 인스턴스로 동작합니다.

```
USMSoundManager
├── SoundClass 3계층: MasterSoundClass → BGMSoundClass / SFXSoundClass
├── PlaySoundAtLocation()  — 3D 위치 기반 SFX
├── PlaySoundAttached()    — 액터에 부착되는 SFX
├── PlaySoundLoopAttached() — 루프 SFX (AudioComponent 반환, 수동 관리)
├── PlaySoundUI()          — 2D UI 사운드
├── PlayBGM() / StopBGM()  — BGM 페이드인/아웃
└── Set*Volume()           — 볼륨 조절 및 GameUserSettings.ini 자동 저장
```

볼륨 설정은 `GConfig`를 통해 `GameUserSettings.ini`에 영구 저장되며, 게임 재시작 시 자동 복원됩니다.

<br>

## 15. UI 시스템

`GameplayMessageSubsystem`을 중심으로 서버→클라 복제 없이 로컬에서 UI를 갱신합니다.

| 채널 태그 | 발행 시점 | 수신 위젯 |
|---|---|---|
| `UI.Event.Wave` | GameState OnRep, SetCombatInfo | `USMWaveTimeWidget` |
| `UI.Event.BaseCamp` | `USMBaseCampAttributeSet::OnRep_Health` | `USMBaseCampHPBarWidget` |
| `UI.Event.Notification` | `ClientRPC_ShowNotification` | `USMNotificationWidget` |
| `UI.Event.BuildMode` | `BuildingModeComponent` Enable/Disable | `USMBuildModeWidget` |
| `UI.Event.EditMode` | `EditModeComponent` Enable/Disable | `USMEditModeWidget` |
| `SM.Message.Inventory.*` | `USMInventoryComponent` 각 조작 후 | 인벤토리 패널 위젯들 |

<br>

## 16. 프로젝트 구조

```
SagoMagic/
├── Source/SagoMagic/
│   ├── Building/          # 그리드 건축 (GridManager, FenceBuilding, BaseCamp, ThornsFence)
│   ├── Character/         # 플레이어 캐릭터, 컨트롤러, 애님 인스턴스
│   ├── Components/        # BuildingMode, EditMode, InteractionScanner, CharacterWidget
│   ├── Core/
│   │   ├── DataManager/   # SyncDataManager, AsyncDataManager, SoundManager
│   │   ├── SessionSubsystem/ # LobbyGameMode/State, TitleGameMode, SessionSubsystem
│   │   ├── State/         # SMStateMachine, BuildState, CombatState, ResultState
│   │   └── Wave/          # WaveManagerSubsystem, MonsterSpawner
│   ├── Data/              # 데이터 구조체 (Monster, Wave, Skill, Building, Sound, ItemDropTable)
│   ├── Enemy/             # MonsterBase, AIController, MonsterProjectile, AnimInstance
│   ├── GAS/
│   │   ├── Abilities/     # 6종 플레이어 스킬 + 몬스터 공격 어빌리티
│   │   │   └── SkillActor/ # 투사체, 장판, 포탑, 폭발 연출 액터
│   │   ├── AnimNotifies/  # AN_SendEvent (GAS 이벤트 전달)
│   │   ├── AttributeSets/ # Player, Monster, Building, BaseCamp AttributeSet
│   │   ├── Effects/       # GE_InstantDamage, GE_SkillCooldown, GE_BuildCost
│   │   └── GameplayCue/   # 빔, 체인, 장판, 투사체 피격 코스메틱 큐
│   ├── GameplayTags/      # 네이티브 GameplayTag 정의 (Character, Skill, GameFlow, UI 등)
│   ├── Inventory/
│   │   ├── Components/    # SMInventoryComponent (핵심 로직)
│   │   ├── Core/          # 타입 정의 (Container, ItemInstance, Drop, SkillRuntime)
│   │   ├── Items/         # ItemDefinition + Fragment 패턴 (Gem, Skill)
│   │   └── World/         # SMBaseItemDropActor (월드 드랍)
│   ├── Library/           # SMFunctionLibrary (로그 유틸)
│   └── UI/                # HUD, 위젯들, SessionUI
│
└── Content/SagoMagic/
    ├── Data/              # DataTable 및 DataAsset
    ├── Maps/              # L_Title, L_Lobby, L_Play, L_Transition
    ├── UI/                # 위젯 블루프린트
    ├── Enemy/             # 몬스터 BP, BT, Blackboard, 애니메이션
    ├── GAS/               # 스킬 BP, GE, GameplayCue
    └── SoundEffects/      # 사운드 에셋, SoundClass, SoundConcurrency
```

<br>

## 17. 맵 구성

| 맵 | 역할 |
|---|---|
| `L_Title` | 타이틀 화면, IP 입력 후 세션 생성 |
| `L_Lobby` | 대기실 (레디, 커스터마이징, 스킬 선택, 닉네임 설정) |
| `L_Play` | 실제 게임 (그리드 건축, 웨이브 전투, 스킬 전투) |
| `L_Transition` | Seamless Travel 중간 레벨 |

<br>

## 18. 주요 GameplayTag 구조

```
Team.*                  — 아군 식별 (Player, HQ, Building) — 스킬 피격 제외 기준
Enemy.*                 — 적군 식별 (State.Death, Attacking)
State.*                 — 캐릭터 상태 (Build.Place, Build.Edit, Attacking)
Ability.Skill.*         — 스킬 어빌리티 태그 (Projectile, LineTrace, SpawnField...)
Cooldown.Skill.*        — 쿨타임 차단 태그 (ActivationBlockedTags에 등록)
Upgrade.*               — 젬 장착 업그레이드 동작 분기 (BehaviorTags에 병합)
Data.Damage.Amount      — SetByCaller 피해량 태그
Data.Cooldown           — SetByCaller 쿨타임 태그
Event.Skill.*           — AnimNotify → GameplayEvent 전달
GameplayCue.Skill.*     — 코스메틱 큐 (빔, 체인, 장판, 투사체 피격)
UI.Event.*              — GameplayMessageSubsystem UI 이벤트 채널
Build.GoldCost          — 건물 배치 골드 차감 SetByCaller 태그
```

<br>

## 19. 코딩 컨벤션

- **클래스 접두사**: `A`(Actor), `U`(UObject/Component), `F`(Struct), `E`(Enum), `I`(Interface)
- **프로젝트 접두사**: `SM` (SagoMagic)
- **로그 매크로**: `SM_LOG(WorldContextObject, LogSM, Log, TEXT("메시지"), ...)` — 넷모드 자동 표기
- **서버 전용 가드**: 함수 상단 `if (!HasAuthority()) return;` 명시
- **복제 등록**: `DOREPLIFETIME`, `DOREPLIFETIME_CONDITION_NOTIFY` 사용
- **GC 방어**: TObjectPtr, TWeakObjectPtr 적극 활용

<br>

## 20. 주요 설계 제약

- `USMSyncDataManager`는 **서버 전용** — 클라이언트에서는 `nullptr` 반환
- `USMAsyncDataManager`는 **L_Play에서만 생성**
- `GridData[].PlacedActor`는 복제 제외 — 서버 전용 참조 (건물 액터 자체가 `bReplicates=true`로 별도 복제)
- 스킬 어빌리티는 반드시 **PlayerState의 ASC에 등록** (캐릭터 ASC 아님)
- 전투 중(`CombatState`) 건축/편집 모드 사용 불가

<br>

## 21. 라이선스

본 프로젝트는 팀 내부 개발 프로젝트입니다.  
외부 공개 및 배포는 팀의 동의가 필요합니다.

---

*README 최종 업데이트: 2026.04*
