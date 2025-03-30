#include "StrategicoGameMode.h"
#include "CameraPawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GridCell.h"
#include "Unit.h"
#include "Brawler.h"
#include "Sniper.h"
#include "StrategicoPlayerController.h"


AStrategicoGameMode::AStrategicoGameMode()
{
    // Imposta il Pawn predefinito come CameraPawn
    DefaultPawnClass = ACameraPawn::StaticClass();
    PlayerControllerClass = AStrategicoPlayerController::StaticClass();
    PlayerUnitsPlaced = 0;
    AIUnitsPlaced = 0;
    CurrentGamePhase = EGamePhase::Placement;
    ActiveUnit = nullptr;

    // Carica il materiale per lo Sniper del giocatore
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BSniperMat(TEXT("/Game/Materiali/B_Sniper"));
    if (BSniperMat.Succeeded())
    {
        B_Sniper = BSniperMat.Object;
        UE_LOG(LogTemp, Warning, TEXT("B_Sniper caricato correttamente."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento di B_Sniper."));
    }

    // Carica il materiale per il Brawler del giocatore
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BBrawlerMat(TEXT("/Game/Materiali/B_Brawler"));
    if (BBrawlerMat.Succeeded())
    {
        B_Brawler = BBrawlerMat.Object;
         UE_LOG(LogTemp, Warning, TEXT("B_Brawler caricato correttamente."));
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento di B_Brawler."));
    }

    // Carica il materiale per lo Sniper dell'IA
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> RSniperMat(TEXT("/Game/Materiali/R_Sniper"));
    if (RSniperMat.Succeeded())
    {
        R_Sniper = RSniperMat.Object;
         UE_LOG(LogTemp, Warning, TEXT("R_Sniper caricato correttamente."));
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento di R_Sniper."));
    }

    // Carica il materiale per il Brawler dell'IA
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> RBrawlerMat(TEXT("/Game/Materiali/R_Brawler"));
    if (RBrawlerMat.Succeeded())
    {
        R_Brawler = RBrawlerMat.Object;
         UE_LOG(LogTemp, Warning, TEXT("R_Brawler caricato correttamente."));
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento di R_Brawler."));
    }

    static ConstructorHelpers::FClassFinder<UHudStrategio> HUDGameBPClass(TEXT("/Game/HUD/BP_HUD"));
    if (HUDGameBPClass.Class != nullptr)
    {
        HUDGame = HUDGameBPClass.Class;
    }

    //Musica

    static ConstructorHelpers::FObjectFinder<USoundBase> MusicAsset(TEXT("/Game/Suoni/Elite_4"));
    if (MusicAsset.Succeeded())
    {
        BackgroundMusic = MusicAsset.Object;
    }
    else
    {
        BackgroundMusic = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Impossibile caricare la musica di sottofondo."));
    }

    // Carica il suono Victory
    static ConstructorHelpers::FObjectFinder<USoundBase> VictorySoundAsset(TEXT("/Game/Suoni/Victory"));
    if (VictorySoundAsset.Succeeded())
    {
        VictorySound = VictorySoundAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento del suono Victory"));
    }

    // Carica il suono Lose
    static ConstructorHelpers::FObjectFinder<USoundBase> LoseSoundAsset(TEXT("/Game/Suoni/Lose"));
    if (LoseSoundAsset.Succeeded())
    {
        LoseSound = LoseSoundAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento del suono Lose"));
    }

}

void AStrategicoGameMode::HandleCellClick(AGridCell* ClickedCell)
{
    if (!ClickedCell) return;

    switch (CurrentGamePhase)
    {
    case EGamePhase::Placement:
        // Usa la funzione PlacePlayerUnit(ClickedCell)
        PlacePlayerUnit(ClickedCell);
        break;

    case EGamePhase::InGame:
        HandleInGameCellClick(ClickedCell);
        break;

    default:
        break;
    }
}

void AStrategicoGameMode::HandleInGameCellClick(AGridCell* ClickedCell)
{
    if (!ActiveUnit || ActiveUnit->CurrentActionState != EUnitActionState::Movement)
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessuna unità in movimento selezionata."));
        return;
    }

    // controllo sul flag bHaMosso:
    if (ActiveUnit->bHaMosso)
    {
        UE_LOG(LogTemp, Warning, TEXT("L'unità ha già mosso in questo turno."));
        return;
    }

    if (!ActiveUnit->HighlightedCells.Contains(ClickedCell))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cella non raggiungibile per il movimento."));
        return;
    }

    TArray<AGridCell*> Path = ComputePath(ActiveUnit->CurrentCell, ClickedCell, ActiveUnit);

    if (Path.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Percorso non trovato."));
        return;
    }

    ActiveUnit->AnimateMovementAlongPath(Path);
}

