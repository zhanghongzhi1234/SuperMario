#include "stdafx.h"
#include "gamemap.h"
#include "myclock.h"
#include "utility.h"
#include "texttool.h"

extern int wwin, hwin;
extern HWND hWndMain;

//base image
extern MYANIOBJ bmMap;

//sky background
extern MYBKSKY bmSky;

//character
extern MYROLE rmain;

//timer
extern MYCLOCK c1;

extern MYANIMAGIC bmMagic;

extern struct MAPINFO allmapinfo[];

extern struct ROLE gl_enemy_normal;

//extern FILEREPORT f1;

GAMEMAP::GAMEMAP()
{
    iScreenScale = 0;
    Init();
}

GAMEMAP::~GAMEMAP()
{
}

int GAMEMAP::LoadMap()
{
    FILE *fp;
    char temp[50] = { 0 };
    int find = 0;
    int i;


    memset(MapArray, 0, sizeof(MapArray));
    iMapObjNum = 0;

    memset(MapBkArray, 0, sizeof(MapBkArray));
    iMapBkObjNum = 0;

    memset(MapEnemyArray, 0, sizeof(MapEnemyArray));

    memset(MapCoinArray, 0, sizeof(MapCoinArray));
    iCoinNum = 0;

    fp = fopen(PATH_MAP, "r");
    if (!fp)
    {
        return 0;
    }

    while (!find && !feof(fp))
    {
        FGetLine(temp, fp);
        if (temp[0] == '*' && temp[1] == '0' + iMatch)
        {
            find = 1;
        }
    }
    if (!find)
    {
        return 0;
    }

    //Found the map data of a level
    i = 0;
    FGetLineJumpCom(temp, fp);
    while (temp[0] != '#' && !feof(fp))
    {
        //map data
        sscanf(temp, "%d %d %d %d %d",
            &MapArray[i].x,
            &MapArray[i].y,
            &MapArray[i].w,
            &MapArray[i].h,
            &MapArray[i].id);
        MapArray[i].show = 0;
        iMapObjNum++;
        i++;
        FGetLineJumpCom(temp, fp);
    }

    i = 0;
    FGetLineJumpCom(temp, fp);
    while (temp[0] != '#' && !feof(fp))
    {
        sscanf(temp, "%d %d %d %d %d",
            &MapBkArray[i].x,
            &MapBkArray[i].y,
            &MapBkArray[i].w,
            &MapBkArray[i].h,
            &MapBkArray[i].id);
        MapBkArray[i].show = 0;
        MapBkArray[i].iframe = 0;
        iMapBkObjNum++;

        i++;
        FGetLineJumpCom(temp, fp);
    }


    i = 0;
    FGetLineJumpCom(temp, fp);
    while (temp[0] != '#' && !feof(fp))
    {
        sscanf(temp, "%d %d %d %d %d %d %d",
            &MapEnemyArray[i].x,
            &MapEnemyArray[i].y,
            &MapEnemyArray[i].w,
            &MapEnemyArray[i].h,
            &MapEnemyArray[i].id,
            &MapEnemyArray[i].xleft,
            &MapEnemyArray[i].xright);

        //Animated components, using absolute coordinates
        MapEnemyArray[i].x *= 32;
        MapEnemyArray[i].y *= 32;
        MapEnemyArray[i].w *= 32;
        MapEnemyArray[i].h *= 32;
        MapEnemyArray[i].xleft *= 32;
        MapEnemyArray[i].xright *= 32;
        MapEnemyArray[i].show = 1;
        MapEnemyArray[i].movex = -ENEMY_STEP_X;
        MapEnemyArray[i].iframe = 0;
        MapEnemyArray[i].iframemax = 2;

        //set life
        switch (MapEnemyArray[i].id)
        {
        case ID_ANI_BOSS_HOUSE:
            MapEnemyArray[i].health = BOSS_HEALTH;
            break;

        case ID_ANI_BOSS_HOUSE_A:
            MapEnemyArray[i].health = BOSS_A_HEALTH;
            break;

        default:
            MapEnemyArray[i].health = 1;
            break;
        }

        //Store BOSS in the second half of the array
        if (i < BOSS_CURSOR
            && (MapEnemyArray[i].id == ID_ANI_BOSS_HOUSE
                || MapEnemyArray[i].id == ID_ANI_BOSS_HOUSE_A))
        {
            //move data to BOSS_CURSOR
            MapEnemyArray[BOSS_CURSOR] = MapEnemyArray[i];
            memset(&MapEnemyArray[i], 0, sizeof(MapEnemyArray[i]));
            i = BOSS_CURSOR;
        }

        i++;
        FGetLineJumpCom(temp, fp);
    }

    i = 0;
    FGetLineJumpCom(temp, fp);
    while (temp[0] != '#' && !feof(fp))
    {
        sscanf(temp, "%d %d %d %d %d",
            &MapCoinArray[i].x,
            &MapCoinArray[i].y,
            &MapCoinArray[i].w,
            &MapCoinArray[i].h,
            &MapCoinArray[i].id);
        MapCoinArray[i].show = 1;
        MapCoinArray[i].iframe = 0;
        //Convert indexed coordinates to absolute coordinates
        MapCoinArray[i].x *= 32;
        MapCoinArray[i].y *= 32;
        //Set the maximum frame of this animation symbol
        switch (MapCoinArray[i].id)
        {
        case ID_ANI_COIN:
            MapCoinArray[i].iframemax = 4;
            break;

        default:
            MapCoinArray[i].iframemax = 2;
            break;
        }

        i++;
        iCoinNum++;

        FGetLineJumpCom(temp, fp);
    }

    fclose(fp);
    return 1;
}

