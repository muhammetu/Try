#include "Stdafx.h"
#include "resource.h"
#include "Locale_inc.h"

#ifdef ENABLE_DISCORD_UTILITY
#include "DiscordManager.h"
#include "PythonCharacterManager.h"
#include "PythonBackground.h"
#include "PythonPlayer.h"
#include "PythonGuild.h"

#include "InstanceBase.h"

#include <csignal>
#include <array>
#include <iostream>

#include <fmt/fmt.h>
#include <fmt/core.h>

#ifdef USE_DISCORD_PRESENCE_NON_SDK
#include "discord_rpc.h"
#include "discord_register.h"
#endif // USE_DISCORD_PRESENCE_NON_SDK

namespace
{
	volatile bool g_interrupted{false};
}
void __cdecl OnSignal(int signal)
{
	g_interrupted = true;
	CDiscordManager::Instance().DisconnectLobby();
}

void CDiscordManager::InitSDK()
{
	if (m_bInitialized)
		return;

	// Create SDK Main
	discord::Core*	core{};
	discord::Result	result = discord::Core::Create(discordID, DiscordCreateFlags_NoRequireDiscord, &core);

	// Reset Core
	currentState.core.reset(core);

	if (!currentState.core)
	{
#ifdef _DEBUG
		TraceError("Discord SDK Cant Created. Error Code(%d)", result);
#endif
		return;
	}

#ifdef USE_DISCORD_RPC_MODULE
	InitRPCModule();
#endif // USE_DISCORD_RPC_MODULE

#ifdef USE_DISCORD_LOBBY_MODULE
	InitLobbyModule();
#endif // USE_DISCORD_LOBBY_MODULE

	std::signal(SIGINT, OnSignal);

#ifdef _DEBUG
	TraceError("Discord SDK Has Initialized.");
#endif

	m_tStartTimeStamp = time(nullptr);
	m_bInitialized = true;
}

#ifdef USE_DISCORD_RPC_MODULE
void CDiscordManager::InitRPCModule()
{
	currentState.core->UserManager().OnCurrentUserUpdate.Connect([&]() {
		currentState.core->UserManager().GetCurrentUser(&currentState.currentUser);
		currentUserName = std::string(currentState.currentUser.GetUsername()) + "#" + std::string(currentState.currentUser.GetDiscriminator());
#ifdef _DEBUG
		TraceError("Current user updated: %s %s", currentState.currentUser.GetUsername(), currentState.currentUser.GetDiscriminator());
#endif
	});

	currentState.core->ActivityManager().OnActivityJoin.Connect([&](const char* secret) {
		TraceError("Discord SDK On Activity Join %s", secret);
	});
	currentState.core->ActivityManager().OnActivitySpectate.Connect([&](const char* secret) {
		TraceError("Discord SDK On Activity Join %s", secret);
	});
	currentState.core->ActivityManager().OnActivityJoinRequest.Connect([&](discord::User const& user) {
#ifdef _DEBUG
		TraceError("Discord SDK On Activity Join %s", user.GetUsername() );
#endif
	});

#ifdef USE_DISCORD_PRESENCE_NON_SDK
	StartPresence();
#endif // USE_DISCORD_PRESENCE_NON_SDK
}
#endif // USE_DISCORD_RPC_MODULE

