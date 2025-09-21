// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
* @Brief PathFinding Plugin main module class
*
* Advanced Zone-based PathFinding system module that provides:
* - Multi-algorithm support (Dijkstra, A*, Floyd-Warshall, Poly-Anya)
* - Level Instance integration with dynamic loading
* - Zone-based path score caching system
* - Cross-zone pathfinding capabilities
*/
class FPathFindingPluginModule : public IModuleInterface
{
public:
	/**
	* @Brief Initialize the PathFinding plugin module
	*/
	virtual void StartupModule() override;

	/**
	* @Brief Cleanup the PathFinding plugin module
	*/
	virtual void ShutdownModule() override;

private:
	/** Handle to the test dll we will load */
	void* testDLLHandle;
};