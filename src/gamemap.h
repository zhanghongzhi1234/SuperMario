#ifndef __GAMEMAP
#define __GAMEMAP

#include "bitmaptool.h"

struct MapObject
{
    int x;
    int y;
    int w;
    int h;
    int id;
    int iframe;
    int iframemax;  //Maximum number of frames
    int show;	    //whether to display
};

struct ROLE
{
    int x;
    int y;
    int w;
    int h;
    int id;
    int iframe;
    int iframemax;  //Maximum number of frames

    //moving part
    int xleft;      //left boundary	
    int xright;     //right limit
    int movex;

    //Character attributes
    int health;

    int show;	    //whether to display
};

struct MAPINFO
{
    int iNextMap;
    int iSubMap;
    int xReturnPoint;
    int yReturnPoint;
    int iBackBmp;
    int viewmax;
};

class GAMEMAP
{
public:
    int LoadMap();

    void Init();
    void InitMatch();
    //Input parameters, object index map
    void Show(MYANIOBJ & bmobj);
    //Input parameters, background object index map
    void ShowBkObj(MYANIOBJ & bmobj);
    //Input parameters, animation object index map
    void ShowAniObj(MYANIOBJ & bmobj);
    void ShowInfo(HDC h);
    void ShowOther(HDC h);

    //keyboard handling
    int KeyProc(int iKey);
    void KeyUpProc(int iKey);

    void MoveView();
    //Set view start coordinates
    void SetView(int x);
    void SetViewState(int i);
    void SetGameState(int i);
    //hittest
    int RoleCanMove(int xoff, int yoff);
    int CheckRole();
    int CheckAni(int itimeclip);
    void ClearEnemy(int i);
    void ClearCoin(int i);
    //animation frame control
    void ChangeFrame(int itimeclip);    //entry, time slice

    //logic detection
    int IsWin();
    void Fail();
    void Fail_Wait();

    //map switching
    void ChangeMap();
    //error checking
    void CodeErr(int i);
    //menu control
    void ShowMenu(MYANIOBJ & bmobj);

    GAMEMAP();
    ~GAMEMAP();

    //data
    int iMatch;
    int iLife;
    int iGameState;

    struct MapObject MapArray[MAX_MAP_OBJECT];
    int iMapObjNum;

    struct MapObject MapBkArray[MAX_MAP_OBJECT];
    int iMapBkObjNum;

    struct ROLE MapEnemyArray[MAX_MAP_OBJECT];
    int iMapEnemyCursor;

    struct MapObject MapCoinArray[MAX_MAP_OBJECT];
    int iCoinNum;
    //MAP DATA
    int iNextMap;

    // role data
    int iMoney;
    int iAttack;	//attack method

    //view
    int viewx;
    int viewy;
    int iViewState;

    //map data
    struct MAPINFO mapinfo;

    //frame control
    int ienemyframe;
    int ibkobjframe;

    //FIRE
    struct ROLE FireArray[MAX_MAP_OBJECT];
    int iFireNum;
    int iTimeFire;      //time interval between two bullets
    int iBeginFire;

    //bomb
    struct MapObject BombArray[MAX_MAP_OBJECT];
    int iBombNum;

    //Attack Object Tips
    char AttackName[20];
    int iAttackLife;
    int iAttackMaxLife;

    //menu section
    int iMenu;

    //screen zoom
    int iScreenScale;
};

#endif