#include "Resource.h"
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <sstream>
typedef unsigned short int USI;

HWND hwnd;
POINT old_point;
HWND button, button_1, button_2, button_3, button_4;
HWND radio, radio_1, radio_2, radio_3, radio_4;
HBITMAP hbmHuman[5];
BOOL Init(HINSTANCE hInstance, int nCmdShow);
ATOM RegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
USI human_destination=0, number_people_above_lim=0, number_of_people=0, g_time_value=50; //spawn block do tego zeby co okreslony czas pojawialy sie ludki gdy ciagle kliamy. Pomaga tez przy zachowaniu odleglosci miedzy ludkami
USI SHAFT_X1=357,SHAFT_X2=667,SHAFT_Y1=5,SHAFT_Y2=768; //const

struct elevator
{
    USI current_level=0, max_weight=600, curr_weight=0, last_place_x=SHAFT_X1+4;
    USI height=150, width=290, pos_x=SHAFT_X1+10,pos_y=SHAFT_Y2-((current_level+1)*height)-7;
    USI door_y=pos_y, direction=0, min_level=999, max_level=0, current_state=0,last_visited_level=5555;; //states - 0 lift is moving / doing nothing, 3 - lift stopped to open door, 4 - door opening, 6 - people in, 7 - people out
};
elevator lift;

std::vector <USI> buttons_on_level;  //// 0-0 1-down 2-up 3-all

class human
{
public:
    human(USI level, USI number);
    ~human(){;}
    void add_values_to_lift();
    void wait_for_lift();
    bool enter_lift();
    void exit_lift();
    void set_pos_x(USI x) {pos_x=x;}
    void set_pos_y(USI temp) {pos_y=temp;}
    void set_dir(USI dir) {direction=dir;}
    USI get_pos_x() {return pos_x;}
    USI get_pos_y() {return pos_y;}
    USI get_side() {return side;}
    USI set_side(USI side_temp ) {side=side_temp;}
    USI set_id(USI id_temp ) {id=id_temp;}
    USI get_dir() {return direction;}
    USI get_level() {return start_level;}
    USI get_dest() {return dest_level;}
    USI get_id() {return id;}
    USI get_weight() {return h_weight;}
    bool waiting_near_shaft() {return reached_shaft;}
private:
    USI start_level=0,dest_level=0, pos_x=0,pos_y=0, side=2, direction=0, h_weight=70, id=0; // SIDE=1 - Lewa
    bool reached_shaft=false;
};

void set_max_or_min(USI lvl)
{
    if(lvl<lift.current_level)
            {
                if(lvl<lift.min_level)
                    lift.min_level=lvl;
                if(lift.direction==0)
                    lift.direction=1;
            }
            else if(lvl>lift.current_level)
            {
                if(lvl>lift.max_level)
                    lift.max_level=lvl;
                if(lift.direction==0)
                    lift.direction=2;
            }
}

human::human(USI level, USI number)
{
    id=number;
    start_level=level;
    (level%2==0) ? set_side(1) : set_side(2);
    dest_level=human_destination;
    (get_side()==1) ? pos_x=10 : pos_x=1014;
    pos_y=(SHAFT_Y2-17 -((start_level)*lift.height));
    if(dest_level>start_level)
    {
        set_dir(2); //UP
        if(buttons_on_level[start_level]==0 || buttons_on_level[start_level]==2 || buttons_on_level[start_level]==4) //DO FUNCKJI????
            buttons_on_level[start_level]=2;
        else if(buttons_on_level[start_level]==1 || buttons_on_level[start_level]==3)
            buttons_on_level[start_level]=3;
    }
    else
    {
        set_dir(1); //DOWN
        if(buttons_on_level[start_level]==0 || buttons_on_level[start_level]==1 || buttons_on_level[start_level]==4) //DO FUNCKJI????
            buttons_on_level[start_level]=1;
        else if(buttons_on_level[start_level]==2 || buttons_on_level[start_level]==3)
            buttons_on_level[start_level]=3;
    }
}

