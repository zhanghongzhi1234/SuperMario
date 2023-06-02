#include "stdafx.h"
#include "bitmaptool.h"
#include "gamemap.h"

extern GAMEMAP gamemap;

extern FILEREPORT f1;

MYBITMAP::MYBITMAP()
{

}

MYBITMAP::~MYBITMAP()
{
    DeleteObject(hBm);
}

void MYBITMAP::Init(HINSTANCE hInstance, int iResource, int row, int col)
{
    BITMAP bm;
    inum = row;
    jnum = col;

    hBm = LoadBitmap(hInstance, MAKEINTRESOURCE(iResource));
    GetObject(hBm, sizeof(BITMAP), &bm);

    width = bm.bmWidth / inum;
    height = bm.bmHeight / jnum;
}

void MYBITMAP::SetDevice(HDC hdest, HDC hsrc, int wwin, int hwin)
{
    hdcdest = hdest;
    hdcsrc = hsrc;
    screenwidth = wwin;
    screenheight = hwin;
}

void MYBITMAP::Show(int x, int y)
{
    xpos = x;
    ypos = y;
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, xpos, ypos, width, height, hdcsrc, 0, 0, SRCCOPY);
}

void MYBITMAP::ShowCenter(int y)
{
    xpos = (screenwidth - width) / 2;
    ypos = y;

    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, xpos, ypos, width, height, hdcsrc, 0, 0, SRCCOPY);
}

void MYBITMAP::ShowLoop(int left, int top, int right, int bottom, int iframe)
{
    int i, j;

    SelectObject(hdcsrc, hBm);
    for (j = top; j < bottom; j += height)
    {
        for (i = left; i < right; i += width)
        {
            BitBlt(hdcdest, i, j, width, height, hdcsrc, iframe*width, 0, SRCCOPY);
        }
    }
}

void MYBITMAP::ShowNoBack(int x, int y, int iFrame)
{
    xpos = x;
    ypos = y;
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, xpos, ypos, width, height / 2, hdcsrc, iFrame*width, height / 2, SRCAND);
    BitBlt(hdcdest, xpos, ypos, width, height / 2, hdcsrc, iFrame*width, 0, SRCPAINT);
}


void MYBITMAP::ShowNoBackLoop(int x, int y, int iFrame, int iNum)
{
    int i;
    xpos = x;
    ypos = y;
    SelectObject(hdcsrc, hBm);
    for (i = 0; i < iNum; i++)
    {
        BitBlt(hdcdest, xpos + i * width, ypos, width, height / 2, hdcsrc, iFrame*width, height / 2, SRCAND);
        BitBlt(hdcdest, xpos + i * width, ypos, width, height / 2, hdcsrc, iFrame*width, 0, SRCPAINT);
    }
}

void MYBITMAP::ShowAni()
{
}

void MYBITMAP::SetAni(int x, int y)
{
    xpos = x;
    ypos = y;
}


void MYBITMAP::SetPos(int istyle, int x, int y)
{
    switch (istyle)
    {
    case BM_CENTER:
        xpos = (screenwidth - width) / 2;
        ypos = y;
        break;
    case BM_USER:
        xpos = x;
        ypos = y;
        break;
    }
}

void MYBITMAP::Draw(DWORD dwRop)
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, xpos, ypos, width, height, hdcsrc, 0, 0, dwRop);
}

void MYBITMAP::Stretch(int x, int y)
{
    SelectObject(hdcsrc, hBm);
    StretchBlt(hdcdest, xpos, ypos, width*x, height*y,
        hdcsrc, 0, 0, width, height,
        SRCCOPY);
}

void MYBITMAP::Stretch(int x, int y, int id)
{
    SelectObject(hdcsrc, hBm);
    StretchBlt(hdcdest, xpos, ypos, width*x, height*y,
        hdcsrc, 0, id*height,
        width, height,
        SRCCOPY);
}


///////////////////////// bitmap object animation//////////////////
MYBKSKY::MYBKSKY()
{
    xseparate = 0;
}

MYBKSKY::~MYBKSKY()
{}

void MYBKSKY::MoveTo(int x, int y)
{
    xpos = x;
    ypos = y;
}

void MYBKSKY::MoveRoll(int x)
{
    xseparate += x;
    xseparate %= width;

    if (xseparate < 0)
    {
        xseparate = width;
    }
}

void MYBKSKY::DrawRoll()
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, xpos, ypos,
        width - xseparate, height,
        hdcsrc, xseparate, 0, SRCCOPY);

    BitBlt(hdcdest, xpos + width - xseparate, ypos,
        xseparate, height,
        hdcsrc, 0, 0, SRCCOPY);
}

