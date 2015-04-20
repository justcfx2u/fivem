/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <scrEngine.h>

#include <FontRenderer.h>

enum NativeIdentifiers : uint64_t
{
	GET_PLAYER_PED = 0x43A66C31C68491C0,
	GET_ENTITY_COORDS = 0x3FEF770D40960D5A,
	GET_FIRST_BLIP_INFO_ID = 0x1BEDE233E6CD2A1F,
	GET_NEXT_BLIP_INFO_ID = 0x14F96AA50D6FBEA7,
	GET_BLIP_INFO_ID_TYPE = 0xBE9B0959FFD0779B,
	GET_BLIP_COORDS = 0x586AFE3FF72D996E,
	GET_GROUND_Z_FOR_3D_COORD = 0xC906A7DAB05C8D2B,
	SET_ENTITY_COORDS = 0x621873ECE1178967,
	SET_ENTITY_COORDS_NO_OFFSET = 0x239A3351AC1DA385,
	LOAD_SCENE = 0x4448EB75B4904BDB,
	REQUEST_MODEL = 0x963D27A58DF860AC,
	HAS_MODEL_LOADED = 0x98A4EB5D89A0C952,
	CREATE_VEHICLE = 0xAF35D0D2583051B0
};

// BLIP_8 in global.gxt2 -> 'Waypoint'
#define BLIP_WAYPOINT 8

class LovelyThread : public GtaThread
{
private:
	bool m_isWaitingForModelToLoad;

public:
	LovelyThread()
	{
		m_isWaitingForModelToLoad = false;
	}

	virtual void DoRun() override
	{
		uint32_t playerPedId = NativeInvoke::Invoke<GET_PLAYER_PED, uint32_t>(-1);

		if (playerPedId != -1 && playerPedId != 0)
		{
			scrVector entityCoords = NativeInvoke::Invoke<GET_ENTITY_COORDS, scrVector>(playerPedId);

			CRect rect(400, 100, 900, 200);
			CRGBA color(255, 255, 255, 255);

			TheFonts->DrawText(va(L"Player coords: %g %g %g", entityCoords.x, entityCoords.y, entityCoords.z), rect, color, 24.0f, 1.0f, "Segoe UI");

			// if the particular key we like is pressed...
			static bool wasPressed = true;

			if (GetAsyncKeyState(VK_F11) & 0x8000)
			{
				if (!wasPressed)
				{
					// iterate through blips to find a waypoint
					int infoId = NativeInvoke::Invoke<GET_FIRST_BLIP_INFO_ID, int>(BLIP_WAYPOINT);

					if (infoId > 0)
					{
						scrVector blipCoords = NativeInvoke::Invoke<GET_BLIP_COORDS, scrVector>(infoId);

						NativeInvoke::Invoke<LOAD_SCENE, int>(blipCoords.x, blipCoords.y, blipCoords.z);

						float newZ = 0.0f;
						NativeInvoke::Invoke<GET_GROUND_Z_FOR_3D_COORD, int>(blipCoords.x, blipCoords.y, 1000.0f, &newZ);

						NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(playerPedId, blipCoords.x, blipCoords.y, newZ);
					}

					wasPressed = true;
				}
			}
			else
			{
				wasPressed = false;
			}

			// spawn the vehicle, somewhere
			static bool wasF9Pressed = true;

			if (GetAsyncKeyState(VK_F9) & 0x8000)
			{
				if (!wasF9Pressed)
				{
					NativeInvoke::Invoke<REQUEST_MODEL, int>(0x2B6DC64A);

					m_isWaitingForModelToLoad = true;

					wasF9Pressed = true;
				}
			}
			else
			{
				wasF9Pressed = false;
			}

			if (m_isWaitingForModelToLoad)
			{
				if (NativeInvoke::Invoke<HAS_MODEL_LOADED, bool>(0x2B6DC64A))
				{
					scrVector entityCoords = NativeInvoke::Invoke<GET_ENTITY_COORDS, scrVector>(playerPedId);

					NativeInvoke::Invoke<CREATE_VEHICLE, int>(0x2B6DC64A, entityCoords.x, entityCoords.y + 2, entityCoords.z, 0.0f, 1, 0);

					m_isWaitingForModelToLoad = false;
				}
			}
		}
	}
};

static LovelyThread lovelyThread;

static InitFunction initFunction([] ()
{
	rage::scrEngine::OnScriptInit.Connect([] ()
	{
		rage::scrEngine::CreateThread(&lovelyThread);
	});
});