void AStrategicoGameMode::BeginPlay()
{
    Super::BeginPlay();


    if (!HUD)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            // Aggiunge widget HUD al viewport
            HUD = CreateWidget<UHudStrategio>(PC, HUDGame);
            if (HUD)
            {
                HUD->AddToViewport();
            }
        }
    }

    // Configura il PlayerController per abilitare il mouse e gli eventi di interazione
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->bEnableMouseOverEvents = true;
        PC->bEnableClickEvents = true;
        PC->SetInputMode(FInputModeGameOnly());
    }

    // Esegue il lancio della moneta per determinare chi inizia
    CoinToss();
    if (TurnOwner == ETurnOwner::AI)
    {
        AutoPlaceAIUnits();
    }
    //MUSICA di BackGround durante il gameplay
    if (BackgroundMusic)
    {
       UGameplayStatics::PlaySound2D(this, BackgroundMusic, 0.1f);
    }
}

void AStrategicoGameMode::CoinToss()
{
    // Genera un numero casuale 0 o 1
    int32 Toss = FMath::RandRange(0, 1);
    if (Toss == 0)
    {
        TurnOwner = ETurnOwner::Player;
        FString Message = TEXT("Testa: Inizia il giocatore.");
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
        if (HUD)
        {
            HUD->AppendMoveHistory(Message);
        }
    }
    else
    {
        TurnOwner = ETurnOwner::AI;
        FString Message = TEXT("Croce: Inizia l'IA.");
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
        if (HUD)
        {
            HUD->AppendMoveHistory(Message);
        }
    }
    FString Message = TEXT("Inizio della fase di PLACEMENT");
    HUD->AppendMoveHistory(Message);
}

void AStrategicoGameMode::PlacePlayerUnit(AGridCell* TargetCell)
{
    if (TurnOwner != ETurnOwner::Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non e' il turno del giocatore."));
        return;
    }

    if (PlayerUnitsPlaced >= 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Tutte le unità sono state posizionate per il giocatore."));
        return;
    }

    if (!TargetCell || TargetCell->bIsObstacle || TargetCell->bOccupied)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cella non valida per il posizionamento."));
        return;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        TargetCell->bOccupied = true;

        FVector SpawnLocation = TargetCell->GetActorLocation();
        FTransform SpawnTransform(TargetCell->GetActorRotation(), SpawnLocation);

        AUnit* NewUnit = nullptr;

        if (PlayerUnitsPlaced == 0)
        {
            ASniper* SpawnedSniper = World->SpawnActor<ASniper>(ASniper::StaticClass(), SpawnTransform);
            if (SpawnedSniper)
            {
                SpawnedSniper->SetUnitMaterial(B_Sniper);
                SpawnedSniper->UnitTeam = EUnitTeam::Player;
                SpawnedSniper->UnitDisplayName = TEXT("Player_Sniper");
                NewUnit = SpawnedSniper;

                FString Message = FString::Printf(TEXT("Unità %s posizionata sulla cella %s"),
                    *SpawnedSniper->UnitDisplayName, *GetGridCellIdentifier(TargetCell));
                UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
                if (HUD)
                {
                    HUD->AppendMoveHistory(Message);
                }
            }
        }
        else if (PlayerUnitsPlaced == 1)
        {
            ABrawler* SpawnedBrawler = World->SpawnActor<ABrawler>(ABrawler::StaticClass(), SpawnTransform);
            if (SpawnedBrawler)
            {
                SpawnedBrawler->SetUnitMaterial(B_Brawler);
                SpawnedBrawler->UnitTeam = EUnitTeam::Player;
                SpawnedBrawler->UnitDisplayName = TEXT("Player_Brawler");
                NewUnit = SpawnedBrawler;

                FString Message = FString::Printf(TEXT("Unità %s posizionata sulla cella %s"),
                    *SpawnedBrawler->UnitDisplayName, *GetGridCellIdentifier(TargetCell));
                UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
                if (HUD)
                {
                    HUD->AppendMoveHistory(Message);
                }
            }
        }

        // Aggiorna la cella occupata
        if (NewUnit)
        {
            NewUnit->CurrentCell = TargetCell;
            TargetCell->OccupyingUnit = NewUnit;
        }

        PlayerUnitsPlaced++;

        if (PlayerUnitsPlaced == 2 && AIUnitsPlaced == 2)
        {
            CurrentGamePhase = EGamePhase::InGame;
            FString Message = TEXT("Tutte le unità piazzate. Ora si passa alla fase di GIOCO");
            UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
            if (HUD)
            {
                HUD->AppendMoveHistory(Message);
            }
        }


        NextTurn();
    }
}

