#include "StrategicoPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "StrategicoGameMode.h"

void AStrategicoPlayerController::OnEndTurnPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("OnEndTurnPressed() chiamato"));

    // Recupera il GameMode corrente
    AStrategicoGameMode* GM = Cast<AStrategicoGameMode>(UGameplayStatics::GetGameMode(this));
    if (GM)
    {
        // Se siamo nella fase di Placement, ignora l'input
        if (GM->CurrentGamePhase == EGamePhase::Placement)
        {
            UE_LOG(LogTemp, Warning, TEXT("Non puoi terminare il turno durante la fase di Placement."));
            return;
        }
        // Se non è il turno del giocatore, ignora l'input
        if (GM->TurnOwner != ETurnOwner::Player)
        {
            UE_LOG(LogTemp, Warning, TEXT("Non puoi terminare il turno: attualmente è il turno dell'IA."));
            return;
        }

        // Se è il turno del giocatore e siamo in InGame, chiama NextTurn()
        GM->NextTurn();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnEndTurnPressed: GameMode non trovato o cast fallito."));
    }
}

void AStrategicoPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (InputComponent)
    {
        // Bind dell'azione "EndTurn"
        InputComponent->BindAction("EndTurn", IE_Pressed, this, &AStrategicoPlayerController::OnEndTurnPressed);

        // Bind degli assi per il movimento della camera
        InputComponent->BindAxis("MoveForward", this, &AStrategicoPlayerController::MoveForward);
        InputComponent->BindAxis("MoveRight", this, &AStrategicoPlayerController::MoveRight);
    }
}

void AStrategicoPlayerController::MoveForward(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;

    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn)
    {
        // Ottiene il vettore forward, azzerando la componente Z per muoversi solo sul piano
        FVector Forward = ControlledPawn->GetActorForwardVector();
        Forward.Z = 0.0f;
        Forward.Normalize();

        // Calcola lo spostamento: velocità * valore dell'input * DeltaTime
        float DeltaSeconds = GetWorld()->GetDeltaSeconds();
        FVector DeltaLocation = Forward * Value * CameraMovementSpeed * DeltaSeconds;

        ControlledPawn->AddActorWorldOffset(DeltaLocation, true);
    }
}

void AStrategicoPlayerController::MoveRight(float Value)
{
    if (FMath::IsNearlyZero(Value)) return;

    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn)
    {
        // Ottiene il vettore right, azzerando la componente Z
        FVector Right = ControlledPawn->GetActorRightVector();
        Right.Z = 0.0f;
        Right.Normalize();

        float DeltaSeconds = GetWorld()->GetDeltaSeconds();
        FVector DeltaLocation = Right * Value * CameraMovementSpeed * DeltaSeconds;

        ControlledPawn->AddActorWorldOffset(DeltaLocation, true);
    }
}