void human::wait_for_lift()
{
    HDC hdcTemp = GetDC( hwnd );
    if(pos_x>SHAFT_X1-20 && pos_x<SHAFT_X2+20)
	reached_shaft=true;
    if(get_side()==1)
    {
        if(GetPixel(hdcTemp, pos_x+40, pos_y-8)==RGB(255,255,255))
            pos_x++;
        if(reached_shaft)
            {
            set_max_or_min(start_level);
            set_max_or_min(dest_level);
            }
    }
    else if(get_side()==2)
        {
        if(GetPixel(hdcTemp, pos_x-10, pos_y)==RGB(255,255,255))
            pos_x--;
        if(reached_shaft)
            {
            set_max_or_min(start_level);
            set_max_or_min(dest_level);
            }
        }
    DeleteDC( hdcTemp );
}

void human::add_values_to_lift()
{
    if(lift.last_place_x+32<(SHAFT_X2-60))
        lift.last_place_x+=32;
    lift.curr_weight+=h_weight;
    if(buttons_on_level[dest_level]==0)
        buttons_on_level[dest_level]=4;
    set_side(0);
    set_max_or_min(start_level);
	set_max_or_min(dest_level);
}

bool human::enter_lift()
{
            if(get_side()==1)
                {
                if(pos_x<=lift.last_place_x+32)
                    pos_x++;
                else
                    {
                    add_values_to_lift();
                    return true;
                    }
                }
            else if(get_side()==2)
                {
                if(pos_x>=lift.last_place_x+32)
                    pos_x--;
                else
                    {
                    add_values_to_lift();
                    return true;
                    }
                }
            return false;
}

void human::exit_lift()
{
    if(dest_level%2==0){
        if(pos_x>=30)
            pos_x--;
        else
        {
            set_side(5);
            lift.last_place_x-=32;
            if(lift.last_place_x<SHAFT_X1+4)
				lift.last_place_x=SHAFT_X1+4;
            lift.curr_weight-=70;
        }
    }
    else if(dest_level%2==1)
        if(pos_x<=994)
            pos_x++;
        else
        {
            set_side(5);
            lift.last_place_x-=32;
            if(lift.last_place_x<SHAFT_X1+4)
				lift.last_place_x=SHAFT_X1+4;
            lift.curr_weight-=70;
        }
}

std::vector <human> people;

void delete_human(USI id)
{
    if(number_of_people>0)
        {
            people[id]=people.back();
            people[id].set_id(id);
            //people.back().~human(); //! ????
            people.pop_back();
            number_of_people--;
        }
}

bool humans_exit_lift(HDC hdcBufor)
{
    bool everyone_out=true, delete_this_human=false;
    USI delete_id=0;
    for (std::vector<human>::iterator it = people.begin() ; it != people.end(); ++it)
        {
            if(it->get_dest()==lift.current_level && it->get_side()==0)
            {
                it->exit_lift();
                if(it->get_side()!=5)
                    everyone_out=false;
                else
                    {
                    delete_this_human=true;
                    delete_id=it->get_id();
                    }
            }
        }
    if(delete_this_human)
        delete_human(delete_id);
    if(everyone_out==true)
    {
	if(buttons_on_level[lift.current_level]==4)
		buttons_on_level[lift.current_level]=0;
        lift.current_state=3;
        return true;
    }
    else
        return false;
}

USI check_limit()
{
    USI number=0;
    for (std::vector<human>::iterator it = people.begin() ; it != people.end(); ++it)
        if(it->get_side()==0 && it->get_dest()!=lift.current_level)
            number++;
    return number;
}

