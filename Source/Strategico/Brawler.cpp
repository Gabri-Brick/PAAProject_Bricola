#include "Brawler.h"

ABrawler::ABrawler()
{
    // Valori specifici per il Brawler
    MovimentoMax = 6;
    AttackType = EAttackType::Melee;
    RangeAttacco = 1;
    DannoMin = 1;
    DannoMax = 6;
    VitaMax = 40;
    Vita = 40;
}
