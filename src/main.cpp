#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "modloader/shared/modloader.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "GlobalNamespace/AuthenticationToken.hpp"
#include "System/AggregateException.hpp"
#include "System/Text/Encoding.hpp"
#include "System/BitConverter.hpp"
#include "System/Func_2.hpp"

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

GlobalNamespace::AuthenticationToken continueFunc(System::Threading::Tasks::Task_1<GlobalNamespace::AuthenticationToken>* orig) {
    logger().debug("Entering continue function for GetAuthenticationToken!");
    if (orig->get_IsFaulted()) {
        logger().debug("Original invoke failed!");
        logger().debug("Resulting exception: %s", to_utf8(csstrtostr(orig->get_Exception()->ToString())).c_str());
    }
    else if (orig->get_IsCompleted()) {
        logger().debug("Completed orig request!");
        auto val = orig->get_Result();
        logger().debug("Platform: %i", val.platform);
        logger().debug("UserID: %s", to_utf8(csstrtostr(val.userId)).c_str());
        logger().debug("UserName: %s", to_utf8(csstrtostr(val.userName)).c_str());
        auto sessionToken = to_utf8(csstrtostr(System::BitConverter::ToString(val.sessionToken)));
        logger().debug("Session token: %s", sessionToken.c_str());
        if (orig->get_Result().sessionToken == nullptr) {
            logger().debug("User token is null! Creating new random garbage!");
            auto* arr = Array<uint8_t>::NewLength(1);
            arr->values[0] = 10;
            return GlobalNamespace::AuthenticationToken(
                GlobalNamespace::AuthenticationToken::Platform::OculusQuest,
                il2cpp_utils::createcsstr("aksudfjhajksdg"),
                il2cpp_utils::createcsstr("asidkgfhjkafvhdbjam,"),
                arr
            );
        }
    }
    return GlobalNamespace::AuthenticationToken();
}

MAKE_HOOK_OFFSETLESS(PlatformAuthenticationTokenProvider_GetAuthenticationToken, System::Threading::Tasks::Task_1<GlobalNamespace::AuthenticationToken>*, Il2CppObject* self) {
    // First call orig
    logger().debug("Calling original GetAuthenticationToken!");
    auto* orig = PlatformAuthenticationTokenProvider_GetAuthenticationToken(self);
    // If orig has no value, we need to make our own.
    return orig->ContinueWith<GlobalNamespace::AuthenticationToken>(reinterpret_cast<System::Func_2<
            System::Threading::Tasks::Task_1<GlobalNamespace::AuthenticationToken>*,
            GlobalNamespace::AuthenticationToken>*>(il2cpp_utils::MakeFunc(std::function(continueFunc))));
}

MAKE_HOOK_OFFSETLESS(ErrorCode, Il2CppString*, int reason) {
    // haha xd
    CRASH_UNLESS(false);
}

extern "C" void load() {
    logger().debug("Installing hooks!");
    INSTALL_HOOK_OFFSETLESS(MultiplayerUnavailableReasonMethods_TryGetMultiplayerUnavailableReason, il2cpp_utils::FindMethodUnsafe("", "MultiplayerUnavailableReasonMethods", "TryGetMultiplayerUnavailableReason", 2));
    INSTALL_HOOK_OFFSETLESS(NetworkConfigSO_get_masterServerEndPoint, il2cpp_utils::FindMethod("", "NetworkConfigSO", "get_masterServerEndPoint"));
    INSTALL_HOOK_OFFSETLESS(BaseClientMessageHandler_VerifySignature, il2cpp_utils::FindMethodUnsafe("MasterServer", "BaseClientMessageHandler", "VerifySignature", 5));
    INSTALL_HOOK_OFFSETLESS(PlatformAuthenticationTokenProvider_GetAuthenticationToken, il2cpp_utils::FindMethod("", "PlatformAuthenticationTokenProvider", "GetAuthenticationToken"));
    INSTALL_HOOK_OFFSETLESS(ErrorCode, il2cpp_utils::FindMethodUnsafe("", "ConnectionFailedReasonMethods", "ErrorCode", 1));
}