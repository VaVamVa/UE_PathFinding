# PathFinding Plugin Test Project

## 📋 개요

이 프로젝트는 **PathFindingPlugin**의 기능을 테스트하고 검증하기 위한 테스트 환경입니다.

Zone 기반 경로탐색 시스템의 모든 기능을 실제 게임 환경에서 테스트할 수 있도록 구성되어 있습니다.

## 🚀 프로젝트 구성

### Plugin 통합
- **PathFindingPlugin**: `Plugins/PathFindingPlugin/` 에 위치
- 자동으로 활성화되어 프로젝트 실행 시 로드됩니다

### 테스트 클래스
1. **APathFindingTestGameMode**: Plugin 테스트를 위한 Game Mode
   - ZoneManager 자동 생성 및 관리
   - 플레이어 위치 자동 추적
   - Zone 자동 로딩/언로딩 시스템

2. **APathFindingTestPlayerController**: 테스트 상호작용 컨트롤러
   - 디버그 시각화 토글
   - Zone 정보 출력
   - 수동 Zone 로딩 기능

## 🎮 테스트 조작법

### 키보드 단축키
- **F1**: 디버그 시각화 토글 (Zone 경계, 상태 정보 표시)
- **F2**: 현재 Zone 정보 출력 (화면 및 로그)

### 디버그 정보
디버그 모드 활성화 시 다음 정보가 표시됩니다:
- Zone 경계 박스 (색상으로 로딩 상태 구분)
- Zone 연결점 (파란색 구체와 연결선)
- Zone 정보 텍스트 (이름, 상태, 캐시 상태)

## 🔧 설정 방법

### 1. 프로젝트 빌드
```bash
# Visual Studio 프로젝트 파일 생성
우클릭 PathFinding.uproject → "Generate Visual Studio project files"

# Visual Studio에서 빌드
Development Editor 구성으로 빌드
```

### 2. Zone 데이터 설정
1. **DataTable 생성**:
   - Content Browser에서 우클릭
   - Miscellaneous > Data Table 선택
   - Row Structure: `ZoneLevelData` 선택

2. **Zone 데이터 입력**:
   - Zone 식별자 (이름, 타입, ID)
   - Level Asset 경로
   - Zone 경계 설정
   - 연결점 정의

3. **Game Mode 설정**:
   - World Settings에서 Game Mode를 `PathFindingTestGameMode`로 설정
   - Test Zone Data Table 참조 설정

## 📊 테스트 시나리오

### 기본 기능 테스트
1. **Zone 로딩**: 플레이어가 Zone 근처로 이동 시 자동 로딩 확인
2. **Zone 언로딩**: 플레이어가 멀어질 때 자동 언로딩 확인
3. **Path Cache**: Zone 로딩 완료 후 경로 캐시 생성 확인
4. **Cross-Zone**: 여러 Zone 간 이동 시 연결점 작동 확인

### 고급 기능 테스트
1. **동시 로딩**: 여러 Zone 동시 로딩 제한 확인
2. **캐시 저장**: Zone 언로딩 시 캐시 파일 저장 확인
3. **성능 모니터링**: 대량 Zone 환경에서 성능 측정

## 📁 파일 구조

```
PathFinding/
├── Plugins/
│   └── PathFindingPlugin/          # PathFinding Plugin
├── Source/PathFinding/
│   ├── Public/
│   │   ├── PathFindingTestGameMode.h
│   │   └── PathFindingTestPlayerController.h
│   └── Private/
│       ├── PathFindingTestGameMode.cpp
│       └── PathFindingTestPlayerController.cpp
├── Content/                        # 게임 에셋 (TPS 캐릭터 등)
├── Config/
│   └── DefaultInput.ini           # F1, F2 키 바인딩
└── README.md                      # 이 파일
```

## 🐛 디버깅

### 로그 확인
- **Output Log**: Zone 로딩/언로딩 상태 확인
- **On-Screen Messages**: 실시간 Zone 정보 확인

### 캐시 파일 위치
- `Saved/PathFindingCache/`: Zone별 경로 캐시 JSON 파일

### 일반적인 문제 해결
1. **Zone이 로딩되지 않음**: DataTable 설정 및 Level Asset 경로 확인
2. **캐시가 생성되지 않음**: Zone 로딩 완료 후 지형 충돌 체크 확인
3. **연결점이 작동하지 않음**: Zone 간 Connection Point 설정 확인

## 📝 참고사항

- TPS 캐릭터는 별도로 설정해주세요
- 테스트 레벨은 World Partition 사용을 권장합니다
- Zone별 Level Instance 파일을 미리 준비해주세요