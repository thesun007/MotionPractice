# 언리얼 플레이어 캐릭터 액션 제작
UE5 어빌리티 시스템 개발 방식 체득 목적
> 사전학습
- UE 용 c++ 기초
- UE 기본 프레임워크
- UE 멀티플레이 프레임워크
- UE 어빌리티 시스템

<p align="center">
  <br>
  <img src="https://github.com/thesun007/metal-slug/assets/39186061/9b89058a-8300-4762-8acb-660e8dd48649" width="500px" height="800px">
  <br>
</p>
- 게임 수학
--- 
### 목차
1. [프레임워크](#1.-프레임워크)
2. [기본 이동 모션](#2.-기본-이동-모션)
3. [파쿠르 모션](#3.-)
4. [암살 모션](#4.-)
5. [기타](#5.-)
--- 

<br/>
## 1. 프레임워크
- 라일라 샘플 프로젝트 방식 일부 차용
    - 데이터 에셋
    - 어빌리티 시스템
    - 태그-인풋 시스템
    - 풀바디 레이어 애니메이션 시스템
    - 메시지 라우터 플러그인 
    - 추가 코스트 시스템

### 데이터 에셋
캐릭터에 적용할 
`어빌리티/이펙트/어트리뷰트/인풋 데이터/맵핑 컨텍스트` 데이터 구조
<img src="https://github.com/thesun007/metal-slug/assets/39186061/f890d3db-448c-4c3b-b636-18fc8c170103" width="500px" height="800px">
<img src="https://github.com/thesun007/metal-slug/assets/39186061/5d04f7a2-eeee-40d5-9853-a47edbb91e5c">