void GAMEMAP::Init()
{
    iGameState = GAME_PRE;
    iMatch = 0;
    iLife = 3;
    iMoney = 0;
    //attack type
    iAttack = ATTACK_NORMAL;

    iMenu = 0;
    viewx = 0;
    InitMatch();
}

void GAMEMAP::InitMatch()
{
    memset(MapArray, 0, sizeof(MapArray));
    memset(MapBkArray, 0, sizeof(MapBkArray));
    memset(MapEnemyArray, 0, sizeof(MapEnemyArray));
    memset(MapCoinArray, 0, sizeof(MapCoinArray));

    memset(FireArray, 0, sizeof(FireArray));
    memset(BombArray, 0, sizeof(BombArray));

    iMapObjNum = 0;
    iMapBkObjNum = 0;
    iBombNum = 0;
    iCoinNum = 0;

    //set view coordinates
    viewy = 0;

    ienemyframe = 0;

    iFireNum = 0;

    iTimeFire = 0;
    iBeginFire = 0;

    ibkobjframe = 0;

    //Get map information
    mapinfo = allmapinfo[iMatch];

    //Initialize the attack object prompt
    memset(AttackName, 0, sizeof(AttackName));
    iAttackLife = 0;
    iAttackMaxLife = 0;

    iMapEnemyCursor = 0;
}

//show map
void GAMEMAP::Show(MYANIOBJ & bmobj)
{
    int i;
    int xstart, ystart;
    int j, k;

    for (i = 0; i < iMapObjNum; i++)
    {
        ystart = MapArray[i].y * 32;

        switch (MapArray[i].id)
        {
            //In and out of the sewer
        case ID_MAP_PUMP_IN:
        case ID_MAP_PUMP_OUT:
            xstart = MapArray[i].x * 32;
            bmobj.DrawItemNoMask(xstart, ystart, MapArray[i].id, 0);
            break;

        default:
            for (j = 0; j < MapArray[i].h; j++)
            {
                xstart = MapArray[i].x * 32;
                for (k = 0; k < MapArray[i].w; k++)
                {
                    bmobj.DrawItemNoMask(xstart, ystart, MapArray[i].id, 0);
                    xstart += 32;
                }
                ystart += 32;
            } // end of for
            break;
        }//end of switch
    }
}

