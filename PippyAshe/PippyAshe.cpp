#include "PluginSDK.h"
#include "Vector3.h"

#define PIPEX_DEBUG

PluginSetup("PippyAshe")

IMenu* mainMenu;
IMenu* comboMenu;
IMenu* harassMenu;
IMenu* rSkillMenu;
IMenu* drawMenu;
#ifdef PIPEX_DEBUG
IMenu* debugMenu;
#endif

IMenuOption* comboQ;
IMenuOption* comboW;
IMenuOption* comboR;

IMenuOption* harassQ;
IMenuOption* harassW;

IMenuOption* rSkillKey;
IMenuOption* rSkillRange;

IMenuOption* drawRangeW;
IMenuOption* drawTargetR;
IMenuOption* drawRangeR;
IMenuOption* drawColor;

#ifdef PIPEX_DEBUG
IMenuOption* qSpellInfo;
IMenuOption* wSpellInfo;
IMenuOption* eSpellInfo;
IMenuOption* rSpellInfo;
#endif

ISpell2* qSpell;
ISpell2* wSpell;
ISpell2* eSpell;
ISpell2* rSpell;

bool isPressingKeyToShootR = false;

void ShowSpellInfo(eSpellSlot slot)
{
	switch (slot)
	{
	case kSlotQ:
		GGame->PrintChat("Info about Q spell:");
		break;
	case kSlotW:
		GGame->PrintChat("Info about W spell:");
		break;
	case kSlotE:
		GGame->PrintChat("Info about E spell:");
		break;
	case kSlotR:
		GGame->PrintChat("Info about R spell");
		break;
	}

	char rangeInfo[64];
	char speedInfo[64];
	char castTimeInfo[64];
	char widthInfo[64];

	sprintf(rangeInfo, "Range: %d", GEntityList->Player()->GetSpellBook()->GetRange(slot));
	GGame->PrintChat(rangeInfo);

	sprintf(speedInfo, "Speed: %d", GEntityList->Player()->GetSpellBook()->GetSpeed(slot));
	GGame->PrintChat(speedInfo);

	sprintf(castTimeInfo, "Delay: %d", GEntityList->Player()->GetSpellBook()->GetSpellCastTime(slot));
	GGame->PrintChat(castTimeInfo);

	sprintf(widthInfo, "Width: %d", GEntityList->Player()->GetSpellBook()->GetRadius(slot));
	GGame->PrintChat(widthInfo);
}

void MyMenu()
{
	mainMenu = GPluginSDK->AddMenu("Pippy Ashe");

	comboMenu = mainMenu->AddMenu("Combo");
	harassMenu = mainMenu->AddMenu("Harass");
	rSkillMenu = mainMenu->AddMenu("R Skill");
	drawMenu = mainMenu->AddMenu("Drawings");
#ifdef PIPEX_DEBUG
	debugMenu = mainMenu->AddMenu("Debug (DEV ONLY)");
#endif

	comboQ = comboMenu->CheckBox("Use Q in Combo", true);
	comboW = comboMenu->CheckBox("Use W in Combo", true);
	comboR = comboMenu->CheckBox("Use R in Combo", false);

	harassQ = harassMenu->CheckBox("Use Q in Harass", false);
	harassW = harassMenu->CheckBox("Use W in Harass", true);

	rSkillKey = rSkillMenu->AddKey("Cast R to target", 84);
	rSkillRange = rSkillMenu->AddInteger("Look for target range", 0, 2500, 2000);

	drawRangeW = drawMenu->CheckBox("Draw W Range", false);
	drawRangeR = drawMenu->CheckBox("Draw R Range", false);
	//drawTargetR = drawMenu->CheckBox("Draw circle around R target", true);
	drawColor = drawMenu->AddColor("Range Color", 0, 175, 175, 255);

#ifdef PIPEX_DEBUG
	qSpellInfo = debugMenu->AddInteger("Q Info", 0, 1, 0);
	wSpellInfo = debugMenu->AddInteger("W Info", 0, 1, 0);
	eSpellInfo = debugMenu->AddInteger("E Info", 0, 1, 0);
	rSpellInfo = debugMenu->AddInteger("R Info", 0, 1, 0);
#endif	
}

void InitSpells()
{
	qSpell = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, kCollidesWithNothing);
	wSpell = GPluginSDK->CreateSpell2(kSlotW, kLineCast, true, true, (eCollisionFlags)(kCollidesWithHeroes | kCollidesWithMinions | kCollidesWithYasuoWall));
	eSpell = GPluginSDK->CreateSpell2(kSlotE, kLineCast, true, false, kCollidesWithYasuoWall);
	rSpell = GPluginSDK->CreateSpell2(kSlotR, kLineCast, true, false, (eCollisionFlags)(kCollidesWithHeroes | kCollidesWithYasuoWall));

	wSpell->SetOverrideRange(1200);
	wSpell->SetOverrideDelay(0.25f);
	wSpell->SetOverrideRadius(20);
	wSpell->SetOverrideSpeed(2000);

	rSpell->SetOverrideRange(25000);
	rSpell->SetOverrideDelay(0.25f);
	rSpell->SetOverrideRadius(130);
	rSpell->SetOverrideSpeed(1600);
}

