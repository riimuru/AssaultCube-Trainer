#include "../include/render.h"

Render render;

void Render::Text(char *text, int x, int y, int orientation, DWORD color, ID3DXFont *pFont)
{
    RECT rect = {x, y, x + 120, y + 16};

    switch (orientation)
    {
    case lefted:
        pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, color);
        break;
    case centered:
        pFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, color);
        break;
    case righted:
        pFont->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, color);
        break;
    }
}

void Render::initFonts()
{

    IDirect3DDevice9 *pDevice;
    pDevice = d3ddev;

    D3DXCreateFont(pDevice, fontSize, 0, FW_BOLD, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, fontName, &font);
}

void Render::InitLine()
{
    IDirect3DDevice9 *pDevice;
    pDevice = d3ddev;

    D3DXCreateLine(pDevice, &line);
}

void Render::DrawLine(float x1, float y1, float x2, float y2, float w, bool antialias, DWORD color)
{
    D3DXVECTOR2 position[] = {D3DXVECTOR2(x1, y1), D3DXVECTOR2(x2, y2)};

    line->SetWidth(w);

    if (antialias)
        line->SetAntialias(1);

    line->Begin();

    line->Draw(position, 2, color);

    line->End();
}

void Render::DrawBorderBox(float x, float y, float c, float h, float w, bool antialias, DWORD color)
{
    DrawLine((x + c), y, (x + c), h, w, antialias, color); // right side
    DrawLine((x - c), y, (x - c), h, w, antialias, color); // left side
    DrawLine((x - c), y, (x + c), y, w, antialias, color); // bottom side
    DrawLine((x - c), h, (x + c), h, w, antialias, color); // top side
}

void Render::initD3D(HWND hwnd)
{
    // create the Direct3D interface
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    // create a struct to hold various device information
    D3DPRESENT_PARAMETERS d3dpp;
    // clear out the struct for use
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    // program windowed, not fullscreen
    d3dpp.Windowed = TRUE;
    // discard old frames
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    // set the window to be used by Direct3D
    d3dpp.hDeviceWindow = hwnd;

    d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);

    initFonts();
    InitLine();
}

void Render::render()
{
    d3ddev->BeginScene(); // begins the 3D scene
    // clear the window alpha
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

    IDirect3DDevice9 *pDevice;
    pDevice = d3ddev;

    D3DVIEWPORT9 screen;

    // Get the screen
    pDevice->GetViewport(&screen);

    if (font)
        font->OnLostDevice();

    if ((GetKeyState(VK_F1) & 0x8000) != 0)
        isEspOn = !isEspOn;

    if ((GetKeyState(VK_F2) & 0x8000) != 0)
        isAimBotOn = !isAimBotOn;

    ScanMemory();

    GetAllPlayers();

    if (isEspOn)
        Esp();

    if (isAimBotOn)
        AimBot();

    d3ddev->EndScene();                      // ends the 3D scene
    d3ddev->Present(NULL, NULL, NULL, NULL); // displays the created frame on the screen
}

void Render::cleanD3D()
{
    d3ddev->Release(); // close and release the 3D device
    d3d->Release();    // close and release Direct3D
}

void Render::ScanMemory()
{
    GameHWND = FindWindowA(0, "AssaultCube");

    GetWindowThreadProcessId(GameHWND, &pid);
    pHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, pid);
    moduleBase = GetModuleBaseAddress(pid, _T("ac_client.exe"));

    entityList = RPM<DWORD>(moduleBase + entityListOffset);
    matrix = RPM<ViewMatrix>(moduleBase + viewMatrixOffset);
    playersNum = RPM<int>(moduleBase + playersNumOffset);
    localPlayerObj = RPM<DWORD>(moduleBase + localPlayerOffset);

    m_pos = RPM<Vector3>(localPlayerObj + vec3_origin);
    m_teamID = RPM<int>(localPlayerObj + teamIdOffset);
}

void Render::GetAllPlayers()
{

    if (!playersList.empty())
        playersList.clear();

    // this will get the number of players in game dynamically.
    for (int i = 1; i < playersNum + 1; i++)
    {
        Player *player;

        DWORD entityObj = RPM<DWORD>(entityList + (i * 0x4));

        if (entityObj == NULL)
            continue;

        PlayerName playerName = RPM<PlayerName>(entityObj + nameOffset);
        Vector3 pos = RPM<Vector3>(entityObj + vec3_origin);

        int health = RPM<int>(entityObj + healthOffset);
        int teamID = RPM<int>(entityObj + teamIdOffset);

        float distance = GetDistance(m_pos, pos);

        player = new Player(entityObj, playerName, pos, health, teamID, distance);
        playersList.push_back(player);
    }

    // if (!playersList.empty()) sort(playersList.begin(), playersList.end(), SortByDistance);
}