void GAMEMAP::ChangeMap()
{
    viewx = 0;

    iMatch = mapinfo.iSubMap;
    InitMatch();
    rmain.SetPos(BM_USER, mapinfo.xReturnPoint * 32, mapinfo.yReturnPoint * 32);
    rmain.InitRole(0, GAMEW * 32 * MAX_PAGE - 32);
    //set view position
    if (rmain.xpos - viewx > 150)
    {
        SetView(mapinfo.xReturnPoint * 32 - 32);        //one space to the left
        if (viewx > (mapinfo.viewmax - 1)*GAMEW * 32)
            viewx = (mapinfo.viewmax - 1)*GAMEW * 32;
    }
    //Set the scope of the character's activities
    rmain.SetLimit(viewx, GAMEW * 32 * MAX_PAGE);
    //Set background image coordinates
    bmSky.SetPos(BM_USER, viewx, 0);
    LoadMap();
}

//Determine whether to move the view according to the positional relationship between the character and the view
int GAMEMAP::KeyProc(int iKey)
{
    int i;


    switch (iGameState)
    {
    case GAME_PRE:
        switch (iKey)
        {
        case 0xd:
            //enter
            switch (iMenu)
            {
            case 0:
                c1.ReStart(TIME_GAME_IN_PRE); //pause for two seconds
                iGameState = GAME_IN_PRE;
                break;

            case 1:
                SetGameState(GAME_HELP);
                break;
            }
            break;

        case VK_UP:
            iMenu = (iMenu + 1) % 2;
            break;

        case VK_DOWN:
            iMenu = (iMenu + 1) % 2;
            break;
        }

        return 1;   //redraw

    case GAME_HELP:
        switch (iKey)
        {
        case 0xd:	//enter
            SetGameState(GAME_PRE);
            break;
        }
        return 1;

    case GAME_IN:
        //Deny keyboard responses if character is playing animation
        if (rmain.IsInAni())
        {
            break;
        }

        switch (iKey)
        {
        case VK_RIGHT:
            if (rmain.movey != 0)
            {
                rmain.jumpx = 4;
            }
            rmain.movex = 4;
            rmain.idirec = 0;
            break;

        case VK_LEFT:
            if (rmain.movey != 0)
            {
                rmain.jumpx = -4;
            }
            rmain.movex = -4;
            rmain.idirec = 1;   //facing left
            break;

        case VK_DOWN:
            for (i = 0; i < iMapObjNum; i++)
            {
                //lower line
                if (LINE_IN_LINE(rmain.xpos,
                    rmain.ypos + 32,
                    32,
                    MapArray[i].x * 32,
                    MapArray[i].y * 32,
                    MapArray[i].w * 32)
                    )
                {
                    if (MapArray[i].id == ID_MAP_PUMP_IN)
                    {
                        //enter the map
                        rmain.SetAni(ROLE_ANI_DOWN);
                        iGameState = GAME_PUMP_IN;
                        c1.ReStart(TIME_GAME_PUMP_WAIT);
                    }
                }
            }
            break;

        case KEY_X: //jump
            if (rmain.movey != 0)
                break;
            rmain.movey = -SPEED_JUMP;
            rmain.jumpx = rmain.movex;  //Solve the response problem of x key and arrow key
            break;

        case KEY_Z: //FIRE
            if (iBeginFire)
                break;
            iTimeFire = 0;
            iBeginFire = 1;
            break;

        case 0x7a://f11
            iAttack = (iAttack + 1) % ATTACK_MAX_TYPE;
            break;

        case 0x7b://f12
            //Direct clearance
            rmain.xpos = MAX_PAGE * GAMEW * 32;
            break;
        }
        break;
    }

    return 0;
}

void GAMEMAP::SetViewState(int i)
{
    iViewState = i;
}
void GAMEMAP::SetGameState(int i)
{
    iGameState = i;
}

void GAMEMAP::SetView(int x)
{
    viewx = x;
}