void AStrategicoGameMode::AutoPlaceAIUnits()
{
    if (AIUnitsPlaced >= 2) {
        UE_LOG(LogTemp, Warning, TEXT("Tutte le unità sono state posizionate per l'IA"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("AutoPlaceAIUnits Chiamata"));
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundCells;
    UGameplayStatics::GetAllActorsOfClass(World, AGridCell::StaticClass(), FoundCells);
    UE_LOG(LogTemp, Warning, TEXT("AutoPlaceAIUnits: trovate %d celle"), FoundCells.Num());

    TArray<AGridCell*> FreeCells;
    for (AActor* Actor : FoundCells)
    {
        AGridCell* Cell = Cast<AGridCell>(Actor);
        if (Cell && !Cell->bIsObstacle && !Cell->bOccupied)
        {
            FreeCells.Add(Cell);
        }
    }

    if (FreeCells.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Nessuna cella libera disponibile per il posizionamento IA"));
        return;
    }

    int32 RandomIndex = FMath::RandRange(0, FreeCells.Num() - 1);
    AGridCell* TargetCell = FreeCells[RandomIndex];

    if (TargetCell)
    {
        TargetCell->bOccupied = true;

        FVector SpawnLocation = TargetCell->GetActorLocation();
        FTransform SpawnTransform(TargetCell->GetActorRotation(), SpawnLocation);

        if (AIUnitsPlaced == 0)
        {
            ASniper* NewUnit = World->SpawnActor<ASniper>(ASniper::StaticClass(), SpawnTransform);
            if (NewUnit)
            {
                NewUnit->SetUnitMaterial(R_Sniper);
                NewUnit->UnitTeam = EUnitTeam::AI;
                NewUnit->UnitDisplayName = TEXT("IA_Sniper");
                NewUnit->CurrentCell = TargetCell;
                TargetCell->OccupyingUnit = NewUnit;
                FString Message = FString::Printf(TEXT("Unità %s posizionata sulla cella %s"),
                    *NewUnit->UnitDisplayName, *GetGridCellIdentifier(TargetCell));
                UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
                if (HUD)
                {
                    HUD->AppendMoveHistory(Message);
                }
            }
        }
        else if (AIUnitsPlaced == 1)
        {
            ABrawler* NewUnit = World->SpawnActor<ABrawler>(ABrawler::StaticClass(), SpawnTransform);
            if (NewUnit)
            {
                NewUnit->SetUnitMaterial(R_Brawler);
                NewUnit->UnitTeam = EUnitTeam::AI;
                NewUnit->UnitDisplayName = TEXT("IA_Brawler");
                NewUnit->CurrentCell = TargetCell;
                TargetCell->OccupyingUnit = NewUnit;
                FString Message = FString::Printf(TEXT("Unità %s posizionata sulla cella %s"),
                    *NewUnit->UnitDisplayName, *GetGridCellIdentifier(TargetCell));
                UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
                if (HUD)
                {
                    HUD->AppendMoveHistory(Message);
                }
            }
        }
        AIUnitsPlaced++;

        if (PlayerUnitsPlaced == 2 && AIUnitsPlaced == 2)
        {
            CurrentGamePhase = EGamePhase::InGame;
            FString Message = TEXT("Tutte le unità piazzate. Ora si passa alla fase di GIOCO");
            UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
            if (HUD)
            {
                HUD->AppendMoveHistory(Message);
            }
        }

        NextTurn();
    }
}

void AStrategicoGameMode::ProcessIATurn()
{
    UE_LOG(LogTemp, Warning, TEXT("Processo turno IA in fase InGame..."));
    TArray<AUnit*> AIUnits = GetAllAIUnits();

    // Pulisci la coda dei movimenti pendenti
    PendingAIMovementUnits.Empty();

    // Per ogni unità IA
    for (AUnit* AIUnit : AIUnits)
    {
        if (!AIUnit || AIUnit->bHaMosso)
            continue; // Salta le unità che hanno già mosso

        // Verifica se c'è un nemico in attacco (range)
        AUnit* EnemyInRange = GetEnemyInAttackRange(AIUnit);
        if (EnemyInRange)
        {
            int32 Damage = AIUnit->Attack(EnemyInRange);
            UE_LOG(LogTemp, Warning, TEXT("Unità IA attacca e infligge %d danni"), Damage);
        }
        else
        {
            // Nessun nemico in attacco: trova il nemico più vicino
            AUnit* ClosestEnemy = FindClosestEnemyUnit(AIUnit);
            if (ClosestEnemy)
            {
                // Ottieni la cella in cui si trova il nemico
                AGridCell* EnemyCell = ClosestEnemy->CurrentCell;
                if (!EnemyCell)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Cella del nemico non valida per AIUnit %s."), *AIUnit->GetName());
                    continue;
                }

                // Crea una lista di celle candidate adiacenti al nemico che siano libere (non ostacolo e non occupate)
                TArray<AGridCell*> CandidateCells;
                TArray<FIntPoint> Directions = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
                for (FIntPoint Dir : Directions)
                {
                    int32 NeighborX = EnemyCell->GridX + Dir.X;
                    int32 NeighborY = EnemyCell->GridY + Dir.Y;
                    AGridCell* NeighborCell = GetGridCellAt(NeighborX, NeighborY);
                    if (NeighborCell && !NeighborCell->bIsObstacle && !NeighborCell->bOccupied)
                    {
                        CandidateCells.Add(NeighborCell);
                    }
                }

                if (CandidateCells.Num() > 0)
                {
                    // Seleziona la cella candidata più vicina all'unità IA
                    AGridCell* TargetCell = CandidateCells[0];
                    float MinDist = FVector::Dist(AIUnit->GetActorLocation(), TargetCell->GetActorLocation());
                    for (int32 i = 1; i < CandidateCells.Num(); i++)
                    {
                        float Dist = FVector::Dist(AIUnit->GetActorLocation(), CandidateCells[i]->GetActorLocation());
                        if (Dist < MinDist)
                        {
                            MinDist = Dist;
                            TargetCell = CandidateCells[i];
                        }
                    }
                    // Salva la cella target nell'unità (dovrai dichiararla in AUnit)
                    AIUnit->PendingTargetCell = TargetCell;
                    // Aggiungi l'unità alla coda dei movimenti pendenti
                    PendingAIMovementUnits.Add(AIUnit);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Nessuna cella libera intorno al nemico per AIUnit %s."), *AIUnit->GetName());
                }
            }
        }
    }

    // Avvia il movimento sequenziale
    ProcessNextAIMovement();
}

void AStrategicoGameMode::NextTurn()
{
    // Se siamo nel turno del giocatore in fase InGame e un'unità sta ancora muovendosi,
    // blocca il cambio turno.
    if (TurnOwner == ETurnOwner::Player && CurrentGamePhase == EGamePhase::InGame &&
        ActiveUnit && ActiveUnit->bIsMoving)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non puoi passare turno: un'unità sta ancora muovendosi."));
        return;
    }

    // Resetta le proprietà delle unità per il nuovo turno (ad es. bHaMosso, bHaAttaccato, ecc.)
    ResetUnitsForNextTurn();

    // Ora passa il turno in base al team.
    if (TurnOwner == ETurnOwner::Player)
    {
        // Passa il turno all'IA
        TurnOwner = ETurnOwner::AI;
        UE_LOG(LogTemp, Warning, TEXT("Turno: IA"));
        HUD->SetTurn(FText::FromString("TURNO: IA"));
        FString Message = TEXT("Turno: IA");
        HUD->AppendMoveHistory(Message);

        if (CurrentGamePhase == EGamePhase::Placement)
        {
            // Durante la fase di placement, l'IA posiziona le unità automaticamente.
            AutoPlaceAIUnits();
        }
        else if (CurrentGamePhase == EGamePhase::InGame)
        {
            // Durante la fase InGame, processa il turno dell'IA (movimento/attacco).
            ProcessIATurn();
        }
    }
    else // Se attualmente è il turno dell'IA
    {
        // Passa il turno al giocatore
        TurnOwner = ETurnOwner::Player;
        UE_LOG(LogTemp, Warning, TEXT("Turno: Giocatore"));
        FString Message = TEXT("Turno: GIOCATORE");
        HUD->AppendMoveHistory(Message);
        HUD->SetTurn(FText::FromString("TURNO: GIOCATORE"));
    }
}

