#include "GridManager.h"
#include "Engine/World.h"

namespace {
    /* Funzione helper per controllare se tutte le celle libere sono connesse (raggiungibili l'una dall'altra) */
    bool AreAllFreeCellsConnected(const TArray<bool>& obstacleMap, int32 gridDim, int32 totalFree)
    {
        if (totalFree <= 1)
        {
            // 0 o 1 cella libera sono sempre "connesse" in modo triviale
            return true;
        }
        const int32 totalCells = gridDim * gridDim;
        // Trova l'indice della prima cella libera come punto di partenza
        int32 startIndex = -1;
        for (int32 i = 0; i < totalCells; ++i)
        {
            if (!obstacleMap[i])
            {
                startIndex = i;
                break;
            }
        }
        if (startIndex == -1)
        {
            return true; // Nessuna cella libera trovata (caso degenerato)
        }

        // BFS per contare le celle libere raggiungibili da startIndex
        TArray<bool> visited;
        visited.Init(false, totalCells);
        TArray<int32> stack;
        visited[startIndex] = true;
        stack.Add(startIndex);
        int32 visitedFreeCount = 0;

        while (stack.Num() > 0)
        {
            int32 currentIndex = stack.Pop(/*bAllowShrinking=*/ false);
            visitedFreeCount++;

            int32 cx = currentIndex % gridDim;
            int32 cy = currentIndex / gridDim;

            // Controlla i 4 vicini (su, giù, sinistra, destra)
            if (cx > 0)
            {
                int32 neighborIdx = (cx - 1) + cy * gridDim;
                if (!obstacleMap[neighborIdx] && !visited[neighborIdx])
                {
                    visited[neighborIdx] = true;
                    stack.Add(neighborIdx);
                }
            }
            if (cx < gridDim - 1)
            {
                int32 neighborIdx = (cx + 1) + cy * gridDim;
                if (!obstacleMap[neighborIdx] && !visited[neighborIdx])
                {
                    visited[neighborIdx] = true;
                    stack.Add(neighborIdx);
                }
            }
            if (cy > 0)
            {
                int32 neighborIdx = cx + (cy - 1) * gridDim;
                if (!obstacleMap[neighborIdx] && !visited[neighborIdx])
                {
                    visited[neighborIdx] = true;
                    stack.Add(neighborIdx);
                }
            }
            if (cy < gridDim - 1)
            {
                int32 neighborIdx = cx + (cy + 1) * gridDim;
                if (!obstacleMap[neighborIdx] && !visited[neighborIdx])
                {
                    visited[neighborIdx] = true;
                    stack.Add(neighborIdx);
                }
            }
        }

        return (visitedFreeCount == totalFree);
    }
}

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
    ObstaclePercentage = 20;   // Imposta il 20% di ostacoli per default
    CellSize = 100.f;          // Ogni cella misura 100 unità
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();

    const int32 totalCells = GridSize * GridSize;
    // Calcola il numero di ostacoli da piazzare
    int32 targetObstacleCount = FMath::RoundToInt(((float)ObstaclePercentage / 100.f) * totalCells);
    if (targetObstacleCount >= totalCells)
    {
        targetObstacleCount = totalCells - 1;
    }

    // Mappa lineare delle celle: false = cella libera, true = ostacolo
    TArray<bool> obstacleMap;
    obstacleMap.Init(false, totalCells);
    int32 obstaclesPlaced = 0;
    int32 freeCellsCount = totalCells;

    // Genera casualmente gli ostacoli, assicurando che le celle libere siano connesse
    while (obstaclesPlaced < targetObstacleCount)
    {
        int32 randIndex;
        int32 attempts = 0;
        do {
            randIndex = FMath::RandRange(0, totalCells - 1);
            attempts++;
        } while (obstacleMap[randIndex] && attempts < 10);
        if (obstacleMap[randIndex])
        {
            for (int32 idx = 0; idx < totalCells; ++idx)
            {
                if (!obstacleMap[idx])
                {
                    randIndex = idx;
                    break;
                }
            }
        }
        if (!obstacleMap[randIndex])
        {
            obstacleMap[randIndex] = true;
            freeCellsCount--;
            if (!AreAllFreeCellsConnected(obstacleMap, GridSize, freeCellsCount))
            {
                obstacleMap[randIndex] = false;
                freeCellsCount++;
                continue;
            }
            obstaclesPlaced++;
        }
    }

    UWorld* World = GetWorld();
    if (!World) return;

    GridCells.Empty();
    GridCells.SetNum(totalCells);

    // Calcola l'offset per centrare la griglia attorno alla posizione dell'GridManager
    const int32 halfIndex = (GridSize - 1) / 2;
    FVector origin = GetActorLocation();

    for (int32 y = 0; y < GridSize; ++y)
    {
        for (int32 x = 0; x < GridSize; ++x)
        {
            FVector cellLocation = origin + FVector((x - halfIndex) * CellSize, (y - halfIndex) * CellSize, 0.f);
            FTransform spawnTransform(FRotator::ZeroRotator, cellLocation);

            AGridCell* Cell = World->SpawnActor<AGridCell>(AGridCell::StaticClass(), spawnTransform);
            if (!Cell) continue;

            int32 index = x + y * GridSize;
            bool isObstacle = obstacleMap[index];

            if (isObstacle)
            {
                if (ObstacleMaterials.Num() > 0)
                {
                    int32 matIndex = FMath::RandRange(0, ObstacleMaterials.Num() - 1);
                    UMaterialInterface* chosenMat = ObstacleMaterials[matIndex];
                    if (chosenMat)
                    {
                        // Propaga il materiale ostacolo alla cella
                        Cell->ObstacleMaterial = chosenMat;
                        Cell->SetCellMaterial(chosenMat);
                    }
                }
            }
            else
            {
                if (BaseMaterial)
                {
                    // Propaga il BaseMaterial alla cella
                    Cell->BaseMaterial = BaseMaterial;
                    Cell->SetCellMaterial(BaseMaterial);
                }
                // Propaga anche il valore di HighlightMaterial dalla proprietà del GridManager alla cella libera
                Cell->HighlightMaterial = HighlightMaterial;
            }

            // Inizializza la cella con le coordinate e lo stato (questo imposta GridX e GridY)
            Cell->Init(x, y, isObstacle);

            GridCells[index] = Cell;
            Cell->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        }
    }
}

AGridCell* AGridManager::GetCellAt(int32 X, int32 Y) const
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize)
    {
        return nullptr;
    }
    return GridCells[X + Y * GridSize];
}
