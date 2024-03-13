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
- 활용 사례 아직 없음. ( Dash 어빌리티에서 쿨타임 데이터 메시지 통신 고려 중)


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
기본적인 움직임은 라일라 프로젝트 애니메이션 방식을 참고하여 그대로 따랐으므로 매우 유사함.  
라일라 프로젝트에서 자연스러운 움직임과 효율적인 개발 환경을 위해 적용한 기술들을 하나씩 분석하여 직접 사용해보며 체득함.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/ecf0cac7-a037-48c0-b6d5-1df53462c7a5">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/0f61e14d-0919-4ac0-8981-0f767b0eb895">
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/ba5c4487-3e91-4a89-abd6-817256eb6143">

- 트랜지션 룰 또는 디스턴스 매칭, 포즈 와핑 등 애니메이션 구현을 위해 계산해야 할 데이터는 직접 C++로 나름의 수학적 계산을 통해 제작.
- 계산 데이터에 따라 트랜지션 룰을 구성하고 실험을 통해 자연스러운 블랜딩 유형(standard/inertialization)과 그래프, 수치를 조절.
<img src="https://github.com/thesun007/MotionPractice/assets/39186061/973d7158-a32e-4a67-b80d-21ff7513b345">

<br/><br/>

### Idle/Crouch/Turn

## 3. 파쿠르
## 4. 암살 모션
## 5. 기타
