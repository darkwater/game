#include "cbase.h"
#include "weapon_csbasegun.h"
#include "fx_cs_shared.h"
#include "mom_player_shared.h"
#include "weapon_mom_shotgun.h"

#if defined( CLIENT_DLL )
#define CWeaponM249 C_WeaponM249
#define CWeaponM3 C_WeaponM3
#define CWeaponXM1014 C_WeaponXM1014
#else
#include "momentum/te_shotgun_shot.h"
#endif

#include "tier0/memdbgon.h"

//M3
class CWeaponM3 : public CMomentumShotgun
{
public:
    DECLARE_CLASS(CWeaponM3, CMomentumShotgun);
    DECLARE_PREDICTABLE();

    CWeaponM3() {}
};

BEGIN_PREDICTION_DATA(CWeaponM3)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m3, CWeaponM3);
PRECACHE_WEAPON_REGISTER(weapon_m3);

//M249
class CWeaponM249 : public CWeaponCSBaseGun
{
public:
    DECLARE_CLASS(CWeaponM249, CWeaponCSBaseGun);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    CWeaponM249() {}

    virtual void PrimaryAttack();

    virtual CSWeaponID GetWeaponID(void) const { return WEAPON_M249; }


private:

    CWeaponM249(const CWeaponM249 &);

    void M249Fire(float flSpread);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponM249, DT_WeaponM249)

BEGIN_NETWORK_TABLE(CWeaponM249, DT_WeaponM249)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponM249)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_m249, CWeaponM249);
PRECACHE_WEAPON_REGISTER(weapon_m249);


void CWeaponM249::PrimaryAttack(void)
{
    CMomentumPlayer *pPlayer = GetPlayerOwner();
    if (!pPlayer)
        return;

    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        M249Fire(0.045f + 0.5f * m_flAccuracy);
    else if (pPlayer->GetAbsVelocity().Length2D() > 140)
        M249Fire(0.045f + 0.095f * m_flAccuracy);
    else
        M249Fire(0.03f * m_flAccuracy);
}

void CWeaponM249::M249Fire(float flSpread)
{
    if (!CSBaseGunFire(flSpread, GetCSWpnData().m_flCycleTime, true))
        return;

    CMomentumPlayer *pPlayer = GetPlayerOwner();

    // CSBaseGunFire can kill us, forcing us to drop our weapon, if we shoot something that explodes
    if (!pPlayer)
        return;

    if (!FBitSet(pPlayer->GetFlags(), FL_ONGROUND))
        pPlayer->KickBack(1.8, 0.65, 0.45, 0.125, 5, 3.5, 8);

    else if (pPlayer->GetAbsVelocity().Length2D() > 5)
        pPlayer->KickBack(1.1, 0.5, 0.3, 0.06, 4, 3, 8);

    else if (FBitSet(pPlayer->GetFlags(), FL_DUCKING))
        pPlayer->KickBack(0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9);

    else
        pPlayer->KickBack(0.8, 0.35, 0.3, 0.03, 3.75, 3, 9);
}

//XM1014
class CWeaponXM1014 : public CMomentumShotgun
{
public:
    DECLARE_CLASS(CWeaponXM1014, CMomentumShotgun);
    DECLARE_PREDICTABLE();

    CWeaponXM1014() {}
};

BEGIN_PREDICTION_DATA(CWeaponXM1014)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_xm1014, CWeaponXM1014);
PRECACHE_WEAPON_REGISTER(weapon_xm1014);