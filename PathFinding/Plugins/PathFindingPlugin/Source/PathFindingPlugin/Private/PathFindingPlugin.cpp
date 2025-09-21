// Copyright Epic Games, Inc. All Rights Reserved.

#include "PathFindingPlugin.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FPathFindingPluginModule"

void FPathFindingPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file
	UE_LOG(LogTemp, Warning, TEXT("PathFinding Plugin module started"));
}

void FPathFindingPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(LogTemp, Warning, TEXT("PathFinding Plugin module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPathFindingPluginModule, PathFindingPlugin)