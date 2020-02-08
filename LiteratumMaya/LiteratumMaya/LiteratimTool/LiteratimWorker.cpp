#include "LiteratimWorker.h"
#include "LiteratimTool/LiteratimNetworking.h"


LiteratimWorker::LiteratimWorker() 
{
	m_WorkerThread = std::thread(&LiteratimWorker::TickThread, this);
}

LiteratimWorker::~LiteratimWorker()
{
	m_PendingKill = true;
	m_WorkerThread.join();
}

void LiteratimWorker::SetConnection(LiteratimNetworking* LitNetwork)
{
	m_LitNetwork = LitNetwork;
}

bool LiteratimWorker::IsReadForNextMesh() const
{
	return m_ActiveMesh == nullptr;
}

void LiteratimWorker::QueryThisMesh(LiteratimMesh* LitMesh)
{
    std::lock_guard<std::mutex> guard(m_MU_TaskAccses);
	m_ActiveMesh = LitMesh;
}

void LiteratimWorker::SendDeferredCommand(std::string& sendCommand)
{
	std::lock_guard<std::mutex> guard(m_MU_MessageListAccses);
	m_MessageToSend.push_back(sendCommand);
}

void LiteratimWorker::TickThread()
{

	while (!m_PendingKill)
	{
		std::lock_guard<std::mutex> guard(m_MU_TaskAccses);

		if (m_ActiveMesh && m_LitNetwork)
		{
			if (!m_ActiveMesh->IsQuryDone())
			{
				m_ActiveMesh->RunQuery(m_LitNetwork);
			}
			else
			{
				m_ActiveMesh = nullptr;
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::microseconds(0));
		}

		if (m_LitNetwork)
		{
			while (m_MessageToSend.size())
			{
				std::lock_guard<std::mutex> guard(m_MU_MessageListAccses);
				std::string& command = m_MessageToSend.front();
				m_LitNetwork->LitSendMessage(command, ResponceHeaders::Command);
				m_MessageToSend.pop_front();
			}
		}
		else
		{
			m_MessageToSend.clear();
		}
	}
}

