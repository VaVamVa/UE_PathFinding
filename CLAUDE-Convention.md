# Code Convention 코드 컨벤션 

## Format 포멧

1. Indent 공백은 4칸. 가능하다면 `Tab`으로
2. 함수명 선언 위에는 주석 기입, 클래스에는 핵심 public features 및 (필요하다면) protected: virtual features에 대해 주석 기입
3. 주석에는 기능, 필요한 매개변수에 대해 기입하고, 필요하다면 Template과 반환 형태를 간략하게 명시할 것.
4. 주석 Format Style은 `Doxygen`의 형태와 유사하게 구성
    - (ex)
    ```
    /**
    * @Breif 요약된 기능 설명
    * @Template      : TemplateName  - 간략한 설명 
    * @ParameterName  : ParameterType  - 간략한 설명
    ...
    * @Return        : ReturnType    - 간략한 설명 // 존재한다면 기입.
    */
    ReturnType FunctionName (ParameterType ParameterName);
    ```
    - [API_Tip](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/exposing-cplusplus-to-blueprints-visual-scripting-in-unreal-engine)
    - 주석의 column 정렬은 제일 긴 줄을 기준으로 2칸 띄우기
5. 함수에서 Value Type 반환으로 성공 여부를 판단해야 한다면, `boolean`을 return하고 참조 변수 Parameter로 전달하게 Argument 구성
6. 만약 코드를 수정하거나 리팩토링 해야 한다면, `/*modify_YYMMDD_: ~~의 이유로 수정*/` 으로 간략하게 설명을 붙여서 수정.
    - 새로 추가되는 설명이 가장 아래에 오게 기입.
    - `YYMMDD`는 현재 시스템 날짜 ( UTC+09:00 ).

## Declare 선언 

1. Convention Case 정의
    1. 파스칼 케이스 (PascalCase) : PrefixNameSuffix
    2. 카멜 케이스 (camelCase) : prefixNameSuffix
    3. 스네이크 케이스 (snake_case) : prefix_name_suffix
    4. 스크리밍 스네이크 케이스 (SCREAMING_SNAKE_CASE) : 스크리밍 스네이크 케이스

2. 명명 예시 형태 : `Prefix[Name]Suffix`
    - (ex)`U[Class]For` == `USomethingNameFor`
    - (ex)`Type [var]` == `UMyClassFor* somethingVariableName;`

3. 지역 선언(In-Scope) | 멤버 선언(Caching In-Object) 기준
    - 만약 자주 사용된다면, member로 Caching
        - 3개 이상의 Method에서 활용
        - 자주 호출 및 이용 (약 "6 calls per second" 이상)
    - 만약 계속 재정의 되어야 하는 Data면, 지역변수로 활용
    - UE 자체 Smart Pointer 이용하는 멤버 변수: 가능하다면, `TObjectPtr<Type> [var];` 형태로 선언

4. 선언부 `.h`헤더파일에는 최소한의 `include`를 하고, 필요한 자료형은 전방선언 이용. 구현부 `.cpp`에서 사용되는 header `include`.
    - `include`와 첫 선언 사이에 전방선언 위치
    - `#pragma region Forward_Declare ~ #pragma endregion Forward_Declare` 으로 래핑

5. UE 자체 자료형, Module 적극 이용
    - [UE_QuickStart](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/QuickStart)
    - 만약 cpp 스타일 library가 필요하다면, 다음의 링크 참조 
    [CppCodingForUE](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine?application_version=5.5)