// move view
// View cannot be moved to the right
void GAMEMAP::MoveView()
{
    //Only one screen wide, does not move the view
    if (mapinfo.viewmax == 1)
        return;

    if (rmain.xpos - viewx > 150)
    {
        viewx += ROLE_STEP;
        if (viewx > (mapinfo.viewmax - 1)*GAMEW * 32)
            viewx = (mapinfo.viewmax - 1)*GAMEW * 32;

        //Set the maximum range of character movement
        rmain.SetLimit(viewx, GAMEW * 32 * MAX_PAGE);
    }

    bmSky.SetPos(BM_USER, viewx, 0);
}

void GAMEMAP::ShowBkObj(MYANIOBJ & bmobj)
{
    int i;
    int xstart, ystart;

    for (i = 0; i < iMapBkObjNum; i++)
    {
        ystart = MapBkArray[i].y * 32;
        xstart = MapBkArray[i].x * 32;
        bmobj.DrawItem(xstart, ystart, MapBkArray[i].id, ibkobjframe);
    }
}


void GAMEMAP::ShowInfo(HDC h)
{
    char temp[50] = { 0 };

    SetTextColor(h, TC_WHITE);
    SetBkColor(h, TC_BLACK);

    sprintf(temp, "LIFE  : %d", iLife);
    TextOut(h, 220, 100, temp, strlen(temp));

    sprintf(temp, "WORLD : %d", iMatch + 1);
    TextOut(h, 220, 130, temp, strlen(temp));
}

void GAMEMAP::ShowOther(HDC h)
{
    char temp[50] = { 0 };
    int xstart;

    //show money
    sprintf(temp, "MONEY: %d", iMoney);
    TextOut(h, viewx + 20, 20, temp, strlen(temp));

    //Show hit target health
    if (iAttackLife)
    {
        TextOut(h, viewx + ATTACK_TO_TEXT_X,
            ATTACK_TO_TEXT_Y, AttackName, strlen(AttackName));

        //draw life bar
        xstart = viewx + ATTACK_TO_X - iAttackMaxLife * 10;

        bmMap.DrawItemNoMaskWidth(xstart - 1, ATTACK_TO_Y - 1, ID_MAP_HEALTH_BK,
            iAttackMaxLife*BMP_WIDTH_HEALTH, 0);

        bmMap.DrawItemNoMaskWidth(xstart, ATTACK_TO_Y, ID_MAP_HEALTH,
            iAttackLife*BMP_WIDTH_HEALTH, 0);
    }
}

void GAMEMAP::ShowAniObj(MYANIOBJ & bmobj)
{
    int i;
    int xstart, ystart;

    //Show coins, and other items
    for (i = 0; i < iCoinNum; i++)
    {
        ystart = MapCoinArray[i].y;
        xstart = MapCoinArray[i].x;
        bmobj.DrawItem(xstart, ystart, MapCoinArray[i].id, MapCoinArray[i].iframe);
    }

    //show enemy
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (MapEnemyArray[i].show)
        {
            bmobj.DrawItem(MapEnemyArray[i].x, MapEnemyArray[i].y,
                MapEnemyArray[i].id, MapEnemyArray[i].iframe);
        }
    }

    //Show bullets, magic attacks
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (FireArray[i].show)
        {
            ystart = FireArray[i].y;
            xstart = FireArray[i].x;

            switch (FireArray[i].id)
            {
            case ID_ANI_FIRE:
                bmobj.DrawItem(xstart, ystart, FireArray[i].id, FireArray[i].iframe);
                break;

            case ID_ANI_FIRE_MAGIC:
                bmMagic.DrawItem(xstart, ystart, 0, FireArray[i].iframe);
                break;

            default:
                bmobj.DrawItem(xstart, ystart, FireArray[i].id, FireArray[i].iframe);
                break;
            }
        }
    }


    //show explosion effect
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (BombArray[i].show)
        {
            ystart = BombArray[i].y;
            xstart = BombArray[i].x;
            bmobj.DrawItem(xstart, ystart, BombArray[i].id, BombArray[i].iframe);
        }
    }
}

