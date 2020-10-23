#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "modloader/shared/modloader.hpp"

#ifndef HOST_NAME
#define HOST_NAME "server1.networkauditor.org"
#endif

#ifndef POST
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
    return CRASH_UNLESS(il2cpp_utils::New("", "MasterServerEndpoint", hostName, PORT));
}

MAKE_HOOK_OFFSETLESS(BaseClientMessageHandler_VerifySignature, Il2CppObject*, Il2CppObject* self, Il2CppArray* clientRandom, Il2CppArray* serverRandom, Il2CppArray* serverKey, Il2CppArray* signature, Il2CppArray* certData) {
    logger().debug("Called VerifySignature! Forwarding to true!");
    return CRASH_UNLESS(il2cpp_utils::RunGenericMethod("System.Threading.Tasks", "Task", "FromResult", {il2cpp_functions::defaults->boolean_class}, true));
}

extern "C" void load() {
    logger().debug("Installing hooks!");
    INSTALL_HOOK_OFFSETLESS(NetworkConfigSO_get_masterServerEndPoint, il2cpp_utils::FindMethod("", "NetworkConfigSO", "get_masterServerEndPoint"));
    INSTALL_HOOK_OFFSETLESS(BaseClientMessageHandler_VerifySignature, il2cpp_utils::FindMethodUnsafe("", "BaseClientMessageHandler", "VerifySignature", 5));
}