6. Unreal Header Tool(`UHT`) Macro 이용
    - 필요 Case : CDO에 등록되어야 하는 Data | UObject 파생 자료형 | Reflection이 필요(Blueprint에서 이용) | Unreal 자체 GC에 의해 제어되어야 하는 Pointer Data
    - `UCLASS()`, `USTRUCT()`, `UENUM()`, `UFUNCTION()`, `UPROPERTY()`, `UINTERFACE()`
    - `UCLASS`, `USTRUCT`는 `GENERATED_BODY()` 필요
        - 명시적으로 `GENERATED_UCLASS_BODY()`, `GENERATED_USTRUCT_BODY()` 도 사용 가능
    - UHT 매크로 관련 URL
        - [Class_UHT](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/gameplay-classes-in-unreal-engine)
        - [Struct_UHT](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/structs-in-unreal-engine)
        - [Function_UHT](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/ufunctions-in-unreal-engine)
        - [Property_UHT](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/unreal-engine-uproperties)
        - [Interface_UHT](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/interfaces-in-unreal-engine)

    - 각 UHT Specifier가 필요하다면 다음의 URL 참조
        - [UHT_Specifier_API](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/exposing-gameplay-elements-to-blueprints-visual-scripting-in-unreal-engine)
    - 각 UHT/Metadata Specifier가 필요하다면 다음의 URL 참조
        - [MetaData_Specifier_API](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/metadata-specifiers-in-unreal-engine)

7. 가능하다면, 선언부 혹은 생성자에서 Default Value로 초기화
    - `0`, `0.f`, `nullptr`, `Initializer()`, ...


### Naming 명명법 

1. 객체 명명 규칙
    - Pascal Case
    - 클래스: `class U[Name]`, 구조체: `struct F[Name]`, 열거형: `enum class|enum E[Name]`,
    템플릿 객체: `template<typename Type> T[Name]Of`, 인터페이스: `InterfaceType I[Name]`,
    슬레이트 위젯 : `S[Name]`, 네임 스페이스: `namespace N[Name]`
    - 특이사항
        - `UBlueprintFunctionLibrary` 상속 시: `class U[Name]Library : public UBlueprintFunctionLibrary` 

2. 함수 명명 규칙
    - Pascal Case
    - 함수: `ReturnType [Name](...)`, 
    템플릿 함수: `template<typename ReturnType> ReturnType [Name]To(...)`|`template<typename UseType> ReturnType [Name]By(UseType (I|i)n[Arg], ...)`

3. 변수 명명 규칙
    - camel Case
    - 변수: `Type [name]`, bool변수(매개변수 아닐 시): `bool b[Name]`,
    입력 매개변수: `const argumentTye&|argumentTye in[Name]`, 출력 매개변수: `argumentReturnType& out[Name]`

