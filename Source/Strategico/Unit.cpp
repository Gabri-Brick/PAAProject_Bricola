#include "Unit.h"
#include "GridCell.h"
#include "StrategicoGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AUnit::AUnit()
{
    //Proprietà di utilità e mesh
    PrimaryActorTick.bCanEverTick = true;
    CurrentCell = nullptr;
    UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
    RootComponent = UnitMesh;
    UnitMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    UnitMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    UnitMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (MeshAsset.Succeeded())
    {
        UnitMesh->SetStaticMesh(MeshAsset.Object);
        UnitMesh->SetWorldScale3D(FVector(0.88f, 0.88f, 0.1f));
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MovMat(TEXT("/Game/Materiali/M_Movimento.M_Movimento"));
    if (MovMat.Succeeded())
    {
        MovementHighlightMaterial = MovMat.Object;
        UE_LOG(LogTemp, Warning, TEXT("M_Movimento caricato correttamente."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento di M_Movimento."));
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> AttackMat(TEXT("/Game/Materiali/M_Attacco.M_Attacco"));
    if (AttackMat.Succeeded())
    {
        AttackHighlightMaterial = AttackMat.Object;
        UE_LOG(LogTemp, Warning, TEXT("M_Attacco caricato correttamente."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento di M_Attacco."));
    }

    //Valori di default sovrascritti dalle effettive unità figlie della classe base
    MovimentoMax = 3;
    AttackType = EAttackType::Melee;
    RangeAttacco = 1;
    DannoMin = 1;
    DannoMax = 1;
    VitaMax = 100;
    Vita = 100;
    bHaMosso = false;
    bHaAttaccato = false;
    UnitTeam = EUnitTeam::Player;  
    CurrentActionState = EUnitActionState::Idle;
    UnitMesh->OnClicked.AddDynamic(this, &AUnit::OnUnitClicked);
    UnitDisplayName = TEXT("");
    bIsMoving = false;
    CurrentPathIndex = 0;
    MovementSpeed = 300.f;

 //Costruttori per i suoni
    static ConstructorHelpers::FObjectFinder<USoundBase> PunchSound(TEXT("/Game/Suoni/Punch"));
    if (PunchSound.Succeeded())
    {
        PunchSoundEffect = PunchSound.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Impossibile caricare il suono Punch"));
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> SniperSound(TEXT("/Game/Suoni/Sniper_Sound"));
    if (SniperSound.Succeeded())
    {
        SniperSoundEffect = SniperSound.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Impossibile caricare il suono Sniper_Sound"));
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> KillConfirmedSoundAsset(TEXT("/Game/Suoni/Kill_Confirmed"));
    if (KillConfirmedSoundAsset.Succeeded())
    {
        KillConfirmedSound = KillConfirmedSoundAsset.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Errore nel caricamento del suono Kill_Confirmed"));
    }
}

void AUnit::ToggleActionState()
{
    // Se l'unità non è Idle, il click la resetta a Idle.
    if (CurrentActionState != EUnitActionState::Idle)
    {
        CurrentActionState = EUnitActionState::Idle;
        HideRanges();
        UE_LOG(LogTemp, Warning, TEXT("Unità torna a Idle (toggle)"));
        return;
    }

    // L'unità è in Idle: scegli il prossimo stato in base a cosa è già stato fatto.
    // Caso 1: nessuna azione eseguita
    if (!bHaMosso && !bHaAttaccato)
    {
        // Alterna in base a LastDesiredActionState (predefinito a Movement)
        if (LastDesiredActionState == EUnitActionState::Movement)
        {
            CurrentActionState = EUnitActionState::Movement;
            UE_LOG(LogTemp, Warning, TEXT("Unità passa a Movimento"));
            ShowMovementRange();
            LastDesiredActionState = EUnitActionState::Attack; // Il prossimo click, da Idle, passerà ad Attacco
        }
        else // LastDesiredActionState == Attack
        {
            CurrentActionState = EUnitActionState::Attack;
            UE_LOG(LogTemp, Warning, TEXT("Unità passa ad Attacco"));
            ShowAttackRange();
            LastDesiredActionState = EUnitActionState::Movement; // Il prossimo click, da Idle, passerà a Movimento
        }
        return;
    }

    // Caso 2: se l'unità ha già mosso ma non attaccato, l'unica opzione è Attacco
    if (bHaMosso && !bHaAttaccato)
    {
        CurrentActionState = EUnitActionState::Attack;
        UE_LOG(LogTemp, Warning, TEXT("Unità passa ad Attacco (movimento già effettuato)"));
        ShowAttackRange();
        return;
    }

    // Caso 3: se l'unità ha già attaccato ma non mosso, l'unica opzione è Movimento
    if (!bHaMosso && bHaAttaccato)
    {
        CurrentActionState = EUnitActionState::Movement;
        UE_LOG(LogTemp, Warning, TEXT("Unità passa a Movimento (attacco già effettuato)"));
        ShowMovementRange();
        return;
    }

    // Caso 4: se entrambe le azioni sono state effettuate, il click mantiene l'unità in Idle.
    if (bHaMosso && bHaAttaccato)
    {
        CurrentActionState = EUnitActionState::Idle;
        HideRanges();
        UE_LOG(LogTemp, Warning, TEXT("L'unità ha esaurito le azioni del turno."));
        return;
    }

    // Fallback (non dovrebbe essere raggiunto)
    CurrentActionState = EUnitActionState::Idle;
    HideRanges();
}

void AUnit::ShowMovementRange()
{
    // Rimuove eventuali highlight precedenti
    HideRanges();

    UE_LOG(LogTemp, Warning, TEXT("Evidenzia range movimento per unità: %s"), *GetName());

    // Se non ho una CurrentCell valida, esco
    if (!CurrentCell)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità %s non ha una CurrentCell. Impossibile calcolare il range."), *GetName());
        return;
    }

    // Recupera il GameMode (per la GetGridCellAt)
    AStrategicoGameMode* GM = Cast<AStrategicoGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode non trovato, ShowMovementRange annullato."));
        return;
    }

    // Struttura per BFS
    struct FCellNode
    {
        AGridCell* Cell;
        int32 Distance;
    };

    TQueue<FCellNode> Queue;
    TSet<AGridCell*> Visited;

    // Enqueue la cella di partenza
    Queue.Enqueue({ CurrentCell, 0 });
    Visited.Add(CurrentCell);

    TArray<AGridCell*> ReachableCells;

    while (!Queue.IsEmpty())
    {
        FCellNode Node;
        Queue.Dequeue(Node);

        // Se la cella non è quella di partenza (Node.Distance > 0), aggiungi al range
        if (Node.Distance > 0)
        {
            ReachableCells.Add(Node.Cell);
        }

        // Se abbiamo già raggiunto il limite di movimento
        if (Node.Distance >= MovimentoMax)
            continue;

        // Controlla i quattro vicini
        int32 X = Node.Cell->GridX;
        int32 Y = Node.Cell->GridY;

        TArray<FIntPoint> Neighbors = {
            FIntPoint(X + 1, Y),
            FIntPoint(X - 1, Y),
            FIntPoint(X, Y + 1),
            FIntPoint(X, Y - 1)
        };

        for (FIntPoint N : Neighbors)
        {
            AGridCell* NeighborCell = GM->GetGridCellAt(N.X, N.Y);
            if (!NeighborCell) continue;

            // Se già visitata, skip
            if (Visited.Contains(NeighborCell))
                continue;

            // Se è ostacolo, skip
            if (NeighborCell->bIsObstacle)
                continue;

            // Se è occupata da un'altra unità, skip
            if (NeighborCell->bOccupied && NeighborCell->OccupyingUnit != this)
                continue;

            // Altrimenti la aggiungiamo
            Queue.Enqueue({ NeighborCell, Node.Distance + 1 });
            Visited.Add(NeighborCell);
        }
    }

    // Ora evidenziamo le celle in ReachableCells
    HighlightedCells = ReachableCells;
    for (AGridCell* Cell : ReachableCells)
    {
        if (MovementHighlightMaterial)
        {
            Cell->SetCellMaterial(MovementHighlightMaterial);
            Cell->bMovementHighlighted = true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Evidenziate %d celle raggiungibili."), ReachableCells.Num());
}

void AUnit::ShowAttackRange()
{
    // Nascondi highlight precedenti
    HideRanges();

    UE_LOG(LogTemp, Warning, TEXT("Evidenzia range attacco per unità: %s"), *GetName());

    // Se non ho la cella corrente
    if (!CurrentCell)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità %s non ha CurrentCell, impossibile calcolare l'attacco."), *GetName());
        return;
    }

    // Ottieni il GameMode (per GetGridCellAt)
    AStrategicoGameMode* GM = Cast<AStrategicoGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode non trovato, ShowAttackRange annullato."));
        return;
    }

    // Struttura BFS
    struct FCellNode
    {
        AGridCell* Cell;
        int32 Distance;
    };

    TQueue<FCellNode> Queue;
    TSet<AGridCell*> Visited;

    // Partiamo dalla cella attuale
    Queue.Enqueue({ CurrentCell, 0 });
    Visited.Add(CurrentCell);

    TArray<AGridCell*> ReachableCells;

    while (!Queue.IsEmpty())
    {
        FCellNode Node;
        Queue.Dequeue(Node);

        // Se non è la cella di partenza, la aggiungo come potenziale bersaglio
        if (Node.Distance > 0)
        {
            ReachableCells.Add(Node.Cell);
        }

        // Se abbiamo raggiunto il RangeAttacco massimo, non proseguiamo
        if (Node.Distance >= RangeAttacco)
            continue;

        // Controlla i 4 vicini
        int32 X = Node.Cell->GridX;
        int32 Y = Node.Cell->GridY;
        TArray<FIntPoint> Neighbors = {
            FIntPoint(X + 1, Y),
            FIntPoint(X - 1, Y),
            FIntPoint(X,     Y + 1),
            FIntPoint(X,     Y - 1)
        };

        for (FIntPoint Coord : Neighbors)
        {
            AGridCell* NeighborCell = GM->GetGridCellAt(Coord.X, Coord.Y);
            if (!NeighborCell)
                continue;

            // Se già visitata, skip
            if (Visited.Contains(NeighborCell))
                continue;

            // Se AttackType == Melee e la cella è un ostacolo, skip
            // (per lo Sniper Ranged ignoriamo gli ostacoli)
            if (AttackType == EAttackType::Melee && NeighborCell->bIsObstacle)
            {
                Visited.Add(NeighborCell);
                continue;
            }

            // Se Occupata da un nemico, la includiamo comunque
            // Se occupata da me o un alleato, proseguiamo pure la BFS
            // (qui facciamo la BFS “libera”)
            if (NeighborCell->bOccupied)
            {
                // Se c'è un nemico, la aggiungo
                if (NeighborCell->OccupyingUnit &&
                    NeighborCell->OccupyingUnit->UnitTeam != this->UnitTeam)
                {
                    ReachableCells.Add(NeighborCell);
                }
            }

            // Aggiungo alla coda BFS con distance+1
            Queue.Enqueue({ NeighborCell, Node.Distance + 1 });
            Visited.Add(NeighborCell);
        }
    }

    // Evidenzio tutte le celle “colpibili”
    HighlightedCells = ReachableCells;
    for (AGridCell* Cell : ReachableCells)
    {
        if (AttackHighlightMaterial)
        {
            Cell->SetCellMaterial(AttackHighlightMaterial);
            Cell->bAttackHighlighted = true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Evidenziate %d celle per attacco (sniper ignora tutto)."), ReachableCells.Num());
}

void AUnit::HideRanges()
{
    for (AGridCell* Cell : HighlightedCells)
    {
        if (Cell)
        {
            // Se la cella è un ostacolo, ripristina il materiale ostacolo
            if (Cell->bIsObstacle)
            {
                if (Cell->ObstacleMaterial)
                {
                    Cell->SetCellMaterial(Cell->ObstacleMaterial);
                }
                else if (Cell->OriginalMaterial)
                {
                    Cell->SetCellMaterial(Cell->OriginalMaterial);
                }
            }
            else
            {
                // Se la cella non è un ostacolo, usa il materiale originale
                if (Cell->OriginalMaterial)
                {
                    Cell->SetCellMaterial(Cell->OriginalMaterial);
                }
                else if (Cell->BaseMaterial)
                {
                    Cell->SetCellMaterial(Cell->BaseMaterial);
                }
            }
            // Reset dei flag di highlight
            Cell->bMovementHighlighted = false;
            Cell->bAttackHighlighted = false;
        }
    }
    HighlightedCells.Empty();
    UE_LOG(LogTemp, Warning, TEXT("Highlight rimosso."));
}

void AUnit::SetUnitMaterial(UMaterialInterface* NewMaterial)
{
    if (UnitMesh && NewMaterial)
    {
        UnitMesh->SetMaterial(0, NewMaterial);
        UnitMaterial = NewMaterial;
        UE_LOG(LogTemp, Warning, TEXT("Materiale assegnato all'unità: %s"), *NewMaterial->GetName());
    }
}

int32 AUnit::Attack(AUnit* TargetUnit)
{
    // Controllo iniziale (se ha già attaccato o se non c'è target)
    if (bHaAttaccato)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attacco già effettuato in questo turno."));
        return 0;
    }
    if (!TargetUnit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attacco fallito: nessun target"));
        return 0;
    }

    // Evidenzia la cella del bersaglio
    if (TargetUnit->CurrentCell)
    {
        AGridCell* cell = TargetUnit->CurrentCell;
        if (AttackHighlightMaterial)
        {
            cell->SetCellMaterial(AttackHighlightMaterial);
            cell->bAttackHighlighted = true;
        }
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [cell]()
            {
                if (cell)
                {
                    if (cell->OriginalMaterial)
                        cell->SetCellMaterial(cell->OriginalMaterial);
                    else if (cell->BaseMaterial)
                        cell->SetCellMaterial(cell->BaseMaterial);
                    cell->bAttackHighlighted = false;
                }
            }, 2.0f, false);
    }

    // Calcola il danno d'attacco.
    int32 danno = FMath::RandRange(DannoMin, DannoMax);
    TargetUnit->Vita -= danno;

    // Riproduci il suono in base al tipo di attacco:
    if (AttackType == EAttackType::Melee && PunchSoundEffect)
    {
        UGameplayStatics::PlaySound2D(this, PunchSoundEffect, 0.5f);
    }
    else if (AttackType == EAttackType::Ranged && SniperSoundEffect)
    {
        UGameplayStatics::PlaySound2D(this, SniperSoundEffect, 0.5f);
    }

    // Recupera il GameMode e aggiorna l'HUD.
    AStrategicoGameMode* GM = Cast<AStrategicoGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM && GM->HUD)
    {
        FString AttackMsg = FString::Printf(TEXT("%s attacca %s, infliggendo %d danni"),
            *UnitDisplayName, *TargetUnit->UnitDisplayName, danno);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *AttackMsg);
        GM->HUD->AppendMoveHistory(AttackMsg);
        GM->UpdateHUDHealthValues();
        GM->CheckForGameOver();  // Controllo dopo l'attacco
    }

    // Se il target viene eliminato, libera la cella, aggiorna l'HUD e distruggi l'unità.
    if (TargetUnit->Vita <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unità eliminata!"));
        if (TargetUnit->CurrentCell)
        {
            TargetUnit->CurrentCell->bOccupied = false;
            TargetUnit->CurrentCell->OccupyingUnit = nullptr;
        }
        if (GM && GM->HUD)
        {
            FString ElimMsg = FString::Printf(TEXT("Unità %s eliminata!"), *TargetUnit->UnitDisplayName);
            GM->HUD->AppendMoveHistory(ElimMsg);
            GM->UpdateHUDHealthValues();
            GM->CheckForGameOver();
        }

        if (KillConfirmedSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), KillConfirmedSound);
        }

        TargetUnit->Destroy();
    }

    // Logica di controattacco
    if (this->AttackType == EAttackType::Ranged)
    {
        bool bCounterAttack = false;
        if (TargetUnit->AttackType == EAttackType::Ranged)
        {
            bCounterAttack = true;
        }
        else if (TargetUnit->AttackType == EAttackType::Melee)
        {
            if (TargetUnit->CurrentCell && this->CurrentCell)
            {
                int32 dx = FMath::Abs(TargetUnit->CurrentCell->GridX - this->CurrentCell->GridX);
                int32 dy = FMath::Abs(TargetUnit->CurrentCell->GridY - this->CurrentCell->GridY);
                if (dx + dy == 1)
                {
                    bCounterAttack = true;
                }
            }
        }
        if (bCounterAttack)
        {
            int32 CounterDamage = FMath::RandRange(1, 3);
            this->Vita -= CounterDamage;
            FString CounterMsg = FString::Printf(TEXT("Controattacco subito: %s subisce %d danni da %s"),
                *UnitDisplayName, CounterDamage, *TargetUnit->UnitDisplayName);
            UE_LOG(LogTemp, Warning, TEXT("%s"), *CounterMsg);
            if (GM && GM->HUD)
            {
                GM->HUD->AppendMoveHistory(CounterMsg);
                GM->UpdateHUDHealthValues();
                GM->CheckForGameOver();
            }
            if (this->Vita <= 0)
            {
                UE_LOG(LogTemp, Warning, TEXT("Lo Sniper attaccante è stato eliminato dal controattacco!"));
                if (this->CurrentCell)
                {
                    this->CurrentCell->bOccupied = false;
                    this->CurrentCell->OccupyingUnit = nullptr;
                }
                this->Vita = 0;
                if (GM && GM->HUD)
                {
                    GM->UpdateHUDHealthValues();
                    GM->CheckForGameOver();
                    FString DeathMsg = FString::Printf(TEXT("Unità %s eliminata dal controattacco!"), *UnitDisplayName);
                    GM->HUD->AppendMoveHistory(DeathMsg);
                }
              
                if (KillConfirmedSound)
                {
                    UGameplayStatics::PlaySound2D(GetWorld(), KillConfirmedSound);
                }

                FTimerHandle SelfDeathTimerHandle;
                GetWorld()->GetTimerManager().SetTimer(SelfDeathTimerHandle, [this]()
                    {
                        this->Destroy();
                    }, 2.0f, false);
            }
        }
    }

    bHaAttaccato = true;
    return danno;
}

void AUnit::OnUnitClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    // Recupera il GameMode
    AStrategicoGameMode* GM = Cast<AStrategicoGameMode>(UGameplayStatics::GetGameMode(this));
    if (!GM)
    {
        UE_LOG(LogTemp, Warning, TEXT("GameMode non trovato."));
        return;
    }

    // Se l'unità cliccata NON appartiene al giocatore, si tratta di una unità nemica:
    if (UnitTeam != EUnitTeam::Player)
    {
        // Verifica che esista già un'unità attiva (amica) in stato di Attack
        if (!GM->ActiveUnit || GM->ActiveUnit->CurrentActionState != EUnitActionState::Attack)
        {
            UE_LOG(LogTemp, Warning, TEXT("Nessuna unità amica in stato di attacco attiva."));
            return;
        }

        // Verifica che l'unità nemica sia in range: controlla se la cella della unità nemica
        // (ossia this->CurrentCell) è tra le celle evidenziate dell'unità attiva.
        if (!GM->ActiveUnit->HighlightedCells.Contains(this->CurrentCell))
        {
            UE_LOG(LogTemp, Warning, TEXT("L'unità nemica non è in range d'attacco."));
            return;
        }

        // Esegui l'attacco: l'unità attiva attacca questa unità nemica
        int32 Damage = GM->ActiveUnit->Attack(this);
        UE_LOG(LogTemp, Warning, TEXT("Attacco eseguito: danno inflitto = %d"), Damage);

        // Dopo l'attacco, resetta lo stato dell'unità attiva: passa a Idle, nascondi gli highlight
        GM->ActiveUnit->CurrentActionState = EUnitActionState::Idle;
        GM->ActiveUnit->HideRanges();
        GM->ActiveUnit = nullptr;

        return;
    }

    // Se invece la unità cliccata appartiene al giocatore, continua con la logica di selezione/toggle:
    if (GM->TurnOwner != ETurnOwner::Player)
    {
        UE_LOG(LogTemp, Warning, TEXT("Non è il tuo turno."));
        return;
    }
    if (GM->CurrentGamePhase == EGamePhase::Placement)
    {
        UE_LOG(LogTemp, Warning, TEXT("Fase di placement: input sull'unità ignorato."));
        return;
    }
    if (GM->ActiveUnit && GM->ActiveUnit != this)
    {
        UE_LOG(LogTemp, Warning, TEXT("Un'altra unità è già attiva."));
        return;
    }
    // Imposta questa unità come attiva
    GM->ActiveUnit = this;

    // Alterna lo stato dell'unità (Idle -> Movement -> Attack -> Idle)
    ToggleActionState();

    // Se, dopo il toggle, l'unità torna allo stato Idle, resetta ActiveUnit
    if (GM && CurrentActionState == EUnitActionState::Idle)
    {
        GM->ActiveUnit = nullptr;
    }
}

void AUnit::AnimateMovementAlongPath(const TArray<AGridCell*>& Path)
{
    if (Path.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Percorso vuoto, movimento annullato."));
        return;
    }

    MovementPath = Path;

    // Evidenzia la cella target (l'ultima del percorso) con il materiale M_Movimento
    AGridCell* TargetCell = MovementPath.Last();
    if (TargetCell && MovementHighlightMaterial)
    {
        TargetCell->SetCellMaterial(MovementHighlightMaterial);
        TargetCell->bMovementHighlighted = true;
    }

    bIsMoving = true;
    CurrentPathIndex = 0;
    PrimaryActorTick.SetTickFunctionEnable(true);
    UE_LOG(LogTemp, Warning, TEXT("Animazione movimento avviata per %s con %d passi."), *GetName(), MovementPath.Num());
}

void AUnit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMoving && MovementPath.Num() > 0)
    {
        AGridCell* TargetCell = MovementPath[CurrentPathIndex];
        if (!TargetCell)
        {
            bIsMoving = false;
            return;
        }

        FVector TargetLocation = TargetCell->GetActorLocation();
        FVector CurrentLocation = GetActorLocation();

        // Interpolazione lineare costante verso il TargetLocation
        FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MovementSpeed);
        SetActorLocation(NewLocation);

        // Se siamo abbastanza vicini alla cella target, passiamo al prossimo passo
        if (FVector::Dist(NewLocation, TargetLocation) < 5.f) // tolleranza di 5 unità
        {
            CurrentPathIndex++;
            if (CurrentPathIndex >= MovementPath.Num())
            {
                bIsMoving = false;
                OnMovementFinished();
            }
        }
    }
}