bool humans_enter_lift(HDC hdcBufor, USI number)            //TRUE WHEN ALL IN  / WEIGHT > LIMIT!, FALSE WHEN LIFT IS NOT MOVING
{
    USI j=0;
    bool everyone_in=true;
    for (std::vector<human>::iterator it = people.begin() ; it != people.end(); ++it)
        {
            if(it->get_level()==lift.current_level && (it->get_dir()==lift.direction || lift.current_level==lift.max_level || lift.current_level==lift.min_level)) //! co gdy winda wjedzie na max floor i bedzie miala zjechac w dol
            {
            if(j>=number && j<8)
                {
                it->enter_lift();
                if(it->get_side()!=0)
                    everyone_in=false;
                }
            j++;
            }
        }

    if(everyone_in==true)
    {
    if(number==0)
        {
            if(buttons_on_level[lift.current_level]==3)
            {
                if(lift.direction==2)
                    buttons_on_level[lift.current_level]=1;
                else
                    buttons_on_level[lift.current_level]=2;
            }
            else
                buttons_on_level[lift.current_level]=0;
        }
        else
        {
            if(lift.direction==2)
                {
                if(lift.current_level<lift.min_level)
                    lift.min_level=lift.current_level;
                }
            else if(lift.direction==1)
                if(lift.current_level>lift.max_level)
                    lift.max_level=lift.current_level;
        }
        lift.current_state=4;
        return true;
    }
    else
        return false;
}

void lift_open_door(HDC hdcBufor)
{
    if(lift.current_state==0)
        lift.current_state=1;
    if(lift.door_y==lift.pos_y+lift.height-5)
        lift.current_state=2;
    USI temp;
    HPEN old,pen;
    pen = CreatePen( PS_SOLID, 14,  RGB( 255, 255, 255 ) );
    old =( HPEN ) SelectObject( hdcBufor, pen );
    if(lift.current_level%2==0)
        temp=lift.pos_x-5;
    else
        temp=lift.pos_x+lift.width+3;
    MoveToEx( hdcBufor, temp, lift.pos_y+5, &old_point );
    LineTo(hdcBufor, temp, lift.door_y );
    SelectObject( hdcBufor, old );
    DeleteObject( pen );
    if(lift.current_state==1)
    {
        lift.door_y++;
        number_people_above_lim=check_limit();
    }
    else
    {
        if(lift.current_state==2)
            humans_exit_lift(hdcBufor);
        if(lift.current_state==3)
            humans_enter_lift(hdcBufor, number_people_above_lim);
        if(lift.current_state==4)
            {
            if(lift.door_y!=lift.pos_y)
                lift.door_y--;
            else
                {
                lift.current_state=0;
                lift.last_visited_level=lift.current_level;
                }
            }
    }
}

void lift_change_state(USI dir, HDC hdcBufor)
{
        if(buttons_on_level[lift.current_level]==dir || buttons_on_level[lift.current_level]==3 || buttons_on_level[lift.current_level]==4 && lift.last_visited_level!=lift.current_level)
            {
            if(lift.current_state==0)
                lift.door_y=lift.pos_y;
            lift_open_door(hdcBufor);
            }
        else if(lift.current_state==4)
            lift_open_door(hdcBufor);
}

void lift_move(HDC hdcBufor)
{
    std::stringstream WText;
    WText.str(std::string());
    WText << "Current lift weight: ";
    WText << lift.curr_weight;
    if(lift.curr_weight+70>=lift.max_weight)
		SetTextColor(hdcBufor, RGB(255,0,0));
    TextOut(hdcBufor, 10, 10, WText.str().c_str(), WText.str().length());
    Rectangle(hdcBufor, lift.pos_x, lift.pos_y, lift.pos_x+lift.width, lift.pos_y+lift.height);
   if(lift.direction==2)
   {
       lift.current_level=4-(((lift.pos_y+lift.height-15)/lift.height));
        if(lift.current_level>10)
            lift.current_level=0;
       if(lift.current_level!=lift.max_level)
            lift_change_state(2,hdcBufor);
        else
        {
        if(lift.current_level!=lift.last_visited_level)
            {
            if(lift.current_state==0)
                lift.door_y=lift.pos_y;
            lift_open_door(hdcBufor);
            if(lift.current_state==0)
                {
                lift.max_level=0;
                if(lift.curr_weight+70>=lift.max_weight &&( buttons_on_level[lift.current_level]==3 || buttons_on_level[lift.current_level]==1))
                    lift.max_level=lift.current_level;
                if(lift.min_level!=999)
                    lift.direction=1;
                else
                    lift.direction=0;
                }
            }
        }
        if(lift.current_state==0)
            lift.pos_y--;
   }
   else if(lift.direction==1)
   {
       lift.current_level=4-(((lift.pos_y-15)/lift.height));
        if(lift.current_level>10)
            lift.current_level=0;
        if(lift.current_level!=lift.min_level)
            lift_change_state(1,hdcBufor);
        else
        {
        if(lift.current_level!=lift.last_visited_level)
            {
            if(lift.current_state==0)
                    lift.door_y=lift.pos_y;
            lift_open_door(hdcBufor);
            if(lift.current_state==0)
                {
                lift.min_level=999;
                if(lift.curr_weight+70>=lift.max_weight &&( buttons_on_level[lift.current_level]==3 || buttons_on_level[lift.current_level]==2))
                    lift.max_level=lift.current_level;
                if(lift.max_level!=0)
                    lift.direction=2;
                else
                    lift.direction=0;
                }
            }
        }
        if(lift.current_state==0)
            lift.pos_y++;
   }
}

