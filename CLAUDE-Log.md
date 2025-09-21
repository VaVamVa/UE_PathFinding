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

1. "`요약` 및 `/compact`" 가 수행되면 최근 Start Log 또는 이전 Compact Log 시점부터 현재까지의 작업을 요약
    - **중요**: Start Log가 작성된 파일(예: AgentLog/250921.md)에 새로운 Compact Log를 추가
    - 만약 날짜가 바뀌어도 현재 진행 중인 Start Log가 있는 파일에 계속 기록
    - 제목과 현재 시각을 log 시작지점에 기입

2. 만약 Local Diff 존재 시 Commit
    - Commit Message 작성 규칙은 [Git](./CLAUDE-Git.md)을 따름

3. 요약 완료 후 Claude Code의 `/compact` 명령 실행하여 컨텍스트 압축

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

1. 최근 Start Log 또는 이전 Compact Log 시점부터 현재까지의 일을 요약하여 해당 파일에 End Log 추가
    - **중요**: Start Log가 작성된 파일(예: AgentLog/250921.md)에 End Log를 추가
    - 만약 날짜가 바뀌어도 현재 진행 중인 Start Log가 있는 파일에 계속 기록
    - 제목과 현재 시각을 log 시작지점에 기입

2. 만약 Local Unstaged 변경 사항 존재 시 Commit
    - Commit Message 작성 규칙은 [Git](./CLAUDE-Git.md)을 따름

3. 요약 완료 후 Claude Code의 `/compact` 명령 실행하여 컨텍스트 압축

4. 모든 작업 완료 후 Session 종료

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
