#include "controler.h"

#include <QTimer>

Controler::Controler(QTcpSocket *socket, QObject *parent)
	: QObject(parent), m_confirmedConnection(true)
{
	m_socket = socket;
	if(socket == nullptr)
		throw "Null socket";

	m_setSocketSettings();

	qDebug() << "CONTROLER CREATED";
}

void Controler::addClient(const CheckConnectionResult &connection, AbstractClient *client)
{
	client->setParent(this);

	connect(client, &AbstractClient::readyCommand, this, &Controler::onReadyCommand);

	client->confirmConnect2Controler(connection == Admin);

	updateRooms(client);

	m_clients.append(client);

	if(connection == Controler::Admin)
	{
		client->setIsAdmin(true);
		client->sendUsersInfoToAdmin(m_users);
	}
}

void Controler::addUser(const QString &login, const QString &pass)
{
	::User u(login.toStdString(), pass.toStdString());
	m_users.append(u);
}

void Controler::onConnected()
{
	qDebug() << "CONTROLER CONNECTED";
}

void Controler::onDisconnected()
{
	qDebug() << "CONTROLER DISCONNECTED";
	emit disconnected();
}

void Controler::onReadyRead()
{
	QByteArray data = m_socket->readAll();
	onReadyReadCheck(data);
}

void Controler::onReadyReadCheck(QByteArray data)
{
	static int alldatasize = 0;
	static bool writedata = false;
	static QByteArray allData = "";
	if(allData.isEmpty())
	{
		writedata = true;
		char* temp_size = new char[4];
		for (unsigned int i = 0; i < 4; i++)
		{
			temp_size[i] = data[i];
		}


		alldatasize = *reinterpret_cast<unsigned int*>(temp_size) + 4;

	}

	if(!writedata)
		return;
	allData += data;
	if(allData.size() == alldatasize)
	{
		writedata = false;
		onReadyReadProccess(allData);
		allData.clear();
	}else if(allData.size() > alldatasize)
	{
		QByteArray firstData = allData.left(alldatasize);
		onReadyReadProccess(firstData);
		QByteArray secondData = allData.right(allData.size() - firstData.size());
		allData.clear();
		onReadyReadCheck(secondData);
	}
}

void Controler::onReadyReadProccess(QByteArray data)
{
	Buffer buffer = Buffer::fromBytes(data);
	Commands commands = Commands::fromBuffer(buffer);
	for(unsigned int i = 0; i < commands.commands.size(); i++)
	{
		Command command = commands.commands[i];
		parseCommand(command);
	}
}

void Controler::onReadyCommand(const Command &command)
{
	AbstractClient *client = static_cast<AbstractClient *>(sender());

	switch(command.title())
	{
		case Command::Control:
		{
			switch(command.controlAction())
			{
				case Command::AddRoom:
				{
					Buffer buffer = command.toBuffer();
					char *s = buffer.toBytes();
					m_socket->write(s, buffer.fullSize());
					delete[] s;
					break;
				}
				case Command::AddItem:
				{
					Buffer buffer = command.toBuffer();
					char *s = buffer.toBytes();
					m_socket->write(s, buffer.fullSize());
					delete[] s;
					break;
				}
				case Command::TurnItem:
				{
					Buffer buffer = command.toBuffer();
					char *s = buffer.toBytes();
					m_socket->write(s, buffer.fullSize());
					delete[] s;

					buffer = command.buffer();
					Item item = Item::fromBuffer(buffer);
					break;
				}
				case Command::UpdateItems:
				{
					Buffer buffer = command.toBuffer();
					char *s = buffer.toBytes();
					m_socket->write(s, buffer.fullSize());
					delete[] s;
					break;
				}
				case Command::UpdateRooms:
				{
					Buffer buffer = command.toBuffer();
					char *s = buffer.toBytes();
					m_socket->write(s, buffer.fullSize());
					delete[] s;
					break;
				}
				default:
				{
					qDebug() << "ERROR COMMAND TITLE";
					return;
				}
			}
			break;
		}
		case Command::Settings:
		{
			switch (command.settingsAction())
			{
				case Command::ChangeUserPassword:
				{
					std::vector<std::string> dataList = command.buffer().split('|');
					QString login = QString::fromStdString(dataList[0]);
					QString newpassword = QString::fromStdString(dataList[1]);


					break;
				}
				case Command::AddUser:
				{
					::User u;
					Buffer buffer = command.buffer();
					BufferStream stream (&buffer, BufferStream::ReadOnly);

					stream >> u;

					m_users.append(u);

					updateUsers();
					break;
				}
			}
			break;
		}
		default:
			break;
	}
}