void MYBKSKY::DrawRollStretch(int x, int y)
{
    SelectObject(hdcsrc, hBm);

    StretchBlt(hdcdest, xpos, ypos,
        (width - xseparate)*x, height*y,
        hdcsrc, xseparate, 0,
        width - xseparate, height,
        SRCCOPY);

    StretchBlt(hdcdest, xpos + (width - xseparate)*x, ypos,
        xseparate*x, height*y,
        hdcsrc, 0, 0,
        xseparate, height,
        SRCCOPY);
}

void MYBKSKY::DrawRollStretch(int x, int y, int id)
{
    SelectObject(hdcsrc, hBm);

    StretchBlt(hdcdest, xpos, ypos,
        (width - xseparate)*x, height*y,
        hdcsrc, xseparate, id*height,
        width - xseparate, height,
        SRCCOPY);

    StretchBlt(hdcdest, xpos + (width - xseparate)*x, ypos,
        xseparate*x, height*y,
        hdcsrc, 0, id*height,
        xseparate, height,
        SRCCOPY);
}

///////////////////////// bitmap role//////////////////
MYROLE::MYROLE()
{
}

MYROLE::~MYROLE()
{}

void MYROLE::MoveTo(int x, int y)
{
    xpos = x;
    ypos = y;
}

void MYROLE::MoveOffset(int x, int y)
{
    if (x == 0 && y == 0)
        return;

    if (!gamemap.RoleCanMove(x, y))
        return;

    xpos += x;
    ypos += y;

    if (xpos < minx)
        xpos = minx;

    if (xpos > maxx)
        xpos = maxx;
}

void MYROLE::MoveStepTo(int x, int y)
{
    if (xpos < x)
        xpos += ROLE_STEP;

    if (ypos < y)
        ypos += 8;

    if (ypos > y)
        ypos = y;
}

// remove background and draw
void MYROLE::Draw()
{
    if (iAniBegin)
    {
        PlayAni();
    }
    else
    {
        SelectObject(hdcsrc, hBm);
        BitBlt(hdcdest, xpos, ypos,
            width, height / 2,
            hdcsrc, iFrame*width, height / 2, SRCAND);

        BitBlt(hdcdest, xpos, ypos,
            width, height / 2,
            hdcsrc, iFrame*width, 0, SRCPAINT);
    }
}

void MYROLE::Draw(int x, int y, int frame)
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, x, y,
        width, height / 2,
        hdcsrc, frame*width, height / 2, SRCAND);

    BitBlt(hdcdest, x, y,
        width, height / 2,
        hdcsrc, frame*width, 0, SRCPAINT);
}

void MYROLE::InitRole(int xleft, int xright)
{
    iFrame = 0;
    iState = 0;

    maxx = xright;
    minx = xleft;

    jumpheight = 0;

    movex = 0;
    movey = 0;

    jumpx = 0;

    idirec = 0;

    iAniBegin = 0;
}

void MYROLE::SetLimit(int xleft, int xright)
{
    maxx = xright;
    minx = xleft;
}

void MYROLE::Move()
{
    if (0 == movey)
    {
        //move horizontally
        MoveOffset(movex, 0);
    }
    else
    {
        //jump		
        MoveOffset(jumpx, 0);
        MoveOffset(0, movey);
    }

    //frame control
    if (movex < 0 && iFrame < 3)
    {
        iFrame = 3;
    }

    if (movex > 0 && iFrame >= 3)
    {
        iFrame = 0;
    }

    if (movex != 0)
    {
        if (0 == idirec)
            iFrame = 1 - iFrame;
        else
            iFrame = 7 - iFrame;
    }
    if (movey != 0)
    {
        iFrame = idirec * 3;
    }


    if (movey < 0)
    {
        //up
        jumpheight += (-movey);

        //According to the influence of gravity, the acceleration slows down
        if (movey < -1)
        {
            movey++;
        }

        //Falling down after reaching the apex
        if (jumpheight >= JUMP_HEIGHT * 32)
        {
            jumpheight = JUMP_HEIGHT * 32;
            movey = 4;
        }
    }
    else if (movey > 0)
    {
        jumpheight -= movey;

        //According to the influence of gravity, the acceleration increases
        movey++;
    }
}

void MYROLE::Jump()
{
    //ascent process
    MoveOffset(0, -4);

}


void MYROLE::ChangeFrame()
{

}

void MYROLE::SetState(int i)
{
    iState = i;
}

void MYROLE::SetAni(int istyle)
{
    iAniStyle = istyle;

    iparam1 = 0;
    iAniBegin = 1;
}
//whether the animation is playing
int  MYROLE::IsInAni()
{
    return iAniBegin;
}

