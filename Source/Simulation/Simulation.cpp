// Fill out your copyright notice in the Description page of Project Settings.

#include "Simulation.h"
#include "Modules/ModuleManager.h"


void FSimulation::StartupModule()
{
	
}

void FSimulation::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl,
                              FSimulation,
                              "Simulation");
