#include "Sniper.h"

ASniper::ASniper()
{
    // Valori specifici per lo Sniper
    MovimentoMax = 3;
    AttackType = EAttackType::Ranged;
    RangeAttacco = 10;
    DannoMin = 4;
    DannoMax = 8;
    VitaMax = 20;
    Vita = 20;
}