#ifdef USE_DISCORD_LOBBY_MODULE
void CDiscordManager::InitLobbyModule()
{
	// Lobby hooks
	currentState.core->LobbyManager().OnLobbyUpdate.Connect([&](int64_t lobbyId) {
#ifdef _DEBUG
		Tracenf("[Discord] OnLobbyUpdate: %lld", lobbyId);
#endif
	});
	currentState.core->LobbyManager().OnLobbyDelete.Connect([&](int64_t lobbyId, uint32_t reason) {
#ifdef _DEBUG
		Tracenf("[Discord] OnLobbyDelete: %lld %u", lobbyId, reason);
#endif
		ResetLobbyID();
	});
	currentState.core->LobbyManager().OnMemberConnect.Connect([&](int64_t lobbyId, int64_t userId) {
#ifdef _DEBUG
		Tracenf("[Discord] OnMemberConnect: %lld %u", lobbyId, userId);
#endif
	});
	currentState.core->LobbyManager().OnMemberUpdate.Connect([&](int64_t lobbyId, int64_t userId) {
#ifdef _DEBUG
		Tracenf("[Discord] OnMemberUpdate: %lld %u", lobbyId, userId);
#endif
	});
	currentState.core->LobbyManager().OnMemberDisconnect.Connect([&](int64_t lobbyId, int64_t userId) {
#ifdef _DEBUG
		Tracenf("[Discord] OnMemberDisconnect: %lld %u", lobbyId, userId);
#endif
		if (userId == currentState.currentUser.GetId())
			ResetLobbyID();
	});

	currentState.core->LobbyManager().OnSpeaking.Connect([&](int64_t lobbyId, int64_t userId, bool speaking) {
#ifdef _DEBUG
		Tracenf("[Discord] OnSpeaking: %lld %lld %d", lobbyId, userId, speaking ? 1 : 0);
#endif
	});
}
#endif // USE_DISCORD_LOBBY_MODULE

void CDiscordManager::ReleaseSDK()
{
	if (!m_bInitialized)
		return;

	m_bInitialized = false;
}

void CDiscordManager::OnUpdate()
{
	if (m_bInitialized && currentState.core)
		currentState.core->RunCallbacks();
}

#ifdef USE_DISCORD_RPC_MODULE
// Rich Presence Suppor
std::string CDiscordManager::GetStateText(const EDiscordPresenceStatus status)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();
	switch (status)
	{
		case Discord_Status_PVM:
			{
				return fmt::format("Lokasyon : {} (PvM)", rkBG.GetRealMapName(rkBG.GetWarpMapName()));
			}
			break;
		
		case Discord_Status_PVP:
			{
				return fmt::format("Lokasyon : {} (PvP)", rkBG.GetRealMapName(rkBG.GetWarpMapName()));
			}
			break;
	}

	return "Bilinmiyor...";
}

std::string CDiscordManager::GetDetailText(const EDiscordPresenceStatus status, const DWORD mainCharacterVID)
{
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(mainCharacterVID);
	if (!pMainInstance)
		return "Bilinmiyor...";

	std::string ret = "Oyuncu :";

	ret += fmt::format(" {}, Lv.{}\n", CPythonPlayer::Instance().GetName(), CPythonPlayer::Instance().GetStatus(POINT_LEVEL));

	// Guild
	std::string guildStr;
	if (CPythonGuild::Instance().GetGuildName(CPythonPlayer::Instance().GetGuildID(), &guildStr))
		ret += fmt::format("Lonca - {}\n", guildStr);

	return ret;
}

std::string CDiscordManager::GetRaceName(const DWORD mainCharacterVID)
{
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(mainCharacterVID);
	if (!pMainInstance)
		return "Savaþçý";

	switch (pMainInstance->GetRace())
	{
		case 0:
		case 4:
			return "Savaþçý";
			break;
		
		case 1:
		case 5:
			return "Ninja";
			break;
		
		case 2:
		case 6:
			return "Sura";
			break;
		
		case 3:
		case 7:
			return "Þaman";
			break;

		case 8:
			return "Lycan";
			break;
	}

	return "Savaþçý";
}

std::string CDiscordManager::GetRaceImage(const DWORD mainCharacterVID)
{
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(mainCharacterVID);
	if (!pMainInstance)
		return "warrior_m";
	
	switch (pMainInstance->GetRace())
	{
		case 0:
			return "warrior_m";
			break;
		case 4:
			return "warrior_w";
			break;
		
		case 1:
			return "assassin_w";
			break;
		case 5:
			return "assassin_m";
			break;
		
		case 2:
			return "sura_m";
			break;
		case 6:
			return "sura_w";
			break;
		
		case 3:
			return "shaman_w";
			break;
		case 7:
			return "shaman_m";
			break;
	}

	return "warrior_m";
}