TArray<AGridCell*> AStrategicoGameMode::ComputePath(AGridCell* Start, AGridCell* Goal, AUnit* Unit)
{
    TArray<AGridCell*> Path;
    if (!Start || !Goal || !Unit)
    {
        return Path;
    }

    // Struttura temporanea per i nodi del percorso
    struct FPathNode
    {
        AGridCell* Cell;
        int32 G; // costo dal nodo di partenza
        int32 H; // costo euristico (distanza Manhattan)
        int32 F() const { return G + H; }
        FPathNode* Parent;

        FPathNode(AGridCell* InCell, int32 InG, int32 InH, FPathNode* InParent = nullptr)
            : Cell(InCell), G(InG), H(InH), Parent(InParent)
        {
        }
    };

    // Funzione lambda per l'euristica (distanza Manhattan)
    auto Heuristic = [](AGridCell* A, AGridCell* B) -> int32
        {
            return FMath::Abs(A->GridX - B->GridX) + FMath::Abs(A->GridY - B->GridY);
        };

    TArray<FPathNode*> OpenSet;
    TArray<FPathNode*> ClosedSet;

    FPathNode* StartNode = new FPathNode(Start, 0, Heuristic(Start, Goal));
    OpenSet.Add(StartNode);

    FPathNode* FoundNode = nullptr;

    while (OpenSet.Num() > 0)
    {
        // Ordina per F crescente
        OpenSet.Sort([](const FPathNode& A, const FPathNode& B) {
            return A.F() < B.F();
            });

        FPathNode* Current = OpenSet[0];
        OpenSet.RemoveAt(0);
        ClosedSet.Add(Current);

        if (Current->Cell == Goal)
        {
            FoundNode = Current;
            break;
        }

        // Considera i vicini (solo 4 direzioni: su, giù, sinistra, destra)
        TArray<FIntPoint> Directions = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };
        for (FIntPoint Dir : Directions)
        {
            int32 NeighborX = Current->Cell->GridX + Dir.X;
            int32 NeighborY = Current->Cell->GridY + Dir.Y;

            // Usa una funzione helper che restituisce la cella data una coppia (X,Y)
            AGridCell* NeighborCell = GetGridCellAt(NeighborX, NeighborY);
            if (!NeighborCell)
                continue;
            if (NeighborCell->bIsObstacle)
                continue;

            // Se la cella è occupata e NON è il goal, e l'occupante non è l'unità corrente, salta
            if (NeighborCell->bOccupied && NeighborCell != Goal && NeighborCell->OccupyingUnit != Unit)
                continue;

            // Salta se già nella closed set
            bool bSkip = false;
            for (FPathNode* Node : ClosedSet)
            {
                if (Node->Cell == NeighborCell)
                {
                    bSkip = true;
                    break;
                }
            }
            if (bSkip)
                continue;

            int32 TentativeG = Current->G + 1;
            FPathNode* ExistingNode = nullptr;
            for (FPathNode* Node : OpenSet)
            {
                if (Node->Cell == NeighborCell)
                {
                    ExistingNode = Node;
                    break;
                }
            }
            if (ExistingNode)
            {
                if (TentativeG < ExistingNode->G)
                {
                    ExistingNode->G = TentativeG;
                    ExistingNode->Parent = Current;
                }
            }
            else
            {
                FPathNode* NeighborNode = new FPathNode(NeighborCell, TentativeG, Heuristic(NeighborCell, Goal), Current);
                OpenSet.Add(NeighborNode);
            }
        }
    }

    // Ricostruisce il percorso se FoundNode è stato trovato
    if (FoundNode)
    {
        FPathNode* Node = FoundNode;
        while (Node)
        {
            Path.Insert(Node->Cell, 0); // Inserisce in testa al percorso
            Node = Node->Parent;
        }
    }

    // Libera la memoria dei nodi temporanei
    for (FPathNode* Node : OpenSet)
    {
        delete Node;
    }
    for (FPathNode* Node : ClosedSet)
    {
        delete Node;
    }

    return Path;
}

