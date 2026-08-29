# League of Legends - Summoner's Rift

> 리그 오브 레전드의 협곡 전투를 재현한 3인 팀 프로젝트

<!-- TODO: Fog of War 시야 변화 + 상점/인벤토리 대표 GIF 1개 -->

<table>
  <tr><td><b>기간</b></td><td>2026.04 ~ 2026.05 (1개월)</td></tr>
  <tr><td><b>인원</b></td><td>3인</td></tr>
  <tr><td><b>엔진</b></td><td>Unreal Engine 5.7 / C++</td></tr>
  <tr><td><b>담당</b></td><td>Fog of War · 아이템/인벤토리 · 상점 UI · 데이터 파이프라인</td></tr>
</table>

---

## 목차

- [코드 가이드](#코드-가이드)
  - [1. Fog of War](#1-fog-of-war)
  - [2. 아이템과 인벤토리](#2-아이템과-인벤토리)
  - [3. 데이터 파이프라인](#3-데이터-파이프라인)
- [그 외 담당](#그-외-담당)
- [팀 담당 범위](#팀-담당-범위)
- [실행](#실행)

---

## 코드 가이드

### 1. Fog of War

128×128 타일맵 위에서 팀별 시야를 계산하고, 결과를 텍스처로 만들어 포스트프로세스로 렌더링한다.
팀마다 별도의 타일맵을 두고, 로컬 클라이언트는 자기 팀 타일맵만 참조한다.

**가시성 계산 — Symmetric Shadowcasting**

시야 원점을 중심으로 4개 사분면을 나누고, 각 사분면을 깊이 방향으로 한 행씩 스캔한다.

- 기울기를 부동소수 대신 **분수(`FFraction`)로 유지**해 누적 오차 없이 경계 타일을 판정한다. 반올림은 tie-up / tie-down 두 규칙을 나눠 적용해 벽 경계에서 시야가 새는 문제를 막았다
- 벽을 만나면 현재 행의 기울기를 잘라 다음 행으로 넘기고, 열린 구간이 다시 나타나면 재귀적으로 스캔을 분기한다
- 대칭성 판정을 별도로 두어, A가 B를 볼 수 있으면 B도 A를 볼 수 있는 성질을 보장한다 — MOBA에서 시야 비대칭은 그 자체로 버그다

**렌더링 — Marching Squares 업스케일**

128×128을 그대로 텍스처로 쓰면 시야 경계가 계단으로 보인다. 4×4 배율로 확대하되 단순 보간 대신,
인접 4타일의 가시 여부 16가지 조합을 미리 정의한 패턴 테이블로 매핑해 대각 경계를 만들어낸다.
룩업 테이블 방식이라 확대 비용이 픽셀당 상수 시간이다.

**동기화**

- 시야를 제공하는 대상은 `ISightProvider` 인터페이스로 추상화 — 챔피언 · 미니언 · 와드 · 건물이 같은 방식으로 등록된다
- 서버가 팀별 타일맵을 기준으로 적 유닛의 가시 여부를 판정하고, 각 클라이언트는 자기 팀 결과만 수신
- 시야 데이터는 별도 텍스처에 담아 GPU로 전달하고, 포스트프로세스 머티리얼이 최종 안개를 합성

| 역할 | 파일 |
|---|---|
| Shadowcasting · 사분면 스캔 · 적 가시성 판정 | [`FOWManager.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/FOW/FOWManager.cpp) |
| 타일맵 생성 · 텍스처 갱신 · 포스트프로세스 연결 | [`FOWTileMap.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/FOW/FOWTileMap.cpp) |
| Marching Squares 업스케일러 | [`FOWUpscaler.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/FOW/FOWUpscaler.cpp) |
| 시야 제공자 인터페이스 | [`SightProvider.h`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/public/Interfaces/SightProvider.h) |
| 분수 기반 기울기 · 사분면 구조체 | [`FOWManager.h`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/public/FOW/FOWManager.h) |

---

### 2. 아이템과 인벤토리

**데이터와 인스턴스 분리** — `UItemDataAsset`은 아이템 정의(스탯, 가격, 패시브 클래스)를 담는 공유 에셋이고,
`UItemInstance`는 실제로 소유한 개체 하나를 나타낸다. 같은 아이템을 여러 개 들어도 정의는 하나만 존재한다.

**스탯 적용의 O(1) 제거** — 아이템 장착 시 `UStatModifierComponent`에 스탯 수정자를 등록하고 핸들을 돌려받는다.
해제 시에는 핸들만 넘기면 되므로, 어떤 수정자가 어떤 아이템에서 왔는지 역추적할 필요가 없다.
동일 스탯을 올리는 아이템이 여러 개여도 서로 간섭하지 않는다. 최종 스탯은 덧셈 수정자를 먼저 적용한 뒤 곱셈을 적용해 계산한다.

**패시브 효과** — `UItemPassiveEffectBase` 추상 클래스를 상속해 아이템별 고유 효과를 구현한다.
데이터 에셋이 효과 클래스를 참조하므로, 새 패시브를 추가할 때 인벤토리 코드는 건드리지 않는다.

**네트워크 동기화** — `UItemInstance`는 `UObject`라 직접 복제하지 않는다.

- 서버가 골드와 슬롯 상태를 권위 있게 관리하고, 슬롯은 **`ItemID` 배열만 복제**한다
- 클라이언트는 `OnRep`에서 ID로 인스턴스를 재구성 — 복제 대역폭을 최소화
- 인벤토리는 `COND_OwnerOnly`로 소유 클라이언트만 수신
- 구매/판매는 Server RPC로 처리하고, 구매 이력을 스택으로 쌓아 **구매 취소(Undo)** 를 지원

| 역할 | 파일 |
|---|---|
| 구매/판매/취소 · 복제 · 골드 관리 | [`InventoryComponent.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/Components/InventoryComponent.cpp) |
| 아이템 인스턴스 · 장착/해제 | [`ItemInstance.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/Item/ItemInstance.cpp) |
| 스탯 수정자 등록/제거 · 최종값 계산 | [`StatModifierComponent.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/Components/StatModifierComponent.cpp) |
| 스탯 타입 · 수정자 핸들 정의 | [`StatModifierTypes.h`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/public/Type/StatModifierTypes.h) |

---

### 3. 데이터 파이프라인

아이템 수십 종의 스탯과 패시브 수치를 코드나 에셋에 하드코딩하지 않도록, **스프레드시트 → DataTable → DataAsset** 경로를 만들었다.

- `UItemDataSubsystem`이 게임 인스턴스 시작 시 DataTable을 읽어 `UItemDataAsset`을 런타임에 조립한다
- 문자열로 들어온 스탯 타입과 연산자를 열거형으로 변환하는 단계를 두어, 시트 오타가 로드 시점에 드러나게 했다
- 아이콘도 ID 규칙으로 로드해 아이템 추가 시 수동 연결이 필요 없다

시트를 DataTable로 자동 동기화하는 부분은 별도 에디터 플러그인(`SheetSync`)으로 분리했다.

| 역할 | 파일 |
|---|---|
| DataTable 로드 · DataAsset 조립 · 아이콘 매핑 | [`ItemDataSubsystem.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/Manager/ItemDataSubsystem.cpp) |
| 상점 UI · 아이템 목록/구매 | [`ShopWidget.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/UI/View/ShopWidget.cpp) |

---

## 그 외 담당

- **MVVM UI** — Multicast Delegate 기반 ViewModel로 인벤토리/상점 위젯을 갱신. 위젯이 게임 로직을 직접 참조하지 않도록 분리 ([`InventoryViewModel.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/UI/ViewModel/InventoryViewModel.cpp))
- **미니맵** — FOW 텍스처를 미니맵 머티리얼에 바인딩, 카메라 뷰 영역 인디케이터 ([`MinimapWidget.cpp`](https://github.com/tang-ka/POTENUP-LeagueofLegends/blob/34423f7a95cc50150baefaacb9011a2e56668afb/Source/LeagueofLegends/private/UI/View/MinimapWidget.cpp))
- **SheetSync 플러그인** — Google Sheets를 DataTable로 자동 동기화하는 에디터 확장

## 팀 담당 범위

| 파트 | 담당 |
|---|---|
| Fog of War · 아이템/인벤토리 · 상점 · 데이터 파이프라인 | 🟢 본인 |
| 챔피언 스킬 · 전투 로직 | ⚪ 팀원 |
| 미니언 AI · 건물 · 게임 모드 진행 | ⚪ 팀원 |

리슨 서버 기반 멀티플레이로, 각자 담당 시스템의 복제 처리를 직접 구현했다.

## 실행

Unreal Engine 5.7 필요. `LeagueofLegends.uproject` 우클릭 → Generate Visual Studio project files → 빌드 후 실행.
멀티플레이 테스트는 에디터에서 Number of Players 2 이상, Net Mode를 Play As Listen Server로 설정.