void MYROLE::PlayAni()
{
    switch (iAniStyle)
    {
    case ROLE_ANI_DOWN:
        if (iparam1 > 31)
        {
            break;
        }
        //character drop animation
        SelectObject(hdcsrc, hBm);
        BitBlt(hdcdest,
            xpos, ypos + iparam1,
            width, height / 2 - iparam1,
            hdcsrc,
            iFrame*width, height / 2, SRCAND);

        BitBlt(hdcdest,
            xpos, ypos + iparam1,
            width, height / 2 - iparam1,
            hdcsrc,
            iFrame*width, 0, SRCPAINT);

        iparam1++;
        break;

    case ROLE_ANI_UP:
        if (iparam1 > 31)
        {
            break;
        }
        //character rising animation
        SelectObject(hdcsrc, hBm);
        BitBlt(hdcdest,
            xpos, ypos + 32 - iparam1,
            width, iparam1,
            hdcsrc,
            iFrame*width, height / 2, SRCAND);

        BitBlt(hdcdest,
            xpos, ypos + 32 - iparam1,
            width, iparam1,
            hdcsrc,
            iFrame*width, 0, SRCPAINT);

        iparam1++;
        if (iparam1 > 31)
        {
            iAniBegin = 0;	//animation end
        }
    default:
        break;
    }
}
/////////////////////////////////// my animation /////////////////////

MYANIOBJ::MYANIOBJ()
{
    memset(wlist, 0, sizeof(wlist));
    memset(hlist, 0, sizeof(hlist));
    memset(ylist, 0, sizeof(ylist));

    iframeplay = 0;
}

MYANIOBJ::~MYANIOBJ()
{}

void MYANIOBJ::DrawItem(int x, int y, int id, int iframe)
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, x, y, wlist[id], hlist[id],
        hdcsrc,
        iframe*wlist[id], ylist[id] + hlist[id], SRCAND);

    BitBlt(hdcdest, x, y, wlist[id], hlist[id],
        hdcsrc,
        iframe*wlist[id], ylist[id], SRCPAINT);
}

void MYANIOBJ::DrawItemNoMask(int x, int y, int id, int iframe)
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, x, y, wlist[id], hlist[id],
        hdcsrc,
        iframe*wlist[id], ylist[id], SRCCOPY);
}

void MYANIOBJ::DrawItemNoMaskWidth(int x, int y, int id, int w, int iframe)
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, x, y, w, hlist[id],
        hdcsrc,
        iframe*wlist[id], ylist[id], SRCCOPY);
}

void MYANIOBJ::PlayItem(int x, int y, int id)
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, x, y, wlist[id], hlist[id],
        hdcsrc,
        iframeplay*wlist[id], ylist[id] + hlist[id], SRCAND);

    BitBlt(hdcdest, x, y, wlist[id], hlist[id],
        hdcsrc,
        iframeplay*wlist[id], ylist[id], SRCPAINT);

    iframeplay = (iframeplay + 1) % 2;
}

void MYANIOBJ::InitAniList(int *pw, int *ph, int inum, int ismask)
{
    int i;
    memcpy(wlist, pw, inum * sizeof(int));
    memcpy(hlist, ph, inum * sizeof(int));

    ylist[0] = 0;
    for (i = 1; i < inum; i++)
    {
        //If it is a mask, the height of the interval between the two images should be multiplied by 2
        ylist[i] = ylist[i - 1] + hlist[i - 1] * (ismask + 1);
    }

}

void MYANIOBJ::InitAniList(int style, int a, int b)
{
    int i;

    switch (style)
    {
    case BM_SQUARE:
        //a : number, b: side length
        for (i = 0; i < a; i++)
        {
            wlist[i] = b;
            hlist[i] = b;
            ylist[i] = i * b;
        }
        break;

    default:
        break;
    }
}

/////////////////////////////////// my animation magic/////////////////////

MYANIMAGIC::MYANIMAGIC()
{
    memset(wlist, 0, sizeof(wlist));
    memset(hlist, 0, sizeof(hlist));
    memset(ylist, 0, sizeof(ylist));
}

MYANIMAGIC::~MYANIMAGIC()
{}

void MYANIMAGIC::DrawItem(int x, int y, int id, int iframe)
{
    SelectObject(hdcsrc, hBm);
    BitBlt(hdcdest, x, y, wlist[id], hlist[id],
        hdcsrc,
        iframe*wlist[id], ylist[id] + hlist[id], SRCAND);

    BitBlt(hdcdest, x, y, wlist[id], hlist[id],
        hdcsrc,
        iframe*wlist[id], ylist[id], SRCPAINT);
}

//with MASK
void MYANIMAGIC::InitAniList(int *pw, int *ph, int inum)
{
    int i;
    memcpy(wlist, pw, inum * sizeof(int));
    memcpy(hlist, ph, inum * sizeof(int));

    ylist[0] = 0;
    for (i = 1; i < inum; i++)
    {
        ylist[i] = ylist[i - 1] + hlist[i - 1] * 2;
    }

}

void MYANIMAGIC::SetDevice(HDC hdest, HDC hsrc, HDC htemp)
{
    hdcdest = hdest;
    hdcsrc = hsrc;
    hdctemp = htemp;
}