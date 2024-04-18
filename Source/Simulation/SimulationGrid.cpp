//Fill out your copyright notice in the Description page of Project Settings.


#include "SimulationGrid.h"
#include "Slate.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

#define LOCTEXT_NAMESPACE "Simulation"

ASimulationGrid::ASimulationGrid()
{
	PrimaryActorTick.bCanEverTick = true;

	GridSelectorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GridSelectorMesh"));
	if (IsValid(GridSelectorMesh))
	{
		GridSelectorMesh->SetVisibility(false);
	}
}

void ASimulationGrid::StepSimulation()
{
	CellsToRender.Empty();
	FGrid2D CurGrid = Grid;

	for (uint32 i = 0; i < Grid.SizeX; i++)
	{
		for (uint32 j = 0; j < Grid.SizeY; j++)
		{
			char TL = CurGrid(i - 1, j - 1);
			char TC = CurGrid(i, j - 1);
			char TR = CurGrid(i + 1, j - 1);

			char CL = CurGrid(i - 1, j);
			char CC = CurGrid(i, j);
			char CR = CurGrid(i + 1, j);

			char BL = CurGrid(i - 1, j + 1);
			char BC = CurGrid(i, j + 1);
			char BR = CurGrid(i + 1, j + 1);

			int N = TL + TC + TR + BL + BC + BR + CL + CR;
			bool Alive = ((N == 2 && CC) || N == 3);
			Grid.Set(i, j, Alive);

			if (Alive)
			{
				CellsToRender.Add(FVector2D(i, j));
			}
		}
	}

	RenderGrid();
}

void ASimulationGrid::PlaceBlock()
{
	Grid.Set(SelectedBlock.X, SelectedBlock.Y, true);
	CellsToRender.Add(SelectedBlock);

	DrawDebugBox(GetWorld(), GridUnitToWorldSpacePosition(SelectedBlock),
	             FVector(40.f, 40.f, 20.f), FColor::Green);

	if (!bIsSimulating)
	{
		RenderGrid();
	}
}

void ASimulationGrid::RemoveBlock()
{
	Grid.Set(SelectedBlock.X, SelectedBlock.Y, false);
	CellsToRender.Remove(SelectedBlock);

	DrawDebugBox(GetWorld(), GridUnitToWorldSpacePosition(SelectedBlock),
	             FVector(40.f, 40.f, 20.f), FColor::Red);

	if (!bIsSimulating)
	{
		RenderGrid();
	}
}

void ASimulationGrid::ToggleSimulation()
{
	PlayerController->SetShowMouseCursor(bIsSimulating);
	GridSelectorMesh->SetVisibility(bIsSimulating);

	bIsSimulating
		? GetWorldTimerManager().PauseTimer(SimStepTimeHandle)
		: GetWorldTimerManager().UnPauseTimer(SimStepTimeHandle);

	bIsSimulating = !bIsSimulating;
}


void ASimulationGrid::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector WorldLocation;
	FVector WorldDirection;
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FVector PlaneOrigin(0.0f, 0.0f, 0.0f);

	FVector CursorWorldLocation = FMath::LinePlaneIntersection(
		WorldLocation,
		WorldLocation + WorldDirection,
		PlaneOrigin,
		FVector::UpVector);

	SelectedBlock = WorldSpacePositionToGridUnit(CursorWorldLocation);

	GridSelectorMesh->SetWorldLocation(GridUnitToWorldSpacePosition(SelectedBlock));
}

FVector2D ASimulationGrid::WorldSpacePositionToGridUnit(const FVector& Position)
{
	return FVector2D(
		FMath::RoundToInt(Position.X / MeshBounds.X),
		FMath::RoundToInt(Position.Y / MeshBounds.Y));
}

FVector ASimulationGrid::GridUnitToWorldSpacePosition(const FVector2D& Unit)
{
	return FVector(
		(Unit.X * MeshBounds.X),
		(Unit.Y * MeshBounds.Y),
		0.f);
}


void ASimulationGrid::RenderGrid()
{

	TArray<FTransform> Instances;
	for (FVector2D Unit : CellsToRender)
	{
		FTransform NewInstanceTransform;
		NewInstanceTransform.SetLocation(GridUnitToWorldSpacePosition(Unit));
		Instances.Add(NewInstanceTransform);
	}

	ISMComp->ClearInstances();
	ISMComp->AddInstances(Instances, false);
}


