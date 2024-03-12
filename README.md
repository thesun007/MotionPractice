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

- 게임 수학

--- 
### 목차
1. [프레임워크](#1.-프레임워크)
2. [기본 이동 모션](#2.-기본-이동-모션)
3. [파쿠르](#3.-파쿠르)
4. [암살 모션](#4.-암살-모션)
5. [기타](#5.-기타)
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
- `UDJAbilitySet` : **[태그-어빌리티]** 데이터 컨테이너, **[게임플레이이펙트]** 컨테이너, **[어트리뷰트]** 컨테이너
  - "게임플레이 어빌리티 스팩"을 생성하여 태그를 지정하고, ASC에 등록하는 함수 보유. (GiveToAbilitySystem(..))
- `UDJInputData` : **[태그-인풋액션]** 컨테이너
<img src="https://github.com/thesun007/metal-slug/assets/39186061/f890d3db-448c-4c3b-b636-18fc8c170103">
<img src="https://github.com/thesun007/metal-slug/assets/39186061/5d04f7a2-eeee-40d5-9853-a47edbb91e5c">
<img src="https://github.com/thesun007/metal-slug/assets/39186061/0d6e4c45-0f43-4eb1-97b6-145c33867a2d">

<br/><br/>

### 어빌리티 시스템
라일라 프로젝트에서 게임 피처를 제외한 어빌리티 시스템 적용 방식을 분석하여 본 프로젝트에 활용.
- `UDJAbilitySystemComponent`(어빌리티 시스템 컴포넌트 확장) : 입력된 태그와 매칭된 어빌리티를 찾고 해당 어빌리티의 작동 방식에 따라 처리하는 기능 추가.
- `UDJGameplayAbility`(게임플레이 어빌리티 확장) : 어빌리티 실행 작동 방식(입력 시/입력 동안에 계속/스폰 시) 추가.
- `ADJCharacterPlayer`에서 빙의 할 때, `UPawnData`(데이터 에셋)을 사용하여 [태그-어빌리티] 초기화 및 등록.
- [구조도] (메모 참고)
<img src="https://github.com/thesun007/metal-slug/assets/39186061/23a056bf-ded7-46c2-ae46-93dd94636f79">

- 빙의 할 때, `ADJCharacterPlayer`에서 어빌리티를 등록하는 `UDJAbilitySet`의 함수 호출
<img src="https://github.com/thesun007/metal-slug/assets/39186061/e926d8c7-2041-4b2e-a76e-33bfd70c937c">

- `UDJAbilitySet`이 가지고있는 어빌리티 데이터에 따라 태그와 소스오브젝트를 지정하고 ASC에 등록
<img src="https://github.com/thesun007/metal-slug/assets/39186061/ed85f3a6-b1f6-4123-85d6-5fc60d9c83a6">

<br/><br/>

### 태그-인풋 시스템
[인풋액션-인풋ID-어빌리티] 방식 대신에 라일라 프로젝트 방식인 <ins>[인풋액션-게임플레이 태그-어빌리티]</ins>를 적용.

- `UDJInputComponent`(향상된 인풋 컴포넌트 확장) : 태그와 함께 인풋액션을 바인딩하는 기능 추가.
- [구조도] (메모 참고)
<img src="https://github.com/thesun007/metal-slug/assets/39186061/aae61132-d812-4f9b-8d50-2dd988186c29">

<br/><br/>

### 풀바디 링크된 레이어 애니메이션 시스템
애니메이션 블루프린트 링크 시스템을 적용한 라일라 애니메이션을 분석하고 이 기반으로 C++과 블루프린트가 혼합된 애니메이션 시스템 제작.

## 2. 기본 이동 모션
## 3. 파쿠르
## 4. 암살 모션
## 5. 기타
