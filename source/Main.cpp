#include "plugin.h"
#include "CFileLoader.h"
#include "CMenuManager.h"
#include "CRadar.h"
#include "CWorld.h"

using namespace plugin;

class RadarMark {
    static bool markExist;
    static CVector markPosition;
    static CSprite2d waypointSprite;
    static const float WAYPOINT_SIZE;

    // إعدادات Crosshair
    static const float CROSSHAIR_THICKNESS;
    static const CRGBA CROSSHAIR_COLOR;

public:
    RadarMark() {
        plugin::Events::initRwEvent += [] {
            RwTexDictionary* hudTxd = CFileLoader::LoadTexDictionary("models/hud.txd");
            waypointSprite.m_pTexture = RwTexDictionaryFindNamedTexture(hudTxd, "radar_waypoint");

            if (!waypointSprite.m_pTexture) {
                RwTexDictionary* txd = CFileLoader::LoadTexDictionary(PLUGIN_PATH((char*)"target.txd"));
                waypointSprite.m_pTexture = GetFirstTexture(txd);
            }
            };

        plugin::Events::drawBlipsEvent += [] {
            CPad* pad = CPad::GetPad(0);
            bool rightMouseButtonClicked = pad->NewMouseControllerState.rmb && !pad->OldMouseControllerState.rmb;
            CVector2D crosshair;
            crosshair.x = FrontEndMenuManager.m_nMouseTempPosX;
            crosshair.y = FrontEndMenuManager.m_nMouseTempPosY;

            if (rightMouseButtonClicked && FrontEndMenuManager.m_bDrawRadarOrMap) {
                markExist = !markExist;
                if (markExist) SetMark(crosshair);
            }

            // رسم Crosshair فقط عندما تكون خريطة القائمة مفتوحة
            if (FrontEndMenuManager.m_bDrawRadarOrMap) {
                DrawFullCrosshair();
            }

            if (markExist) {
                DrawWaypoint();
            }
            };
    }

private:
    static void DrawFullCrosshair() {
        float mouseX = FrontEndMenuManager.m_nMouseTempPosX;
        float mouseY = FrontEndMenuManager.m_nMouseTempPosY;

        // حساب السماكة النسبية بناءً على دقة الشاشة
        float resolutionFactor = RsGlobal.screenWidth / 1280.0f;
        float scaledThickness = CROSSHAIR_THICKNESS * resolutionFactor;

        // رسم الخط الأفقي الكامل (من أقصى اليسار إلى أقصى اليمين)
        CSprite2d::DrawRect(CRect(
            0.0f,
            mouseY - scaledThickness / 2,
            RsGlobal.screenWidth,
            mouseY + scaledThickness / 2),
            CROSSHAIR_COLOR);

        // رسم الخط العمودي الكامل (من أقصى الأعلى إلى أقصى الأسفل)
        CSprite2d::DrawRect(CRect(
            mouseX - scaledThickness / 2,
            0.0f,
            mouseX + scaledThickness / 2,
            RsGlobal.screenHeight),
            CROSSHAIR_COLOR);
    }

    static void SetMark(CVector2D position) {
        CVector2D RadarPoint, WorldPoint2D;
        RadarPoint.x = (position.x / ((float)RsGlobal.screenWidth * *(float*)0x68FD14) -
            (*(float*)0x68FD10 * *(float*)0x869670 + *(float*)0x869674)) /
            (*(float*)0x68FD0C * *(float*)0x869670);
        RadarPoint.y = (*(float*)0x869678 - *(float*)0x68FD1C * *(float*)0x869670 -
            (position.y / ((float)RsGlobal.screenHeight * *(float*)0x68FD20))) /
            (*(float*)0x68FD18 * *(float*)0x869670);
        CRadar::TransformRadarPointToRealWorldSpace(WorldPoint2D, RadarPoint);
        markPosition = CVector(WorldPoint2D.x, WorldPoint2D.y,
            CWorld::FindGroundZForCoord(WorldPoint2D.x, WorldPoint2D.y));
    }

    static void DrawWaypoint() {
        static CVector2D radarPosition, screenPosition;
        CRadar::TransformRealWorldPointToRadarSpace(radarPosition, CVector2D(markPosition.x, markPosition.y));
        CRadar::LimitRadarPoint(radarPosition);
        CRadar::TransformRadarPointToScreenSpace(screenPosition, radarPosition);

        if (waypointSprite.m_pTexture) {
            float resolutionFactor = RsGlobal.screenWidth / 1280.0f;
            float scaledSize = WAYPOINT_SIZE * resolutionFactor;

            waypointSprite.Draw(CRect(screenPosition.x - scaledSize,
                screenPosition.y - scaledSize,
                screenPosition.x + scaledSize,
                screenPosition.y + scaledSize),
                CRGBA(255, 255, 255, 255));
        }
    }
};

// تعريف المتغيرات الثابتة
bool RadarMark::markExist = false;
CVector RadarMark::markPosition = CVector(0, 0, 0);
CSprite2d RadarMark::waypointSprite;
const float RadarMark::WAYPOINT_SIZE = 13.0f;

// إعدادات Crosshair
const float RadarMark::CROSSHAIR_THICKNESS = 2.0f;
const CRGBA RadarMark::CROSSHAIR_COLOR = CRGBA(255, 105, 180, 255); // لون وردي

RadarMark RadarMarkPlugin;