std::string CDiscordManager::GetEmpireName(const DWORD mainCharacterVID)
{
	std::string ret = "Krallýk: ";
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(mainCharacterVID);
	if (!pMainInstance)
		return ret += "Shinsoo";
	
	switch (pMainInstance->GetEmpireID())
	{
		case 1:
			ret += "Shinsoo";
			break;
		
		case 2:
			ret += "Chunjo";
			break;
		
		case 3:
			ret += "Jinno";
			break;

		default:
			ret += "Shinsoo";
			break;
	}

	return ret;
}

std::string CDiscordManager::GetEmpireImage(const DWORD mainCharacterVID)
{
	CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(mainCharacterVID);
	if (!pMainInstance)
		return "empire_1";

	return fmt::format("empire_%d", pMainInstance->GetEmpireID());
}

void CDiscordManager::UpdatePresence(const EDiscordPresenceStatus status, const DWORD mainCharacterVID)
{
	if (!currentState.core || !m_bInitialized)
		return;

#ifdef USE_DISCORD_PRESENCE_NON_SDK
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.startTimestamp = m_tStartTimeStamp;

	discordPresence.state = "Vatansever Oynuyor";
	discordPresence.details = "www.hermes2.com.tr";
	discordPresence.largeImageKey = "None";
	discordPresence.largeImageText = "";
	discordPresence.smallImageKey = "None";
	discordPresence.smallImageText = "";
	discordPresence.buttonLabel = "Hermes2 Discord";
	discordPresence.buttonURL = "https://discord.gg/hermes2";
	Discord_UpdatePresence(&discordPresence);
#else
	discord::Activity activity;
	ZeroMemory(&activity, sizeof(activity));
	activity.SetDetails(GetDetailText(status, mainCharacterVID).c_str());
	activity.SetState(GetStateText(status).c_str());
	activity.GetAssets().SetSmallImage(GetEmpireImage(mainCharacterVID).c_str());
	activity.GetAssets().SetSmallText(GetEmpireName(mainCharacterVID).c_str());
	activity.GetAssets().SetLargeImage(GetRaceImage(mainCharacterVID).c_str());
	activity.GetAssets().SetLargeText(GetRaceName(mainCharacterVID).c_str());
	activity.GetSecrets().SetMatch("xyzzy");
	activity.GetSecrets().SetJoin("https://discord.gg/s3XY5Spe6f");
	activity.GetSecrets().SetSpectate("look");
	activity.SetApplicationId(discordID);
	activity.SetType(discord::ActivityType::Playing);
	activity.GetTimestamps().SetStart(m_tStartTimeStamp);

	currentState.core->ActivityManager().UpdateActivity(activity, [&](discord::Result result) {});
#endif // USE_DISCORD_PRESENCE_NON_SDK
}

#ifdef USE_DISCORD_PRESENCE_NON_SDK
void CDiscordManager::StartPresence()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	Discord_Initialize(std::to_string(discordID).c_str(), &handlers, 1, nullptr);
}

void CDiscordManager::ResetPresence()
{
	Discord_Shutdown();
}
#endif // USE_DISCORD_PRESENCE_NON_SDK
#endif // USE_DISCORD_RPC_MODULE

#ifdef USE_DISCORD_LOBBY_MODULE
void CDiscordManager::ResetLobbyID()
{
	m_lobbyId = 0;
}

