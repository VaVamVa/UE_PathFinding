#pragma once

#include "CoreMinimal.h"

/**
* @Brief Debug logging and printing utility macros
*
* Provides convenient macros for logging and on-screen debug printing
* with consistent formatting and easy enable/disable functionality
*/

// Debug 로그 출력 매크로
#define LOG_TEXT(Format, ...) \
	UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)

// Debug 화면 출력 매크로
#define PRINT_TEXT(Key, Time, Format, ...) \
	if (GEngine) \
	{ \
		GEngine->AddOnScreenDebugMessage(Key, Time, FColor::Red, FString::Printf(Format, ##__VA_ARGS__)); \
	}

// Debug 조건부 로그 출력
#define LOG_TEXT_IF(Condition, Format, ...) \
	if (Condition) \
	{ \
		LOG_TEXT(Format, ##__VA_ARGS__) \
	}

// Debug 조건부 화면 출력
#define PRINT_TEXT_IF(Condition, Key, Time, Format, ...) \
	if (Condition) \
	{ \
		PRINT_TEXT(Key, Time, Format, ##__VA_ARGS__) \
	}