// Return 0, can not go; 1, can go
int GAMEMAP::RoleCanMove(int xoff, int yoff)
{
    int canmove = 1;
    int i;


    for (i = 0; i < iMapObjNum; i++)
    {
        if (RECT_HIT_RECT(rmain.xpos + xoff,
            rmain.ypos + yoff,
            32,
            32,
            MapArray[i].x * 32,
            MapArray[i].y * 32,
            MapArray[i].w * 32,
            MapArray[i].h * 32)
            )
        {
            canmove = 0;
            if (yoff < 0)
            {
                //When it hits an object on it, it will fall down.
                rmain.movey = 1;
            }
            if (yoff > 0)
            {
                //Stop falling when you hit something below
                rmain.jumpheight = 0;
                rmain.movey = 0;
                rmain.ypos = MapArray[i].y * 32 - 32;   //The vertical coordinate needs to be corrected
            }
            break;
        }
    }

    return canmove;
}

//Check whether the character is out of the boundary of the map object during the horizontal movement
int GAMEMAP::CheckRole()
{
    int i;

    //role check
    if (rmain.movey == 0)
    {
        //move horizontally
        //Detect if the character is standing on an object
        for (i = 0; i < iMapObjNum; i++)
        {
            //lower line
            if (LINE_ON_LINE(rmain.xpos,
                rmain.ypos + 32,
                32,
                MapArray[i].x * 32,
                MapArray[i].y * 32,
                MapArray[i].w * 32)
                )
            {
                return 1;
            }
        }
        //character starts to fall
        rmain.movey = 1;
        rmain.jumpx = 0;    //At this time, the jump speed should be cleared, otherwise it will become a jump instead of a fall
        return 0;
    }

    return 0;
}

void GAMEMAP::ChangeFrame(int itimeclip)
{
    int i;

    ///////////// frame control ////////////////
    if (0 == itimeclip % ENEMY_SPEED)
    {
        //enemy move
        for (i = 0; i < MAX_MAP_OBJECT; i++)
        {
            if (MapEnemyArray[i].show)
            {
                //frame control
                MapEnemyArray[i].iframe = (MapEnemyArray[i].iframe + 1) % MapEnemyArray[i].iframemax;

                switch (MapEnemyArray[i].id)
                {
                case ID_ANI_ENEMY_NORMAL:
                case ID_ANI_ENEMY_SWORD:
                    MapEnemyArray[i].x += MapEnemyArray[i].movex;

                    //control enemy movement
                    if (MapEnemyArray[i].movex < 0)
                    {
                        if (MapEnemyArray[i].x <= MapEnemyArray[i].xleft)
                        {
                            MapEnemyArray[i].movex = ENEMY_STEP_X;
                        }
                    }
                    else
                    {
                        if (MapEnemyArray[i].x >= MapEnemyArray[i].xright)
                        {
                            MapEnemyArray[i].movex = -ENEMY_STEP_X;
                        }
                    }
                    break;
                }
            }
        }
    }

    if (0 == itimeclip % WATER_SPEED)
    {
        ibkobjframe = 1 - ibkobjframe;

        for (i = 0; i < MAX_MAP_OBJECT; i++)
        {
            if (MapCoinArray[i].show)
            {
                MapCoinArray[i].iframe = (MapCoinArray[i].iframe + 1) % MapCoinArray[i].iframemax;
            }
        }
    }

    //bullets, attack control
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (FireArray[i].show)
        {
            switch (FireArray[i].id)
            {
            case ID_ANI_FIRE_MAGIC:
                FireArray[i].iframe++;
                if (FireArray[i].iframe == 2)
                {
                    FireArray[i].x += FIRE_MAGIC_W;
                }
                if (FireArray[i].iframe > 3)
                {
                    FireArray[i].show = 0;
                }
                break;

            default:
                FireArray[i].iframe = 1 - FireArray[i].iframe;
                break;
            }
        }
    }

    //bomb frame control
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (BombArray[i].show)
        {
            BombArray[i].iframe++;
            if (BombArray[i].iframe > 3)
            {
                BombArray[i].show = 0;
            }
        }
    }
}