PLUGIN_EVENT(void) PippyAsheUpdate()
{
#ifdef PIPEX_DEBUG
	if (qSpellInfo->GetInteger() == 1)
	{
		ShowSpellInfo(kSlotQ);
		qSpellInfo->UpdateInteger(0);
	}

	if (wSpellInfo->GetInteger() == 1)
	{
		ShowSpellInfo(kSlotW);
		wSpellInfo->UpdateInteger(0);
	}

	if (eSpellInfo->GetInteger() == 1)
	{
		ShowSpellInfo(kSlotE);
		eSpellInfo->UpdateInteger(0);
	}

	if (rSpellInfo->GetInteger() == 1)
	{
		ShowSpellInfo(kSlotR);
		rSpellInfo->UpdateInteger(0);
	}
#endif

	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		if (comboQ->Enabled() && GEntityList->Player()->GetSpellState(kSlotQ) == Ready)
		{
			if (GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, GEntityList->Player()->AttackRange()) != nullptr)
			{
				qSpell->CastOnPlayer();
			}
		}

		if (comboW->Enabled() && GEntityList->Player()->GetSpellState(kSlotW) == Ready)
		{
			IUnit *wTarget = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, wSpell->Range());
			if (wTarget)
			{
				wSpell->CastOnTarget(wTarget, 4);
			}
		}

		if (comboR->Enabled() && GEntityList->Player()->GetSpellState(kSlotR) == Ready)
		{
			IUnit *rTarget = GTargetSelector->FindTarget(QuickestKill, SpellDamage, 1250);
			if (rTarget)
			{
				rSpell->CastOnTarget(rTarget, 5);
			}
		}
		break;
	case kModeMixed:
		if (harassQ->Enabled() && GEntityList->Player()->GetSpellState(kSlotQ) == Ready)
		{
			if (GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, GEntityList->Player()->AttackRange()) != nullptr)
			{
				qSpell->CastOnPlayer();
			}
		}

		if (harassW->Enabled() && GEntityList->Player()->GetSpellState(kSlotW) == Ready)
		{
			IUnit *wTarget = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, wSpell->Range());
			if (wTarget)
			{
				wSpell->CastOnTarget(wTarget, 4);
			}
		}
		break;
	}

	if (isPressingKeyToShootR && GEntityList->Player()->GetSpellState(kSlotR) == Ready)
	{
		IUnit *rTarget = GTargetSelector->FindTarget(QuickestKill, SpellDamage, rSkillRange->GetInteger());
		if (rTarget)
		{
			rSpell->CastOnTarget(rTarget, 5);
		}
	}
}

PLUGIN_EVENT(void) PippyAsheRender()
{
	Vec4 rangeColor;
	drawColor->GetColor(&rangeColor);

	if (drawRangeW->Enabled() && GEntityList->Player()->GetSpellState(kSlotW) == Ready)
	{
		GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), rangeColor, wSpell->Range());
	}

	if (drawRangeR->Enabled() && GEntityList->Player()->GetSpellState(kSlotR) == Ready)
	{
		GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), rangeColor, rSkillRange->GetInteger());
	}

	if (isPressingKeyToShootR)
	{
		GRender->DrawTextW(Vec2(60, 20), Vec4(0, 255, 0, 255), "Shoot R Key is ENABLED");
	}
	else
	{
		GRender->DrawTextW(Vec2(60, 20), Vec4(255, 0, 0, 255), "Shoot R Key is DISABLED");
	}
}

PLUGIN_EVENT(bool) PippyAsheWndProc(HWND Wnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
#if defined PIPEX_DEBUG && defined KEY_PRESSES_STUFF
	if (Message == WM_CHAR)
	{
		char message[64];
		sprintf(message, "Key pressed: %i", wParam);
		GGame->PrintChat(message);
	}
#endif

	if (Message == WM_CHAR && wParam == rSkillKey->GetInteger())
	{
		isPressingKeyToShootR = true;
		return false;
	}
	if (Message = WM_KEYUP && wParam == rSkillKey->GetInteger())
	{
		isPressingKeyToShootR = false;
		return false;
	}

	return true;
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	GEventManager->AddEventHandler(kEventOnGameUpdate, PippyAsheUpdate);
	GEventManager->AddEventHandler(kEventOnRender, PippyAsheRender);
	GEventManager->AddEventHandler(kEventOnWndProc, PippyAsheWndProc);

	MyMenu();
	InitSpells();
	GGame->PrintChat("Remember to have CAPS LOCK ACTIVATED so L++ recognizes PippyAshe's keybindings");
}

PLUGIN_API void OnUnload()
{
	mainMenu->Remove();

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, PippyAsheUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, PippyAsheRender);
	GEventManager->RemoveEventHandler(kEventOnWndProc, PippyAsheWndProc);
}