void ASimulationGrid::BeginPlay()
{
	Super::BeginPlay();

	if (!GridCellStaticMesh)
	{
		return;
	}

	ISMComp = NewObject<UInstancedStaticMeshComponent>(this);
	ISMComp->RegisterComponent();
	ISMComp->SetStaticMesh(GridCellStaticMesh);
	ISMComp->SetMaterial(0, GridCellStaticMesh->GetMaterial(0));
	ISMComp->SetFlags(RF_Transactional);
	AddInstanceComponent(ISMComp);

	//INIT SIM
	Grid.Init(GridShape.X, GridShape.Y);

	GetWorldTimerManager().SetTimer(SimStepTimeHandle,
	                                this,
	                                &ASimulationGrid::StepSimulation,
	                                SimulationStepTime,
	                                true);
	bIsSimulating = true;

	PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	MeshBounds = GridCellStaticMesh->GetBoundingBox().GetSize();


	//INPUT
	EnableInput(PlayerController);
	InputComponent->BindAction("PlaceBlock", IE_Pressed, this,
	                           &ASimulationGrid::PlaceBlock);
	InputComponent->BindAction("RemoveBlock", IE_Pressed, this,
	                           &ASimulationGrid::RemoveBlock);
	InputComponent->BindAction("ToggleSimulation", IE_Pressed, this,
	                           &ASimulationGrid::ToggleSimulation);


	//UI
	auto SimulationUI = SNew(SOverlay)
		+ SOverlay::Slot()
		  .HAlign(HAlign_Center)
		  .VAlign(VAlign_Top)
		[
			SNew(STextBlock)
				.ColorAndOpacity_Lambda([this]
			                {
				                return bIsSimulating
					                       ? FLinearColor::Red
					                       : FLinearColor::Green;
			                })
				.Text_Lambda([this]
			                {
				                return bIsSimulating
					                       ? LOCTEXT("Toggle Simulation", "Press space to pause simulation")
					                       : LOCTEXT("Toggle Simulation", "Press space to play simulation");
			                })
		]
		+ SOverlay::Slot()
		  .HAlign(HAlign_Center)
		  .VAlign(VAlign_Bottom)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				//Clear Simulation Button
				SNew(SButton)
        				.HAlign(HAlign_Center)
        				.Text(LOCTEXT("Clear", "Clear"))
        				.ContentPadding(FMargin(4.f, 4.f))
        				.OnClicked_Lambda([this]()
				             {
					             Grid.Clear();
					             StepSimulation();
					             return FReply::Handled();
				             })

			]
			+ SHorizontalBox::Slot()
			[
				//Clear Simulation Button
				SNew(SButton)
        				.HAlign(HAlign_Center)
        				.Text(LOCTEXT("RandClear", "RandClear"))
        				.ContentPadding(FMargin(4.f, 4.f))
        				.OnClicked_Lambda([this]()
				             {
					             Grid.RandClear();
					             StepSimulation();
					             return FReply::Handled();
				             })

			]

			+ SHorizontalBox::Slot()
			[
				//Toggle Simulation Button
				SNew(SButton)
        				.HAlign(HAlign_Center)
        				.Text_Lambda([this]()
				             {
					             return bIsSimulating ? LOCTEXT("Pause", "Pause") : LOCTEXT("Play", "Play");
				             })
        				.ContentPadding(FMargin(4.f, 4.f))
        				.OnClicked_Lambda([this]()
				             {
					             ToggleSimulation();
					             return FReply::Handled();
				             })

			]
			+ SHorizontalBox::Slot()
			  .AutoWidth()
			  .HAlign(HAlign_Center)
			[
				//Simulation TimeStep Slider
				SNew(SSpinBox<float>)
        			.MinValue(0.f)
        			.MaxValue(10.f)
        			.Value(SimulationStepTime)
        			.ToolTipText(LOCTEXT("Simulation StepTime", "Simulation Step Time"))
        			.OnValueCommitted_Lambda([this](float NewValue, ETextCommit::Type CommitType)
				                     {
					                     SimulationStepTime = NewValue;

					                     GetWorldTimerManager().ClearTimer(SimStepTimeHandle);
					                     GetWorldTimerManager().SetTimer(SimStepTimeHandle,
					                                                     this,
					                                                     &ASimulationGrid::StepSimulation,
					                                                     SimulationStepTime,
					                                                     true);

					                     //Pause Timer if we're currently not simulating
					                     if (!bIsSimulating)
					                     {
						                     GetWorldTimerManager().PauseTimer(SimStepTimeHandle);
					                     }
				                     })
			]

		];

	GEngine->GameViewport->AddViewportWidgetContent(SimulationUI);
}


#undef LOCTEXT_NAMESPACE