int GAMEMAP::CheckAni(int itimeclip)
{
    int i, j;
    int tempx, tempy;

    //Detect collisions between bullets and enemies (including magic attacks)
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (MapEnemyArray[i].show)
        {
            for (j = 0; j < MAX_MAP_OBJECT; j++)
            {
                if (FireArray[j].show)
                {
                    if (RECT_HIT_RECT(FireArray[j].x + FIRE_XOFF,
                        FireArray[j].y,
                        FireArray[j].w,
                        FireArray[j].h,
                        MapEnemyArray[i].x,
                        MapEnemyArray[i].y,
                        MapEnemyArray[i].w,
                        MapEnemyArray[i].h)
                        )
                    {
                        ClearEnemy(i);

                        //When magic attack, the elimination of the attack effect is controlled by the frame
                        switch (iAttack)
                        {
                        case ATTACK_NORMAL:
                            FireArray[j].show = 0;
                            break;

                        default:
                            break;
                        }
                        return 0;
                    }
                }
            }
        }
    }

    //fire bullet
    if (iBeginFire)
    {
        if (0 == iTimeFire)
        {
            FireArray[iFireNum].show = 1;
            FireArray[iFireNum].iframe = 0;

            //bullet direction
            if (0 == rmain.idirec)
            {
                FireArray[iFireNum].movex = 1;
            }
            else
            {
                FireArray[iFireNum].movex = -1;
            }

            switch (iAttack)
            {
            case ATTACK_NORMAL:
                FireArray[iFireNum].id = ID_ANI_FIRE;
                FireArray[iFireNum].x = rmain.xpos;
                FireArray[iFireNum].y = rmain.ypos;
                FireArray[iFireNum].w = FIREW;
                FireArray[iFireNum].h = FIREH;
                FireArray[iFireNum].movex *= FIRE_SPEED;
                break;

            case ATTACK_MAGIC:
                FireArray[iFireNum].id = ID_ANI_FIRE_MAGIC;
                FireArray[iFireNum].x = rmain.xpos - ID_ANI_FIRE_MAGIC_XOFF;
                FireArray[iFireNum].y = rmain.ypos - ID_ANI_FIRE_MAGIC_YOFF;
                FireArray[iFireNum].w = FIRE_MAGIC_W;
                FireArray[iFireNum].h = FIRE_MAGIC_H;
                FireArray[iFireNum].movex = 0;
                break;
            }
            //move array cursor
            iFireNum = (iFireNum + 1) % MAX_MAP_OBJECT;
        }
        iTimeFire = (iTimeFire + 1) % TIME_FIRE_BETWEEN;
    }


    //Inspect enemies, animated components
    //If it is within the display range, set the display property
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (IN_AREA(MapEnemyArray[i].x, viewx, VIEWW))
        {
            if (MapEnemyArray[i].health)
            {
                MapEnemyArray[i].show = 1;

                //boss trigger
                switch (MapEnemyArray[i].id)
                {
                case ID_ANI_BOSS_HOUSE:
                    //Every so often, new enemies spawn
                    if (itimeclip == TIME_CREATE_ENEMY)
                    {
                        MapEnemyArray[iMapEnemyCursor] = gl_enemy_normal;
                        MapEnemyArray[iMapEnemyCursor].x = MapEnemyArray[i].x;
                        MapEnemyArray[iMapEnemyCursor].y = MapEnemyArray[i].y + 32;

                        //move cursor
                        iMapEnemyCursor = (iMapEnemyCursor + 1) % BOSS_CURSOR;
                    }
                    break;

                case ID_ANI_BOSS_HOUSE_A:
                    if (itimeclip == TIME_CREATE_ENEMY)
                    {
                        MapEnemyArray[iMapEnemyCursor] = gl_enemy_normal;
                        MapEnemyArray[iMapEnemyCursor].x = MapEnemyArray[i].x;
                        MapEnemyArray[iMapEnemyCursor].y = MapEnemyArray[i].y + 32;
                        MapEnemyArray[iMapEnemyCursor].id = ID_ANI_ENEMY_SWORD;
                        MapEnemyArray[iMapEnemyCursor].health = ENEMY_SWORD_HEALTH;

                        //move cursor
                        iMapEnemyCursor = (iMapEnemyCursor + 1) % BOSS_CURSOR;
                    }
                    break;

                default:
                    break;
                }
            }
        }
        else
        {
            MapEnemyArray[i].show = 0;
        }
    }

    //bullet movement
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (FireArray[i].show)
        {
            FireArray[i].x += FireArray[i].movex;
            //The bullet is beyond the display range and disappears
            if (FireArray[i].x > viewx + VIEWW || FireArray[i].x < viewx - FIRE_MAGIC_MAX_W)
            {
                FireArray[i].show = 0;
            }
        }
    }



    //Detect collisions between characters and gold coins, collisions between characters and other objects
    for (i = 0; i < iCoinNum; i++)
    {
        tempx = MapCoinArray[i].x;
        tempy = MapCoinArray[i].y;

        if (IN_AREA(tempx, viewx - 32, VIEWW))
        {
            if (RECT_HIT_RECT(rmain.xpos,
                rmain.ypos,
                32, 32,
                tempx,
                tempy,
                MapCoinArray[i].w, MapCoinArray[i].h)
                )
            {
                switch (MapCoinArray[i].id)
                {
                case ID_ANI_COIN:
                    //hit gold coins
                    iMoney += 10;
                    ClearCoin(i);
                    break;

                case ID_ANI_ATTACK:
                    //touch the weapon
                    iAttack = ATTACK_MAGIC;
                    ClearCoin(i);
                    break;
                }
                return 0;
            }
        }
    } // end of for

    return 0;
}

