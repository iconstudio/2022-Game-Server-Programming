#pragma once

class Framework
{
public:
	Framework(const char* title, unsigned width, unsigned height);
	~Framework();
	
	void Awake();
	void Start(const char* ip_address, unsigned short port_number);
	void Update();
	void PrepareRendering();
	void Render();
	void Close();
	void Release();

	shared_ptr<Session> CreateNPC(PID id);
	shared_ptr<Session> CreatePlayer(PID id);
	std::pair<PID, shared_ptr<Session>> CreateSession(PID id, bool is_player);
	void RemoveSession(PID id);

	shared_ptr<GameObject> CreateInstance();

	sf::Socket::Status TryReceive(const size_t buffer_limit);
	void ProcessStream();
	void ProcessPacket(const void* packet);
	void SendPacket(const Packet& packet);
	void SendPacket(Packet&& packet);
	void SendPacket(const void* ptr, size_t size);

	shared_ptr<GameSprite> CreateSprite(const Filepath& path, float ox, float oy);
	shared_ptr<GameSprite> CreateSprite(const Filepath& path, float ox, float oy, int tx, int ty, int w, int h);
	shared_ptr<GameSprite> AddSprite(const char* name, shared_ptr<GameSprite> handle);
	shared_ptr<GameSprite> LoadSprite(const char* name, const Filepath& path, float ox, float oy);
	shared_ptr<GameSprite> LoadSprite(const char* name, const Filepath& path, float ox, float oy, int tx, int ty, int w, int h);

	bool IsOpened() const;

	shared_ptr<GameSprite> GetSprite(const char* name) const;

	PID myID;
	shared_ptr<Session> mySession;
	shared_ptr<GameObject> myAvatar;

	std::unordered_map<PID, shared_ptr<GameObject>> everyInstances;
	std::unordered_map<PID, shared_ptr<Session>> everyPlayers;
	std::unordered_map<PID, shared_ptr<Session>> everyNPCs;
	std::unordered_map<string, shared_ptr<GameSprite>> everySprites;
	float view_x, view_y;

	//std::thread clientWorker;
	//std::thread clientRenderer;

	const std::string clientTitle;
	const int clientW, clientH;
	sf::RenderWindow* clientPanel;
	sf::Font defaultFont;

	unsigned char recvBuffer[BUF_SIZE];
	size_t recvSize;
	unsigned char savedBuffer[512];
	size_t savedSize;
};

void CallbackNetwork(Framework& framework);
void CallbackRender(Framework& framework);
