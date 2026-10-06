#include <Windows.h>
#include <cmath>
#include <string>
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

std::string g_modName = "Judgement Cut End - ap05's Remake.esp";
uint32_t g_fullFormID = 0xFE00084B;
uint32_t g_hotkey = 0x2E;

void LoadConfiguration() {
    char configPath[MAX_PATH];
    GetModuleFileNameA(GetModuleHandleA("warpspacetime.dll"), configPath, MAX_PATH);
    std::string pathStr(configPath);
    size_t lastSlash = pathStr.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        pathStr = pathStr.substr(0, lastSlash) + "\\warpspacetime.ini";
    }

    char resultBuffer[256];

    GetPrivateProfileStringA("Settings", "ModName", g_modName.c_str(), resultBuffer, sizeof(resultBuffer),
                             pathStr.c_str());
    g_modName = resultBuffer;

    GetPrivateProfileStringA("Settings", "SpellFormID", "0xFE00084B", resultBuffer, sizeof(resultBuffer),
                             pathStr.c_str());
    try {
        g_fullFormID = std::stoul(resultBuffer, nullptr, 16);
    } catch (...) {
        g_fullFormID = 0xFE00084B;
    }

    GetPrivateProfileStringA("Settings", "Hotkey", "0x2E", resultBuffer, sizeof(resultBuffer), pathStr.c_str());
    try {
        g_hotkey = std::stoul(resultBuffer, nullptr, 16);
    } catch (...) {
        g_hotkey = 0x2E;
    }

    SKSE::log::info("Config Loaded. Mod: {}, FormID: 0x{:X}, Key: 0x{:X}", g_modName, g_fullFormID, g_hotkey);
}

void SwapLocationsWithTarget() {
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    auto* lookDirSetting = RE::GameSettingCollection::GetSingleton();
    if (lookDirSetting) {
        auto* pickDistanceSetting = lookDirSetting->GetSetting("fUpdatePickRefEveryFrameDistance");
        if (pickDistanceSetting) {
            pickDistanceSetting->data.f = 20000.0f;
        }
    }

    RE::TESObjectREFR* targetRef = nullptr;

    float pitch = player->data.angle.x;
    float yaw = player->data.angle.z;
    RE::NiPoint3 lookDir;
    lookDir.x = std::sin(yaw) * std::cos(pitch);
    lookDir.y = std::cos(yaw) * std::cos(pitch);
    lookDir.z = -std::sin(pitch);

    RE::NiPoint3 startPos = player->data.location;
    startPos.z += 120.0f;

    auto* currentCell = player->GetParentCell();
    if (currentCell) {
        float closestDistance = 30000.0f;

        currentCell->ForEachReference([&](RE::TESObjectREFR& a_ref) {
            auto* ref = &a_ref;
            if (!ref || ref == player || !ref->Is(RE::FormType::ActorCharacter))
                return RE::BSContainer::ForEachResult::kContinue;

            RE::NiPoint3 refPos = ref->data.location;
            RE::NiPoint3 toRef = refPos - startPos;
            float projection = toRef.Dot(lookDir);
            if (projection > 0.0f && projection < closestDistance) {
                RE::NiPoint3 closestPointOnLine = startPos + (lookDir * projection);
                float distanceToLine = (refPos - closestPointOnLine).Length();
                if (distanceToLine < 180.0f) {
                    targetRef = ref;
                    closestDistance = projection;
                }
            }
            return RE::BSContainer::ForEachResult::kContinue;
        });
    }

    if (!targetRef) {
        SKSE::log::info("No target found along the raycast line.");
        return;
    }

    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    RE::SpellItem* customVFX = nullptr;

    if (dataHandler) {
        const RE::TESFile* modFile = dataHandler->LookupModByName(g_modName.c_str());
        if (modFile) {
            uint32_t localFormID = g_fullFormID & 0x00000FFF;
            uint32_t runtimeFormID = dataHandler->LookupFormID(localFormID, g_modName.c_str());
            if (runtimeFormID != 0) {
                customVFX = RE::TESForm::LookupByID<RE::SpellItem>(runtimeFormID);
            } else {
                SKSE::log::info("Failed to resolve the form ID for the custom VFX spell.");
            }
        }
    }

    if (customVFX) {
        auto* playerMagicCaster = player->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
        auto* targetMagicCaster = targetRef->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
        auto* targetActor = targetRef->As<RE::Actor>();

        if (playerMagicCaster) {
            playerMagicCaster->CastSpellImmediate(customVFX, false, player, 1.0f, false, 0.0f, player);
        }
        if (targetMagicCaster && targetActor) {
            targetMagicCaster->CastSpellImmediate(customVFX, false, targetActor, 1.0f, false, 0.0f, player);
        }
    }

    RE::NiPoint3 playerPosCopy = player->data.location;
    RE::NiPoint3 targetPosCopy = targetRef->data.location;

    player->SetPosition(targetPosCopy, true);
    targetRef->SetPosition(playerPosCopy);

    auto* targetActor = targetRef->As<RE::Actor>();
    if (targetActor) {
        targetActor->UpdateActor3DPosition();
    }
    player->UpdateActor3DPosition();

    SKSE::log::info("Positions successfully swapped with mismatched signatures resolved!");
}

class InputEventHandler : public RE::BSTEventSink<RE::InputEvent*> {
public:
    static InputEventHandler* GetSingleton() {
        static InputEventHandler singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
                                          RE::BSTEventSource<RE::InputEvent*>*) override {
        if (!a_event) return RE::BSEventNotifyControl::kContinue;

        for (auto* event = *a_event; event; event = event->next) {
            if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
                auto* buttonEvent = event->AsButtonEvent();
                if (buttonEvent && buttonEvent->GetIDCode() == g_hotkey && buttonEvent->IsDown()) {
                    SwapLocationsWithTarget();
                }
            }
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

void MessageListener(SKSE::MessagingInterface::Message* a_msg) {
    if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
        LoadConfiguration();
        auto* inputDeviceManager = RE::BSInputDeviceManager::GetSingleton();
        if (inputDeviceManager) {
            inputDeviceManager->AddEventSink(InputEventHandler::GetSingleton());
            SKSE::log::info("Input event sink injected into game loop.");
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);
    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener(MessageListener);
    return true;
}
