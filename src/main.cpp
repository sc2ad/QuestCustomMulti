#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "modloader/shared/modloader.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "GlobalNamespace/AuthenticationToken.hpp"
#include "GlobalNamespace/MasterServerEndPoint.hpp"
#include "MasterServer/BaseClientMessageHandler.hpp"
#include "System/AggregateException.hpp"
#include "System/Text/Encoding.hpp"
#include "System/BitConverter.hpp"
#include "System/Func_2.hpp"
#include "System/Security/Authentication/AuthenticationException.hpp"

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

MAKE_HOOK_OFFSETLESS(NetworkConfigSO_get_masterServerEndPoint, GlobalNamespace::MasterServerEndPoint*, Il2CppObject* self) {
    static auto* hostName = il2cpp_utils::createcsstr(HOST_NAME, il2cpp_utils::StringType::Manual);
    logger().debug("Created new MasterServerEndpoint! Host: %s:%u", HOST_NAME, PORT);
    return GlobalNamespace::MasterServerEndPoint::New_ctor(hostName, PORT);
}

MAKE_HOOK_OFFSETLESS(BaseClientMessageHandler_VerifySignature, System::Threading::Tasks::Task_1<bool>*, Il2CppObject* self, Il2CppArray* clientRandom, Il2CppArray* serverRandom, Il2CppArray* serverKey, Il2CppArray* signature, Il2CppArray* certData) {
    logger().debug("Called VerifySignature! Forwarding to true!");
    // Return type is a Task<bool>
    return System::Threading::Tasks::Task_1<bool>::New_ctor(true);
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

MAKE_HOOK_OFFSETLESS(PlatformAuthenticationTokenProvider_GetAuthenticationToken, System::Threading::Tasks::Task_1<GlobalNamespace::AuthenticationToken>*, Il2CppObject* self) {
    logger().debug("Ignoring original GetAuthenticationToken!");
    auto* arr = Array<uint8_t>::NewLength(1);
    arr->values[0] = 10;
    GlobalNamespace::AuthenticationToken token = GlobalNamespace::AuthenticationToken(
        GlobalNamespace::AuthenticationToken::Platform::OculusQuest,
        il2cpp_utils::createcsstr("aksudfjhajksdg"),
        il2cpp_utils::createcsstr("asidkgfhjkafvhdbjam,"),
        arr
    );
    return System::Threading::Tasks::Task_1<GlobalNamespace::AuthenticationToken>::New_ctor(token);
}

MAKE_HOOK_OFFSETLESS(OculusPlatformUserModel_GetUserAuthToken, System::Threading::Tasks::Task_1<Il2CppString*>*, Il2CppObject* self) {
    logger().debug("Calling original: OculusPlatformUserModel.GetUserAuthToken!");
    return OculusPlatformUserModel_GetUserAuthToken(self);
}

extern "C" void load() {
    logger().debug("Installing hooks!");
    INSTALL_HOOK_OFFSETLESS(MultiplayerUnavailableReasonMethods_TryGetMultiplayerUnavailableReason, il2cpp_utils::FindMethodUnsafe("", "MultiplayerUnavailableReasonMethods", "TryGetMultiplayerUnavailableReason", 2));
    INSTALL_HOOK_OFFSETLESS(NetworkConfigSO_get_masterServerEndPoint, il2cpp_utils::FindMethod("", "NetworkConfigSO", "get_masterServerEndPoint"));
    INSTALL_HOOK_OFFSETLESS(BaseClientMessageHandler_VerifySignature, il2cpp_utils::FindMethodUnsafe("MasterServer", "BaseClientMessageHandler", "VerifySignature", 5));
    INSTALL_HOOK_OFFSETLESS(PlatformAuthenticationTokenProvider_GetAuthenticationToken, il2cpp_utils::FindMethod("", "PlatformAuthenticationTokenProvider", "GetAuthenticationToken"));
    INSTALL_HOOK_OFFSETLESS(OculusPlatformUserModel_GetUserAuthToken, il2cpp_utils::FindMethod("", "OculusPlatformUserModel", "GetUserAuthToken"));
}