void draw_level(HDC hdcBufor, USI level)
{
    if(level%2==0)
    {
        MoveToEx( hdcBufor, 0,(SHAFT_Y2-7 -((level)*lift.height)), &old_point );
        LineTo(hdcBufor, SHAFT_X1, (SHAFT_Y2-7 -((level)*lift.height)) );
    }
    else
    {
        MoveToEx( hdcBufor, lift.pos_x+lift.width+10,(SHAFT_Y2-7 -((level)*lift.height)), &old_point );
        LineTo(hdcBufor, 1024, (SHAFT_Y2-7 -((level)*lift.height)) );
    }
}

void move_humans(HDC hdcBufor)
{
    BITMAP bmInfo;
    HDC hdcBitmap = CreateCompatibleDC( hdcBufor );
    HDC hdcOkno = GetDC( hwnd );
    Rectangle(hdcBitmap, 0, 0, 1024, 768);
    HBITMAP hbmOld,hbmMask =( HBITMAP ) LoadImage( NULL, "mask.bmp", IMAGE_BITMAP, 30, 65, LR_LOADFROMFILE );
    for (std::vector<human>::iterator it = people.begin() ; it != people.end(); ++it)
        {
            if(it->get_side()!=0 && !it->waiting_near_shaft())   //!
                it->wait_for_lift(); //!
            else if(it->get_side()==0)
            {
                it->set_pos_y(lift.pos_y+lift.height-10);
            if(it->get_pos_x()-5>SHAFT_X1+24 && it->get_dest()!=lift.current_level && lift.current_state>0)
				if(GetPixel(hdcOkno, it->get_pos_x()-10, it->get_pos_y())==RGB(255,255,255))
                    it->set_pos_x(it->get_pos_x()-1);
			}
            if(it->get_side()!=5)
            {
                GetObject( hbmHuman[it->get_dest()], sizeof( bmInfo ), & bmInfo );
                hbmOld =( HBITMAP ) SelectObject( hdcBitmap, hbmMask );
                BitBlt( hdcBufor, it->get_pos_x(), it->get_pos_y()-60, bmInfo.bmWidth, bmInfo.bmHeight, hdcBitmap, 0, 0, SRCAND );
                SelectObject( hdcBitmap, hbmHuman[it->get_dest()]);
                BitBlt( hdcBufor, it->get_pos_x(), it->get_pos_y()-60, bmInfo.bmWidth, bmInfo.bmHeight, hdcBitmap, 0, 0, SRCPAINT);
            }
        }
    SelectObject( hdcBitmap, hbmOld );
    DeleteDC( hdcBitmap );
    DeleteObject( hbmMask );
    ReleaseDC( hwnd, hdcOkno); //!!!!!!!
}


void draw_buff_elem(HDC hdcBufor, HPEN pen2, HPEN old, USI init)
{
    Rectangle(hdcBufor, 0, 0, 1024, 768);
    Rectangle(hdcBufor, SHAFT_X1, SHAFT_Y1, SHAFT_X2, SHAFT_Y2);
    for(int i=0;i<5;i++)
        draw_level(hdcBufor, i);
    old =( HPEN ) SelectObject( hdcBufor, pen2 );
    if(init==1)
        Rectangle(hdcBufor, lift.pos_x, lift.pos_y, lift.pos_x+lift.width, lift.pos_y+lift.height);
}

