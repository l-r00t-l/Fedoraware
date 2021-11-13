#pragma once

#include "../../SDK/SDK.h"

class CGlowEffect
{
private:
	IMaterial* m_pMatGlowColor;
	ITexture* m_pRtFullFrame;
	ITexture* m_pRenderBuffer1;
	ITexture* m_pRenderBuffer2;
	IMaterial* m_pMatBlurX;
	IMaterial* m_pMatBlurY;
	IMaterial* m_pMatHaloAddToScreen;

	struct GlowEnt_t {
		CBaseEntity* m_pEntity;
		Color_t m_Color;
		float m_flAlpha;
	};

	std::vector<GlowEnt_t> m_vecGlowEntities;
	std::map<CBaseEntity*, bool> m_DrawnEntities;

private:
	void DrawModel(CBaseEntity* pEntity, int nFlags, bool bIsDrawingModels);
	void SetScale(int nScale);

public:
	void Init();
	void Render();

	inline bool HasDrawn(CBaseEntity* pEntity) {
		return m_DrawnEntities.find(pEntity) != m_DrawnEntities.end();
	}

	inline bool IsGlowMaterial(IMaterial* pMat) {
		return pMat == m_pMatGlowColor;
	}

public:
	bool m_bDrawingGlow;
	bool m_bRendering;
};

inline CGlowEffect g_Glow;