AGridCell* AStrategicoGameMode::GetGridCellAt(int32 X, int32 Y) const
{
    TArray<AActor*> FoundCells;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridCell::StaticClass(), FoundCells);

    for (AActor* Actor : FoundCells)
    {
        AGridCell* Cell = Cast<AGridCell>(Actor);
        if (Cell && Cell->GridX == X && Cell->GridY == Y)
        {
            return Cell;
        }
    }
    return nullptr;
}

void AStrategicoGameMode::ResetUnitsForNextTurn()
{
    // Ottiene tutte le unità presenti nel livello
    TArray<AActor*> FoundUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundUnits);

    for (AActor* Actor : FoundUnits)
    {
        AUnit* Unit = Cast<AUnit>(Actor);
        if (Unit)
        {
            // Reimposta i flag di azione
            Unit->bHaMosso = false;
            Unit->bHaAttaccato = false;

            // Forza lo stato a Idle
            Unit->CurrentActionState = EUnitActionState::Idle;

            // Se l'unità stava ancora animando un movimento, la interrompe
            Unit->bIsMoving = false;
            Unit->MovementPath.Empty();
            Unit->CurrentPathIndex = 0;

            // Rimuovi eventuali highlight
            Unit->HideRanges();
        }
    }

    // Svuota ActiveUnit in modo che nessuna unità risulti selezionata all'inizio del nuovo turno
    ActiveUnit = nullptr;

    UE_LOG(LogTemp, Warning, TEXT("Tutte le unità sono state resettate per il nuovo turno; ActiveUnit = nullptr."));
}