void Render::Esp()
{

    Text((char *)"Esp is On", 10, 75, centered, RED, font);

    for (int i = 0; i < playersList.size(); i++)
    {

        Player *player = playersList.at(i);

        if (player->health < 1)
            continue;

        if (!WorldToScreen(player->pos, vScreen, matrix, RenderWidth, RenderHeight))
            continue;

        Vector3 headPos = RPM<Vector3>(player->entityObj + vec3_head);

        if (!WorldToScreen(headPos, vHead, matrix, RenderWidth, RenderHeight))
            continue;

        char healthChar[16];
        int BorderWidth = 2;

        float head = vHead.y - vScreen.y - 15;
        float width = head / 2;
        float center = width / -2;

        Text((char *)player->playerName.playerName, vScreen.x - 50, vScreen.y + head, centered, WHITE, font);

        string distanceInMeter = to_string(player->distance).substr(0, 5) + " f";

        if (m_teamID != player->teamID)
        {

            // DrawLine(m_pos.x + 0x2FF, m_pos.y + 0x2FF, vScreen.x, vScreen.y, 2, false, BLUE(255));
            // DrawBorderBox(vScreen.x, vScreen.y, center, vHead.y, BorderWidth, false, BLUE(255));

            DrawLine(m_pos.x + 0x2FF, m_pos.y + 0x2FF, vScreen.x, vScreen.y, 2, false, RED);
            DrawBorderBox(vScreen.x, vScreen.y, center, vHead.y, BorderWidth, false, RED);
            Text((char *)distanceInMeter.c_str(), vScreen.x - 50, vScreen.y, centered, GREEN, font);
        }
    }
}

void Render::AimBot()
{

    Text((char *)"AimBot is On", 10, 50, centered, RED, font);
    // TODO: aimbot
    for (int i = 0; i < playersList.size(); i++)
    {

        Player *player = playersList.at(i);

        if (player->health < 1 || player->teamID == m_teamID)
            continue;

        if ((GetKeyState(VK_RBUTTON) & 0x8000) != 0)
        {
            Vector3 new_target_pos = setOrigin(m_pos, player->pos);

            vector<DWORD> addrs = {localPlayerObj + 0x34, localPlayerObj + 0x38};

            setTarget(addrs, new_target_pos, GetHypotenuse(new_target_pos));

            Text((char *)"REM", 100, 100, centered, GREEN, font);
        }
    }
}

// TODO
bool Render::SortByDistance(const Player &a, const Player &b)
{

    return a.distance < b.distance;
}

bool Render::WorldToScreen(Vector3 pos, Vector2 &screen, ViewMatrix matrix, int windowWidth, int windowHeight)
{
    // Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
    Vector4 clipCoords;
    clipCoords.x = pos.x * matrix.matrix[0] + pos.y * matrix.matrix[4] + pos.z * matrix.matrix[8] + matrix.matrix[12];
    clipCoords.y = pos.x * matrix.matrix[1] + pos.y * matrix.matrix[5] + pos.z * matrix.matrix[9] + matrix.matrix[13];
    clipCoords.z = pos.x * matrix.matrix[2] + pos.y * matrix.matrix[6] + pos.z * matrix.matrix[10] + matrix.matrix[14];
    clipCoords.w = pos.x * matrix.matrix[3] + pos.y * matrix.matrix[7] + pos.z * matrix.matrix[11] + matrix.matrix[15];

    if (clipCoords.w < 0.1f)
        return false;

    // perspective division, dividing by clip.W = Normalized Device Coordinates
    Vector3 NDC;

    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;
    NDC.z = clipCoords.z / clipCoords.w;

    // Transform to window coordinates
    screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
    screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);

    return true;
}

float Render::GetDistance(Vector3 m_pos, Vector3 player_pos)
{

    return sqrt(pow((player_pos.x - m_pos.x), 2) + pow((player_pos.y - m_pos.y), 2) + pow((player_pos.z - m_pos.z), 2));
};

Render::Vector3 Render::setOrigin(Vector3 m_pos, Vector3 target_pos)
{
    Vector3 new_pos;

    new_pos.x = target_pos.x - m_pos.x;
    new_pos.y = target_pos.y - m_pos.y;
    new_pos.z = target_pos.z - m_pos.z;

    return new_pos;
}

float Render::GetHypotenuse(Vector3 target_pos)
{
    return sqrt(pow(target_pos.x, 2) + pow(target_pos.y, 2));
}

void Render::setTarget(vector<DWORD> addrs, Vector3 target_pos, float h)
{
    float yaw = RadiansToDegrees(atan(target_pos.y / target_pos.x));
    float pitch = RadiansToDegrees(-atan(target_pos.z / h));

    if (yaw < 0.0f)
    {
        yaw += 360.0f;
    }

    if (yaw > 270.0f)
        yaw += 90.0f;
    else
        yaw -= 90.0f;

    if (yaw > 360.0f)
        yaw -= 180.0f;
    // if (yaw < 0.0f) yaw += 180.0f;

    WPM<float>(addrs[0], yaw);
    WPM<float>(addrs[1], pitch);
}

float get_PI()
{
    float pi;
    __asm
    {
		fldpi
		fstp pi
    }
    return pi;
}

float Render::RadiansToDegrees(float radians)
{

    float PI = get_PI();
    return (radians * (180.0f / PI));
}