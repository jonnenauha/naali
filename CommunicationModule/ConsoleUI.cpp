#include "StableHeaders.h"
#include "Foundation.h"

#include "ConsoleUI.h"

namespace CommunicationUI
{
	ConsoleUI::	ConsoleUI(Foundation::Framework* framework): framework_(framework), default_connection_(NULL), default_chat_session_(NULL)
	{
		communication_manager_ = TpQt4Communication::CommunicationManager::GetInstance();
		PublishConsoleCommands();
		LogInfo("ConsoleUI initialized.");
	}

	ConsoleUI::~ConsoleUI(void)
	{
	}

	void ConsoleUI::PublishConsoleCommands()
	{
		boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();

        if (console_service)
        {
			console_service->RegisterCommand(Console::CreateCommand("comm_state", "Show general state of comm module", Console::Bind(this, &ConsoleUI::OnCommandState)));
			console_service->RegisterCommand(Console::CreateCommand("comm_help", "Lists all connection", Console::Bind(this, &ConsoleUI::OnCommandHelp)));
			console_service->RegisterCommand(Console::CreateCommand("comm_connections", "Lists all connection", Console::Bind(this, &ConsoleUI::OnCommandConnections)));
			console_service->RegisterCommand(Console::CreateCommand("comm_login", "Login to jabber server: comm_login(uid, pwd, server, port)", Console::Bind(this, &ConsoleUI::OnCommandLogin)));
			console_service->RegisterCommand(Console::CreateCommand("comm_user", "Show information about current user", Console::Bind(this, &ConsoleUI::OnCommandUser)));
		}
		else
		{
			// TODO: Error message
		}
	}

	Console::CommandResult ConsoleUI::OnCommandState(const Core::StringVector &params)
	{
		std::string result;
		result.append("CommunicationManager is ");
		if (communication_manager_->GetState() == TpQt4Communication::CommunicationManager::STATE_INITIALIZING)
			result.append("initializing...");
		if (communication_manager_->GetState() == TpQt4Communication::CommunicationManager::STATE_READY)
			result.append("ready.");
		if (communication_manager_->GetState() == TpQt4Communication::CommunicationManager::STATE_ERROR)
			result.append("failed.");
		return Console::ResultSuccess(result);
	}

	Console::CommandResult ConsoleUI::OnCommandHelp(const Core::StringVector &params)
	{
		return Console::ResultFailure("Not implemented.");
	}

	Console::CommandResult ConsoleUI::OnCommandConnections(const Core::StringVector &params)
	{
		std::string result = "";
		TpQt4Communication::ConnectionList list = communication_manager_->GetAllConnections();
		if (list.size() == 0)
			result.append("No connection.");
		else
		{
			result.append("Connections:");
			for (TpQt4Communication::ConnectionList::iterator i = list.begin(); i != list.end(); i++)
			{
				TpQt4Communication::Connection* connection = (TpQt4Communication::Connection*)*i;

				result.append(connection->GetID());
				result.append(" ");
				if (connection->GetState() == TpQt4Communication::Connection::STATE_CONNECTING)
					result.append("[Connecting...]");
				if (connection->GetState() == TpQt4Communication::Connection::STATE_OPEN)
					result.append("[Open]");
				if (connection->GetState() == TpQt4Communication::Connection::STATE_ERROR)
					result.append("[Error]");
				if (connection->GetState() == TpQt4Communication::Connection::STATE_CLOSED)
					result.append("[Closed]");

				result.append(connection->GetProtocol());
				result.append("  ");
				result.append(connection->GetUser()->GetUserID());
			}
		}
		return Console::ResultSuccess(result);
		//return Console::ResultFailure("Not implemented.");
	}

	Console::CommandResult ConsoleUI::OnCommandLogin(const Core::StringVector &params)
	{
		if ( params.size() != 4 && params.size() != 0 )
		{	
			std::string reason = "Wrong number of arguments!\ncomm_login(<address>,<pwd>,<server>,<port>)";
			return Console::ResultFailure(reason);
		}

		std::string result = "";

		TpQt4Communication::Credentials credentials;
		credentials.LoadFromFile("data/connection.ini");
		default_connection_ = communication_manager_->OpenConnection(credentials);
		if (default_connection_ == NULL)
			result = "Cannot create a IM connection.";
		else
			result = "ok.";

		return Console::ResultSuccess(result);
	}

	Console::CommandResult ConsoleUI::OnCommandUser(const Core::StringVector &params)
	{
		std::string result = "";

		if (!default_connection_)
			return Console::ResultSuccess("No connections.");

		std::string user_id = default_connection_->GetUser()->GetUserID();
		std::string protocol = default_connection_->GetUser()->GetProtocol();
		std::string presence_status = default_connection_->GetUser()->GetPresenceStatus()->GetStatusText();
		std::string presence_message = default_connection_->GetUser()->GetPresenceStatus()->GetMessageText();
		result.append("user: ");
		result.append(user_id);
		result.append("\n");
		result.append("protocol: ");
		result.append(protocol);
		result.append("\n");
		result.append("presence: ");
		result.append(presence_status);
		result.append(", ");
		result.append(presence_message);
		result.append("\n");

		return Console::ResultSuccess(result);
	}

} //end if namespace: CommunicationUI