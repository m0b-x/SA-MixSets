#include "plugin.h"
#include "game_sa\common.h"
#include "game_sa\CFont.h"
#include "game_sa\CSprite2d.h"
#include "game_sa\CKeyGen.h"
#include <map>

#include <CAnimBlendAssociation.h>
#include <CAnimBlendHierarchy.h>

#include "MixSets.h"

using namespace plugin;

class AnimViewer {
public:
    AnimViewer() {
        static std::map<unsigned int, std::string> animNamesMap;
        static CdeclEvent<AddressList<0x4CF2D8, H_CALL>, PRIORITY_AFTER, ArgPickN<char*, 0>, unsigned int(char*)> myStoreAnimNameEvent;

        myStoreAnimNameEvent += [](char* name) {
            animNamesMap[CKeyGen::GetUppercaseKey(name)] = name;
            };

        Events::drawingEvent += [] {
            if (MixSets::G_ShowAnimNameViewer)
            {
                CPed* ped = FindPlayerPed();
                if (ped && ped->m_pRwClump) {
                    unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
                    if (numAnimations > 0) {
                        CSprite2d::DrawRect(CRect(SCREEN_COORD_RIGHT(310.0f), SCREEN_COORD(265.0f), SCREEN_COORD_RIGHT(40.0f),
                            SCREEN_COORD(275.0f + 50.0f * numAnimations)), CRGBA(30, 30, 30, 255));
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
                        while (association) {
                            static char text[256];
                            sprintf(text, "%d. %s", counter + 1, animNamesMap[association->m_pHierarchy->m_hashKey].c_str());
                            if (association->m_nFlags & 0x10)
                                strcat(text, " (P)");
                            CFont::SetScale(SCREEN_MULTIPLIER(0.6f), SCREEN_MULTIPLIER(1.2f));
                            float width = CFont::GetStringWidth(text, true, false);
                            if (width > SCREEN_COORD(250.0f))
                                CFont::SetScale(SCREEN_MULTIPLIER(0.6f * SCREEN_COORD(250.0f) / width), SCREEN_MULTIPLIER(1.2f));
                            float baseY = SCREEN_COORD(300.0f) + SCREEN_COORD(50.0f) * counter;
                            CFont::PrintString(SCREEN_COORD_RIGHT(300.0f), baseY - SCREEN_COORD(25.0f), text);
                            CSprite2d::DrawRect(CRect(SCREEN_COORD_RIGHT(300.0f), baseY, SCREEN_COORD_RIGHT(50.0f), baseY + SCREEN_COORD(8.0f)),
                                CRGBA(130, 130, 130, 255));
                            CSprite2d::DrawRect(CRect(SCREEN_COORD_RIGHT(300.0f), baseY,
                                SCREEN_COORD_RIGHT(300.0f - association->m_fCurrentTime / association->m_pHierarchy->m_fTotalTime * 250.0f),
                                baseY + SCREEN_COORD(6.0f)), CRGBA(210, 210, 210, 255));
                            association = RpAnimBlendGetNextAssociation(association);
                            ++counter;
                        }
                        CFont::DrawFonts();
                    }
                }
            }
            };
    }
} animNamesView;