4. 매크로 명명 규칙
    - SCREAMING_SNAKE_CASE
    - `#define MAX_NUM(x, y) `
    - 여러 줄이 필요한 경우 `\` 사용

5. 디렉토리 명명 규칙
    - 폴더/네임스페이스: `[ProjectName]/Source/[Access Modifier]/...`, `[ProjectName]/Content/...`
        - (ex) `Component`, `AI`, `UI`, `Utility`, `Asset`, ...

6. 파일 명명 규칙
    - Pascal Case
    - 이름: 객체 명, 주요 기능 명, 주요 용도 명
    - `.uasset` 파일 Prefix
        - Actor 관련: `BP_`
        - UI 관련
            - UserWidget 기반: `WBP_`, HUD 기반: `HUD_`
        - Data 관련
            - Data Asset 기반: `DA_`, Data Table 기반: `DT_`, Data Layer 기반: `DL_`
        - Art 관련
            - Material 기반: `M_`, Material Instance 기반: `MI_`, Texture : `Tex[Feature]_`
                - `Feature` = Normal Map `N`, Metalic Map : `M`, ...
        - Animation 관련
            - AnimInstance 기반: `ABP_[SkeletonName|FeatureName]`, Aim Offset 기반: `AO_`, Blend Space: `BS_`, 
        - Utility 관련
            - Function Library 기반: `BPFunc_[Name]Library`

### 예시

- (ex) <br>
    ```
    // "Source/Private/MyClass.h"

    #include "CoreMinimal.h"
    #include "Actor.h"
    #include "MyClass.generated.h"

    #pragma region Forward_Declare
    class UStaticMeshComponent;
    class UBoxComponent;
    Type TypeName;
    ...
    #pragma endregion Forward_Declare

    UCLASS(BlueprintType)
    class UMyClass : public AActor 
    {
        // Generated Body 최상단 선언
        GENERATED_BODY()

    public:  // 생성자 단일로 public 선언
        UMyClass();

    protected:                                                  // protected로 Begin Play, Initialize, End Play 등 Method Override
        virtual void BeginPlay() override;
        virtual void PostInitialize() override;
        virtual void EndPlay() override;

    public:                                                     // 여기서 부터 `MyClass` public-protected-private Method Declare/Override
        virtual void Tick(float DeltaTime) override;            // override 함수는 원본 함수와 매개변수 등 이름 같게.

        UFUNCTION(BlueprintCallable, Category=Component)
        FORCEINLINE UStaticMeshComponent* GetStaticMeshComponent() {return compStaticMesh;}

        UFUNCTION(BlueprintCallable, Category=Component)
        FORCEINLINE UBoxComponent* GetColliderComponent() {return compCollider;}

        UFUCNITON(BlueprintCallable, Category=Speed)
        virtual void SetSpeed(float inSpeed);                   // 새로 만드는 함수는 자체 Convention에 맞게
        UFUNCTION(BlueprintPure, Category=Speed)
        virtual float GetSpeed();

    protected:

    private:
        void CalcSpeed(float deltaTime);


    public:  // 여기서 부터 `MyClass` 자체 public-protected-private Property Declare

    protected:  
        UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Component, meta=(AllowPrivateAccess=true))
        TObjectPtr<UStaticMeshComponent> compStaticMesh = nullptr;
        UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Component, meta=(AllowPrivateAccess=true))
        TObjectPtr<UBoxComponent> compCollider = nullptr;

    private:
        UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
        float speed = 0.f;
        ...
    }
    ```

## 정의 Definition

1. **가능하다면**, "if with initializer"의 형태로 조건문 생성.
    - (ex)
    ```
    if (AGameModeBase* GM = GetWorld()->GetAuthGameMode<AGameModeBase>())
	{
        GM->...
    }
    ```
    - 만약 if문 밖에서도 해당 지역변수가 필요하다면, if 문 밖에서 선언.
    - `static UConstructorHelper::[]Finder tempFinder(PATH)` 의 경우 무조건 "if with initializer" 형태로 구현
        - (ex)
        ```
        if (UConstructorHelper::FObjectFinder<UStaticMesh> tempSMFinder(PATH);
            if (tempSMFinder.Succeeded()))
        {
            staticMeshComponent->SetStaticMesh(tempSMFinder.Object);
        }
        ```
    
2. Scope Body`{}`가 필요 없더라도 명시적으로, 개행을 포함하여 표현
    - (ex)
    ```
    // (x)
    if (var) var = nullptr;  
      ==>
    // (o)
    if (var) 
    {
        var = nullptr; 
    }
    ```
    - 단, `FORCEINLINE`, `inline` 은 최대한 한줄로 표현하고, 개행이 필요할 시 `2`의 규칙을 적용

3. Debugging
    - `Utility/DebugHeler.h` 에 정의된 매크로를 이용하여 Log 및 Print
        - (ex)
        ```
        LOG_TEXT(TEXT("%s"), *myInt.ToString());
        PRINT_TEXT(11, 3.f, TEXT("%.3f, %.3f, %.3f"), myVector.x, myVector.y, myVector.z);
        ```
        - PRINT_TEXT는 갱신되어야할 정보가 아닌 이상 서로 다른 키를 이용
        - 추후 프로젝트가 완성되면 디버깅 메시지는 모두 주석 처리하거나 제거


4. 내부 구현 중 복잡한 기능은 간략하게 주석으로 설명을 기입

5. 매크로 정의가 필요한 경우에는, 아래의 주소 참조하여 이용 (매크로 정의 하지 않을 경우 참조 불필요)
    - [Microsoft_preprocesso](https://learn.microsoft.com/ko-kr/cpp/preprocessor/predefined-macros?view=msvc-170)