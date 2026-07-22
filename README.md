# 7th-Team3-Final-Project

# 🧟 The Enclosure

> 폐쇄된 쇼핑몰에서 살아남아라 — 좀비 웨이브 서바이벌 FPS

---

## 📌 프로젝트 소개

**The Enclosure**는 Unreal Engine 5로 개발한 좀비 웨이브 서바이벌 TPS입니다.  
폐쇄된 쇼핑몰을 배경으로 몰려오는 좀비를 처치하고, 점수를 모아 탈출하는 것이 목표입니다.

| 항목 | 내용 |
|---|---|
| 엔진 | Unreal Engine 5.6 |
| 언어 | C++ / Blueprint |
| 플랫폼 | Windows |
| 장르 | 좀비 웨이브 서바이벌 TPS |
| 플레이어 | 싱글 플레이 |
| 팀 구성 | 8인 |

---

## 👥 팀원

| 이름 | 역할 |
|---|---|
| 서우정 | 팀장 · 최적화 · 온라인 세션 |
| 성헌호 | 부팀장 · 좀비 시스템 |
| 강민승 | UI 시스템 |
| 유시환 | 아이템 · 상점 |
| 박찬우 | 플레이어 · 애니메이션 · 사운드 |
| 조현준 | 좀비 시스템 |
| 동윤재 | 레벨 디자인 · 최적화 |
| 하상빈 | 총기 시스템 |

---

## 🎮 핵심 게임 루프
좀비 처치 → 점수 획득 → 상점에서 무기·아이템 구매 → 더 강한 웨이브 대응 → 탈출 조건 달성
- 후반으로 갈수록 특수 좀비(러너, 탱크, 위치, 투척) 비율 증가
- 총기 4종 (라이플, 피스톨, 샷건, 스나이퍼) + 근접 무기

---

## 🛠 기술 스택

### 핵심 시스템

| 시스템 | 기술 |
|---|---|
| 전투 | GAS (Gameplay Ability System) |
| 렌더링 | Lumen (하드웨어 레이트레이싱) · Nanite · VSM |
| AI | Behavior Tree · Detour Crowd (최대 128 에이전트) |
| 애니메이션 | PoseSearch · MotionWarping · AnimGraph |
| 이펙트 | Niagara |
| 입력 | Enhanced Input |
| 데이터 | DataTable 기반 데이터 주도 설계 |

### 플러그인

- GameplayAbilities
- OnlineSubsystemSteam
- Niagara
- MotionWarping
- PoseSearch
- AnimationLocomotionLibrary
- Chooser

---

## 🗺 맵 구성

| 맵 | 설명 |
|---|---|
| | 타이틀 |
| | 로비 |
| 플레이 맵1 | 쇼핑몰 |
| 플레이 맵2 | 중세 마을 |
| 플레이 맵3 | 조선 |

---

## ⚙️ 주요 구현

### GAS 기반 전투 시스템
- GameplayEffect로 데미지·버프 처리
- 부위별 데미지 배율 (헤드샷 등)
- 총기·좀비 공격 공통 처리

### 총기 시스템
- `UNCGunComponent` 베이스에서 4종 파생
- 스탯·사운드·이펙트를 DataTable 한 행으로 관리
- 발사·장전·ADS·반동·탄 퍼짐·관통 구현

### AI 좀비 시스템
- 공통 베이스 1개 + DataTable로 5종 구현
- 동일 Behavior Tree, 값만 달리해 근접/원거리 구분
- AttackSlot 컴포넌트로 다수 좀비 협조 공격 관리

### 최적화
- `FStreamableManager` 비동기 에셋 프리로딩
- `SeamlessTravel` + `PrepareMapChange` 맵 프리로딩
- URO · 동적 그림자 제거 · 레이트레이싱 제외
- 거리 기반 물리 뼈 LOD
- BT 서비스 0.2초 간격 · 좀비 스폰 시간 램프

---

## 📋 물리 표면 타입

`Default` · `Ground` · `Wooden` · `Grass` · `Glass` · `Metal` · `Tile` · `Concrete` · `Water` · `Stair`

---

## 🔧 빌드 환경

- Visual Studio 2022
- Unreal Engine 5.6
- Windows 10 / 11
- DirectX 12