void main_loop(USI init=0)
{
    HDC hdcOkno,hdcBufor;
    HPEN old,pen,pen2;
    hdcOkno = GetDC( hwnd );
    HBITMAP Membitmap = CreateCompatibleBitmap(hdcOkno, 1024, 768);
    hdcBufor = CreateCompatibleDC( hdcOkno );
	SelectObject(hdcBufor, Membitmap);
    pen = CreatePen( PS_SOLID, 2,  RGB( 0, 0, 0 ) );
    pen2 = CreatePen( PS_SOLID, 1,  RGB( 250, 10, 10 ) );
    old =( HPEN ) SelectObject( hdcBufor, pen );
    draw_buff_elem(hdcBufor, pen2, old, init);
    lift_move(hdcBufor);
    SelectObject( hdcBufor, old );
    move_humans(hdcBufor);
    BitBlt( hdcOkno, 0, 0, 1024, 768, hdcBufor, 0, 0, SRCCOPY );  //zmienic na SHIFT_WIDTH ITP!!!!
    BitBlt( hdcOkno, SHAFT_X1, SHAFT_Y1, 309, 768, hdcBufor, SHAFT_X1, SHAFT_Y1, SRCCOPY );  //zmienic na SHIFT_WIDTH ITP!!!!
    BitBlt( hdcOkno, lift.pos_x, lift.pos_y, lift.width, lift.height, hdcBufor, lift.pos_x, lift.pos_y, SRCCOPY );  //zmienic na SHIFT_WIDTH ITP!!!!
    DeleteObject( pen );
    DeleteObject( pen2 );
    DeleteObject(Membitmap);
    ReleaseDC( hwnd, hdcOkno );
    DeleteDC( hdcBufor );
}

BOOL Init(HINSTANCE hInstance, int nCmdShow)
{
    button = CreateWindowEx( WS_EX_CLIENTEDGE, "BUTTON", "LEVEL 0", WS_CHILD |  WS_VISIBLE,1120, 170, 90, 30, hwnd,(HMENU) ID_BUTTON_1, hInstance, NULL );
    button_1 = CreateWindowEx( WS_EX_CLIENTEDGE, "BUTTON", "LEVEL 1", WS_CHILD | WS_VISIBLE,1120, 210, 90, 30, hwnd,(HMENU) ID_BUTTON_2, hInstance, NULL );
    button_2 = CreateWindowEx( WS_EX_CLIENTEDGE, "BUTTON", "LEVEL 2", WS_CHILD | WS_VISIBLE,1120, 250, 90, 30, hwnd,(HMENU) ID_BUTTON_3, hInstance, NULL );
    button_3 = CreateWindowEx( WS_EX_CLIENTEDGE, "BUTTON", "LEVEL 3", WS_CHILD | WS_VISIBLE,1120, 290, 90, 30, hwnd,(HMENU) ID_BUTTON_4, hInstance, NULL );
    button_4 = CreateWindowEx( WS_EX_CLIENTEDGE, "BUTTON", "LEVEL 4", WS_CHILD | WS_VISIBLE,1120, 330, 90, 30, hwnd,(HMENU) ID_BUTTON_5, hInstance, NULL );
    radio = CreateWindowEx( 0, "BUTTON", "DEST. LEVEL 0", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,1120, 10, 160, 20, hwnd,(HMENU) ID_RADIO_1, hInstance, NULL );
    radio_1 = CreateWindowEx( 0, "BUTTON", "DEST. LEVEL 1", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,1120, 40, 160, 20, hwnd,(HMENU) ID_RADIO_2, hInstance, NULL );
    radio_2 = CreateWindowEx( 0, "BUTTON", "DEST. LEVEL 2", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,1120, 70, 160, 20, hwnd,(HMENU) ID_RADIO_3, hInstance, NULL );
    radio_3 = CreateWindowEx( 0, "BUTTON", "DEST. LEVEL 3", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,1120, 100, 160, 20, hwnd,(HMENU) ID_RADIO_4, hInstance, NULL );
    radio_4 = CreateWindowEx( 0, "BUTTON", "DEST. LEVEL 4", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,1120, 130, 160, 20, hwnd,(HMENU) ID_RADIO_5, hInstance, NULL );
    hbmHuman[0] =( HBITMAP ) LoadImage( NULL, "0.bmp", IMAGE_BITMAP, 30, 65, LR_LOADFROMFILE );
    hbmHuman[1] =( HBITMAP ) LoadImage( NULL, "1.bmp", IMAGE_BITMAP, 30, 65, LR_LOADFROMFILE );
    hbmHuman[2] =( HBITMAP ) LoadImage( NULL, "2.bmp", IMAGE_BITMAP, 30, 65, LR_LOADFROMFILE );
    hbmHuman[3] =( HBITMAP ) LoadImage( NULL, "3.bmp", IMAGE_BITMAP, 30, 65, LR_LOADFROMFILE );
    hbmHuman[4] =( HBITMAP ) LoadImage( NULL, "4.bmp", IMAGE_BITMAP, 30, 65, LR_LOADFROMFILE );
    ShowWindow (hwnd, nCmdShow);
    buttons_on_level.reserve(5);
    buttons_on_level.resize(5,0);
    main_loop(1);
}