TArray<AUnit*> AStrategicoGameMode::GetAllAIUnits() const
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundActors);
    TArray<AUnit*> AIUnits;
    for (AActor* Actor : FoundActors)
    {
        AUnit* Unit = Cast<AUnit>(Actor);
        if (Unit && Unit->UnitTeam == EUnitTeam::AI)
        {
            AIUnits.Add(Unit);
        }
    }
    return AIUnits;
}

AUnit* AStrategicoGameMode::FindClosestEnemyUnit(AUnit* AIUnit) const
{
    if (!AIUnit) return nullptr;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundActors);
    AUnit* ClosestEnemy = nullptr;
    float MinDistance = FLT_MAX;
    FVector MyLocation = AIUnit->GetActorLocation();

    for (AActor* Actor : FoundActors)
    {
        AUnit* OtherUnit = Cast<AUnit>(Actor);
        // Considera solo le unità del giocatore come nemiche
        if (OtherUnit && OtherUnit->UnitTeam == EUnitTeam::Player && OtherUnit->CurrentCell)
        {
            float Distance = FVector::Dist(MyLocation, OtherUnit->GetActorLocation());
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                ClosestEnemy = OtherUnit;
            }
        }
    }
    return ClosestEnemy;
}

bool AStrategicoGameMode::IsEnemyInAttackRange(AUnit* AIUnit) const
{
    if (!AIUnit || !AIUnit->CurrentCell) return false;

    AUnit* Enemy = GetEnemyInAttackRange(AIUnit);
    return (Enemy != nullptr);
}

AUnit* AStrategicoGameMode::GetEnemyInAttackRange(AUnit* AIUnit) const
{
    if (!AIUnit || !AIUnit->CurrentCell)
        return nullptr;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundActors);
    AUnit* ClosestEnemy = nullptr;
    int32 MinDistance = TNumericLimits<int32>::Max(); // usiamo un intero per la distanza Manhattan

    for (AActor* Actor : FoundActors)
    {
        AUnit* OtherUnit = Cast<AUnit>(Actor);
        if (OtherUnit && OtherUnit->UnitTeam == EUnitTeam::Player && OtherUnit->CurrentCell)
        {
            // Calcoliamo la distanza Manhattan tra le due celle
            int32 DistX = FMath::Abs(AIUnit->CurrentCell->GridX - OtherUnit->CurrentCell->GridX);
            int32 DistY = FMath::Abs(AIUnit->CurrentCell->GridY - OtherUnit->CurrentCell->GridY);
            int32 ManhattanDistance = DistX + DistY;

            // Confronto col RangeAttacco
            if (ManhattanDistance <= AIUnit->RangeAttacco)
            {
                // Se è più vicino di tutti i nemici precedenti, aggiorniamo
                if (ManhattanDistance < MinDistance)
                {
                    MinDistance = ManhattanDistance;
                    ClosestEnemy = OtherUnit;
                }
            }
        }
    }
    return ClosestEnemy;
}

