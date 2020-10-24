#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "modloader/shared/modloader.hpp"

#ifndef HOST_NAME
#define HOST_NAME "server1.networkauditor.org"
#endif

#ifndef PORT
#define PORT 2328
#endif

static ModInfo modInfo;

const Logger& logger() {
    static const Logger& logger(modInfo);
    return logger;
}

extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
}

MAKE_HOOK_OFFSETLESS(NetworkConfigSO_get_masterServerEndPoint, Il2CppObject*, Il2CppObject* self) {
    static auto* hostName = il2cpp_utils::createcsstr(HOST_NAME, il2cpp_utils::StringType::Manual);
    logger().debug("Created new MasterServerEndpoint! Host: %s:%u", HOST_NAME, PORT);
    return CRASH_UNLESS(il2cpp_utils::New("", "MasterServerEndPoint", hostName, PORT));
}

MAKE_HOOK_OFFSETLESS(BaseClientMessageHandler_VerifySignature, Il2CppObject*, Il2CppObject* self, Il2CppArray* clientRandom, Il2CppArray* serverRandom, Il2CppArray* serverKey, Il2CppArray* signature, Il2CppArray* certData) {
    logger().debug("Called VerifySignature! Forwarding to true!");
    // Return type is a Task<bool>
    static auto* taskType = il2cpp_utils::MakeGeneric(il2cpp_utils::GetClassFromName("System.Threading.Tasks", "Task`1"), {il2cpp_functions::defaults->boolean_class});
    return CRASH_UNLESS(il2cpp_utils::New(taskType, true));
}

MAKE_HOOK_OFFSETLESS(MultiplayerUnavailableReasonMethods_TryGetMultiplayerUnavailableReason, bool, Il2CppObject* data, int* reason) {
    logger().debug("Calling original GetMultiplayerUnavailableReason!");
    bool orig = MultiplayerUnavailableReasonMethods_TryGetMultiplayerUnavailableReason(data, reason);
    if (orig && *reason == 2) {
        logger().debug("Ignoring version mismatch!");
        *reason = 0;
        return false;
    }
    return orig;
}

extern "C" void load() {
    logger().debug("Installing hooks!");
    INSTALL_HOOK_OFFSETLESS(MultiplayerUnavailableReasonMethods_TryGetMultiplayerUnavailableReason, il2cpp_utils::FindMethodUnsafe("", "MultiplayerUnavailableReasonMethods", "TryGetMultiplayerUnavailableReason", 2));
    INSTALL_HOOK_OFFSETLESS(NetworkConfigSO_get_masterServerEndPoint, il2cpp_utils::FindMethod("", "NetworkConfigSO", "get_masterServerEndPoint"));
    INSTALL_HOOK_OFFSETLESS(BaseClientMessageHandler_VerifySignature, il2cpp_utils::FindMethodUnsafe("MasterServer", "BaseClientMessageHandler", "VerifySignature", 5));
}