#include "game_sa/common.h"
#include "game_sa/CFont.h"
#include "game_sa/CSprite2d.h"
#include "game_sa/CKeyGen.h"

#include <CAnimBlendAssociation.h>
#include <CAnimBlendHierarchy.h>

#include "MixSets.h"

using namespace plugin;

class AnimViewer
{
public:
    AnimViewer()
    {
        // Map of animation hash keys to their original names
        static std::unordered_map<unsigned int, std::string> animNamesMap;
        animNamesMap.reserve(4096);

        // Hook into the animation loader to store names
        static CdeclEvent<
            AddressList<0x4CF2D8, H_CALL>,
            PRIORITY_AFTER,
            ArgPickN<char*, 0>,
            unsigned int(char*)
        > storeAnimNameEvent;

        storeAnimNameEvent += [](char* name) {
            unsigned int key = CKeyGen::GetUppercaseKey(name);
            if (animNamesMap.find(key) == animNamesMap.end())
            {
                animNamesMap.emplace(key, std::string{ name });
            }
            };

        Events::drawingEvent += [] {
            if (!MixSets::G_ShowAnimNameViewer)
                return;

            CPed* ped = FindPlayerPed();
            if (!ped || !ped->m_pRwClump)
                return;

            unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
            if (numAnimations == 0)
                return;

            CSprite2d::DrawRect(
                CRect(
                    SCREEN_COORD_RIGHT(310.0f),
                    SCREEN_COORD(265.0f),
                    SCREEN_COORD_RIGHT(40.0f),
                    SCREEN_COORD(275.0f + 50.0f * numAnimations)
                ),
                CRGBA(30, 30, 30, 255)
            );

            CFont::SetOrientation(ALIGN_LEFT);
            CFont::SetColor(CRGBA(255, 255, 255, 255));
            CFont::SetDropShadowPosition(0);
            CFont::SetDropColor(CRGBA(0, 0, 0, 255));
            CFont::SetFontStyle(FONT_MENU);
            CFont::SetProportional(true);
            CFont::SetWrapx(SCREEN_COORD_MAX_X * 2);
            CFont::SetBackground(false, false);

            int counter = 0;
            CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
            while (association)
            {
                unsigned int key = association->m_pHierarchy->m_hashKey;
                auto it = animNamesMap.find(key);
                const char* animName = (it != animNamesMap.end()) ? it->second.c_str() : "Unknown";

                char text[256];
                if (association->m_nFlags & 0x10)
                {
                    snprintf(text, sizeof(text), "%d. %s (P)", counter + 1, animName);
                }
                else
                {
                    snprintf(text, sizeof(text), "%d. %s", counter + 1, animName);
                }

                CFont::SetScale(
                    SCREEN_MULTIPLIER(0.6f),
                    SCREEN_MULTIPLIER(1.2f)
                );
                float width = CFont::GetStringWidth(text, true, false);
                if (width > SCREEN_COORD(250.0f))
                {
                    CFont::SetScale(
                        SCREEN_MULTIPLIER(0.6f * SCREEN_COORD(250.0f) / width),
                        SCREEN_MULTIPLIER(1.2f)
                    );
                }

                float baseY = SCREEN_COORD(300.0f) + SCREEN_COORD(50.0f) * counter;
                CFont::PrintString(
                    SCREEN_COORD_RIGHT(300.0f),
                    baseY - SCREEN_COORD(25.0f),
                    text
                );

                CSprite2d::DrawRect(
                    CRect(
                        SCREEN_COORD_RIGHT(300.0f),
                        baseY,
                        SCREEN_COORD_RIGHT(50.0f),
                        baseY + SCREEN_COORD(8.0f)
                    ),
                    CRGBA(130, 130, 130, 255)
                );
                CSprite2d::DrawRect(
                    CRect(
                        SCREEN_COORD_RIGHT(300.0f),
                        baseY,
                        SCREEN_COORD_RIGHT(
                            300.0f - association->m_fCurrentTime / association->m_pHierarchy->m_fTotalTime * 250.0f
                        ),
                        baseY + SCREEN_COORD(6.0f)
                    ),
                    CRGBA(210, 210, 210, 255)
                );

                association = RpAnimBlendGetNextAssociation(association);
                ++counter;
            }

            CFont::DrawFonts();
            };
    }
};

static AnimViewer animNamesView;