//To clear enemy data, you only need to set the display attribute, no need to move the array
void GAMEMAP::ClearEnemy(int i)
{

    //create bomb
    BombArray[iBombNum].show = 1;
    BombArray[iBombNum].id = ID_ANI_BOMB;
    BombArray[iBombNum].iframe = 0;
    BombArray[iBombNum].x = MapEnemyArray[i].x - BOMB_XOFF;
    BombArray[iBombNum].y = MapEnemyArray[i].y - BOMB_YOFF;
    iBombNum = (iBombNum + 1) % MAX_MAP_OBJECT;

    //clear display properties	
    MapEnemyArray[i].health--;


    if (MapEnemyArray[i].health <= 0)
    {
        MapEnemyArray[i].show = 0;
    }

    //Attack Object Tips
    iAttackLife = MapEnemyArray[i].health;
    switch (MapEnemyArray[i].id)
    {
    case ID_ANI_BOSS_HOUSE:
        strcpy(AttackName, "Normal Boss");
        iAttackMaxLife = BOSS_HEALTH;
        break;

    case ID_ANI_BOSS_HOUSE_A:
        strcpy(AttackName, "Combat Boss");
        iAttackMaxLife = BOSS_A_HEALTH;
        break;

    case ID_ANI_ENEMY_SWORD:
        strcpy(AttackName, "Mushroom Assassin");
        iAttackMaxLife = ENEMY_SWORD_HEALTH;
        break;

    default:
        strcpy(AttackName, "Enemy ");
        break;
    }
}

void GAMEMAP::ClearCoin(int i)
{
    if (i < 0 || i >= iCoinNum)
        return;

    switch (MapCoinArray[i].id)
    {
    case ID_ANI_COIN:
        //When a gold coin is touched, a scoring animation is played
        BombArray[iBombNum].show = 1;
        BombArray[iBombNum].id = ID_ANI_COIN_SCORE;
        BombArray[iBombNum].iframe = 0;
        BombArray[iBombNum].x = MapCoinArray[i].x - COIN_XOFF;
        BombArray[iBombNum].y = MapCoinArray[i].y - COIN_YOFF;
        iBombNum = (iBombNum + 1) % MAX_MAP_OBJECT;
        break;
    }

    //Reduce one gold coin, or reduce one other item
    for (; i < iCoinNum; i++)
    {
        MapCoinArray[i] = MapCoinArray[i + 1];
    }
    iCoinNum--;
}