void AStrategicoGameMode::AIMovementCompleted()
{
    AIMovingUnitsCount--;
    UE_LOG(LogTemp, Warning, TEXT("Un'unità IA ha completato il movimento. Contatore: %d"), AIMovingUnitsCount);
    // Se tutte le unità IA hanno completato il movimento, passa il turno al giocatore
    if (AIMovingUnitsCount <= 0)
    {
        NextTurn();
    }
}

void AStrategicoGameMode::ProcessNextAIMovement()
{
    // Se non ci sono più unità in coda, passa il turno al giocatore.
    if (PendingAIMovementUnits.Num() == 0)
    {
        NextTurn();
        return;
    }

    // Prendi la prima unità dalla coda e rimuovila.
    AUnit* AIUnit = PendingAIMovementUnits[0];
    PendingAIMovementUnits.RemoveAt(0);

    if (!AIUnit || AIUnit->bHaMosso)
    {
        ProcessNextAIMovement();
        return;
    }

    // Recupera la cella target pendente salvata nell'unità.
    AGridCell* TargetCell = AIUnit->PendingTargetCell;
    if (!TargetCell)
    {
        ProcessNextAIMovement();
        return;
    }

    // Calcola il percorso dalla cella corrente dell'unità al TargetCell.
    TArray<AGridCell*> FullPath = ComputePath(AIUnit->CurrentCell, TargetCell, AIUnit);
    if (FullPath.Num() > 0)
    {
        // Il percorso restituito include la cella di partenza all'indice 0,
        // quindi i passi effettivi sono (FullPath.Num() - 1).
        int32 StepsAvailable = FullPath.Num() - 1;

        // Se il percorso termina in una cella occupata da un'unità del giocatore, rimuovi quell'ultimo passo
        if (FullPath.Num() > 1)
        {
            AGridCell* LastCell = FullPath.Last();
            if (LastCell->bOccupied && LastCell->OccupyingUnit && LastCell->OccupyingUnit->UnitTeam == EUnitTeam::Player)
            {
                StepsAvailable = FullPath.Num() - 2;
            }
        }

        // Calcola quanti passi può effettivamente compiere l'unità.
        int32 StepsToTake = FMath::Min(StepsAvailable, AIUnit->MovimentoMax);

        // Costruisci il LimitedPath saltando la cella di partenza (indice 0).
        TArray<AGridCell*> LimitedPath;
        for (int32 i = 1; i <= StepsToTake; i++)
        {
            LimitedPath.Add(FullPath[i]);
        }

        UE_LOG(LogTemp, Warning, TEXT("AIUnit %s muove %d passi"), *AIUnit->GetName(), LimitedPath.Num());
        AIUnit->AnimateMovementAlongPath(LimitedPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Percorso per AIUnit %s non trovato in ProcessNextAIMovement."), *AIUnit->GetName());
        ProcessNextAIMovement();
    }
}

void AStrategicoGameMode::UpdateHUDHealthValues()
{
    FString Health1 = "";
    FString Health2 = "";
    FString Health3 = "";
    FString Health4 = "";

    TArray<AActor*> FoundUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundUnits);

    int32 PlayerCount = 0;
    int32 AICount = 0;
    for (AActor* Actor : FoundUnits)
    {
        AUnit* Unit = Cast<AUnit>(Actor);
        if (!Unit)
            continue;

        FString UnitHealth;
        if (Unit->Vita <= 0)
        {
            UnitHealth = FString::Printf(TEXT("%s: Morto"), *Unit->UnitDisplayName);
        }
        else
        {
            int32 DisplayVita = FMath::Max(Unit->Vita, 0);
            UnitHealth = FString::Printf(TEXT("%s: %d/%d"), *Unit->UnitDisplayName, DisplayVita, Unit->VitaMax);
        }

        if (Unit->UnitTeam == EUnitTeam::Player)
        {
            if (PlayerCount == 0)
                Health1 = UnitHealth;
            else if (PlayerCount == 1)
                Health2 = UnitHealth;
            PlayerCount++;
        }
        else if (Unit->UnitTeam == EUnitTeam::AI)
        {
            if (AICount == 0)
                Health3 = UnitHealth;
            else if (AICount == 1)
                Health4 = UnitHealth;
            AICount++;
        }
    }

    if (HUD)
    {
        HUD->UpdateHealthTexts(Health1, Health2, Health3, Health4);
    }
}

