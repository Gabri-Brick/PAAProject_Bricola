#include "CameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ACameraPawn::ACameraPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    // Crea un componente di root (opzionale, ma consigliato)
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootScene;

    // Crea il SpringArm (Camera Boom) per gestire la distanza della telecamera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 3000.0f;  // Imposta l'altezza/distanza; aggiusta questo valore in base alle dimensioni della griglia
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;
    SpringArm->bDoCollisionTest = false; // Disabilita il test di collisione per mantenere la telecamera fissa
    // Ruota il braccio per puntare la telecamera direttamente verso il basso
    SpringArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, -90.0f));

    // Crea il componente Camera e attaccalo al SpringArm
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
    CameraComponent->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false;
}

void ACameraPawn::ZoomCamera(float AxisValue)
{
 

    if (FMath::IsNearlyZero(AxisValue))
    {
        return;  // Se AxisValue è zero, non fare nulla.
    }

    // Imposta la velocità dello zoom (puoi modificarla in base alle tue necessità)
    float ZoomSpeed = 100.0f;

    // Calcola il nuovo TargetArmLength; se AxisValue è positivo, zoom in (diminuisce la distanza)
    float NewArmLength = SpringArm->TargetArmLength - AxisValue * ZoomSpeed;

    // Limita il TargetArmLength in un range desiderato, per esempio da 500 a 5000 unità
    NewArmLength = FMath::Clamp(NewArmLength, 500.0f, 5000.0f);

    // Applica il nuovo valore
    SpringArm->TargetArmLength = NewArmLength;
}

void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Associa l'input della rotellina (Axis) alla funzione ZoomCamera.
    PlayerInputComponent->BindAxis("MouseWheel", this, &ACameraPawn::ZoomCamera);
}