int GAMEMAP::IsWin()
{
    int i;

    //Detect collisions between characters and enemies
    for (i = 0; i < MAX_MAP_OBJECT; i++)
    {
        if (MapEnemyArray[i].show)
        {
            if (HLINE_ON_RECT(rmain.xpos, rmain.ypos + 32, 32,
                MapEnemyArray[i].x,
                MapEnemyArray[i].y,
                MapEnemyArray[i].w,
                MapEnemyArray[i].h))
            {
                if (0 == rmain.movey)
                {
                    Fail();
                }
                else
                {
                    //step on the enemy
                    switch (MapEnemyArray[i].id)
                    {
                    case ID_ANI_BOSS_HOUSE:
                    case ID_ANI_BOSS_HOUSE_A:
                        //Stepping on a mushroom fire ring, fail
                        Fail();
                        break;

                    default:
                        ClearEnemy(i);
                        break;
                    }
                }
                return 0;
            }
        }
    }

    if (rmain.ypos > GAMEH * 32)
    {

        Fail();
        return 0;
    }

    //After arriving at the destination, automatically walk to the right side of the screen
    if (rmain.xpos > X_WIN * 32)
    {
        //Already won, but want to play the animation of the player walking out of the screen, do not accept message response
        iGameState = GAME_WIN;

        //reach the destination
        rmain.MoveStepTo(MAX_PAGE*GAMEW * 32, Y_WIN * 32);
    }

    if (rmain.xpos >= MAX_PAGE * GAMEW * 32)
    {
        //The player has stepped off the screen, start a certain time interval
        iGameState = GAME_WIN_WAIT;
        c1.ReStart(TIME_GAME_WIN_WAIT);

        iMatch = mapinfo.iNextMap;

        if (iMatch >= MAX_MATCH)
        {
            //pass all
            Init();
            iGameState = GAME_PASS;
            LoadMap();
        }
        else
        {
            InitMatch();

            rmain.SetPos(BM_USER, 3 * 32, 8 * 32);
            rmain.InitRole(0, GAMEW * 32 * MAX_PAGE - 32);
            //Load the map for the next level
            LoadMap();
        }
    }

    return 1;
}

void GAMEMAP::Fail_Wait()
{
    InitMatch();

    if (iLife <= 0)
    {
        Init();
        iGameState = GAME_OVER;
        c1.ReStart(TIME_GAME_END);
    }
    else
    {
        iGameState = GAME_IN_PRE;
        c1.ReStart(TIME_GAME_IN_PRE);
    }

    rmain.SetPos(BM_USER, 3 * 32, 8 * 32);
    rmain.InitRole(0, GAMEW * 32 * MAX_PAGE - 32);

    LoadMap();

}

void GAMEMAP::Fail()
{
    rmain.movex = 0;
    rmain.movey = 0;

    iLife--;
    rmain.iFrame = FAIL_FRAME;

    iGameState = GAME_FAIL_WAIT;
    c1.ReStart(TIME_GAME_FAIL_WAIT);

}


void GAMEMAP::KeyUpProc(int iKey)
{
    switch (iKey)
    {
    case VK_RIGHT:
        rmain.movex = 0;
        break;

    case VK_LEFT:
        rmain.movex = 0;
        break;

    case KEY_X: //jump
        break;

    case KEY_Z: //FIRE
        iBeginFire = 0;
        break;

    case KEY_W: //resize window

        MoveWindow(hWndMain,
            (wwin - GAMEW * 32) / 2,
            (hwin - GAMEH * 32) / 2,
            GAMEW * 32,
            GAMEH * 32 + 32,
            true);

        break;
    }
}

void GAMEMAP::CodeErr(int i)
{
    switch (i)
    {
    case ERR_MAP_FILE:
        SetGameState(GAME_ERR);
        break;

    default:
        break;

    }
}

void GAMEMAP::ShowMenu(MYANIOBJ & bmobj)
{
    bmobj.PlayItem(115, 280 + iMenu * 30, ID_ANI_MENU_ARROW);
}