void AStrategicoGameMode::HideGrid()
{
    // Nascondi tutte le celle della griglia
    TArray<AActor*> FoundCells;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridCell::StaticClass(), FoundCells);
    for (AActor* Actor : FoundCells)
    {
        AGridCell* Cell = Cast<AGridCell>(Actor);
        if (Cell)
        {
            Cell->SetActorHiddenInGame(true);
            Cell->SetActorEnableCollision(false);
        }
    }

    // Ora nascondi tutte le unità
    TArray<AActor*> FoundUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundUnits);
    for (AActor* Actor : FoundUnits)
    {
        AUnit* Unit = Cast<AUnit>(Actor);
        if (Unit)
        {
            Unit->SetActorHiddenInGame(true);
            Unit->SetActorEnableCollision(false);
        }
    }

    if (HUD)
    {
        HUD->HideHealthTexts();
    }
}

void AStrategicoGameMode::CheckForGameOver()
{
    // Ottieni tutte le unità attualmente presenti
    TArray<AActor*> FoundUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUnit::StaticClass(), FoundUnits);

    int32 PlayerAlive = 0;
    int32 AIAlive = 0;

    for (AActor* Actor : FoundUnits)
    {
        AUnit* Unit = Cast<AUnit>(Actor);
        if (Unit)
        {
            // Considera un'unità viva se Vita > 0 (o eventualmente se non è contrassegnata come morta)
            if (Unit->Vita > 0)
            {
                if (Unit->UnitTeam == EUnitTeam::Player)
                {
                    PlayerAlive++;
                }
                else if (Unit->UnitTeam == EUnitTeam::AI)
                {
                    AIAlive++;
                }
            }
        }
    }

    // Se tutte le unità del giocatore sono morte:
    if (PlayerAlive == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Game Over: Hai perso!"));
        if (HUD)
        {
            HUD->AppendMoveHistory(TEXT("Game Over: Hai perso!"));
            HUD->ShowGameOverMessage(FText::FromString("GAME OVER - HAI PERSO!"));
        }

        if (LoseSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), LoseSound, 0.3f);
        }

        // Disabilita l'input e pausa il gioco
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            PC->DisableInput(PC);
        }
        UGameplayStatics::SetGamePaused(GetWorld(), true);

        HideGrid();
    }
    // Se tutte le unità dell'IA sono morte:
    else if (AIAlive == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Game Over: Hai vinto!"));
        if (HUD)
        {
            HUD->AppendMoveHistory(TEXT("Game Over: Hai vinto!"));
            HUD->ShowGameOverMessage(FText::FromString("GAME OVER - HAI VINTO!"));
        }

        if (VictorySound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), VictorySound, 0.5f);
        }

        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            PC->DisableInput(PC);
        }
        UGameplayStatics::SetGamePaused(GetWorld(), true);

        HideGrid();
    }
}

FString AStrategicoGameMode::GetGridCellIdentifier(AGridCell* Cell) const
{
    if (!Cell)
    {
        return FString("Unknown");
    }

    // Supponiamo che la griglia abbia 25 righe (GridY da 0 a 24)
    const int32 GridHeight = 25;

    // Le colonne si mappano come 'A', 'B', ... (supponendo che GridX sia 0-indexato)
    TCHAR Letter = 'A' + Cell->GridX;

    // Inverti il calcolo per il numero: se Cell->GridY è grande, il numero è piccolo.
    int32 Number = GridHeight - Cell->GridY;

    return FString::Printf(TEXT("%c%d"), Letter, Number);
}

