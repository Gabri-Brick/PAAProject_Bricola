#include "GridCell.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "StrategicoGameMode.h"
#include "TimerManager.h"

//Costruttore della cella
AGridCell::AGridCell()
{
    PrimaryActorTick.bCanEverTick = false;

    CellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CellMesh"));
    RootComponent = CellMesh;
    CellMesh->SetGenerateOverlapEvents(true);
    CellMesh->SetMobility(EComponentMobility::Static);
    CellMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (MeshAsset.Succeeded())
    {
        CellMesh->SetStaticMesh(MeshAsset.Object);
    }

    CellMesh->SetWorldScale3D(FVector(0.95f, 0.95f, 1.f));

    // Abilita il clic sulla mesh e aggiunge il binding
    CellMesh->SetNotifyRigidBodyCollision(true);
    CellMesh->OnClicked.AddDynamic(this, &AGridCell::OnCellClicked);

    GridX = 0;
    GridY = 0;
    bIsObstacle = false;
    BaseMaterial = nullptr;
    HighlightMaterial = nullptr;
    ObstacleMaterial = nullptr;
    bOccupied = false;
    bMovementHighlighted = false;
    bAttackHighlighted = false;
    OccupyingUnit = nullptr;

}
//Inizializzazione
void AGridCell::Init(int32 InX, int32 InY, bool bObstacle)
{
    GridX = InX;
    GridY = InY;
    bIsObstacle = bObstacle;

    if (bIsObstacle)
    {
        if (ObstacleMaterial)
            CellMesh->SetMaterial(0, ObstacleMaterial);
        else if (BaseMaterial)
            CellMesh->SetMaterial(0, BaseMaterial);
    }
    else
    {
        if (BaseMaterial)
        {
            // Assegnazione del materiale base memorizzato come OriginalMaterial
            CellMesh->SetMaterial(0, BaseMaterial);
            OriginalMaterial = BaseMaterial;
        }
    }
}
//Setter dei materiali
void AGridCell::SetCellMaterial(UMaterialInterface* NewMaterial)
{
    if (CellMesh && NewMaterial)
    {
        CellMesh->SetMaterial(0, NewMaterial);
    }
}
//Funzioni di MouseOver
void AGridCell::NotifyActorBeginCursorOver()
{
    Super::NotifyActorBeginCursorOver();

    if (bMovementHighlighted || bAttackHighlighted)
    {
        return;
    }

    // UE_LOG(LogTemp, Warning, TEXT("Mouse over sulla cella (%d, %d)"), GridX, GridY);
    if (!bIsObstacle && HighlightMaterial)
    {
        CellMesh->SetMaterial(0, HighlightMaterial);
    }
}

void AGridCell::NotifyActorEndCursorOver()
{
    Super::NotifyActorEndCursorOver();
    // Se la cella è evidenziata per il movimento non fa nulla
    if (bMovementHighlighted || bAttackHighlighted)
    {
        return;
    }

    if (bIsObstacle)
    {
        if (ObstacleMaterial)
        {
            CellMesh->SetMaterial(0, ObstacleMaterial);
        }
        else if (BaseMaterial)
        {
            CellMesh->SetMaterial(0, BaseMaterial);
        }
    }
    else
    {
        if (OriginalMaterial)
        {
            CellMesh->SetMaterial(0, OriginalMaterial);
            CellMesh->MarkRenderStateDirty();
        }
        else if (BaseMaterial)
        {
            CellMesh->SetMaterial(0, BaseMaterial);
            CellMesh->MarkRenderStateDirty();
        }
    }
}
//Gestione del click per il passaggio al GameMode
void AGridCell::OnCellClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    UE_LOG(LogTemp, Warning, TEXT("Cella cliccata (%d, %d) con il tasto: %s"),
        GridX, GridY, *ButtonPressed.ToString());

    // 1) Recupera il GameMode
    AStrategicoGameMode* GM = Cast<AStrategicoGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnCellClicked: GameMode non trovato!"));
        return;
    }

    // 2) Passa la cella cliccata al GameMode
    GM->HandleCellClick(this);
}






