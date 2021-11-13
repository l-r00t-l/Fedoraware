#include "Visuals.h"
#include "../Vars.h"
#include "../Menu/Menu.h"

typedef bool(_cdecl* LoadNamedSkysFn)(const char*);
static LoadNamedSkysFn LoadSkys = (LoadNamedSkysFn)g_Pattern.Find(_(L"engine.dll"), _(L"55 8B EC 81 EC ? ? ? ? 8B 0D ? ? ? ? 53 56 57 8B 01 C7 45"));

void CVisuals::TransparentProps()
{
	std::vector<IMaterial*> /*world, */props;

	for (uint16_t h{ g_Interfaces.MatSystem->First() }; h != g_Interfaces.MatSystem->Invalid(); h = g_Interfaces.MatSystem->Next(h)) {
		IMaterial* mat = g_Interfaces.MatSystem->Get(h);
		if (!mat)
			continue;

		/*if (mat->GetTextureGroupName() == "World textures") {
			world.push_back(mat);
		}*/

		else if (mat->GetTextureGroupName() == "StaticProp textures")
			props.push_back(mat);
	}

	if (g_Interfaces.CVars->FindVar(_("r_DrawSpecificStaticProp"))->GetInt() != 0) {
		g_Interfaces.CVars->FindVar(_("r_DrawSpecificStaticProp"))->SetValue(0);
	}

	for (const auto& p : props) {
		p->AlphaModulate(0.85f);
	}
}

void CVisuals::SkyboxChanger() {
	const char* skybNames[] = {
		"Custom",
		"sky_tf2_04",
		"sky_upward",
		"sky_dustbowl_01",
		"sky_goldrush_01",
		"sky_granary_01",
		"sky_well_01",
		"sky_gravel_01",
		"sky_badlands_01",
		"sky_hydro_01",
		"sky_night_01",
		"sky_nightfall_01",
		"sky_trainyard_01",
		"sky_stormfront_01",
		"sky_morningsnow_01",
		"sky_alpinestorm_01",
		"sky_harvest_01",
		"sky_harvest_night_01",
		"sky_halloween",
		"sky_halloween_night_01",
		"sky_halloween_night2014_01",
		"sky_island_01",
		"sky_rainbow_01"
	};
	if (Vars::Visuals::SkyboxChanger.m_Var) {
		if (Vars::Skybox::skyboxnum == 0) {
			if (Vars::Misc::BypassPure.m_Var) {
				LoadSkys(Vars::Skybox::SkyboxName.c_str());
			}
			else {
				LoadSkys(g_Interfaces.CVars->FindVar("sv_skyname")->GetString());
			}
		}
		else {
			LoadSkys(skybNames[Vars::Skybox::skyboxnum]);
		}
	}
	else {
		LoadSkys(g_Interfaces.CVars->FindVar("sv_skyname")->GetString());
	}
}

void CVisuals::BigHeads(float headSize, float torsoSize, float handSize)
{
	if (Vars::ESP::Players::Funnybodypartslol.m_Var) {
		for (auto& Player : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL)) {
			float* headScale = Player->GetHeadScale();
			float* torsoScale = Player->GetTorsoScale();
			float* handScale = Player->GetHandScale();
			*headScale = headSize;
			*torsoScale = torsoSize;
			*handScale = handSize;
		}
	}
}

bool CVisuals::RemoveScope(int nPanel)
{
	if (!m_nHudZoom && Hash::IsHudScope(g_Interfaces.Panel->GetName(nPanel)))
		m_nHudZoom = nPanel;

	return (Vars::Visuals::RemoveScope.m_Var && nPanel == m_nHudZoom);
}


void CVisuals::FOV(CViewSetup *pView)
{
	CBaseEntity *pLocal = g_EntityCache.m_pLocal;

	if (pLocal && pView)
	{
		if (pLocal->GetObserverMode() == OBS_MODE_FIRSTPERSON || (pLocal->IsScoped() && !Vars::Visuals::RemoveZoom.m_Var))
			return;

		pView->fov = Vars::Visuals::FieldOfView.m_Var;

		if (pLocal->IsAlive())
			pLocal->SetFov(Vars::Visuals::FieldOfView.m_Var);
	}
}