ATOM RegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    wincl.hInstance = hInstance;
	wincl.lpszClassName = _T("WindowsApp");
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    wincl.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(255, 255, 255));;
    return RegisterClassEx(&wincl);
}

int WINAPI WinMain (HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszArgument,int nCmdShow)
{
    MSG messages;
    RegisterClass(hInstance);
    HFONT hNormalFont =( HFONT ) GetStockObject( DEFAULT_GUI_FONT );
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           _T("WindowsApp"),         /* Classname */
           _T("Projekt 4"),       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           100,       /* Windows decides the position */
           0,       /* where the window ends up on the screen */
           1280,                 /* The programs width */
           820,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );
    if (!Init(hInstance, nCmdShow))
        return 1;
    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return messages.wParam;
}

void spawn_human(USI level)
{
    if(level!=human_destination)
        {
            if(g_time_value>=40)
                {
                    SetTimer( hwnd, ID_TIMER, 1, NULL );
                    people.push_back(human(level,number_of_people));
                    people.back().wait_for_lift();
                    number_of_people++;
                    g_time_value=0;
                }
        }
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            KillTimer( hwnd, ID_TIMER );
            for(int i=0;i<5;i++)
                DeleteObject( hbmHuman[i]);
            PostQuitMessage (0);
            break;
        case WM_TIMER:
            main_loop();
            if(g_time_value!=41)
                g_time_value++;
            break;
        case WM_COMMAND:
            switch( wParam )
            {
            case ID_BUTTON_1:
                spawn_human(0);
                break;
            case ID_BUTTON_2:
                spawn_human(1);
                break;
            case ID_BUTTON_3:
                spawn_human(2);
                break;
            case ID_BUTTON_4:
                spawn_human(3);
                break;
            case ID_BUTTON_5:
                spawn_human(4);
                break;
            case ID_RADIO_1:
                CheckRadioButton( hwnd, ID_RADIO_1, ID_RADIO_5, ID_RADIO_1 );
                human_destination=0;
                break;
            case ID_RADIO_2:
                CheckRadioButton( hwnd, ID_RADIO_1, ID_RADIO_5, ID_RADIO_2 );
                human_destination=1;
                break;
            case ID_RADIO_3:
                CheckRadioButton( hwnd, ID_RADIO_1, ID_RADIO_5, ID_RADIO_3 );
                human_destination=2;
                break;
            case ID_RADIO_4:
                CheckRadioButton( hwnd, ID_RADIO_1, ID_RADIO_5, ID_RADIO_4 );
                human_destination=3;
                break;
            case ID_RADIO_5:
                CheckRadioButton( hwnd, ID_RADIO_1, ID_RADIO_5, ID_RADIO_5 );
                human_destination=4;
                break;
            default:
                break;
            }
            break;

        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
    return 0;
}