void AUnit::OnMovementFinished()
{
    // Salva la cella di partenza prima di aggiornare CurrentCell.
    AGridCell* StartingCell = CurrentCell;

    bHaMosso = true;
    // Usa l'ultima cella del percorso come nuova cella corrente.
    AGridCell* NewCell = MovementPath.Last();

    // Libera la vecchia cella.
    if (CurrentCell)
    {
        CurrentCell->bOccupied = false;
        CurrentCell->OccupyingUnit = nullptr;
    }
    // Aggiorna CurrentCell e segna NewCell come occupata.
    CurrentCell = NewCell;
    if (NewCell)
    {
        NewCell->bOccupied = true;
        NewCell->OccupyingUnit = this;
    }

    // Se la cella target era evidenziata, ripristina il materiale originale.
    if (NewCell && NewCell->bMovementHighlighted)
    {
        if (NewCell->OriginalMaterial)
            NewCell->SetCellMaterial(NewCell->OriginalMaterial);
        else if (NewCell->BaseMaterial)
            NewCell->SetCellMaterial(NewCell->BaseMaterial);
        NewCell->bMovementHighlighted = false;
    }

    MovementPath.Empty();
    CurrentPathIndex = 0;
    bIsMoving = false;
    PrimaryActorTick.SetTickFunctionEnable(false);

    UE_LOG(LogTemp, Warning, TEXT("Movimento completato per %s."), *GetName());

    // Registra lo storico del movimento nell'HUD.
    AStrategicoGameMode* GM = Cast<AStrategicoGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM && GM->HUD && StartingCell && NewCell)
    {
        // Usa la funzione helper per ottenere l'identificatore della cella
        FString StartingIdentifier = GM->GetGridCellIdentifier(StartingCell);
        FString NewIdentifier = GM->GetGridCellIdentifier(NewCell);
        FString MoveMsg = FString::Printf(TEXT("%s si e' mosso da %s a %s"),
            *UnitDisplayName, *StartingIdentifier, *NewIdentifier);
        GM->HUD->AppendMoveHistory(MoveMsg);
    }

    // Riporta l'unità allo stato Idle e nascondi eventuali highlight.
    CurrentActionState = EUnitActionState::Idle;
    HideRanges();

    // Resetta ActiveUnit se questa unità era attiva.
    if (GM && GM->ActiveUnit == this)
    {
        GM->ActiveUnit = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("ActiveUnit resettato per %s."), *GetName());
    }

    // Se l'unità è IA, controlla se ora è in range di attacco e attacca se possibile,
    // quindi avvia il prossimo movimento.
    if (UnitTeam == EUnitTeam::AI && GM)
    {
        if (GM->IsEnemyInAttackRange(this))
        {
            AUnit* target = GM->GetEnemyInAttackRange(this);
            if (target)
            {
                int32 Damage = Attack(target);
                UE_LOG(LogTemp, Warning, TEXT("AIUnit %s attacca dopo il movimento e infligge %d danni"), *GetName(), Damage);
            }
        }
        GM->ProcessNextAIMovement();
    }
}