void CVisuals::ThirdPerson()
{
	if (const auto &pLocal = g_EntityCache.m_pLocal)
	{
		if (Vars::Visuals::ThirdPersonKey.m_Var)
		{
			if (!g_Interfaces.EngineVGui->IsGameUIVisible() && !g_Interfaces.Surface->IsCursorVisible() && !g_Menu.m_bTyping)
			{
				static float flPressedTime = g_Interfaces.Engine->Time();
				float flElapsed = g_Interfaces.Engine->Time() - flPressedTime;

				if ((GetAsyncKeyState(Vars::Visuals::ThirdPersonKey.m_Var) & 0x8000) && flElapsed > 0.2f) {
					Vars::Visuals::ThirdPerson.m_Var = !Vars::Visuals::ThirdPerson.m_Var;
					flPressedTime = g_Interfaces.Engine->Time();
				}
			}
		}
		
		bool bIsInThirdPerson = g_Interfaces.Input->CAM_IsThirdPerson();

		if (!Vars::Visuals::ThirdPerson.m_Var
			|| ((!Vars::Visuals::RemoveScope.m_Var || !Vars::Visuals::RemoveZoom.m_Var) && pLocal->IsScoped()))
		{
			if (bIsInThirdPerson)
				pLocal->ForceTauntCam(0);

			return;
		}

		if (!bIsInThirdPerson)
			pLocal->ForceTauntCam(1);

		if (bIsInThirdPerson && Vars::Visuals::ThirdPersonSilentAngles.m_Var)
		{
			g_Interfaces.Prediction->SetLocalViewAngles(g_GlobalInfo.m_vRealViewAngles);

			if (Vars::Visuals::ThirdPersonInstantYaw.m_Var)
			{
				if (const auto &pAnimState = pLocal->GetAnimState())
					pAnimState->m_flCurrentFeetYaw = g_GlobalInfo.m_vRealViewAngles.y;
			}
		}
	}
}


void CVisuals::BulletTrace(CBaseEntity* pEntity, Color_t color)
{
	Vector src3D, dst3D, forward, src, dst;
	CGameTrace tr;
	Ray_t ray;
	CTraceFilterHitscan filter;

	Math::AngleVectors(pEntity->GetEyeAngles(), &forward);
	filter.pSkip = pEntity;
	src3D = pEntity->GetBonePos(6) - Vector(0, 0, 0);
	dst3D = src3D + (forward * 1000);

	ray.Init(src3D, dst3D);

	g_Interfaces.EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (!Utils::W2S(src3D, src) || !Utils::W2S(tr.vEndPos, dst))
	{
		return;
	}

	//g_Interfaces.Surface->DrawLine(src.x, src.y, dst.x, dst.y);
	g_Draw.Line(src.x, src.y, dst.x, dst.y, color);
}

bool bWorldIsModulated = false;
bool bSkyIsModulated = false;



void ApplyModulation(const Color_t &clr)
{
	for (MaterialHandle_t h = g_Interfaces.MatSystem->First(); h != g_Interfaces.MatSystem->Invalid(); h = g_Interfaces.MatSystem->Next(h))
	{
		if (const auto &pMaterial = g_Interfaces.MatSystem->Get(h))
		{
			//bool bFound2 = false;
			//IMaterialVar* rain = pMaterial->FindVar(_("env_global"), &bFound2);

			if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
				continue;

			std::string_view group(pMaterial->GetTextureGroupName());

			if (group.find(_(TEXTURE_GROUP_WORLD)) != group.npos/* || group.find(_(TEXTURE_GROUP_SKYBOX)) != group.npos*/)
			{
				bool bFound = false;
				IMaterialVar *pVar = pMaterial->FindVar(_("$color2"), &bFound);

				if (bFound && pVar)
					pVar->SetVecValue(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));

				else pMaterial->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
			}


		}
	}

	bWorldIsModulated = true;
}

