# Log 

작성 시간이 오름차순이 되게 위부터 아래로 작성. (시작이 맨 위)

## Agent 시작

1. 직전 Commit의 Commit Message 및 Comment를 알려줌.
2. 현재 시점에서 해야할 ToDo 리스트를 알려줌.
    - ToDo List는 지금 당장 해야할 일, 크지 않은 범주로 한정함.
    - ToDo를 추가로 요청했을 때에도, 다음 step 한정으로 작은 범주로 할 일을 나눠서 알려줌.
3. 위의 내용을 `[GitRoot]/AgentLog/YYMMDD.md`에 추가
    - `YYMMDD`는 현재 시스템 날짜 ( UTC+09:00 )
    - 만약 디렉토리 및 파일이 존재하지 않으면, 생성
        - 새로운 파일 최상단엔 `# Workflow` 기입 후 개행
        - 이미 존재할 경우, 해당 파일 맨 아래에 기입
    - 제목과 현재 시간을 log 시작지점에 기입

4. 예시
    ```

    ## Start Log
    - YYYYMMDD hh:mm

    ### To Do

    1. "Todo 내용 1" : 
        (필요하다면 아래에 항목 추가 가능)
        1. ~~
            - ~~
    2. "Todo 내용 2" : 
    ...

    ```

## Agent 중간 요약

1. "`요약` 및 `/compact`" 가 수행되면 시작 시점부터 현재 시점까지의 일을 요약
    - 시작 시점을 명령 수행 시점으로 갱신하여, 다음 요약 및 End Log의 요약을 시작할 시점을 현재로 변경
    - `YYMMDD`는 현재 시스템 날짜 ( UTC+09:00 ).
        - 만약 시작 날짜로부터 날짜가 변경되었어도, 현재 진행 중인 `## Start Log`를 기준으로 파일에 작성
    - 제목과 현재 시각을 log 시작지점에 기입

2. 만약 Local Diff 존재 시 Commit
    - Commit Message 작성 규칙은 [Git](./CLAUDE-Git.md)을 따름

3. 요약 완료 후 `compact` 되지 않았다면, `compact` 진행

4. 예시
    ```
    ## Compact Log N (N은 1부터 증가하는 수. Start 이후로 Compact 한 만큼.)
    - YYYYMMDD hh:mm 

    ### 요약 내용
    
    "...Content..."

    ### Commit 정보
    "Summary 원문"

    1. "Summary 범주 1"
        - "Summary 범주 1의 Description"
    2. "Summary 범주 2"
        - "Summary 범주 2의 Description"
    ...

    ```

## Agent 종료

1. Start (및 compact) 시점부터 현재 시점까지의 일을 요약(`/compact`)하여 `[GitRoot]/AgentLog/YYMMDD.md`에 추가
    - `YYMMDD`는 현재 시스템 날짜 ( UTC+09:00 ).
        - 만약 시작 날짜로부터 날짜가 변경되었어도, 현재 진행 중인 `## Start Log`를 기준으로 파일에 작성
    - 제목과 현재 시각을 log 시작지점에 기입

2. 만약 Local Unstaged 변경 사항 존재 시 Commit
    - Commit Message 작성 규칙은 [Git](./CLAUDE-Git.md)을 따름

3. 요약 완료 후 `compact` 되지 않았다면, `compact` 진행

4. 모두 완료 후 Session 종료

5. 예시
    ```
    ## End Log
    - YYYYMMDD hh:mm

    ### 요약 내용

    "...Content..."

    ### Commit 정보
    "Summary 원문"

    1. "Summary 범주 1"
        - "Summary 범주 1의 Description"
    2. "Summary 범주 2"
        - "Summary 범주 2의 Description"
    ...


    ```