void CDiscordManager::DisconnectLobby()
{
	// Disconnect than lobby
	if (m_lobbyId)
	{
		currentState.core->LobbyManager().DisconnectVoice(m_lobbyId, [&](discord::Result result) {
#ifdef _DEBUG
			Tracenf("[Discord] DisconnectVoice ret: %d", result);
#endif
		});

		int32_t member_count = 0;
		const auto ret = currentState.core->LobbyManager().MemberCount(m_lobbyId, &member_count);
		Tracenf("[Discord] Lobby member count: %d", member_count);

		if (ret != discord::Result::Ok || member_count == 1)
		{
			currentState.core->LobbyManager().DeleteLobby(m_lobbyId, [&](discord::Result result) {
#ifdef _DEBUG
				Tracenf("[Discord] DeleteLobby ret: %d", result);
#endif
			});
		} else {
			currentState.core->LobbyManager().DisconnectLobby(m_lobbyId, [&](discord::Result result) {
#ifdef _DEBUG
				Tracenf("[Discord] DisconnectLobby ret: %d", result);
#endif
			});
		}
	}

	ResetLobbyID();
}

void CDiscordManager::ConnectToLobby(const DiscordLobbyData& data)
{
#ifdef _DEBUG
	Tracenf("[Discord] ConnectToLobby | Target lobby: %lld secret: %s", data.lobbyId, data.secret);
#endif

	// Connect to already exist lobby
	currentState.core->LobbyManager().ConnectLobby(data.lobbyId, data.secret, [&](discord::Result result, const discord::Lobby& lobby) {
		if (result == discord::Result::Ok)
		{
#ifdef _DEBUG
			Tracenf("[Discord] Succesfully connected to lobby: %lld", data.lobbyId);
#endif
			currentState.core->LobbyManager().SendLobbyMessage(m_lobbyId, (PBYTE)"\x54\x45\x53\x54", sizeof("\x54\x45\x53\x54"), [&](discord::Result result) {});

			currentState.core->LobbyManager().ConnectVoice(lobby.GetId(), [&](discord::Result result) {
				if (result != discord::Result::Ok)
				{
#ifdef _DEBUG
					TraceError("[Discord] Could not connected to voice channel. Error: %d", result);
#endif
					return;
				}
#ifdef _DEBUG
				Tracenf("[Discord] Connected to voice channel");
#endif
			});

			m_lobbyId = data.lobbyId;
		}
		else
		{
#ifdef _DEBUG
			TraceError("[Discord] Could not connected to lobby. Error: %d", result);
#endif
		}
	});
}

void CDiscordManager::CreateLobby(uint32_t capacity, discord::LobbyType type)
{
	// Create the transaction
	discord::LobbyTransaction transaction{};
	auto result = currentState.core->LobbyManager().GetLobbyCreateTransaction(&transaction);
	if (result != discord::Result::Ok)
	{
#ifdef _DEBUG
		TraceError("[Discord] Failed creating lobby transaction. Error: %d", result);
#endif
		return;
	}

	// Set lobby information
	transaction.SetCapacity(capacity);
	transaction.SetType(type);

	// Create it!
	currentState.core->LobbyManager().CreateLobby(transaction, [&](discord::Result result, const discord::Lobby& lobby) {
		if (result == discord::Result::Ok)
		{
#ifdef _DEBUG
			Tracenf("[Discord] %s lobby %lld created with secret %s", (result == discord::Result::Ok) ? "Succeeded" : "Failed", lobby.GetId(), lobby.GetSecret());
#endif
			currentState.core->LobbyManager().ConnectVoice(lobby.GetId(), [&](discord::Result result) {
				if (result != discord::Result::Ok)
				{
#ifdef _DEBUG
					TraceError("Could not connected to voice channel. Error: %d", result);
#endif
					return;
				}
#ifdef _DEBUG
				Tracenf("[Discord] Connected to voice channel");
#endif
			});

			m_lobbyId = lobby.GetId();
			m_lobbySecret = lobby.GetSecret();
		}
		else
		{
#ifdef _DEBUG
			TraceError("[Discord] Could not created the lobby. Error: %d", result);
#endif
		}
	});
}
#endif // USE_DISCORD_LOBBY_MODULE

CDiscordManager::CDiscordManager()
{
	m_bInitialized = false;
	m_tStartTimeStamp = 0;
	currentUserName.clear();
	ResetLobbyID();
}

CDiscordManager::~CDiscordManager()
{
}
#endif