void ApplySkyboxModulation(const Color_t& clr)
{
	for (MaterialHandle_t h = g_Interfaces.MatSystem->First(); h != g_Interfaces.MatSystem->Invalid(); h = g_Interfaces.MatSystem->Next(h))
	{
		const auto& pMaterial = g_Interfaces.MatSystem->Get(h);

		if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
			continue;

		std::string_view group(pMaterial->GetTextureGroupName());

		if (group._Starts_with("SkyBox"))
		{
			pMaterial->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
		}
	}
	bSkyIsModulated = true;
}


void CVisuals::ModulateWorld()
{
	if (!Vars::Visuals::WorldModulation.m_Var)
		return;

	ApplyModulation(Colors::WorldModulation);
	ApplySkyboxModulation(Colors::SkyModulation);
}



void CVisuals::OverrideWorldTextures()
{
		static KeyValues *kv = nullptr;
		if (!kv) {
			kv = new KeyValues("LightmappedGeneric");
			//kv->SetString("$basetexture", "dev/dev_measuregeneric01b"); //Nitro (dev texture)
			kv->SetString("$basetexture", "vgui/white_additive"); //flat 
			kv->SetString("$color2", "[0.12 0.12 0.15]"); //grey
		}

		if (Vars::Visuals::OverrideWorldTextures.m_Var) {
			for (auto h = g_Interfaces.MatSystem->First(); h != g_Interfaces.MatSystem->Invalid(); h = g_Interfaces.MatSystem->Next(h)) {
				IMaterial* pMaterial = g_Interfaces.MatSystem->Get(h);

				if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached()
					|| pMaterial->IsTranslucent() || pMaterial->IsSpriteCard()
					|| std::string_view(pMaterial->GetTextureGroupName()).find("World") == std::string_view::npos)
					continue;

				std::string_view sName = std::string_view(pMaterial->GetName());

				if (sName.find("water") != std::string_view::npos || sName.find("glass") != std::string_view::npos
					|| sName.find("door") != std::string_view::npos || sName.find("tools") != std::string_view::npos
					|| sName.find("player") != std::string_view::npos || sName.find("chicken") != std::string_view::npos
					|| sName.find("wall28") != std::string_view::npos || sName.find("wall26") != std::string_view::npos
					|| sName.find("decal") != std::string_view::npos || sName.find("overlay") != std::string_view::npos
					|| sName.find("hay") != std::string_view::npos)
					continue;

				pMaterial->SetShaderAndParams(kv);
			}
		}
}

void CVisuals::UpdateWorldModulation()
{
	if (!Vars::Visuals::WorldModulation.m_Var) {
		RestoreWorldModulation();
		return;
	}

	auto ColorChanged = [&]() -> bool
	{
		static Color_t old = Colors::WorldModulation;
		Color_t cur = Colors::WorldModulation;

		if (cur.r != old.r || cur.g != old.g || cur.b != old.b) {
			old = cur;
			return true;
		}

		return false;
	};

	if (!bWorldIsModulated || ColorChanged())
		ApplyModulation(Colors::WorldModulation);
}

void CVisuals::UpdateSkyModulation()
{
	if (!Vars::Visuals::SkyModulation.m_Var) {
		RestoreWorldModulation();
		return;
	}

	auto ColorChanged = [&]() -> bool
	{
		static Color_t old = Colors::SkyModulation;
		Color_t cur = Colors::SkyModulation;

		if (cur.r != old.r || cur.g != old.g || cur.b != old.b) {
			old = cur;
			return true;
		}

		return false;
	};

	if (!bWorldIsModulated || ColorChanged())
		ApplySkyboxModulation(Colors::SkyModulation);
}

void CVisuals::RestoreWorldModulation()
{
	if (!bWorldIsModulated)
		return;

	if (!bSkyIsModulated)
		return;

	ApplyModulation({ 255, 255, 255, 255 });
	ApplySkyboxModulation({ 255, 255, 255, 255 });
	bWorldIsModulated = false;
	bSkyIsModulated = false;
}