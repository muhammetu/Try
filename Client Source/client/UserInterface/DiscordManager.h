#pragma once
#include "Stdafx.h"

#ifdef ENABLE_DISCORD_UTILITY
#include <discord/discord.h>

struct DiscordLobbyData
{
	discord::LobbyId lobbyId{};
	discord::LobbySecret secret{};
};

class CDiscordManager : public CSingleton<CDiscordManager>
{
	public:
		struct DiscordState
		{
			discord::User currentUser;
			std::unique_ptr<discord::Core> core;
		};

#ifdef USE_DISCORD_RPC_MODULE
		enum EDiscordPresenceStatus
		{
			Discord_Status_Closed,
			Discord_Status_PVM,
			Discord_Status_PVP,
		};
#endif

	public:
		CDiscordManager();
		virtual ~CDiscordManager();

		void InitSDK();
#ifdef USE_DISCORD_RPC_MODULE
		void InitRPCModule();
#endif // USE_DISCORD_RPC_MODULE
#ifdef USE_DISCORD_LOBBY_MODULE
		void InitLobbyModule();
#endif // USE_DISCORD_LOBBY_MODULE
		void ReleaseSDK();
		void OnUpdate();

		std::string	GetCurrentUserName() const { return currentUserName; }

#ifdef USE_DISCORD_RPC_MODULE
		// Rich Presence Support
		std::string GetStateText(const EDiscordPresenceStatus status);
		std::string GetDetailText(const EDiscordPresenceStatus status, const DWORD mainCharacterVID);
		std::string GetRaceName(const DWORD mainCharacterVID);
		std::string GetRaceImage(const DWORD mainCharacterVID);
		std::string GetEmpireName(const DWORD mainCharacterVID);
		std::string GetEmpireImage(const DWORD mainCharacterVID);
		void UpdatePresence(const EDiscordPresenceStatus status, const DWORD mainCharacterVID);
#ifdef USE_DISCORD_PRESENCE_NON_SDK
		void StartPresence();
		void ResetPresence();
#endif // USE_DISCORD_PRESENCE_NON_SDK
#endif // USE_DISCORD_RPC_MODULE

#ifdef USE_DISCORD_LOBBY_MODULE
	// Lobby Support
		void ResetLobbyID();
		void DisconnectLobby();
		void ConnectToLobby(const DiscordLobbyData& data);
		void CreateLobby(uint32_t capacity, discord::LobbyType type);
#endif // USE_DISCORD_LOBBY_MODULE

	protected:
		DiscordState	currentState;
#ifdef USE_DISCORD_LOBBY_MODULE
		discord::LobbyId m_lobbyId;
		discord::LobbySecret m_lobbySecret;
#endif // USE_DISCORD_LOBBY_MODULE

	private:
		bool			m_bInitialized;
		time_t			m_tStartTimeStamp;
		std::string		currentUserName;
};
#endif