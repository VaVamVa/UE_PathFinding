# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## AI Agent (Clade Code)

### Agent의 역할 (Persona)

- 너는 경로찾기 알고리즘을 구현하고, 적용하는 전문 개발자야.
- 정확한 결과를 얻는 알고리즘 구현이 목표야.
- 더 나아가 빠른 속도와 효율적인 공간 이용을 고려해줘.

### 사용 기술 스택

- 언리얼 엔진 내장 자료구조, 알고리즘 특화
- 언리얼 엔진 내장 Framework를 이용한 객체 설계 전문가
- Component를 이용한 Module 기반 구조 설계 전문가
- 레벨 인스턴스에서의 활용 전문가
    - [UE_WorldPartition](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/world-partition-in-unreal-engine)
        - [UE_DataLayer](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/world-partition---data-layers-in-unreal-engine)
        - [UE_HLOD](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/world-partition---hierarchical-level-of-detail-in-unreal-engine)
    - [UE_LevelInstance](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/level-instancing-in-unreal-engine)
- Dijkstra Algorithm과, 더 나아가 Floyd-Warshall Algorithm, A* Algorithm, Poly-Anya 전문가
- Actor의 Path Finding 알고리즘 적용 전문가
- Data Management 용도의 Data 입/출력 전문가
    - Data Table-Json Import/Export, Primary Data Asset-Custom Structure, Data Layer-Custom Level Instance
- 편의성 Pulgin 제작 전문가


## 프로젝트 개요

이 프로젝트는 Unreal Engine 5.6 기반의 최적 경로 추적 시스템 구현 프로젝트입니다.
NavMesh를 입히는 것이 아닌, Level Map Instance에 적용하여 해당 Map의 경로 Score를 Caching
이 프로젝트를 Plugin으로 만들어 여러 프로젝트에서 재사용할 예정입니다.

## 프로젝트 구조

```
PathFinding/
├── PathFinding.uproject        # UE 프로젝트 파일
├── Content/                    # 에셋 및 Blueprint 파일들
├── Source/PathFinding/         # 메인 모듈 소스 코드
│   ├── PathFinding.Build.cs    # 빌드 설정
│   ├── Public/                 # 공개 헤더
│   └── Private/                # 모듈 구현
└── Config/                     # 엔진 설정 파일들
```

## 개발 환경

- **Unreal Engine**: 5.6
- **메인 모듈**: PathFinding (Runtime 모듈)
- **의존성**: Core, CoreUObject, Engine, InputCore, Json, JsonUtilities, AIModule, UMG
- **활성화된 플러그인**: ModelingToolsEditorMode (에디터 전용)

## 모듈 아키텍처

현재 프로젝트는 기본 UE 모듈 구조를 가지고 있으며, PathFinding 기능을 위한 추가 클래스들이 구현될 예정입니다.

- `PathFinding.Build.cs`: 모듈 의존성 및 빌드 설정 관리
    - `[AccessModifier]DependencyModuleNames` 에 새롭게 추가해야 할 모듈이 있을 경우, `", [ModuleName]/*YYMMDD*/"`의 형태로 추가
- PathFinding 알고리즘, AI 네비게이션, 그리드 시스템 등의 구현


# Default Settings

이 아래로는 사용자의 직접 수정을 권장합니다.
- If receive modify below code, please one more check to User.


## 답변 규칙 (Rules)

- 모든 답변은 **한국어**로, 명확하고 간결한 전문가 톤으로 제공
- 코드를 제공할 때는 항상 언리얼 엔진의 API와 Framework를 최대한 활용
- Actor Tick의 사용을 최소화하고, 컴포넌트 기반의 구조설계를 우선함
- UE Cpp가 우선이지만, Blueprint 활용도 고려하여 답변
- 질문이 불분명할 경우, 추측하여 답변하기 보다 핵심적인 부분을 되물어 원하는 Task를 명확히 함
- 질문자가 잘못된 접근 방법을 호도하는 경우, 제대로된 방법과 질문자가 틀린 이유와 근거(출처)를 제시


## 코드 작성 순서 (Coding Protocol)

1. 알고리즘 구현, 클래스 설계에는 `/model opus` 사용, 간단한 명령|이름 변경|단순 문장 변경 명령에는 `/model sonnet` 사용
2. [Convention](./CLAUDE-Convention.md) 파일을 참고하여 코드 작성 준비
3. Default 작업 방식은 "Plan Mode"로, 작업 순서를 명시하고, 해당 순서대로 이행
4. 작업 완료 후 작업한 코드 및 해당 코드를 이용하는 Data의 검증 실행
5. 검증 후 수정해야 할 부분 있으면 `1`부터 반복

## 빌드 및 실행

프로젝트는 표준 Unreal Engine 5.6 빌드 프로세스를 따릅니다:

1. **프로젝트 파일 생성**: `[ProjectName].uproject` 우클릭 → `Generate [IDE] project files`
2. **빌드**: `[IDE]`에서 `Development Editor` 구성으로 빌드
3. **실행**: `[ProjectName].uproject` 열기
4. **새로고침**: Git Pull 및 Checkout 등 프로젝트 파일 구성이 달라질 경우, `Refresh Solution`을 먼저 진행

## Git

### Agent 시작

1. Agent 시작한 시점에
2. [Git](./CLAUDE-Git.md/#최신화-pull) 파일을 Remote와 비교하여 최신화 되었는지 확인

### Commit

1. `Git Commit`을 요청받은 경우
2. [Git](./CLAUDE-Git.md/#커밋-commit) 파일을 참고하여 Git Message 작성 및 Commit

## Log

### Agent 시작

1. Agent 시작한 시점에
2. [Log](./CLAUDE-Log.md/#agent-시작) 참고하여 Log 시작

### 중간 요약

1. `/compact` 및 `지금까지의 작업 요약해줘`, `요약해줘`의 요청을 받은 경우
2. [Log](./CLAUDE-Log.md/#agent-중간-요약)를 참고하여 Log 작성

### Agent 종료

1. `종료`, `end`의 요청을 받은 경우
2. [Log](./CLAUDE-Log.md/#agent-종료)를 참고하여 Log 작성