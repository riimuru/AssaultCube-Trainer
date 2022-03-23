#pragma once

#include "includes.h"

class Render
{
public:
    LPDIRECT3DVERTEXBUFFER9 g_pVB;
    LPDIRECT3DINDEXBUFFER9 g_pIB;

#pragma region Window Properties
    uintptr_t RenderWidth;
    uintptr_t RenderHeight;

#pragma endregion

#pragma region DXD

    LPDIRECT3DDEVICE9 d3ddev;
    LPDIRECT3D9 d3d; // the pointer to the Direct3D interface

    void initD3D(HWND hwnd);
    void render();
    void cleanD3D();

#pragma region fonts
    LPD3DXFONT font;

    int fontSize = 13;
    const wchar_t *fontName = L"Tahoma";

    void initFonts();
#pragma endregion

#pragma region drawing
    ID3DXLine *line;

    void InitLine();
    void DrawLine(float x1, float y1, float x2, float y2, float w, bool antialias, DWORD color);
    void DrawBorderBox(float x, float y, float c, float h, float w, bool antialias, DWORD color);

    void Text(char *text, int x, int y, int orientation, DWORD color, ID3DXFont *pFont);

    enum text_alignment
    {
        lefted,
        centered,
        righted
    };

#pragma endregion

#pragma region ESP

    struct Vector2
    {
        float x, y;
    };

    struct Vector3
    {
        float x, y, z;
    };

    struct Vector4
    {
        float x, y, z, w;
    };

    struct ViewMatrix
    {
        float matrix[16];
    };

    struct PlayerName
    {
        char playerName[16];
    };

    struct Player
    {
        DWORD entityObj;
        PlayerName playerName;
        Vector3 pos;

        int health, teamID;

        float distance;

        Player(DWORD entityObj, PlayerName playerName, Vector3 pos, int health, int teamID, float distance)
        {
            this->entityObj = entityObj;
            this->playerName = playerName;
            this->pos = pos;
            this->health = health;
            this->teamID = teamID;
            this->distance = distance;
        }
    };

    void ScanMemory();

    void GetAllPlayers();

    void Esp();

    void AimBot();

    bool SortByDistance(const Player &a, const Player &b);

    float GetDistance(Vector3 m_pos, Vector3 entity);

    bool WorldToScreen(Vector3 pos, Vector2 &screen, ViewMatrix matrix, int windowWidth, int windowHeight);

    Vector3 setOrigin(Vector3 m_pos, Vector3 target_pos);

    float GetHypotenuse(Vector3 target_pos);

    void setTarget(vector<DWORD> addrs, Vector3 target_pos, float h);

    float RadiansToDegrees(float radians);

private:
    vector<Player *> playersList;

    DWORD entityList, localPlayerObj;
    ViewMatrix matrix;
    Vector2 vScreen, vHead;
    Vector3 m_pos;

    int m_teamID;
    int playersNum;

    bool isEspOn = false;
    bool isAimBotOn = false;

#pragma endregion
#pragma endregion
};

extern Render render;