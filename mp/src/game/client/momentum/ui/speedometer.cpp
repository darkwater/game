#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include <math.h>
#include "vphysics_interface.h"

using namespace vgui;

static ConVar speedmeter_hvel("mom_speedmeter_hvel", "0", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "If set to 1, doesn't take the vertical velocity component into account.\n", true, 0, true, 1);

static ConVar speedmeter_units("mom_speedmeter_units", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Changes the units of measure of the speedmeter. \n 1: Units per second. \n 2: Kilometers per hour. \n 3: Milles per hour.\n", true, 1, true, 3);

static ConVar speedmeter_draw("mom_drawspeedmeter", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Toggles displaying the speedmeter.\n", true, 0, true, 1);

static ConVar speedmeter_colorize("mom_speedmeter_colorize", "1", FCVAR_CLIENTDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_ARCHIVE,
    "Toggles speedmeter colorization based on acceleration\n", true, 0, true, 1);

class CHudSpeedMeter : public CHudElement, public CHudNumericDisplay
{
    DECLARE_CLASS_SIMPLE(CHudSpeedMeter, CHudNumericDisplay);

public:
    CHudSpeedMeter(const char *pElementName);
    virtual void Init()
    {
        Reset();
    }
    virtual void VidInit()
    {
        Reset();
    }
    virtual void Reset()
    {
        //We set the proper LabelText based on mom_speedmeter_units value
        switch (speedmeter_units.GetInt())
        {
        case 1:
            SetLabelText(L"UPS");
            break;
        case 2:
            SetLabelText(L"KM/H");
            break;
        case 3:
            SetLabelText(L"MPH");
            break;
        default:
            //If its value is not supported, USP is assumed (Even though this shouln't happen as Max and Min values are set)
            SetLabelText(L"UPS");
            break;
        }
        SetDisplayValue(0);
    }
    virtual void OnThink();
    virtual bool ShouldDraw()
    {
        return speedmeter_draw.GetBool() && CHudElement::ShouldDraw();
    }

    bool ShouldColorize()
    {
        return speedmeter_colorize.GetBool();
    }

    Color GetColorFromVariation(float variation, Color normalcolor, Color increasecolor, Color decreasecolor);
private:
    float m_flNextColorizeCheck;
    float m_flLastVelocity;
    const Color m_WHITE = Color::Color(255, 255, 255, 255);
    const Color m_RED = Color::Color(255, 0, 0, 255);
    const Color m_BLUE = Color::Color(0, 0, 255, 255);
};

DECLARE_HUDELEMENT(CHudSpeedMeter);


CHudSpeedMeter::CHudSpeedMeter(const char *pElementName) : CHudElement(pElementName), CHudNumericDisplay(g_pClientMode->GetViewport(), "HudSpeedMeter")
{
    // This is already set for HUD elements, but still...
    SetProportional(true);
    SetKeyBoardInputEnabled(false);
    SetMouseInputEnabled(false);
    SetHiddenBits(HIDEHUD_WEAPONSELECTION);
}

void CHudSpeedMeter::OnThink()
{
    Vector velocity = vec3_origin;
    C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
    if (player) {
        velocity = player->GetLocalVelocity();

        // Remove the vertical component if necessary
        if (!speedmeter_hvel.GetBool())
        {
            velocity.z = 0;
        }

        //Conversions based on https://developer.valvesoftware.com/wiki/Dimensions#Map_Grid_Units:_quick_reference
        float vel = (float)velocity.Length();
        switch (speedmeter_units.GetInt())
        {
        case 1:
            //We do nothing but break out of the switch, as default vel is already in UPS
            SetLabelText(L"UPS");
            break;
        case 2:
            //1 unit = 19.05mm -> 0.01905m -> 0.00001905Km(/s) -> 0.06858Km(/h)
            vel *= 0.06858;
            SetLabelText(L"KM/H");
            break;
        case 3:
            //1 unit = 0.75", 1 mile = 63360. 0.75 / 63360 ~~> 0.00001184"(/s) ~~> 0.04262MPH 
            vel *= 0.04262;
            SetLabelText(L"MPH");
            break;
        default:
            //We do nothing but break out of the switch, as default vel is already in UPS
            SetLabelText(L"UPS");
            break;
        }
        
        if (m_flNextColorizeCheck <= gpGlobals->curtime)
        {
            if (ShouldColorize())
            {
                if (m_flLastVelocity != 0)
                {
                    Color fgColor = GetColorFromVariation(abs(vel) - abs(m_flLastVelocity), m_WHITE, m_BLUE, m_RED);
                    DevMsg("R %i  G %i  B %i\n", fgColor.r(), fgColor.g(), fgColor.b());
                    SetFgColor(fgColor);
                }
                else
                    SetFgColor(m_WHITE);
            }
            else
            {
                SetFgColor(m_WHITE);
            }
            m_flLastVelocity = vel;
            m_flNextColorizeCheck = gpGlobals->curtime + 0.1f;

        }

        //With this round we ensure that the speed is as precise as possible, instead of taking the floor value of the float
        SetDisplayValue(round(vel));
    }
}

Color CHudSpeedMeter::GetColorFromVariation(float variation, Color normalcolor, Color increasecolor, Color decreasecolor)
{
    //variation is current velocity minus previous velocity. 
    Color pFinalColor = normalcolor;
    int r, g, b;

    //our velocity decreased
    if (variation < 0)
    {
        r = normalcolor.r() * (1 / variation) + (decreasecolor.r() * (1 - 1 / variation));
        g = normalcolor.g() * (1 / variation) + (decreasecolor.g() * (1 - 1 / variation));
        b = normalcolor.b() * (1 / variation) + (decreasecolor.b() * (1 - 1 / variation));
        pFinalColor.SetColor(r, g, b, 255);
    }
    //our velocity increased
    else if (variation > 0)
    {
        r = normalcolor.r() * (1 / variation) + (increasecolor.r() * (1 - 1 / variation));
        g = normalcolor.g() * (1 / variation) + (increasecolor.g() * (1 - 1 / variation));
        b = normalcolor.b() * (1 / variation) + (increasecolor.b() * (1 - 1 / variation));
        pFinalColor.SetColor(r, g, b, 255);
    }
    else
        pFinalColor.SetColor(255,255,255,255);
    return pFinalColor;
}