void Controler::m_setSocketSettings()
{
	connect(m_socket, &QTcpSocket::connected, this, &Controler::onConnected);
	connect(m_socket, &QTcpSocket::disconnected, this, &Controler::onDisconnected);
	connect(m_socket, &QTcpSocket::readyRead, this, &Controler::onReadyRead);
}

bool Controler::confirmedConnection() const
{
	return m_confirmedConnection;
}

void Controler::setConfirmedConnection(bool confirmedConnection)
{
	m_confirmedConnection = confirmedConnection;
}

Controler::CheckConnectionResult
Controler::checkConnectionData(const QString &login,
							   const QString &password)
{
	if(login == "admin" && password == "admin")
		return Controler::Admin;
	for(auto user : m_users)
	{
		if(user.login() == login.toStdString() && user.password() == password.toStdString())
			return Controler::User;
	}
	return Controler::Unknown;
}

void Controler::changeSocket(QTcpSocket *socket)
{
	m_socket->disconnect();
	m_socket->deleteLater();
	m_socket = socket;

	m_setSocketSettings();
}

void Controler::parseCommand(const Command &command)
{
	Command::Title title = command.title();
	switch (title)
	{
		case Command::Control:
		{
			Command::ControlAction controlAction = command.controlAction();
			switch (controlAction)
			{
				case Command::UpdateItems:
				{
					return;
					Buffer buffer = command.buffer();
					//m_items = Items::fromBuffer(buffer);

					for(auto client : m_clients)
					{
						updateItems(client);
					}

					break;
				}
				case Command::UpdateRooms:
				{

					Buffer buffer = command.buffer();
					m_rooms = Rooms::fromBuffer(buffer);
					for(auto client : m_clients)
					{
						updateRooms(client);
					}

					break;
				}
				case Command::TriggeredItem:
				{
					Buffer buffer = command.buffer();
					Item item = Item::fromBuffer(buffer);
					for(auto client : m_clients)
					{
						triggeredItem(item, client);
					}
					break;
				}
				default:
					break;
			}
			break;
		}
		case Command::ConfirmConnection:
		{
			m_confirmedConnection = true;
			break;
		}
		default:
			break;
	}
}

void Controler::updateItems(AbstractClient *client)
{
	Buffer buffer = m_rooms.toBuffer();
	Command command(Command::Control, Command::UpdateItems, buffer);
	buffer = command.toBuffer();
	char *s = buffer.toBytes();
	client->m_socket->write(s, buffer.fullSize());
	delete[] s;
}

void Controler::updateRooms(AbstractClient *client)
{
	Buffer buffer = m_rooms.toBuffer();
	Command command(Command::Control, Command::UpdateRooms, buffer);
	buffer = command.toBuffer();
	char *s = buffer.toBytes();
	client->m_socket->write(s, buffer.fullSize());
	delete[] s;
}

void Controler::triggeredItem(Item item, AbstractClient *client)
{
	Buffer buffer = item.toBuffer();
	Command command(Command::Control, Command::TriggeredItem, buffer);
	buffer = command.toBuffer();
	char *s = buffer.toBytes();
	client->m_socket->write(s, buffer.fullSize());
	delete[] s;
}

void Controler::updateUsers()
{
	for(auto client : m_clients)
	{
		if(!client->isAdmin())
			continue;
		client->sendUsersInfoToAdmin(m_users);
	}
}

QString Controler::key() const
{
	return m_key;
}

void Controler::setKey(const QString &key)
{
	m_key = key;
	qDebug() << "KEY:" << key;
}

QList<AbstractClient *> Controler::clients() const
{
	return m_clients;
}
