# 언리얼 플레이어 캐릭터 액션 제작
UE5 어빌리티 시스템 개발 방식 체득 목적
> **사전학습**
- UE 용 c++ 기초
- UE 기본 프레임워크
- UE 멀티플레이 프레임워크
- UE 어빌리티 시스템
<p align="center">
  <br>
  <img src="https://github.com/thesun007/metal-slug/assets/39186061/9b89058a-8300-4762-8acb-660e8dd48649">
  <br>
</p>

- 게임 수학 (책)

--- 
### 목차
1. [프레임워크](#1-프레임워크)
2. [기본 이동 모션](#2-기본-이동-모션)
3. [파쿠르](#3-파쿠르)
4. [암살 모션](#4-암살-모션)
5. [기타](#5-기타)
--- 

<br/><br/>

## 1. 프레임워크
- 라일라 샘플 프로젝트 프레임워크 중 일부 차용
    - 데이터 에셋
    - 어빌리티 시스템
    - 태그-인풋 시스템
    - 풀바디 링크된 레이어 애니메이션 시스템
    - 메시지 라우터 플러그인 
    - 추가 코스트 시스템
    - 게임플레이 태그

<br/><br/>

### 데이터 에셋
캐릭터에 적용할 데이터 에셋 구조.
- `UPawnData` : <ins>데이터 최종 패키지</ins> ( `UDJAbilitySet` 컨테이너, `UDJInputData`, `FInputMappingContextAndPriority` 컨테이너)
- `UDJAbilitySet` : <ins>어빌리티 시스템 데이터</ins> (**[태그-어빌리티]** 데이터 컨테이너, **[게임플레이이펙트]** 컨테이너, **[어트리뷰트]** 컨테이너)
  - "게임플레이 어빌리티 스팩"을 생성하여 태그를 지정하고, ASC에 등록하는 함수 보유. (GiveToAbilitySystem(..))
- `UDJInputData` : <ins>입력 데이터</ins> (**[태그-인풋액션]** 컨테이너)
<img src="https://github.com/thesun007/metal-slug/assets/39186061/f890d3db-448c-4c3b-b636-18fc8c170103">
<img src="https://github.com/thesun007/metal-slug/assets/39186061/5d04f7a2-eeee-40d5-9853-a47edbb91e5c">
<img src="https://github.com/thesun007/metal-slug/assets/39186061/0d6e4c45-0f43-4eb1-97b6-145c33867a2d">

<br/><br/>

### 어빌리티 시스템
라일라 프로젝트에서 게임 피처를 제외한 어빌리티 시스템 적용 방식을 분석하여 본 프로젝트에 활용.
- `UDJAbilitySystemComponent`(어빌리티 시스템 컴포넌트 확장) : 입력된 태그와 매칭된 어빌리티를 찾고 해당 어빌리티의 작동 방식에 따라 처리하는 기능 포함되어있음.
- `UDJGameplayAbility`(게임플레이 어빌리티 확장) : 어빌리티 실행 작동 방식(입력 시/입력 동안에 계속/스폰 시)이 추가되어 있음..
- `ADJCharacterPlayer`에서 빙의 할 때, `UPawnData`(데이터 에셋)을 사용하여 [태그-어빌리티] 초기화 및 등록.
- **[구조도]** (메모 참고)
<img src="https://github.com/thesun007/metal-slug/assets/39186061/23a056bf-ded7-46c2-ae46-93dd94636f79">

- 빙의 할 때, `ADJCharacterPlayer`에서 어빌리티를 등록하는 `UDJAbilitySet`의 함수 호출 코드 샘플.
<img src="https://github.com/thesun007/metal-slug/assets/39186061/e926d8c7-2041-4b2e-a76e-33bfd70c937c">

- `UDJAbilitySet`이 가지고있는 어빌리티 데이터에 따라 **어빌리티 스팩**에 **[태그와 소스오브젝트]**를 지정하고 ASC에 등록
<img src="https://github.com/thesun007/metal-slug/assets/39186061/ed85f3a6-b1f6-4123-85d6-5fc60d9c83a6">

<br/><br/>

### 태그-인풋 시스템
[인풋액션-인풋ID-어빌리티] 방식 대신에 라일라 프로젝트 방식인 <ins>[인풋액션-게임플레이 태그-어빌리티]</ins>를 적용.
- `UDJInputComponent`(향상된 인풋 컴포넌트 확장) : 태그와 함께 인풋액션을 바인딩하는 기능 추가.
- `ADJCharacterPlayer`에서 "SetupPlayerInputComponent(...)" 가 진행될 때, `UDJInputData`(데이터 에셋)을 사용하여 [일반/어빌리티] 인풋액션 바인딩.
- **[구조도]** (메모 참고)
<img src="https://github.com/thesun007/metal-slug/assets/39186061/aae61132-d812-4f9b-8d50-2dd988186c29">
<br/>

- "SetupPlayerInputComponent(...)" 에서 `UDJInputComponent`의 바인딩 함수 호출 코드 샘플.
<img src="https://github.com/thesun007/metal-slug/assets/39186061/dba4b893-a537-4f20-b38e-77065f26486d">

<br/><br/>

### 풀바디 링크된 레이어 애니메이션 시스템
@<ins>애니메이션 블루프린트 링크 시스템</ins>을 적용한 라일라 애니메이션을 분석하고 이 기반으로 **C++과 블루프린트가 혼합된 애니메이션 시스템** 제작.  
@**게임플레이 태그**와 **애님 인스턴스의 변수**를 매칭하여 태그 상태에 따라 변수가 변경되는 UE기능 (`FGameplayTagBlueprintPropertyMap`) 활용.  
@외부에서 계산되어 setter가 필요한 변수를 위한 수단으로 **setter 전용 인터페이스(`IAnimSourceSetInterface`)**를 추가. (의존성 해소 목적)  
@현재 UnArmed 레이어 애니메이션만 구현.  
<img src="https://github.com/thesun007/metal-slug/assets/39186061/7a8d4b97-17e1-4d15-b84b-2a6c6233ade2">

- 애니메이션 관련 데이터를 계산하는 로직은 대부분 `UDJAnimInstance`(메인 애님인스턴스) 에서 C++(NativeThreadSafeUpdateAnimation())로 작성.
- `UDJAnimInstance` : 위치/속도/방향/상태/회전 등 각종 데이터 계산.
<br/>

- 애님 레이어 인터페이스를 구현하는 `UDJLayerBaseAnimInstance`는 블루프린트로 로직 작성.
- `UDJLayerBaseAnimInstance` : <Linked Anim Layer 구현>. 현 상태에서 실행할 애니메이션르 관련 변수 setter만 있음.
- **[구조도]** (메모 참고)
<img src="https://github.com/thesun007/metal-slug/assets/39186061/3b9c6a90-cbef-44f0-a0ad-b4d8ed540fce">

<br/><br/>

### 메시지 라우터 플러그인 
연결되지 않은 게임플레이 오브젝트가 서로 통신할 수 있도록 하는 게임인스턴스 서브시스템 (`GameplayMessageSubsystem`).
- 채널(태그)와 구조체로 데이터 통신을 진행하며 C++, 블루프린트 둘 다 활용 가능.


### 추가 코스트 시스템

### 게임플레이 태그
c++에서 사용할 태그를 미리 준비.
- 태그 정의 모음 파일
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/5d2861a3-08e9-4cef-a4e1-203f57173177">

- 또는 태그가 필요한 파일에 태그를 즉석으로 매크로 정의 (예)
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/4cce872c-3f46-4049-9d98-7fa7ef22a792">

<br/><br/>

--- 

## 2. 기본 이동 모션
<ins>기본적인 움직임은 **라일라 프로젝트 애니메이션** 방식을 참고하여 그대로 따랐으므로 매우 유사함.</ins>  
라일라 프로젝트에서 자연스러운 움직임과 효율적인 개발 환경을 위해 적용한 기술들을 하나씩 분석하여 직접 사용해보며 체득함.  
<**메인 애님 블루프린트**의 애님 그래프>
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/ecf0cac7-a037-48c0-b6d5-1df53462c7a5">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/0f61e14d-0919-4ac0-8981-0f767b0eb895">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/ba5c4487-3e91-4a89-abd6-817256eb6143">

- 트랜지션 룰 또는 디스턴스 매칭, 포즈 와핑 등 애니메이션 구현을 위해 계산해야 할 데이터는 직접 C++로 나름의 수학적 계산을 통해 제작.
  - 대표적으로 <ins>**현재 움직임 상태 / 방향 / YawOffset**</ins>(root에 적용할 캐릭터 Yaw회전 반대 각도, 즉 액터가 회전해도 캐릭터는 현 방향 유지)
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/8c9dc1f9-0063-4489-99fc-3a8b34dfbb0f">

- 계산 데이터에 따라 애님 그래프의 트랜지션 룰을 구성하고 실험을 통해 자연스러운 블랜딩 유형(standard/inertialization)과 그래프, 수치를 조절.
- 정확한 트랜지션 조건 적용을 위해 <ins>프레임당 1개로 제한</ins>, 또한 몽타주 ending 과 로코모션의 자연스러운 전환을 위해 <ins>재초기화 비활성</ins>.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/712e3a13-a793-4234-8f0d-537c6ba3bed8">

- 애니메이션 시퀀스 에셋에는 디스턴스 매칭을 위한 Curve가 준비되어 있음.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/309227af-78f8-416f-b67a-c8113308e3bf">

<br/><br/>

### Idle/Crouch/Turn
기본적으로 <ins>**메인 애님 블루프린트**</ins>에서는 링크된 애님 레이어 노드를 하고 필요한 계산을 수행한다.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/ee7a2f69-94aa-42b3-b77a-3e2e61ea3a84">
- Idle state일 때 필요한 계산은 대표적으로 turn을 한다면 Curve값을 이용하여 진행한 회전 정도에 따라 root rotate에 영향을 미치는 YawOffset을 수정함.
<br/>

<ins>링크할 애님 레이어를 구현하는 블루프린트</ins>에서는 애니메이션 시퀀스를 선택하고 재생을 조절하는 실질적인 애니메이션 기능을 수행한다.  
<Turn 샘플>  
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/f2c9893e-0e4f-4fb7-a087-2eba2e916ebd">

- Setup Turn Data (회전할 방향을 구함)
- On Update Turn ( 계산된 회전 방향에 따라 시퀀스를 선택하고 시간의 흐름대로 재생. 시간 흐름 값은 연속 회전 기능에 활용)
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/d2914159-16ea-4b03-a013-a6120263c2b0">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/93c691f4-138d-4bf7-95af-0f01205e186e" width="200px" height="210px">
</p>

<br/><br/>

### Start
<메인 애님 블루프린트> 이동 방향에 따른 기울기 변경 기능 적용.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/31360653-0f49-42f1-8fde-6255e48057fb">
<br/>

<레이어 애님 블루프린트> **이동한 거리**에 따라 재생시간을 결정하는 <ins>디스턴스 매칭 노드</ins>와 <ins>회전/거리 와핑 노드</ins>를 사용함.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/66d75653-4966-4785-9221-1a8c07a68965">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/2c6bf8e2-c8b8-4952-8f05-7b137102f698">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/39395856-e17d-49b2-9b07-abb50efaa9ef" width="200px" height="210px">
</p>

<br/><br/>

### Cycle
기울기 적용, 현재 **이동 속도**에 따라 재생속도를 조절하는 디스턴스 매칭 노드와 회전/거리 와핑 노드 사용.
매 프레임 이동 방향을 확인하여 애니메이션 결정.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/6740cd9b-77b0-4edc-82b5-6b8a589146e5">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/cd27ee8c-c08c-4faf-866d-8ce4954f0722" width="200px" height="210px">
</p>

<br/><br/>

### Stop
정지 예상 거리를 계산해주는 노드를 사용하여 남은 거리에 따라 재생시간을 결정하는 디스턴스 매칭 노드 적용.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/89e5b5f3-dcb3-4049-a872-b6d5b4b730b3">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/a4c8d712-0c7c-4658-bc3b-03d94515c66f">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/28bb28a7-43fe-44e5-a412-d39c98a2a1ad" width="200px" height="210px">
</p>

<br/><br/>

### Pivot
<레이어 애님 블루프린트> 연속 Pivot 을 위해 이중 대칭으로 설정.  
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/60a33d60-54f9-41a2-898e-f34ba0913410">

- Pivot 모션 진행 중 방향 전환 전, 속도 0 되기 까지 Distance Match to Target 노드 사용.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/7dbd376f-5ea8-4049-8f1b-5e1d6bfb850d">

- Pivot 모션 진행 중 방향 전환 후, 이동한 거리로 Advance Time by Distance Matching 노드 사용.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/9d32b8e5-4c0f-4e1a-a5c8-5ba422aeb7d2">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/21d5bb7a-0022-4280-bcee-df06727cba95" width="200px" height="210px">
</p>

<br/><br/>

### Jump/Fall/Landing
정확히는 모션이 점프시작/점프 중/점프 최고점/낙하 중/랜딩 으로 나뉘어져 있다.
- 일반적인 점프 시 모든 과정을 거치며, 갑작스런 낙하는 점프 최고점 부터 시작한다.
- 랜딩 과정에는 땅과의 거리를 적용하여 땅에 닿기까지 모션 재생을 조절한다.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/c94f2c23-64fb-4c52-be6c-766d0fe7598a">
<br/><br/>

- 랜딩 시 충격 모션을 추가하는 Additive 레이어가 메인 애님 그래프에 준비되어 있다.
- 공중에 있던 시간에 비례해서 Additive 강도를 조절한다.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/1fd1f845-5606-49d0-b909-efbbe58933ab">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/cab9e6b7-7df3-4937-be95-5231e74ccd8e" width="200px" height="210px">
</p>
<br/>

- 랜딩 시, 이동 상태에 따라 움직임을 연결.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/bb328d8a-85a9-434d-9159-b3c6bdd8f10a">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/4158d971-b538-417f-ab68-82b820b1afe4" width="240px" height="260px">
</p>

---

## 3. 파쿠르
게임플레이 어빌리티로 파쿠르 기능들을 제작.
<br/>

### Dash
> Dash는 라일라 프로젝트와 거의 동일. c++로 재구성하면서 어빌리티 활용방법 파악.

- **PlayMontageAndWait** 태스크와 **ApplyRootMotionConstantForce** 태스크를 이용.
- 멀티플레이 호완 포함.
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/830857b7-ca08-40dc-acea-fa77556d55db" width="250px" height="230px">
</p>
<br/><br/>

### Mantling/Vaulting
1. <ins>**모션 와핑**</ins> 을 활용하여 파쿠르 중 위치해야할 지점을 적용.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/03a1f10b-1ba5-4b04-bc8a-e38a2a2a1171">

3. 장애물 높이와 깊이에 따라 상황에 맞는 모션이 발생하도록 함.   **`(DJGA_Parkour.h)`**
> 시스템  계산상 구분을 해놓았지만 모션 종류 부족으로 "낮음"과 "중간" 모션이 동일함.

|높이 \ 깊이|얕음|중간|깊음|
|------|---|---|---|
|낮음|짧은 넘기|디딛고 넘기|낮게 오르기|
|중간|짧은 넘기|디딛고 넘기|낮게 오르기|
|높음|높게 오르기|높게 오르기|높게 오르기|
<br/>

<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/219e070e-1a98-4733-a3ad-39acb4e8e6a3" width="250px" height="230px">
</p>
<br/>

- "점프" 중에 `(DJGA_Parkour.h)`어빌리티 사용 시, 모서리에 부딪 몸의 높이에 따라서 높게 오르기 모션 시작 지점을 조절하여 오르는 행동 구현.
- 이를 위해 시퀀스 에셋에서 root 기준 발의 높이를 구한 커브 데이터를 뽑음.  
**`(DistanceBoneModifier.h)`** ( 기존 엔진의 DistanceCurveModifier 를 개량)
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/269c39af-8b64-4199-82cd-d4d2483612fb">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/deb087a3-b272-4744-a8b8-dfb77a89aaf6">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/b0ef5a48-19af-40ba-ab8c-d3a379d76b86" width="250px" height="280px">
</p>
<br/><br/>

### WallRun
"점프" 중에 `(DJGA_Parkour.h)`어빌리티 사용 시, <ins>특정 조건하에 Mantling 대신 WallRun 실행.</ins>
- **`조건`** : <옆 사선 벽><이동 방향과 이동 입력 방향 일치><속도, 현재 상승 속도 일정 이상 조건>
- 속도에 따라 보폭 조절을 위해 몽타주가 아닌 링크 레이어 애니메이션으로 모션 실행. <int>(디스턴스 매칭과 Stride Warping 사용)</ins>
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/09b3613a-b67a-4692-8e54-0b85f096c4a3">

<br/>

- 실질적인 액터 이동/회전과 진행 중 필요한 계산들은 <ins>**WallRun 태스크(`DJAT_WallRun.h`)**</ins> 에서 수행.
- 애님 인스턴스에서 WallRun에 필요한 변수를 얻기 위해 Setter인터페이스를 적용하고, **WallRun 태스크**에서 필요 값을 계산하고 넘겨 줌.
> WallRun 시작 시, 현재 속도와 수직 상승 속도를 고려하여 이동한다.  
> 바닥과 가까워지면 발을 디딛는다. ( WallRun 초기 상승 중일 땐 바닥이 있어도 무시한다.)
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/e6772abc-469a-4884-b404-1a58292ffa1b" width="260px" height="240px">
</p>

> 가파르게 떨어지기 시작하면 WallRun을 멈추고 자유낙하한다.  
> 곡선 벽을 탈 수 있다. (역 곡선은 불가)
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/9128ec7f-051e-4093-b097-362f9e6ccc6e" width="290px" height="250px">
</p>
<br/><br/>

### Sliding
달리는 중 일정 속도 이상에서 "앉기"를 누르면 슬라이딩 어빌리티 발동. **`DJGA_Sliding.h`**
- 발 부분 장애물 체크를 위해 <ins>Tick 기능과 위치 offset 설정 기능</ins>을 더한 TargetActor_Radius 확장 버전을 제작. (`TargetActor_TickableRadius.h`)
- (지면 각도에 따른 액터회전, 취소 감시, 낙하 감지 등) 어빌리티에 Tick 기능 지원을 위해 델리게이트로 콜백함수를 받아 대신 Tick 실행해주는 어빌리티 태스크 제작.(`DJAT_TickBind`)
- 현재속도와 오르막/내리막에 따라 초기 속도와 감쇠계수 차등적용.
- 슬라이딩 중 어빌리디 재 발동 시, 모션 취소.
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/0af949a2-b4d2-4f10-ab41-8f0abff7b2e2" width="260px" height="240px">
</p>
<br/>

- 슬라이딩 중 낙하 시, 모션 취소.
- 슬라이딩 중 장애물 충돌 시, 모션 취소.
- 슬라이딩 후 상단 천장 존재할 시, 앉기.
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/53bb5322-eeac-4765-a766-d472ef677652" width="260px" height="240px">
</p>
<br/><br/>

## 4. 암살 모션
1. <ins>"**암살 어빌리티**"</ins>와 <ins>"**암살 피해 어빌리티**"</ins> 로 암살 상호작용 구성.  
2. 게임플레이 어빌리티의 확장 기능인 실행 타입을 `OnSpawn`으로 하여 <ins>**어빌리티가 등록되면 바로 활성화**</ins> 되도록 설정.  
  - **<암살 어빌리티>** 가 활성화 되면, `DJAT_Trigger.h`를 이용하여 매 Tick에 **TargetActorData**를 얻어 암살 가능 대상을 추적한다.
    - `DJAT_Trigger.h` : 기존 "**WaitTargetData 태스크**"에 매 프레임 TargetActor 컨펌을 진행하는 기능을 추가한 <ins>어빌리티 태스크</ins>.
    - (`DJAT_Trigger.h`는 `TargetActor_TickableRadius.h`처럼 TargetActor가 직접 매 프레임 Data를 전송하는 방법 대신, <ins>Task에서 매 프레임 Confirm을 진행</ins>하여 TargetActor로부터 Data 수신.)

<br/>

### 암살 어빌리티 
`DJGA_Assassination.h`  
활성화 중인 이 어빌리티에서 InputPressed 가 발생되면 등록된 암살 몽타주 중 랜덤으로 암살을 진행하며, 타이밍을 맞춰 **암살 피해 어빌리티** 에 관련 정보와 함께 이벤트 전송하여 상대가
암살 피해 모션을 진행하도록 유도한다.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/c0547750-2e65-4d93-8e69-659851582364">

- `Type` : 몽타주가 실행될 수 있는 상대 위치에 대한 정보.
- `Sync Position` : 몽타주 실행 시 최종적으로 위치해야할 피해 대상 액터 기준 <ins>상대 좌표</ins>.

<br/>

### 암살 피해 어빌리티 
`DJGA_AssassinationVictim.h`  
암살 어빌리티에서 이벤트를 받으면 실행한 암살 종류에 따라 맞는 모션을 진행하고 랙돌 시뮬레이션을 활성화하여 죽는 설정을 한다.  
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/5b62e304-7da6-451f-a64b-176b87246045">
<p align="center">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/28346411-f7e5-40da-85e5-b12f8afb56e0" width="260px" height="240px">
</p>

## 5. 기타
