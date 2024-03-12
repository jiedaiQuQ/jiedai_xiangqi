#include<graphics.h>
#include<bits/stdc++.h>
using namespace std;
#define BK_COLOR WHITE//背景色 
#define BOARD_COLOR EGERGB(222,195,142)//棋盘底色 
#define MENU_COLOR EGERGB(48,176,239)//菜单选项色 
#define MENU2_COLOR EGERGB(233,233,233)//面板底色 
const int SCREEN_W=1000;
const int SCREEN_H=700;
const int BOARD_X=50;
const int BOARD_Y=60;
const int GRID_LEN=60;
const int MENU_X=BOARD_X+GRID_LEN*8+140;
const int MENU_Y=BOARD_Y+50;
const int MENU2_X=BOARD_X+GRID_LEN*8+120-50;
const int MENU2_Y=BOARD_Y+50+160;
const int PUT_X=MENU2_X;
const int PUT_Y=MENU_Y;

const int Red=1;
const int Black=0;

const int Empty=0;
const int Shuai=1;
const int Shi=2;
const int Xiang=3;
const int Ma=4;
const int Ju=5;
const int Pao=6;
const int Bing=7;

static char ChessName[2][8][5]={
	{"  ","將","士","象","馬","車","炮","卒"},
	{"  ","帥","仕","相","馬","車","炮","兵"}
};

const int IDShuai=0;
const int IDShi1=1;
const int IDShi2=2;
const int IDXiang1=3;
const int IDXiang2=4;
const int IDMa1=5;
const int IDMa2=6;
const int IDJu1=7;
const int IDJu2=8;
const int IDPao1=9;
const int IDPao2=10;
const int IDBing1=11;
const int IDBing2=12;
const int IDBing3=13;
const int IDBing4=14;
const int IDBing5=15;
const int IDType[]={Shuai,Shi,Shi,Xiang,Xiang,Ma,Ma,Ju,Ju,Pao,Pao,Bing,Bing,Bing,Bing,Bing};
const int IDLeft[]={0,0,1,3,5,7,9,11};
const int IDRight[]={0,0,2,4,6,8,10,15};

const int RolePlayer=0;
const int RoleComputer=1;

const int ShuaiDelta[4]={-1,-16,1,16};
const int MaDelta[4][2]={{-18,14},{-33,-31},{-14,18},{31,33}};
const int ShiDelta[4]={-17,-15,17,15};
const int XiangDelta[4]={-34,-30,34,30};

const int Infinite=1e9;
const int WinValue=100000;
const int WinCheck=WinValue/2;
const int BanValue=WinValue+100;
const int DrawValue=-300;

const int ModePlayerPlayer=1;
const int ModePlayerComputer=2;
const int ModeComputerPlayer=3;
const int ModeComputerComputer=4;
const int ModeFirst=5;
const int ModeBack=6;

#define H(x) ((x)>>4)
#define W(x) ((x)&15)
#define HW(x,y) ((x)<<4|(y))

namespace Ctrl{
	const int GREY=7,WHITE=15,GREEN=10,BLUE=11,PURPLE=13,YELLOW=14,RED=12;
	const int GREEN_=2,BLUE_=3,PURPLE_=5,YELLOW_=6,RED_=4;
	void SetColor(int color){
		static int nowColor=GREY;
		if(nowColor==color)return;
		nowColor=color;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),color);
	}
	void SetWindowSize(int wide,int high){
		HANDLE hOut=GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleScreenBufferSize(hOut,(COORD){wide,500});
		SMALL_RECT rc={0,0,wide-1,high-1};
		SetConsoleWindowInfo(hOut,true,&rc);
	}
}

class Timer {
public:
    Timer(string message="") : message(message), start_time(chrono::steady_clock::now()) {
    	shutdown_=false;
	}
    ~Timer() {
    	if(shutdown_)return;
        auto end_time = chrono::steady_clock::now();
        auto elapsed_time = chrono::duration_cast<chrono::duration<double>>(end_time - start_time).count();
        cout << message << " cost " << fixed << setprecision(3) << elapsed_time << " s" << endl;
    }
    void Shutdown(){
    	shutdown_=true;
    }
private:
	string message;
    chrono::time_point<chrono::steady_clock> start_time;
	bool shutdown_;
};

struct Chess{
	int legal;
	bool color;
	int type;
	bool IsLegal(){
		return legal&1;
	}
	bool IsLegalShuai(){
		return legal>>Shuai&1;
	}
	bool IsLegalShi(){
		return legal>>Shi&1;
	}
	bool IsLegalXiang(){
		return legal>>Xiang&1;
	}
};

struct Move{
	int begin;
	int end;
	int kill;
};

struct HistoryTable{
	int value[256][256];
	void Init(){
		memset(value,0,sizeof(value));
	}
	void Attenuate(){
		for(int i=0;i<256;i++)for(int j=0;j<256;j++)value[i][j]>>=2;
	}
	void Update(Move &move,int depth){
		if(move.begin==0&&move.end==0)return;
		value[move.begin][move.end]+=depth*depth;
	}
};

struct KillerTable{
	static const int Capacity=100;
	Move killer1[Capacity];
	Move killer2[Capacity];
	void Insert(Move &move,int distance){
		if(move.begin==killer1[distance].begin&&move.end==killer1[distance].end)return;
		killer2[distance]=killer1[distance];
		killer1[distance]=move;
	}
	void Init(){
		memset(killer1,0,sizeof(killer1));
		memset(killer2,0,sizeof(killer2));
	}
};

typedef unsigned __int128 ZobristType;
struct Zobrist{
	ZobristType value[2][8][256],round;
	ZobristType RandomValue(int offset){
		return ZobristType(random(0))<<offset;
	}
	void Init(){
		if(round)return;
		round=RandomValue(0)^RandomValue(32)^RandomValue(64)^RandomValue(96);
		for(int i=0;i<2;i++)for(int j=1;j<8;j++)for(int k=0;k<256;k++)
			value[i][j][k]=RandomValue(0)^RandomValue(32)^RandomValue(64)^RandomValue(96);
	}
};

struct TTItem{
	ZobristType key;
	int minValue,maxValue;
	char depth;
	unsigned char begin;
	unsigned char end;
	TTItem(){}
	TTItem(ZobristType key,int minValue,int maxValue,char depth,unsigned char begin,unsigned char end){
		this->key=key;
		this->minValue=minValue;
		this->maxValue=maxValue;
		this->depth=depth;
		this->begin=begin;
		this->end=end;
	}
	void Init(){
		key=0;
		minValue=-Infinite;
		maxValue=Infinite;
		depth=-1;
		begin=0;
		end=0;
	}
};

struct TranspositionTable{
	static const int Layer=3;
	static const int Capacity=1<<20;
	static const int BitMod=Capacity-1;
	static const int LayerMinus=Layer-1;
	TTItem table[Layer][Capacity];
	TTItem* Query(ZobristType key){
		int position=key&BitMod;
		for(int i=0;i<Layer;i++)if(table[i][position].key==key)return &table[i][position];
		return NULL;
	}
	void Insert(ZobristType key,int minValue,int maxValue,char depth,char distance,Move &move){
		int position=key&BitMod;
		int minDepth=Infinite,choose=0;
		static int beginIndex=0;
		for(int i=beginIndex;;){
			if(key==table[i][position].key){
				choose=i;
				break;
			}
			if(table[i][position].depth<minDepth){
				minDepth=table[i][position].depth;
				choose=i;
			}
			i=(i==LayerMinus?0:i+1);
			if(i==beginIndex)break;
		}
		if(minValue==maxValue){
			if(minValue>WinCheck)minValue+=distance,maxValue+=distance;
			else if(minValue<-WinCheck)minValue-=distance,maxValue-=distance;
		}
		TTItem &item=table[choose][position];
		if(key==item.key){
			if(depth>item.depth){
				if(move.begin)item=TTItem(key,minValue,maxValue,depth,move.begin,move.end);
				else item=TTItem(key,minValue,maxValue,depth,item.begin,item.end);
			}
			else if(depth<item.depth){
				if(item.begin==0&&depth>=item.depth/2){
					item.begin=move.begin;
					item.end=move.end;
				}
			}
			else{
				item.maxValue=max(item.maxValue,maxValue);
				item.minValue=min(item.minValue,minValue);
				if(item.minValue>item.maxValue)swap(item.minValue,item.maxValue);
				if(move.begin){
					item.begin=move.begin;
					item.end=move.end;
				}
			}
		}
		else{
			item=TTItem(key,minValue,maxValue,depth,move.begin,move.end);
		}
		beginIndex=(beginIndex==LayerMinus?0:beginIndex+1);
	}
	void Init(){
		for(int i=0;i<Layer;i++)for(int j=0;j<Capacity;j++)table[i][j].Init();
	}
};

struct LineSituation{
	static const int Capacity=3*3*3*3*3*3*3*3*3*3;
	char juMoveCountH[16][Capacity];//横移(9)
	char juMoveCountW[16][Capacity];//竖移(10)
	char paoMoveCountH[16][Capacity];
	char paoMoveCountW[16][Capacity];
	unsigned char juKillH[16][Capacity];
	unsigned char juKillW[16][Capacity];
	unsigned char paoKillH[16][Capacity];
	unsigned char paoKillW[16][Capacity];
	unsigned char pao2KillH[16][Capacity];
	unsigned char pao2KillW[16][Capacity];
	int three[16];
	int valueH[2][256],valueW[2][256];
	int changeH[2][256],changeW[2][256];//!color变成color的增量
	void Init(){
		if(three[0])return;
		three[0]=1;
		for(int i=1;i<16;i++)three[i]=three[i-1]*3;
		for(int color=0;color<2;color++)for(int h=3;h<=12;h++)for(int w=3;w<=11;w++){
			valueH[color][HW(h,w)]=(1+color)*three[h-3];
			valueW[color][HW(h,w)]=(1+color)*three[w-3];
			changeH[color][HW(h,w)]=(color-!color)*three[h-3];
			changeW[color][HW(h,w)]=(color-!color)*three[w-3];
		}
		for(int s=0;s<Capacity;s++){
			static int color[16];
			for(int i=3;i<=12;i++)color[i]=s/three[i-3]%3;
			for(int i=3;i<=11;i++){
				if(color[i]==0)continue;
				for(int j=i+1,count=0;j<=11;j++){
					if(color[j])count++;
					if(count==0||count==1&&color[i]+color[j]==3)juMoveCountH[i][s]++;
					if(count==0||count==2&&color[i]+color[j]==3)paoMoveCountH[i][s]++;
					if(count==1&&color[j])juKillH[i][s]+=j;
					if(count==2&&color[j])paoKillH[i][s]+=j;
					if(count==3&&color[j])pao2KillH[i][s]+=j;
				}
				for(int j=i-1,count=0;j>=3;j--){
					if(color[j])count++;
					if(count==0||count==1&&color[i]+color[j]==3)juMoveCountH[i][s]++;
					if(count==0||count==2&&color[i]+color[j]==3)paoMoveCountH[i][s]++;
					if(count==1&&color[j])juKillH[i][s]+=16*j;
					if(count==2&&color[j])paoKillH[i][s]+=16*j;
					if(count==3&&color[j])pao2KillH[i][s]+=16*j;
				}
			}
			for(int i=3;i<=12;i++){
				if(color[i]==0)continue;
				for(int j=i+1,count=0;j<=12;j++){
					if(color[j])count++;
					if(count==0||count==1&&color[i]+color[j]==3)juMoveCountW[i][s]++;
					if(count==0||count==2&&color[i]+color[j]==3)paoMoveCountW[i][s]++;
					if(count==1&&color[j])juKillW[i][s]+=j;
					if(count==2&&color[j])paoKillW[i][s]+=j;
					if(count==3&&color[j])pao2KillW[i][s]+=j;
				}
				for(int j=i-1,count=0;j>=3;j--){
					if(color[j])count++;
					if(count==0||count==1&&color[i]+color[j]==3)juMoveCountW[i][s]++;
					if(count==0||count==2&&color[i]+color[j]==3)paoMoveCountW[i][s]++;
					if(count==1&&color[j])juKillW[i][s]+=16*j;
					if(count==2&&color[j])paoKillW[i][s]+=16*j;
					if(count==3&&color[j])pao2KillW[i][s]+=16*j;
				}
			}
		}
	}
};

struct RepeatTable{
    static const int Capacity=1<<10;
    static const int BitMod=Capacity-1;
	int top,stack[Capacity],head[Capacity],value[Capacity],next[Capacity];
	ZobristType key[Capacity];
	void Add(ZobristType x){
		int position=x&BitMod;
		for(int i=head[position];i;i=next[i])if(key[i]==x){
			value[i]++;
			return;
		}
		int id=stack[top--];
		key[id]=x;
		value[id]=1;
		next[id]=head[position];
		head[position]=id;
	}
	void Reduce(ZobristType x){
		int position=x&BitMod;
		for(int i=head[position],j=0;i;j=i,i=next[i])if(key[i]==x){
			value[i]--;
			if(value[i]==0){
				if(j)next[j]=next[i];
				else head[position]=next[i];
				stack[++top]=i;
			}
			return;
		}
	}
	int Query(ZobristType x){
		int position=x&BitMod;
		for(int i=head[position];i;i=next[i])if(key[i]==x)return value[i];
		return 0;
	}
	void Init(){
		memset(head,0,sizeof(head));
		top=0;
		for(int i=1;i<Capacity;i++)stack[++top]=i;
	}
};

struct LineupTable{
//	int smallValue[3][3][3][3][3][6];
	int endGameDegree[100][100];
	int lineupValue[3][3][3][3][3][6][3][3][3][3][3][6];
	void Init(){
//		for(shi=0;shi<=2;shi++)
//		for(xiang=0;xiang<=2;xiang++)
//		for(ma=0;ma<=2;ma++)
//		for(ju=0;ju<=2;ju++)
//		for(pao=0;pao<=2;pao++)
//		for(bing=0;bing<=5;bing++){
//			smallValue[shi][xiang][ma][ju][pao][bing]=shi*2+xiang*2+ma*5+ju*10+pao*5+bing*1;
//		}
		for(int i=0;i<100;i++)for(int j=0;j<100;j++){
			double degree1=max(0,30-max(i,j))/30.0;
			double degree2=max(0,30-min(i,j))/30.0;
			endGameDegree[i][j]=0.8*degree1+0.2*degree2;
		}
		int shi[2],xiang[2],ma[2],ju[2],pao[2],bing[2];
		for(shi  [0]=0;shi  [0]<=2;shi  [0]++)
		for(xiang[0]=0;xiang[0]<=2;xiang[0]++)
		for(ma   [0]=0;ma   [0]<=2;ma   [0]++)
		for(ju   [0]=0;ju   [0]<=2;ju   [0]++)
		for(pao  [0]=0;pao  [0]<=2;pao  [0]++)
		for(bing [0]=0;bing [0]<=5;bing [0]++)
		for(shi  [1]=0;shi  [1]<=2;shi  [1]++)
		for(xiang[1]=0;xiang[1]<=2;xiang[1]++)
		for(ma   [1]=0;ma   [1]<=2;ma   [1]++)
		for(ju   [1]=0;ju   [1]<=2;ju   [1]++)
		for(pao  [1]=0;pao  [1]<=2;pao  [1]++)
		for(bing [1]=0;bing [1]<=5;bing [1]++){
			int value[2]={0};
			int small[2]={
				shi[0]*2+xiang[0]*2+ma[0]*5+ju[0]*10+pao[0]*5+bing[0]*1,
				shi[1]*2+xiang[1]*2+ma[1]*5+ju[1]*10+pao[1]*5+bing[1]*1
			};
			double degree=endGameDegree[small[0]][small[1]];
			bool isEndGame=(ma[0]+ju[0]+pao[0]<=2&&ma[1]+ju[1]+pao[1]<=2);
			for(int i=0;i<2;i++){
				value[i]+=150 *(shi  [i]>=1)+200 *(shi  [i]>=2);
				value[i]+=150 *(xiang[i]>=1)+225 *(xiang[i]>=2);
				value[i]+=425 *(ma   [i]>=1)+425 *(ma   [i]>=2);
				value[i]+=1000*(ju   [i]>=1)+1000*(ju   [i]>=2);
				value[i]+=475 *(pao  [i]>=1)+475 *(pao  [i]>=2);
				double bingValue=100+100*degree;
				double bingK=(1-degree)*15;
				value[i]+=int(
					(bingValue+1*bingK)*(bing[i]>=1)+
					(bingValue+0.5*bingK)*(bing[i]>=2)+
					(bingValue)*(bing[i]>=3)+
					(bingValue-1*bingK)*(bing[i]>=4)+
					(bingValue-2*bingK)*(bing[i]>=5)
				);
				value[i]+=max(0,(small[i]-small[!i]-5)*(30-small[!i]));//换子激励
				if(ma[i]+ju[i]+pao[i]==1){
					if(ma[i]==1)value[i]-=100;
					if(ju[i]==1)value[i]-=300;
					if(pao[i]==1)value[i]-=150;
				}
			}
			lineupValue[shi[0]][xiang[0]][ma[0]][ju[0]][pao[0]][bing[0]]
			[shi[1]][xiang[1]][ma[1]][ju[1]][pao[1]][bing[1]]=value[0]-value[1];
		}
	}
};

HistoryTable historyTable;
KillerTable killerTable;
Zobrist zobrist;
TranspositionTable transpositionTable;
LineSituation lineSituation;
int bingNextPosition[3][2][256];
LineupTable lineupTable;

struct ChessBoard{
	Chess chess[256];
	int position[2][16];
	bool color;
	int roleRed;
	int roleBlack;
	bool flip;
	int computerLevel;
	int computerStep;
	int hintLevel;
	int hintStep;
	vector<Move>moveStack;
	ZobristType key;
	int lineH[16];
	int lineW[16];
	RepeatTable repeatTable;
	int typeCount[2][16];
	void Render(){
		puts("|------------------|");
		for(int i=3;i<=12;i++){
			printf("|");
			for(int j=3;j<=11;j++){
				if(chess[HW(i,j)].type){
					Ctrl::SetColor(chess[HW(i,j)].color==Red?Ctrl::RED:Ctrl::GREY);
					static char str[8][5]={"  ","将","士","相","马","车","炮","兵"};
					printf("%s",str[chess[HW(i,j)].type]);
					Ctrl::SetColor(Ctrl::GREY);
				}
				else printf("  ");
			}
			puts("|");
		}
		puts("|------------------|");
	}
	bool IsEndGame(){
		return typeCount[Red][Ma]+typeCount[Red][Ju]+typeCount[Red][Pao]<=2&&
			typeCount[Black][Ma]+typeCount[Black][Ju]+typeCount[Black][Pao]<=2;
	}
	bool NullMoveSafe(){
		return typeCount[color][Ma]+typeCount[color][Ju]+typeCount[color][Pao]>=3;
	}
	bool CheckMove(int begin,int end){//判断移动是否合法
		if(begin==end)return false;
		if(!chess[begin].IsLegal()||!chess[end].IsLegal())return false;
		if(chess[end].type&&chess[begin].color==chess[end].color)return false;
		if(chess[begin].type==Shuai){
			if(!chess[end].IsLegalShuai())return false;
			for(int i=0;i<4;i++)if(begin+ShuaiDelta[i]==end)return true;
		}
		else if(chess[begin].type==Shi){
			if(!chess[end].IsLegalShi())return false;
			for(int i=0;i<4;i++)if(begin+ShiDelta[i]==end)return true;
		}
		else if(chess[begin].type==Xiang){
			if(!chess[end].IsLegalXiang())return false;
			for(int i=0;i<4;i++)if(!chess[begin+ShiDelta[i]].type&&begin+XiangDelta[i]==end)return true;
		}
		else if(chess[begin].type==Ma){
			for(int i=0;i<4;i++){
				if(chess[begin+ShuaiDelta[i]].type)continue;
				for(int j=0;j<2;j++)if(begin+MaDelta[i][j]==end)return true;
			}
		}
		else if(chess[begin].type==Ju){
			if(H(begin)==H(end)){
				int minPosition=min(begin,end);
				int maxPosition=max(begin,end);
				for(int i=minPosition+1;i<maxPosition;i++)if(chess[i].type)return false;
				return true;
			}
			else if(W(begin)==W(end)){
				int minPosition=min(begin,end);
				int maxPosition=max(begin,end);
				for(int i=minPosition+16;i<maxPosition;i+=16)if(chess[i].type)return false;
				return true;
			}
		}
		else if(chess[begin].type==Pao){
			if(H(begin)==H(end)){
				int minPosition=min(begin,end);
				int maxPosition=max(begin,end);
				int count=0;
				for(int i=minPosition+1;i<maxPosition;i++)if(chess[i].type)count++;
				if(count==0&&!chess[end].type||count==1&&chess[end].type)return true;
			}
			else if(W(begin)==W(end)){
				int minPosition=min(begin,end);
				int maxPosition=max(begin,end);
				int count=0;
				for(int i=minPosition+16;i<maxPosition;i+=16)if(chess[i].type)count++;
				if(count==0&&!chess[end].type||count==1&&chess[end].type)return true;
			}
		}
		else if(chess[begin].type==Bing){
			if(chess[begin].color==Red){
				if(begin-16==end)return true;
				if(H(begin)<8&&(begin-1==end||begin+1==end))return true;
			}
			else{
				if(begin+16==end)return true;
				if(H(begin)>7&&(begin-1==end||begin+1==end))return true;
			}
		}
		return false;
	}
	vector<Move>GenerateAllMoves(int color){
		vector<Move>moves;
		for(int i=0;i<16;i++){
			int begin=position[color][i];
			int type=chess[begin].type;
			if(begin==0)continue;
			if(type==Shuai){
				for(int i=0;i<4;i++){
					int end=begin+ShuaiDelta[i];
					if(!chess[end].IsLegalShuai())continue;
					if(chess[end].type&&chess[end].color==color)continue;
					moves.push_back((Move){begin,end,chess[end].type});
				}
			}
			else if(type==Shi){
				for(int i=0;i<4;i++){
					int end=begin+ShiDelta[i];
					if(!chess[end].IsLegalShi())continue;
					if(chess[end].type&&chess[end].color==color)continue;
					moves.push_back((Move){begin,end,chess[end].type});
				}
			}
			else if(type==Xiang){
				for(int i=0;i<4;i++){
					int end=begin+XiangDelta[i];
					if(!chess[end].IsLegalXiang())continue;
					if(chess[begin+ShiDelta[i]].type)continue;
					if(chess[end].type&&chess[end].color==color)continue;
					moves.push_back((Move){begin,end,chess[end].type});
				}
			}
			else if(type==Ma){
				for(int i=0;i<4;i++){
					if(chess[begin+ShuaiDelta[i]].type)continue;
					for(int j=0;j<2;j++){
						int end=begin+MaDelta[i][j];
						if(!chess[end].IsLegal())continue;
						if(chess[end].type&&chess[end].color==color)continue;
						moves.push_back((Move){begin,end,chess[end].type});
					}
				}
			}
			else if(type==Ju){
				for(int i=0;i<4;i++){
					for(int end=begin+ShuaiDelta[i];;end+=ShuaiDelta[i]){
						if(!chess[end].IsLegal())break;
						if(chess[end].type&&chess[end].color==color)break;
						moves.push_back((Move){begin,end,chess[end].type});
						if(chess[end].type)break;
					}
				}
			}
			else if(type==Pao){
				for(int i=0;i<4;i++){
					int count=0;
					for(int end=begin+ShuaiDelta[i];count<2;end+=ShuaiDelta[i]){
						if(!chess[end].IsLegal())break;
						if(chess[end].type)count++;
						if(count==0||count==2&&chess[end].color!=color)
							moves.push_back((Move){begin,end,chess[end].type});
					}
				}
			}
			else if(type==Bing){
				if(chess[begin].color==Red){
					int end=begin-16;
					if(chess[end].IsLegal())
						if(!(chess[end].type&&chess[end].color==color))
							moves.push_back((Move){begin,end,chess[end].type});
					if(H(begin)<8){
						end=begin-1;
						if(chess[end].IsLegal())
							if(!(chess[end].type&&chess[end].color==color))
								moves.push_back((Move){begin,end,chess[end].type});
						end=begin+1;
						if(chess[end].IsLegal())
							if(!(chess[end].type&&chess[end].color==color))
								moves.push_back((Move){begin,end,chess[end].type});
					}
				}
				else{
					int end=begin+16;
					if(chess[end].IsLegal())
						if(!(chess[end].type&&chess[end].color==color))
							moves.push_back((Move){begin,end,chess[end].type});
					if(H(begin)>7){
						end=begin-1;
						if(chess[end].IsLegal())
							if(!(chess[end].type&&chess[end].color==color))
								moves.push_back((Move){begin,end,chess[end].type});
						end=begin+1;
						if(chess[end].IsLegal())
							if(!(chess[end].type&&chess[end].color==color))
								moves.push_back((Move){begin,end,chess[end].type});
					}
				}
			}
		}
		return moves;
	}
	vector<Move>GenerateEatMoves(int nowColor){
		vector<Move>moves;
		int begin,end;
		if(begin=position[nowColor][IDShuai]){
			for(int i=0;i<4;i++)
				if(chess[end=begin+ShuaiDelta[i]].type&&chess[end].color!=nowColor)
					if(chess[end].IsLegalShuai())
						moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDShi1]){
			for(int i=0;i<4;i++)
				if(chess[end=begin+ShiDelta[i]].type&&chess[end].color!=nowColor)
					if(chess[end].IsLegalShi())
						moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDShi2]){
			for(int i=0;i<4;i++)
				if(chess[end=begin+ShiDelta[i]].type&&chess[end].color!=nowColor)
					if(chess[end].IsLegalShi())
						moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDXiang1]){
			for(int i=0;i<4;i++)
				if(chess[end=begin+XiangDelta[i]].type&&chess[end].color!=nowColor&&!chess[begin+ShiDelta[i]].type)
					if(chess[end].IsLegalXiang())
						moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDXiang2]){
			for(int i=0;i<4;i++)
				if(chess[end=begin+XiangDelta[i]].type&&chess[end].color!=nowColor&&!chess[begin+ShiDelta[i]].type)
					if(chess[end].IsLegalXiang())
						moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDMa1]){
			for(int i=0;i<4;i++)if(!chess[begin+ShuaiDelta[i]].type)
				for(int j=0;j<2;j++)if(chess[end=begin+MaDelta[i][j]].type&&chess[end].color!=nowColor)
					moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDMa2]){
			for(int i=0;i<4;i++)if(!chess[begin+ShuaiDelta[i]].type)
				for(int j=0;j<2;j++)if(chess[end=begin+MaDelta[i][j]].type&&chess[end].color!=nowColor)
					moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDJu1]){
			int h=H(begin),w=W(begin);
			if(chess[end=HW(h,H(lineSituation.juKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(h,W(lineSituation.juKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(H(lineSituation.juKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(W(lineSituation.juKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDJu2]){
			int h=H(begin),w=W(begin);
			if(chess[end=HW(h,H(lineSituation.juKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(h,W(lineSituation.juKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(H(lineSituation.juKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(W(lineSituation.juKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDPao1]){
			int h=H(begin),w=W(begin);
			if(chess[end=HW(h,H(lineSituation.paoKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(h,W(lineSituation.paoKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(H(lineSituation.paoKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(W(lineSituation.paoKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDPao2]){
			int h=H(begin),w=W(begin);
			if(chess[end=HW(h,H(lineSituation.paoKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(h,W(lineSituation.paoKillH[w][lineH[h]]))].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(H(lineSituation.paoKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=HW(W(lineSituation.paoKillW[h][lineW[w]]),w)].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDBing1]){
			if(chess[end=bingNextPosition[0][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[1][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[2][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDBing2]){
			if(chess[end=bingNextPosition[0][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[1][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[2][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDBing3]){
			if(chess[end=bingNextPosition[0][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[1][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[2][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDBing4]){
			if(chess[end=bingNextPosition[0][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[1][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[2][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		if(begin=position[nowColor][IDBing5]){
			if(chess[end=bingNextPosition[0][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[1][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
			if(chess[end=bingNextPosition[2][nowColor][begin]].type&&chess[end].color!=nowColor)
				moves.push_back((Move){begin,end,chess[end].type});
		}
		return moves;
	}
	bool CheckKill(bool nowColor){//判断是否被将军
		int begin=position[nowColor][IDShuai],end;
		int h=H(begin),w=W(begin);
		if(chess[end=HW(h,H(lineSituation.paoKillH[w][lineH[h]]))].type==Pao)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=HW(h,W(lineSituation.paoKillH[w][lineH[h]]))].type==Pao)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=HW(H(lineSituation.paoKillW[h][lineW[w]]),w)].type==Pao)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=HW(W(lineSituation.paoKillW[h][lineW[w]]),w)].type==Pao)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=HW(h,H(lineSituation.juKillH[w][lineH[h]]))].type==Ju)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=HW(h,W(lineSituation.juKillH[w][lineH[h]]))].type==Ju)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=HW(H(lineSituation.juKillW[h][lineW[w]]),w)].type==Ju||chess[end].type==Shuai)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=HW(W(lineSituation.juKillW[h][lineW[w]]),w)].type==Ju||chess[end].type==Shuai)
			if(chess[end].color!=nowColor)return true;
		if(chess[end=begin-18].type==Ma&&chess[end].color!=nowColor&&!chess[begin-17].type)return true;
		if(chess[end=begin-33].type==Ma&&chess[end].color!=nowColor&&!chess[begin-17].type)return true;
		if(chess[end=begin-31].type==Ma&&chess[end].color!=nowColor&&!chess[begin-15].type)return true;
		if(chess[end=begin-14].type==Ma&&chess[end].color!=nowColor&&!chess[begin-15].type)return true;
		if(chess[end=begin+18].type==Ma&&chess[end].color!=nowColor&&!chess[begin+17].type)return true;
		if(chess[end=begin+33].type==Ma&&chess[end].color!=nowColor&&!chess[begin+17].type)return true;
		if(chess[end=begin+31].type==Ma&&chess[end].color!=nowColor&&!chess[begin+15].type)return true;
		if(chess[end=begin+14].type==Ma&&chess[end].color!=nowColor&&!chess[begin+15].type)return true;
		if(chess[end=begin-16].type==Bing&&chess[end].color!=nowColor&&nowColor==Red)return true;
		if(chess[end=begin+16].type==Bing&&chess[end].color!=nowColor&&nowColor==Black)return true;
		if(chess[begin-1].type==Bing)return true;
		if(chess[begin+1].type==Bing)return true;
		return false;
	}
	bool CheckProtect(int begin){//判断是否被保护
		int end;
		int nowColor=chess[begin].color;
		int h=H(begin),w=W(begin);
		if(chess[end=HW(h,H(lineSituation.paoKillH[w][lineH[h]]))].type==Pao)
			if(chess[end].color==nowColor)return true;
		if(chess[end=HW(h,W(lineSituation.paoKillH[w][lineH[h]]))].type==Pao)
			if(chess[end].color==nowColor)return true;
		if(chess[end=HW(H(lineSituation.paoKillW[h][lineW[w]]),w)].type==Pao)
			if(chess[end].color==nowColor)return true;
		if(chess[end=HW(W(lineSituation.paoKillW[h][lineW[w]]),w)].type==Pao)
			if(chess[end].color==nowColor)return true;
		if(chess[end=HW(h,H(lineSituation.juKillH[w][lineH[h]]))].type==Ju)
			if(chess[end].color==nowColor)return true;
		if(chess[end=HW(h,W(lineSituation.juKillH[w][lineH[h]]))].type==Ju)
			if(chess[end].color==nowColor)return true;
		if(chess[end=HW(H(lineSituation.juKillW[h][lineW[w]]),w)].type==Ju)
			if(chess[end].color==nowColor)return true;
		if(chess[end=HW(W(lineSituation.juKillW[h][lineW[w]]),w)].type==Ju)
			if(chess[end].color==nowColor)return true;
		if(chess[end=begin-18].type==Ma&&chess[end].color==nowColor&&!chess[begin-17].type)return true;
		if(chess[end=begin-33].type==Ma&&chess[end].color==nowColor&&!chess[begin-17].type)return true;
		if(chess[end=begin-31].type==Ma&&chess[end].color==nowColor&&!chess[begin-15].type)return true;
		if(chess[end=begin-14].type==Ma&&chess[end].color==nowColor&&!chess[begin-15].type)return true;
		if(chess[end=begin+18].type==Ma&&chess[end].color==nowColor&&!chess[begin+17].type)return true;
		if(chess[end=begin+33].type==Ma&&chess[end].color==nowColor&&!chess[begin+17].type)return true;
		if(chess[end=begin+31].type==Ma&&chess[end].color==nowColor&&!chess[begin+15].type)return true;
		if(chess[end=begin+14].type==Ma&&chess[end].color==nowColor&&!chess[begin+15].type)return true;
		if(nowColor==Red){
			if(chess[end=begin+16].type==Bing&&chess[end].color==nowColor)return true;
			if(chess[end=begin-1].type==Bing&&chess[end].color==nowColor&&h<=7)return true;
			if(chess[end=begin+1].type==Bing&&chess[end].color==nowColor&&h<=7)return true;
		}
		else{
			if(chess[end=begin-16].type==Bing&&chess[end].color==nowColor)return true;
			if(chess[end=begin-1].type==Bing&&chess[end].color==nowColor&&h>=8)return true;
			if(chess[end=begin+1].type==Bing&&chess[end].color==nowColor&&h>=8)return true;
		}
		if(chess[begin].IsLegalShuai())for(int i=0;i<4;i++)
			if(chess[end=begin+ShuaiDelta[i]].type==Shuai&&chess[end].color==nowColor)return true;
		if(chess[begin].IsLegalShi())for(int i=0;i<4;i++)
			if(chess[end=begin+ShiDelta[i]].type==Shi&&chess[end].color==nowColor)return true;
		if(chess[begin].IsLegalXiang())for(int i=0;i<4;i++)
			if(chess[end=begin+XiangDelta[i]].type==Xiang&&chess[end].color==nowColor)
				if(!chess[begin+ShiDelta[i]].type)return true;
		return false;
	}
	bool CheckLose(){
		vector<Move>moves=GenerateAllMoves(color);
		for(auto &move:moves){
			ExecuteMove(move);
			bool isKill=CheckKill(!color);
			RescindMove(move);
			if(!isKill)return false;
		}
		return true;
	}
	int MVVLVA(Move &move){
		if(!move.kill)return -1;
//		static int typeValue[8]={0,5,2,2,3,4,3,1};
//		return (typeValue[move.kill]<<3)-typeValue[chess[move.begin].type];
		
		static int simpleValue[8]={0,20,1,1,5,10,5,2};
//		static int simpleValue[8]={0,5,1,1,3,4,3,2};
		int value=simpleValue[move.kill]-(CheckProtect(move.end)?simpleValue[chess[move.begin].type]:0);
		if(value>=0)return value;
		if(simpleValue[move.kill]>=simpleValue[Ma])return 0;
		return -1;
	}
	int GetID(int p){
		for(int i=IDLeft[chess[p].type];i<=IDRight[chess[p].type];i++)
			if(position[chess[p].color][i]==p)return i;
		assert(0);
	}
	int GetNewID(bool color,int type){
		for(int i=IDLeft[type];i<=IDRight[type];i++)
			if(position[color][i]==0)return i;
		assert(0);
	}
	void ExecuteMove(Move &move){
		if(move.kill)typeCount[!color][move.kill]--;
		lineH[H(move.begin)]-=lineSituation.valueW[color][move.begin];
		lineW[W(move.begin)]-=lineSituation.valueH[color][move.begin];
		if(move.kill){
			lineH[H(move.end)]+=lineSituation.changeW[color][move.end];
			lineW[W(move.end)]+=lineSituation.changeH[color][move.end];
		}
		else{
			lineH[H(move.end)]+=lineSituation.valueW[color][move.end];
			lineW[W(move.end)]+=lineSituation.valueH[color][move.end];
		}
		key^=zobrist.round;
		if(move.kill)key^=zobrist.value[!color][move.kill][move.end];
		key^=zobrist.value[color][chess[move.begin].type][move.begin];
		key^=zobrist.value[color][chess[move.begin].type][move.end];
		repeatTable.Add(key);
		if(move.kill)position[!color][GetID(move.end)]=0;
		position[color][GetID(move.begin)]=move.end;
		chess[move.end].type=chess[move.begin].type;
		chess[move.end].color=chess[move.begin].color;
		chess[move.begin].type=Empty;
		color=!color;
	}
	void RescindMove(Move &move){
		color=!color;
		if(move.kill)typeCount[!color][move.kill]++;
		lineH[H(move.begin)]+=lineSituation.valueW[color][move.begin];
		lineW[W(move.begin)]+=lineSituation.valueH[color][move.begin];
		if(move.kill){
			lineH[H(move.end)]-=lineSituation.changeW[color][move.end];
			lineW[W(move.end)]-=lineSituation.changeH[color][move.end];
		}
		else{
			lineH[H(move.end)]-=lineSituation.valueW[color][move.end];
			lineW[W(move.end)]-=lineSituation.valueH[color][move.end];
		}
		repeatTable.Reduce(key);
		key^=zobrist.round;
		key^=zobrist.value[color][chess[move.end].type][move.begin];
		key^=zobrist.value[color][chess[move.end].type][move.end];
		if(move.kill)key^=zobrist.value[!color][move.kill][move.end];
		position[color][GetID(move.end)]=move.begin;
		if(move.kill)position[!color][GetNewID(!color,move.kill)]=move.end;
		chess[move.begin].type=chess[move.end].type;
		chess[move.begin].color=chess[move.end].color;
		chess[move.end].type=move.kill;
		chess[move.end].color=!color;
	}
	void ExecuteNullMove(){
		key^=zobrist.round;
		color=!color;
	}
	void RescindNullMove(){
		color=!color;
		key^=zobrist.round;
	}
	void AddChess(int p,bool color,int type){
		chess[p].color=color;
		chess[p].type=type;
		position[color][GetNewID(color,type)]=p;
		key^=zobrist.value[color][type][p];
		lineH[H(p)]+=lineSituation.valueW[color][p];
		lineW[W(p)]+=lineSituation.valueH[color][p];
		typeCount[color][type]++;
	}
	void Init(int nowColor=Red){
		color=nowColor;
		key=0;
		memset(position,0,sizeof(position));
		memset(chess,0,sizeof(chess));
		for(int i=3;i<=12;i++)for(int j=3;j<=11;j++)chess[HW(i,j)].legal|=1;
		for(int i=3;i<=5;i++)for(int j=6;j<=8;j++)chess[HW(i,j)].legal|=1<<Shuai;
		for(int i=10;i<=12;i++)for(int j=6;j<=8;j++)chess[HW(i,j)].legal|=1<<Shuai;
		chess[HW(3,6)].legal|=1<<Shi;
		chess[HW(3,8)].legal|=1<<Shi;
		chess[HW(4,7)].legal|=1<<Shi;
		chess[HW(5,6)].legal|=1<<Shi;
		chess[HW(5,8)].legal|=1<<Shi;
		chess[HW(10,6)].legal|=1<<Shi;
		chess[HW(10,8)].legal|=1<<Shi;
		chess[HW(11,7)].legal|=1<<Shi;
		chess[HW(12,6)].legal|=1<<Shi;
		chess[HW(12,8)].legal|=1<<Shi;
		chess[HW(3,5)].legal|=1<<Xiang;
		chess[HW(3,9)].legal|=1<<Xiang;
		chess[HW(5,3)].legal|=1<<Xiang;
		chess[HW(5,7)].legal|=1<<Xiang;
		chess[HW(5,11)].legal|=1<<Xiang;
		chess[HW(7,5)].legal|=1<<Xiang;
		chess[HW(7,9)].legal|=1<<Xiang;
		chess[HW(8,5)].legal|=1<<Xiang;
		chess[HW(8,9)].legal|=1<<Xiang;
		chess[HW(10,3)].legal|=1<<Xiang;
		chess[HW(10,7)].legal|=1<<Xiang;
		chess[HW(10,11)].legal|=1<<Xiang;
		chess[HW(12,5)].legal|=1<<Xiang;
		chess[HW(12,9)].legal|=1<<Xiang;
		
		moveStack.clear();
		memset(lineH,0,sizeof(lineH));
		memset(lineW,0,sizeof(lineW));
		repeatTable.Init();
		memset(typeCount,0,sizeof(typeCount));
		
		historyTable.Init();
		killerTable.Init();
		zobrist.Init();
		transpositionTable.Init();
	}
	int Evaluate1(int debug=0){
		static int controlValue[8][256]={//控制分
			{
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{//帅
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,  -9,  -9,  -9,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,  -8,  -8,  -8,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   1,   5,   1,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{//士
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   3,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{//相
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,  -2,   0,   0,   0,   3,   0,   0,   0,  -2,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{//马
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   2,   2,   2,   8,   2,   8,   2,   2,   2,   0,0,0,0,
				0,0,0,   2,   8,  15,   9,   6,   9,  15,   8,   2,   0,0,0,0,
				0,0,0,   4,  10,  11,  15,  11,  15,  11,  10,   4,   0,0,0,0,
				0,0,0,   5,  20,  12,  19,  12,  19,  12,  20,   5,   0,0,0,0,
				0,0,0,   2,  12,  11,  15,  16,  15,  11,  12,   2,   0,0,0,0,
				0,0,0,   2,  10,  13,  14,  15,  14,  13,  10,   2,   0,0,0,0,
				0,0,0,   4,   6,  10,   7,  10,   7,  10,   6,   4,   0,0,0,0,
				0,0,0,   5,   4,   6,   7,   4,   7,   6,   4,   5,   0,0,0,0,
				0,0,0,  -3,   2,   4,   5, -10,   5,   4,   2,  -3,   0,0,0,0,
				0,0,0,   0,  -3,   2,   0,   2,   0,   2,  -3,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{//车
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   6,   8,   7,  13,  14,  13,   7,   8,   6,   0,0,0,0,
				0,0,0,   6,  12,   9,  16,  33,  16,   9,  12,   6,   0,0,0,0,
				0,0,0,   6,   8,   7,  14,  16,  14,   7,   8,   6,   0,0,0,0,
				0,0,0,   6,  13,  13,  16,  16,  16,  13,  13,   6,   0,0,0,0,
				0,0,0,   8,  11,  11,  14,  15,  14,  11,  11,   8,   0,0,0,0,
				0,0,0,   8,  12,  12,  14,  15,  14,  12,  12,   8,   0,0,0,0,
				0,0,0,   4,   9,   4,  12,  14,  12,   4,   9,   4,   0,0,0,0,
				0,0,0,  -2,   8,   4,  12,  12,  12,   4,   8,  -2,   0,0,0,0,
				0,0,0,   5,   8,   6,  12,   0,  12,   6,   8,   5,   0,0,0,0,
				0,0,0,  -6,   6,   4,  12,   0,  12,   4,   6,  -6,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{//炮
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   4,   4,   0,  -5,  -6,  -5,   0,   4,   4,   0,0,0,0,
				0,0,0,   2,   2,   0,  -4,  -7,  -4,   0,   2,   2,   0,0,0,0,
				0,0,0,   1,   1,   0,  -5,  -4,  -5,   0,   1,   1,   0,0,0,0,
				0,0,0,   0,   3,   3,   2,   4,   2,   3,   3,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   4,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,  -1,   0,   3,   0,   4,   0,   3,   0,  -1,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   1,   0,   4,   3,   5,   3,   4,   0,   1,   0,0,0,0,
				0,0,0,   0,   1,   2,   2,   2,   2,   2,   1,   0,   0,0,0,0,
				0,0,0,   0,   0,   1,   3,   3,   3,   1,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			},{//兵
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,   0,   0,   0,   2,   4,   2,   0,   0,   0,   0,0,0,0,
				0,0,0,  20,  30,  50,  65,  70,  65,  50,  30,  20,   0,0,0,0,
				0,0,0,  20,  30,  45,  55,  55,  55,  45,  30,  20,   0,0,0,0,
				0,0,0,  20,  27,  30,  40,  42,  40,  30,  27,  20,   0,0,0,0,
				0,0,0,  10,  18,  22,  35,  40,  35,  22,  18,  10,   0,0,0,0,
				0,0,0,   3,   0,   4,   0,   7,   0,   4,   0,   3,   0,0,0,0,
				0,0,0,  -2,   0,-2-5,   0,   6,   0,-2-5,   0,  -2,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
			}
		};
		static double controlFactor[8]={0,2,2,2,5,3,3,3};//控制系数
//		static int basicValue[8]={0,10000,250,250,300,500,300,80};//子力价值
//		static int basicValue[8]={0,10000,175,200,425,1000,475,100};//子力价值
//		static int mobilityValue[8]={0,0,1,1,12,6,6,15};//机动性
		static int mobilityValue[8]={0,0,0,0,6,3,3,8};//机动性
		
		static int killValue[8]={0,50,10,15,20,20,20,10};//击杀分
		static int protectValue[8]={0,0,2,4,4,4,4,2};//保护分
		static int killValue2[8]={0,25,5,8,10,10,10,5};//潜在击杀分
		static int protectValue2[8]={0,0,1,2,2,2,2,1};//潜在保护分
		
		static int directPaoValue[10]={0,0,30,120,135,135,135,120,120,120};//当头炮
		static int doublePaoValue[10]={0,0,0,50,40,30,20,20,20,20};//隔山炮
		static int firstValue=25;
		
		int totalValue[2]={0};
		int basic=(typeCount[color][Shuai]-typeCount[!color][Shuai])*10000+
			lineupTable.lineupValue
			[typeCount[color][Shi]]
			[typeCount[color][Xiang]]
			[typeCount[color][Ma]]
			[typeCount[color][Ju]]
			[typeCount[color][Pao]]
			[typeCount[color][Bing]]
			[typeCount[!color][Shi]]
			[typeCount[!color][Xiang]]
			[typeCount[!color][Ma]]
			[typeCount[!color][Ju]]
			[typeCount[!color][Pao]]
			[typeCount[!color][Bing]];
		int control[2]={0};
		int mobility[2]={0};
		int killProtect[2]={0};
		int directPao[2]={0};
		int safety[2]={0};
		for(int i=0;i<16;i++){
			double f=controlFactor[IDType[i]];
			if(position[Red][i]){
				control[Red]+=int(f*controlValue[IDType[i]][position[Red][i]]);
			}
			if(position[Black][i]){
				control[Black]+=int(f*controlValue[IDType[i]][HW(15^H(position[Black][i]),W(position[Black][i]))]);
			}
		}
		

		#define KillProtect(p,c) (chess[p].color==c?protectValue[chess[p].type]:killValue[chess[p].type])
		#define KillProtect2(p,c) (chess[p].color==c?protectValue2[chess[p].type]:killValue2[chess[p].type])
		
		for(int nowColor=0;nowColor<2;nowColor++){
			int begin,end;
//			if(begin=position[nowColor][IDShuai]){
//				for(int i=0;i<4;i++)
//					if(chess[end=begin+ShuaiDelta[i]].IsLegalShuai()&&chess[end].type)
//						killProtect[nowColor]+=KillProtect(end,nowColor);
//			}
			if(begin=position[nowColor][IDShi1]){
				for(int i=0;i<4;i++)
					if(chess[end=begin+ShiDelta[i]].IsLegalShi()&&chess[end].type)
						killProtect[nowColor]+=KillProtect(end,nowColor);
			}
			if(begin=position[nowColor][IDShi2]){
				for(int i=0;i<4;i++)
					if(chess[end=begin+ShiDelta[i]].IsLegalShi()&&chess[end].type)
						killProtect[nowColor]+=KillProtect(end,nowColor);
			}
			if(begin=position[nowColor][IDXiang1]){
				for(int i=0;i<4;i++)if(chess[end=begin+XiangDelta[i]].IsLegalXiang()){
					if(chess[begin+ShiDelta[i]].type){
						if(chess[end].type)killProtect[nowColor]+=KillProtect2(end,nowColor);
					}
					else{
						if(chess[end].type){
							if(chess[end].color==nowColor)
								killProtect[nowColor]+=protectValue[chess[end].type];
							else{
								killProtect[nowColor]+=killValue[chess[end].type];
								mobility[nowColor]+=mobilityValue[Xiang];
							}
						}
						else mobility[nowColor]+=mobilityValue[Xiang];
					}
				}
			}
			if(begin=position[nowColor][IDXiang2]){
				for(int i=0;i<4;i++)if(chess[end=begin+XiangDelta[i]].IsLegalXiang()){
					if(chess[begin+ShiDelta[i]].type){
						if(chess[end].type)killProtect[nowColor]+=KillProtect2(end,nowColor);
					}
					else{
						if(chess[end].type){
							if(chess[end].color==nowColor)
								killProtect[nowColor]+=protectValue[chess[end].type];
							else{
								killProtect[nowColor]+=killValue[chess[end].type];
								mobility[nowColor]+=mobilityValue[Xiang];
							}
						}
						else mobility[nowColor]+=mobilityValue[Xiang];
					}
				}
			}
			if(begin=position[nowColor][IDMa1]){
				for(int i=0;i<4;i++){
					if(chess[begin+ShuaiDelta[i]].type){
						if(chess[end=begin+MaDelta[i][0]].type)
							killProtect[nowColor]+=KillProtect2(end,nowColor);
						if(chess[end=begin+MaDelta[i][1]].type)
							killProtect[nowColor]+=KillProtect2(end,nowColor);
					}
					else{
						if(chess[end=begin+MaDelta[i][0]].type){
							if(chess[end].color==nowColor)
								killProtect[nowColor]+=protectValue[chess[end].type];
							else{
								killProtect[nowColor]+=killValue[chess[end].type];
								mobility[nowColor]+=mobilityValue[Ma];
							}
						}
						else if(chess[end].IsLegal())
							mobility[nowColor]+=mobilityValue[Ma];
						if(chess[end=begin+MaDelta[i][1]].type){
							if(chess[end].color==nowColor)
								killProtect[nowColor]+=protectValue[chess[end].type];
							else{
								killProtect[nowColor]+=killValue[chess[end].type];
								mobility[nowColor]+=mobilityValue[Ma];
							}
						}
						else if(chess[end].IsLegal())
							mobility[nowColor]+=mobilityValue[Ma];
					}
				}
			}
			if(begin=position[nowColor][IDMa2]){
				for(int i=0;i<4;i++){
					if(chess[begin+ShuaiDelta[i]].type){
						if(chess[end=begin+MaDelta[i][0]].type)
							killProtect[nowColor]+=KillProtect2(end,nowColor);
						if(chess[end=begin+MaDelta[i][1]].type)
							killProtect[nowColor]+=KillProtect2(end,nowColor);
					}
					else{
						if(chess[end=begin+MaDelta[i][0]].type){
							if(chess[end].color==nowColor)
								killProtect[nowColor]+=protectValue[chess[end].type];
							else{
								killProtect[nowColor]+=killValue[chess[end].type];
								mobility[nowColor]+=mobilityValue[Ma];
							}
						}
						else if(chess[end].IsLegal())
							mobility[nowColor]+=mobilityValue[Ma];
						if(chess[end=begin+MaDelta[i][1]].type){
							if(chess[end].color==nowColor)
								killProtect[nowColor]+=protectValue[chess[end].type];
							else{
								killProtect[nowColor]+=killValue[chess[end].type];
								mobility[nowColor]+=mobilityValue[Ma];
							}
						}
						else if(chess[end].IsLegal())
							mobility[nowColor]+=mobilityValue[Ma];
					}
				}
			}
			if(begin=position[nowColor][IDJu1]){
				int h=H(begin);
				int w=W(begin);
				mobility[nowColor]+=mobilityValue[Ju]*lineSituation.juMoveCountH[w][lineH[h]];
				mobility[nowColor]+=mobilityValue[Ju]*lineSituation.juMoveCountW[h][lineW[w]];
				if(chess[end=HW(h,H(lineSituation.juKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(h,W(lineSituation.juKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(H(lineSituation.juKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(W(lineSituation.juKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
			}
			if(begin=position[nowColor][IDJu2]){
				int h=H(begin);
				int w=W(begin);
				mobility[nowColor]+=mobilityValue[Ju]*lineSituation.juMoveCountH[w][lineH[h]];
				mobility[nowColor]+=mobilityValue[Ju]*lineSituation.juMoveCountW[h][lineW[w]];
				if(chess[end=HW(h,H(lineSituation.juKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(h,W(lineSituation.juKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(H(lineSituation.juKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(W(lineSituation.juKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
			}
			if(begin=position[nowColor][IDPao1]){
				int h=H(begin);
				int w=W(begin);
				mobility[nowColor]+=mobilityValue[Pao]*lineSituation.paoMoveCountH[w][lineH[h]];
				mobility[nowColor]+=mobilityValue[Pao]*lineSituation.paoMoveCountW[h][lineW[w]];
				if(chess[end=HW(h,H(lineSituation.paoKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(h,W(lineSituation.paoKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(H(lineSituation.paoKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(W(lineSituation.paoKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				end=position[!nowColor][IDShuai];
				int h0=H(end);
				int w0=W(end);
				if(h==h0&&(H(lineSituation.juKillH[w][lineH[h]])==w0||W(lineSituation.juKillH[w][lineH[h]])==w0)){
					directPao[nowColor]+=int(0.05*(
						(end==55||end==199?3:0)+
						(h==3||h==12?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*directPaoValue[abs(w0-w)]);
				}
				if(w==w0&&(H(lineSituation.juKillW[h][lineW[w]])==h0||W(lineSituation.juKillW[h][lineW[w]])==h0)){
					directPao[nowColor]+=int(0.1*(
						(end==55||end==199?3:0)+
						(w==7?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*directPaoValue[abs(h0-h)]);
				}
				if(h==h0&&(H(lineSituation.pao2KillH[w][lineH[h]])==w0||W(lineSituation.pao2KillH[w][lineH[h]])==w0)){
					directPao[nowColor]+=int(0.05*(
						(end==55||end==199?3:0)+
						(h==3||h==12?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*doublePaoValue[abs(w0-w)]);
				}
				if(w==w0&&(H(lineSituation.pao2KillW[h][lineW[w]])==h0||W(lineSituation.pao2KillW[h][lineW[w]])==h0)){
					directPao[nowColor]+=int(0.1*(
						(end==55||end==199?3:0)+
						(w==7?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*doublePaoValue[abs(h0-h)]);
				}
			}
			if(begin=position[nowColor][IDPao2]){
				int h=H(begin);
				int w=W(begin);
				mobility[nowColor]+=mobilityValue[Pao]*lineSituation.paoMoveCountH[w][lineH[h]];
				mobility[nowColor]+=mobilityValue[Pao]*lineSituation.paoMoveCountW[h][lineW[w]];
				if(chess[end=HW(h,H(lineSituation.paoKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(h,W(lineSituation.paoKillH[w][lineH[h]]))].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(H(lineSituation.paoKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				if(chess[end=HW(W(lineSituation.paoKillW[h][lineW[w]]),w)].type)
					killProtect[nowColor]+=KillProtect(end,nowColor);
				end=position[!nowColor][IDShuai];
				int h0=H(end);
				int w0=W(end);
				if(h==h0&&(H(lineSituation.juKillH[w][lineH[h]])==w0||W(lineSituation.juKillH[w][lineH[h]])==w0)){
					directPao[nowColor]+=int(0.05*(
						(end==55||end==199?3:0)+
						(h==3||h==12?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*directPaoValue[abs(w0-w)]);
				}
				if(w==w0&&(H(lineSituation.juKillW[h][lineW[w]])==h0||W(lineSituation.juKillW[h][lineW[w]])==h0)){
					directPao[nowColor]+=int(0.1*(
						(end==55||end==199?3:0)+
						(w==7?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*directPaoValue[abs(h0-h)]);
				}
				if(h==h0&&(H(lineSituation.pao2KillH[w][lineH[h]])==w0||W(lineSituation.pao2KillH[w][lineH[h]])==w0)){
					directPao[nowColor]+=int(0.05*(
						(end==55||end==199?3:0)+
						(h==3||h==12?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*doublePaoValue[abs(w0-w)]);
				}
				if(w==w0&&(H(lineSituation.pao2KillW[h][lineW[w]])==h0||W(lineSituation.pao2KillW[h][lineW[w]])==h0)){
					directPao[nowColor]+=int(0.1*(
						(end==55||end==199?3:0)+
						(w==7?1:0)+
						(position[!nowColor][IDShi1]?2:0)+
						(position[!nowColor][IDShi2]?2:0)+
						(position[!nowColor][IDXiang1]?1:0)+
						(position[!nowColor][IDXiang2]?1:0)
					)*doublePaoValue[abs(h0-h)]);
				}
			}
			if(begin=position[nowColor][IDBing1]){
				if(end=bingNextPosition[0][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[1][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[2][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
			}
			if(begin=position[nowColor][IDBing2]){
				if(end=bingNextPosition[0][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[1][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[2][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
			}
			if(begin=position[nowColor][IDBing3]){
				if(end=bingNextPosition[0][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[1][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[2][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
			}
			if(begin=position[nowColor][IDBing4]){
				if(end=bingNextPosition[0][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[1][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[2][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
			}
			if(begin=position[nowColor][IDBing5]){
				if(end=bingNextPosition[0][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[1][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
				if(end=bingNextPosition[2][nowColor][begin]){
					if(chess[end].type){
						if(chess[end].color==nowColor)
							killProtect[nowColor]+=protectValue[chess[end].type];
						else{
							killProtect[nowColor]+=killValue[chess[end].type];
							mobility[nowColor]+=mobilityValue[Bing];
						}
					}
					else mobility[nowColor]+=mobilityValue[Bing];
				}
			}
		}
		
		for(int i=0;i<2;i++)totalValue[i]=control[i]+mobility[i]+killProtect[i]+directPao[i]+safety[i];
		
		
		if(debug){
			printf("|-----------------------|\n");
			printf("| color        = %s |\n",color==Red?string("Red   ").data():string("Black ").data());
			printf("| total        = %-6d |\n",basic+totalValue[color]-totalValue[!color]+firstValue);
			printf("| basic        = %-6d |\n",basic);
			printf("| control      = %-6d |\n",control[color]-control[!color]);
			printf("| mobility     = %-6d |\n",mobility[color]-mobility[!color]);
			printf("| kill protect = %-6d |\n",killProtect[color]-killProtect[!color]);
			printf("| direct pao   = %-6d |\n",directPao[color]-directPao[!color]);
			printf("| safety       = %-6d |\n",safety[color]-safety[!color]);
			printf("| first        = %-6d |\n",firstValue);
			printf("|-----------------------|\n");
		}
		return basic+totalValue[color]-totalValue[!color]+firstValue;
	}
};

ChessBoard chessBoard;
ChessBoard board;
Move computerMove;
long long nodeCount,leafCount,hitCount,quiesCount,evaluateCount;

int Quies(int alpha,int beta,int distance){
	quiesCount++;
	if(-WinValue+distance>=beta)return beta;//无害裁剪
	int value=board.Evaluate1();
	if(value>=beta){
		return beta;
	}
	if(value>alpha){
		alpha=value;
	}
	vector<Move>moves=board.GenerateEatMoves(board.color);
	static int mvvlva[256][256];
	for(Move &move:moves)mvvlva[move.begin][move.end]=board.MVVLVA(move);
	sort(moves.begin(),moves.end(),[&](Move &a,Move &b){
		if(mvvlva[a.begin][a.end]!=mvvlva[b.begin][b.end])
			return mvvlva[a.begin][a.end]>mvvlva[b.begin][b.end];
		static int simpleValue[8]={0,5,1,1,3,4,3,2};
		return simpleValue[a.kill]>simpleValue[b.kill];
	});
	while(moves.size()&&mvvlva[moves.back().begin][moves.back().end]==-1)moves.pop_back();
	for(auto &move:moves){
		board.ExecuteMove(move);
		if(board.CheckKill(!board.color)){
			board.RescindMove(move);
		}
		else{
			value=-Quies(-beta,-alpha,distance+1);
			board.RescindMove(move);
			if(value>=beta){
				return beta;
			}
			if(value>alpha){
				alpha=value;
			}
		}
	}
	return alpha;
}

int Evaluate(int distance){
	evaluateCount++;
//	return board.Evaluate1();
	return Quies(-Infinite,Infinite,distance);
}

int AlphaBetaNonRoot(int alpha,int beta,int depth,int distance,bool canNullMove,deque<Move>&moveTrace){
	nodeCount++;
	if(-WinValue+distance>=beta)return beta;//无害裁剪
	Move bestMove={0};
	Move imagineMove={0};
	TTItem *item=transpositionTable.Query(board.key);
	if(item!=NULL){
		if(item->depth>=depth){
			int minValue=item->minValue;
			int maxValue=item->maxValue;
			if(minValue==maxValue){
				hitCount++;
				if(minValue>=WinCheck)minValue-=distance;
				else if(minValue<=-WinCheck)minValue+=distance;
				return minValue;
			}
			if(minValue>=beta){
				hitCount++;
				return beta;
			}
			if(maxValue<=alpha){
				hitCount++;
				return alpha;
			}
		}
		imagineMove=(Move){item->begin,item->end,board.chess[item->end].type};
	}
	if(depth==0){
		leafCount++;
		int value=Evaluate(distance);
		transpositionTable.Insert(board.key,value,value,depth,distance,bestMove);//更新置换表
		return value;
	}
	if(canNullMove&&depth>=3&&!board.CheckKill(board.color)){//空着
		board.ExecuteNullMove();
		deque<Move>trace;
		int value=-AlphaBetaNonRoot(-beta,-beta+1,depth-1-2,distance+1,false,trace);
		board.RescindNullMove();
		if(value>=beta){
			if(board.NullMoveSafe())return beta;
			if(AlphaBetaNonRoot(beta-1,beta,depth-2,distance,false,trace)>=beta)return beta;//空着检验
		}
	}
	if(depth>=3&&imagineMove.begin==0&&beta-alpha>1){//内部迭代加深
		deque<Move>trace;
		int value=AlphaBetaNonRoot(alpha,beta,depth/2,distance,false,trace);
		if(value<=alpha){
			value=AlphaBetaNonRoot(-Infinite,beta,depth/2,distance,false,trace);
		}
		item=transpositionTable.Query(board.key);
		if(item!=NULL)imagineMove=(Move){item->begin,item->end,board.chess[item->end].type};
	}
	
	vector<Move>moves;
	vector<Move>eatMoves;
	int state=0;
	int index=0;
	Move killerMove1={0};
	Move killerMove2={0};
	bool canMove=false;
	bool isPV=false;
	
	function<Move()>NextMove=[&](){
		if(state==0){
			state=1;
			if(imagineMove.begin)return imagineMove;
		}
		if(state==1){
			state=2;
			index=0;
			eatMoves=board.GenerateEatMoves(board.color);
			static int mvvlva[256][256];
			for(Move &move:eatMoves)mvvlva[move.begin][move.end]=board.MVVLVA(move);
			sort(eatMoves.begin(),eatMoves.end(),[&](Move &a,Move &b){
				return mvvlva[a.begin][a.end]>mvvlva[b.begin][b.end];
			});//根据MVV(LVA)值排序
			while(eatMoves.size()&&mvvlva[eatMoves.back().begin][eatMoves.back().end]<=0)eatMoves.pop_back();
		}
		if(state==2){
			while(index<eatMoves.size()){
				Move &move=eatMoves[index++];
				if(move.begin==imagineMove.begin&&move.end==imagineMove.end)continue;
				if(move.begin==killerMove1.begin&&move.end==killerMove1.end)continue;
				if(move.begin==killerMove2.begin&&move.end==killerMove2.end)continue;
				return move;
			}
			state=3;
		}
		if(state==3){
			state=4;
			killerMove1=killerTable.killer1[distance];
			killerMove1.kill=board.chess[killerMove1.end].type;
			if(board.chess[killerMove1.begin].color==board.color)
				if(killerMove1.begin!=imagineMove.begin||killerMove1.end!=imagineMove.end)
					if(board.CheckMove(killerMove1.begin,killerMove1.end))return killerMove1;
		}
		if(state==4){
			state=5;
			killerMove2=killerTable.killer2[distance];
			killerMove2.kill=board.chess[killerMove2.end].type;
			if(board.chess[killerMove2.begin].color==board.color)
				if(killerMove2.begin!=imagineMove.begin||killerMove2.end!=imagineMove.end)
					if(board.CheckMove(killerMove2.begin,killerMove2.end))return killerMove2;
		}
		if(state==5){
			state=6;
			index=0;
			moves=board.GenerateAllMoves(board.color);
			static bool moved[256][256];
			for(Move &move:moves)moved[move.begin][move.end]=0;
			for(Move &move:eatMoves)moved[move.begin][move.end]=1;
			moved[imagineMove.begin][imagineMove.end]=1;
			moved[killerMove1.begin][killerMove1.end]=1;
			moved[killerMove2.begin][killerMove2.end]=1;
			for(int i=0;i<moves.size();){
				if(moved[moves[i].begin][moves[i].end]){
					swap(moves[i],moves.back());
					moves.pop_back();
				}
				else i++;
			}
			sort(moves.begin(),moves.end(),[&](Move &a,Move &b){
				return historyTable.value[a.begin][a.end]>historyTable.value[b.begin][b.end];
			});//根据历史表排序
		}
		if(state==6){
			while(index<moves.size())return moves[index++];
		}
		return Move{0};
	};
	
	for(Move move=NextMove();move.begin;move=NextMove()){
		board.ExecuteMove(move);
		if(board.CheckKill(!board.color)){
			board.RescindMove(move);
		}
		else{
			int value;
			deque<Move>trace;
			int repeatCount=board.repeatTable.Query(board.key);
			if(repeatCount>1){
				int repeatState=1;
				if(board.CheckKill(board.color))repeatState+=2;
				board.RescindMove(move);
				if(board.CheckKill(board.color))repeatState+=4;
				if(repeatState==3)value=-BanValue;
				else if(repeatState==5)value=BanValue;
				else value=DrawValue;
			}
			else{
				int nextDepth=depth-1;
				if(board.CheckKill(board.color))nextDepth=depth;
				if(canMove){
					value=-AlphaBetaNonRoot(-alpha-1,-alpha,nextDepth,distance+1,canNullMove,trace);
					if(value>alpha&&value<beta){
						value=-AlphaBetaNonRoot(-beta,-alpha,nextDepth,distance+1,canNullMove,trace);
					}
				}
				else{
					value=-AlphaBetaNonRoot(-beta,-alpha,nextDepth,distance+1,canNullMove,trace);
					canMove=true;
				}
				board.RescindMove(move);
			}
			if(value>=beta){
				historyTable.Update(move,depth);//更新历史表
				killerTable.Insert(move,distance);//更新杀手表
				if(value!=BanValue)
					transpositionTable.Insert(board.key,value,Infinite,depth,distance,move);//更新置换表
				return beta;
			}
			if(value>alpha){
				moveTrace=trace;
				isPV=true;
				bestMove=move;
				alpha=value;
			}
		}
	}
	if(!canMove){
		return -WinValue+distance;
	}
	historyTable.Update(bestMove,depth);//更新历史表
	if(alpha!=BanValue){
		if(isPV)transpositionTable.Insert(board.key,alpha,alpha,depth,distance,bestMove);//更新置换表
		else transpositionTable.Insert(board.key,-Infinite,alpha,depth,distance,bestMove);//更新置换表
	}
	moveTrace.push_front(bestMove);
	return alpha;
}

int AlphaBetaRoot(int alpha,int beta,int depth,deque<Move>&moveTrace,vector<Move>&moves){
	nodeCount++;
	Move bestMove={0};
	bool canMove=false;
	bool isPV=false;
	
	if(moves.size()==0){
		moves=board.GenerateAllMoves(board.color);
		sort(moves.begin(),moves.end(),[&](Move &a,Move &b){
			if(board.MVVLVA(a)>0||board.MVVLVA(b)>0)
				return board.MVVLVA(a)>board.MVVLVA(b);
			return historyTable.value[a.begin][a.end]>historyTable.value[b.begin][b.end];
		});//根据历史表排序
	}
	static int moveValue[256][256];
	for(int i=0;i<moves.size();i++){
		Move &move=moves[i];
		moveValue[move.begin][move.end]=-Infinite-i;
	}
	for(Move &move:moves){
		Timer timer("depth "+to_string(depth));
		if(depth<7)timer.Shutdown();
		board.ExecuteMove(move);
		if(board.CheckKill(!board.color)){
			board.RescindMove(move);
		}
		else{
			int value;
			deque<Move>trace;
			int repeatCount=board.repeatTable.Query(board.key);
			if(repeatCount>1){
				int repeatState=1;
				if(board.CheckKill(board.color))repeatState+=2;
				board.RescindMove(move);
				if(board.CheckKill(board.color))repeatState+=4;
				if(repeatState==3)value=-BanValue;
				else if(repeatState==5&&repeatCount>=3)value=BanValue;
				else value=DrawValue;
			}
			else{
				int nextDepth=depth-1;
				if(board.CheckKill(board.color))nextDepth=depth;
				if(canMove){
					value=-AlphaBetaNonRoot(-alpha-1,-alpha,nextDepth,1,true,trace);
					if(value>alpha&&value<beta){
						value=-AlphaBetaNonRoot(-beta,-alpha,nextDepth,1,true,trace);
					}
				}
				else{
					value=-AlphaBetaNonRoot(-beta,-alpha,nextDepth,1,true,trace);
					canMove=true;
				}
				board.RescindMove(move);
			}
			if(value>alpha){
				moveValue[move.begin][move.end]=value;
			}
			if(value>=beta){
				historyTable.Update(bestMove,depth);//更新历史表
				if(value!=BanValue)
					transpositionTable.Insert(board.key,value,Infinite,depth,0,move);//更新置换表
				sort(moves.begin(),moves.end(),[&](Move &a,Move &b){
					if(moveValue[a.begin][a.end]!=moveValue[b.begin][b.end])
						return moveValue[a.begin][a.end]>moveValue[b.begin][b.end];
					if(board.MVVLVA(a)>0||board.MVVLVA(b)>0)
						return board.MVVLVA(a)>board.MVVLVA(b);
					return historyTable.value[a.begin][a.end]>historyTable.value[b.begin][b.end];
				});//更新根节点着法顺序
				return beta;
			}
			if(value>alpha){
				moveTrace=trace;
				isPV=true;
				bestMove=move;
				computerMove=move;
				alpha=value;
			}
		}
	}
	if(!canMove){
		return -WinValue;
	}
	historyTable.Update(bestMove,depth);//更新历史表
	if(alpha!=BanValue){
		if(isPV)transpositionTable.Insert(board.key,alpha,alpha,depth,0,bestMove);//更新置换表
		else transpositionTable.Insert(board.key,-Infinite,alpha,depth,0,bestMove);//更新置换表
	}
	moveTrace.push_front(bestMove);
	sort(moves.begin(),moves.end(),[&](Move &a,Move &b){
		if(moveValue[a.begin][a.end]!=moveValue[b.begin][b.end])
			return moveValue[a.begin][a.end]>moveValue[b.begin][b.end];
		return historyTable.value[a.begin][a.end]>historyTable.value[b.begin][b.end];
	});//更新根节点着法顺序
	return alpha;
}

Move CalculationComputerMove(int level,int step){
	board=chessBoard;
	historyTable.Attenuate();//每次搜索前衰减历史表
	killerTable.Init();//每次搜索前清空杀手表
	nodeCount=0;
	leafCount=0;
	hitCount=0;
	computerMove={0};
	deque<Move>moveTrace;
	
	int t1=clock();
	printf("------------------------------------------\n");
	printf("start calc...\n");

	int value=0;
	vector<Move>moves;
	for(int depth=(level-1)%step+1;depth<=level;depth+=step){
		int window=60;
//		window=Infinite; 
		int alpha=value-window;
		int beta=value+window;
		while(1){
			value=AlphaBetaRoot(alpha,beta,depth,moveTrace,moves);
			printf("depth=%d value=%d [%d %d]\n",depth,value,alpha,beta);
			if(value>=WinCheck||value<=-WinCheck)break;
			else if(value<=alpha)alpha=-Infinite;
			else if(value>=beta)beta=Infinite;
			else break;
		}
		if(value>=WinCheck||value<=-WinCheck)break;
	}
	
	int t2=clock();
	double t=1.0*(t2-t1)/CLOCKS_PER_SEC;
	printf("end calc:\n");
	printf("time = %.3lfs   value = %d\n",t,value);
	printf("node = %lld   leaf = %lld   k = %.0lf\n",nodeCount,leafCount,1e8*t/nodeCount);
	printf("hit = %lld   rate = %.2lf\%\n",hitCount,100.0*hitCount/nodeCount);
	printf("quies = %.2lf\n",1.0*quiesCount/evaluateCount);
	static double a=0,b=0;
	a+=1.0*quiesCount/evaluateCount,b+=1;
	printf("quies == %.2lf\n",a/b);
//	for(auto &move:moveTrace)printf("%x -> %x\n",move.begin,move.end);
	printf("------------------------------------------\n");
	assert(computerMove.begin&&computerMove.end);
	return computerMove;
}

void ApiInit(){
	lineSituation.Init();
	for(int i=4;i<=7;i++)for(int j=3;j<=11;j++)bingNextPosition[0][Red][HW(i,j)]=HW(i-1,j);
	for(int i=8;i<=9;i++)for(int j=3;j<=11;j+=2)bingNextPosition[0][Red][HW(i,j)]=HW(i-1,j);
	for(int i=3;i<=7;i++)for(int j=4;j<=11;j++)bingNextPosition[1][Red][HW(i,j)]=HW(i,j-1);
	for(int i=3;i<=7;i++)for(int j=3;j<=10;j++)bingNextPosition[2][Red][HW(i,j)]=HW(i,j+1);
	for(int i=8;i<=11;i++)for(int j=3;j<=11;j++)bingNextPosition[0][Black][HW(i,j)]=HW(i+1,j);
	for(int i=6;i<=7;i++)for(int j=3;j<=11;j+=2)bingNextPosition[0][Black][HW(i,j)]=HW(i+1,j);
	for(int i=8;i<=12;i++)for(int j=4;j<=11;j++)bingNextPosition[1][Black][HW(i,j)]=HW(i,j-1);
	for(int i=8;i<=12;i++)for(int j=3;j<=10;j++)bingNextPosition[2][Black][HW(i,j)]=HW(i,j+1);
	lineupTable.Init();
}
void ApiNewGame(int mode,int level,int step){
	if(mode==ModePlayerPlayer){
		chessBoard.roleRed=RolePlayer;
		chessBoard.roleBlack=RolePlayer;
		chessBoard.flip=false;
	}
	else if(mode==ModePlayerComputer){
		chessBoard.roleRed=RolePlayer;
		chessBoard.roleBlack=RoleComputer;
		chessBoard.flip=false;
	}
	else if(mode==ModeComputerPlayer){
		chessBoard.roleRed=RoleComputer;
		chessBoard.roleBlack=RolePlayer;
		chessBoard.flip=true;
	}
	else if(mode==ModeComputerComputer){
		chessBoard.roleRed=RoleComputer;
		chessBoard.roleBlack=RoleComputer;
		chessBoard.flip=false;
	}
	else if(mode==ModeFirst){
		chessBoard.roleRed=RoleComputer;
		chessBoard.roleBlack=RolePlayer;
		chessBoard.flip=false;
	}
	else if(mode==ModeBack){
		chessBoard.roleRed=RolePlayer;
		chessBoard.roleBlack=RoleComputer;
		chessBoard.flip=true;
	}
	chessBoard.computerLevel=level;
	chessBoard.computerStep=step;
	chessBoard.hintLevel=level;
	chessBoard.hintStep=step;
	chessBoard.Init();
	chessBoard.AddChess(HW(12,3),Red,Ju);
	chessBoard.AddChess(HW(12,4),Red,Ma);
	chessBoard.AddChess(HW(12,5),Red,Xiang);
	chessBoard.AddChess(HW(12,6),Red,Shi);
	chessBoard.AddChess(HW(12,7),Red,Shuai);
	chessBoard.AddChess(HW(12,8),Red,Shi);
	chessBoard.AddChess(HW(12,9),Red,Xiang);
	chessBoard.AddChess(HW(12,10),Red,Ma);
	chessBoard.AddChess(HW(12,11),Red,Ju);
	chessBoard.AddChess(HW(10,4),Red,Pao);
	chessBoard.AddChess(HW(10,10),Red,Pao);
	chessBoard.AddChess(HW(9,3),Red,Bing);
	chessBoard.AddChess(HW(9,5),Red,Bing);
	chessBoard.AddChess(HW(9,7),Red,Bing);
	chessBoard.AddChess(HW(9,9),Red,Bing);
	chessBoard.AddChess(HW(9,11),Red,Bing);
	chessBoard.AddChess(HW(3,3),Black,Ju);
	chessBoard.AddChess(HW(3,4),Black,Ma);
	chessBoard.AddChess(HW(3,5),Black,Xiang);
	chessBoard.AddChess(HW(3,6),Black,Shi);
	chessBoard.AddChess(HW(3,7),Black,Shuai);
	chessBoard.AddChess(HW(3,8),Black,Shi);
	chessBoard.AddChess(HW(3,9),Black,Xiang);
	chessBoard.AddChess(HW(3,10),Black,Ma);
	chessBoard.AddChess(HW(3,11),Black,Ju);
	chessBoard.AddChess(HW(5,4),Black,Pao);
	chessBoard.AddChess(HW(5,10),Black,Pao);
	chessBoard.AddChess(HW(6,3),Black,Bing);
	chessBoard.AddChess(HW(6,5),Black,Bing);
	chessBoard.AddChess(HW(6,7),Black,Bing);
	chessBoard.AddChess(HW(6,9),Black,Bing);
	chessBoard.AddChess(HW(6,11),Black,Bing);
	
	chessBoard.repeatTable.Add(chessBoard.key);
	
	printf("|----------------------|\n");
	printf("| level = %d   step = %d |\n",chessBoard.computerLevel,chessBoard.computerStep);
	printf("|----------------------|\n");
	
}
void ApiPlace(ChessBoard board){
	chessBoard.roleRed=board.roleRed;
	chessBoard.roleBlack=board.roleBlack;
	chessBoard.flip=board.flip;
	chessBoard.computerLevel=board.computerLevel;
	chessBoard.computerStep=board.computerStep;
	chessBoard.hintLevel=board.hintLevel;
	chessBoard.hintStep=board.hintStep;
	chessBoard.Init(board.color);
	
	for(int i=0;i<256;i++)
		if(board.chess[i].type)
			chessBoard.AddChess(i,board.chess[i].color,board.chess[i].type);
	
	chessBoard.repeatTable.Add(chessBoard.key);
	
	printf("|----------------------|\n");
	printf("| level = %d   step = %d |\n",chessBoard.computerLevel,chessBoard.computerStep);
	printf("|----------------------|\n");
}
void ApiSet(int roleRed,int roleBlack,int computerLevel,int computerStep,int hintLevel,int hintStep){
	chessBoard.roleRed=roleRed;
	chessBoard.roleBlack=roleBlack;
	chessBoard.computerLevel=computerLevel;
	chessBoard.computerStep=computerStep;
	chessBoard.hintLevel=hintLevel;
	chessBoard.hintStep=hintStep;
}
void ApiPlayerMove(int begin,int end){
	Move move=(Move){begin,end,chessBoard.chess[end].type};
	chessBoard.ExecuteMove(move);
	chessBoard.moveStack.push_back(move);
}
void ApiComputerMove(){
	Move move=CalculationComputerMove(chessBoard.computerLevel,chessBoard.computerStep);
	if(move.begin==0&&move.end==0)return;
	chessBoard.ExecuteMove(move);
	chessBoard.moveStack.push_back(move);
	chessBoard.Evaluate1(1);//debug
}
void ApiHint(){
	Move move=CalculationComputerMove(chessBoard.hintLevel,chessBoard.hintStep);
	if(move.begin==0&&move.end==0)return;
	chessBoard.ExecuteMove(move);
	chessBoard.moveStack.push_back(move);
	chessBoard.Evaluate1(1);//debug
}
void ApiRepent(){
	int count=0;
	if(chessBoard.roleRed==RoleComputer&&chessBoard.roleBlack==RoleComputer)count=1;
	else if(chessBoard.color==Red&&chessBoard.roleBlack==RolePlayer)count=1;
	else if(chessBoard.color==Black&&chessBoard.roleRed==RolePlayer)count=1;
	else count=2;
	while(count>0&&chessBoard.moveStack.size()>0){
		chessBoard.RescindMove(chessBoard.moveStack.back());
		chessBoard.moveStack.pop_back();
		count--;
	}
}
bool ApiCheckLose(){
	return chessBoard.CheckLose();
}
bool ApiCanMove(int a,int b){
	int canMove=chessBoard.CheckMove(a,b);
	if(canMove){
		Move move=(Move){a,b,chessBoard.chess[b].type};
		chessBoard.ExecuteMove(move);
		if(chessBoard.CheckKill(!chessBoard.color))canMove=false;
		chessBoard.RescindMove(move);
	}
	return canMove;
}



/*----------------------------------------------------------------------------------------------*/



bool CheckSame(int x1,int y1,int x2,int y2){
	return x1==x2&&y1==y2;
}
int CalculateIdx(int pos,int flip){
	if(flip)return 11-(pos&15);
	return (pos&15)-3;
}
int CalculateIdy(int pos,int flip){
	if(flip)return 12-(pos>>4);
	return (pos>>4)-3;
}
int CalculatePosition(int x,int y,int flip){
	if(flip)return (12-y)*16+(11-x);
	return (3+y)*16+x+3;
}

ChessBoard chessBoardTemp;
LOGFONTA standardFont={0};
LOGFONTA chessFont={0};
struct Area{
	int x1,y1,x2,y2;
	Area(){}
	Area(int a1,int b1,int a2,int b2){
		x1=a1,y1=b1,x2=a2,y2=b2;
	}
	void Bar(){
		bar(x1,y1,x2,y2);
	}
	void Bar(color_t color){
		setfillcolor(color);
		Bar();
	}
	bool InArea(mouse_msg msg){
		int x=msg.x,y=msg.y;
		return x>=x1&&x<x2&&y>=y1&&y<y2;
	}
	void PrintCenter(string str){
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		xyprintf((x1+x2)>>1,(y1+y2)>>1,"%s",str.data());
	}
	void PrintCenter(string str,color_t color){
		setcolor(color);
		PrintCenter(str);
	}
};
Area areaBoard,areaMenu[9],areaMenu2;
Area areaNewGame[9];
Area areaSet[9];
Area areaPutMenu[9],areaPutMenu2[9];

struct ScreenChess{
	int color;
	int type;
	ScreenChess(){
		color=type=0;
	}
	ScreenChess(int a,int b){
		color=a;
		type=b;
	}
	void Paint(int x,int y){
		if(type==0)return;
		if(color==Red)setfillcolor(EGERGB(218,151,99));
		else setfillcolor(EGERGB(218,156,77));
		fillellipse(x,y,27,27);//画圆
		if(color==Red)setcolor(RED);
		else setcolor(BLACK);
		setfont(&chessFont);
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		setbkmode(TRANSPARENT);
		xyprintf(x,y,ChessName[color][type]);
	}
};
struct ScreenChessMove{
	int x1,y1,x2,y2;
	bool illegal;
	void clear(){
		x1=y1=x2=y2=-1;
		illegal=1;
	}
	ScreenChessMove(){
		clear();
	}
	ScreenChessMove(int x1,int y1,int x2,int y2){
		this->x1=x1;
		this->y1=y1;
		this->x2=x2;
		this->y2=y2;
		illegal=0;
	}
	bool Exist(){
		return !illegal;
	}
	bool IsBegin(int x,int y){
		return x==x1&&y==y1;
	}
	bool IsEnd(int x,int y){
		return x==x2&&y==y2;
	}
};
struct ScreenChessBoard{
	int basicScreenX;
	int basicScreenY;
	int flip;
	int color;
	int roleRed;
	int roleBlack;
	int computerLevel;
	int computerStep;
	int hintLevel;
	int hintStep;
	ScreenChess chess[9][10];
	ScreenChessMove lastMove;
	void Scan(ChessBoard &board){
		flip=board.flip;
		color=board.color;
		roleRed=board.roleRed;
		roleBlack=board.roleBlack;
		computerLevel=board.computerLevel;
		computerStep=board.computerStep;
		hintLevel=board.hintLevel;
		hintStep=board.hintStep;
		for(int i=0;i<9;i++)for(int j=0;j<10;j++){
			chess[i][j].type=board.chess[CalculatePosition(i,j,flip)].type;
			chess[i][j].color=board.chess[CalculatePosition(i,j,flip)].color;
		}
		if(!board.moveStack.empty()){
			Move t=board.moveStack.back();
			int x1=CalculateIdx(t.begin,flip);
			int y1=CalculateIdy(t.begin,flip);
			int x2=CalculateIdx(t.end,flip);
			int y2=CalculateIdy(t.end,flip);
			lastMove=ScreenChessMove(x1,y1,x2,y2);
		}
		else lastMove.clear();
	}
	void Print(ChessBoard &board){
		board.flip=flip;
		board.color=color;
		board.roleRed=roleRed;
		board.roleBlack=roleBlack;
		board.computerLevel=computerLevel;
		board.computerStep=computerStep;
		board.hintLevel=hintLevel;
		board.hintStep=hintStep;
		
		memset(board.chess,0,sizeof(board.chess));
		for(int i=0;i<9;i++)for(int j=0;j<10;j++)if(chess[i][j].type){
			int id=CalculatePosition(i,j,flip);
			board.chess[id].color=chess[i][j].color;
			board.chess[id].type=chess[i][j].type;
		}
	}
	bool Legal(int x,int y,int color,int type){
		if(type==Shuai){
			if(!flip&&color==Red||flip&&color==Black)
				return x>=3&&x<=5&&y>=7&&y<=9;
			else return x>=3&&x<=5&&y>=0&&y<=2;
		}
		if(type==Shi){
			if(!flip&&color==Red||flip&&color==Black)
				return x>=3&&x<=5&&y>=7&&y<=9&&(x+y)%2==0;
			else return x>=3&&x<=5&&y>=0&&y<=2&&(x+y)%2==1;
		}
		if(type==Xiang){
			if(!flip&&color==Red||flip&&color==Black)
				return y>4&&x%2==0&&y%2&&(x+y)%4==3;
			else return y<=4&&x%2==0&&y%2==0&&(x+y)%4==2;
		}
		if(type==Bing){
			if(!flip&&color==Red||flip&&color==Black)
				return y<=4||x%2==0&&y<=6;
			else return y>=5||x%2==0&&y>=3;
		}
		return true;
	}
	int GetScreenX(int idx){return basicScreenX+GRID_LEN*idx+GRID_LEN/2;}
	int GetScreenY(int idy){return basicScreenY+GRID_LEN*idy+GRID_LEN/2;}
	int GetIdx(mouse_msg msg){
		int x=msg.x;
		for(int i=0;i<9;i++)
			if(x>=basicScreenX+GRID_LEN*i&&x<basicScreenX+GRID_LEN*(i+1))return i;
		return -1;
	}
	int GetIdy(mouse_msg msg){
		int y=msg.y;
		for(int j=0;j<10;j++)
			if(y>=basicScreenY+GRID_LEN*j&&y<basicScreenY+GRID_LEN*(j+1))return j;
		return -1;
	}
	void PaintGrid(int idx,int idy){
		int x=GetScreenX(idx);
		int y=GetScreenY(idy);
		int mid=GRID_LEN/2;
		setfillcolor(BOARD_COLOR);
		bar(x-mid,y-mid,x+mid,y+mid);
		setcolor(BLACK);
		if(idx!=0)line(x-mid,y,x+1,y);
		if(idx!=8)line(x,y,x+mid,y);
		if(idy!=0&&(idy!=5||idx==0||idx==8))line(x,y-mid,x,y+1);
		if(idy!=9&&(idy!=4||idx==0||idx==8))line(x,y,x,y+mid);
		if(idx==4&&idy==1||idx==4&&idy==8){
			line(x-mid,y-mid,x+mid,y+mid);
			line(x-mid,y+mid,x+mid,y-mid);
		}
		if(idx==3&&idy==0||idx==3&&idy==7)line(x,y,x+mid,y+mid);
		if(idx==5&&idy==0||idx==5&&idy==7)line(x-mid,y+mid,x,y);
		if(idx==3&&idy==2||idx==3&&idy==9)line(x,y,x+mid,y-mid);
		if(idx==5&&idy==2||idx==5&&idy==9)line(x-mid,y-mid,x,y);
		if(idx==4&&idy==2||idx==5&&idy==1||idx==4&&idy==9||idx==5&&idy==8)putpixel(x-mid,y-mid,BLACK);
	}
	void PaintFrame(int idx,int idy,int colorType){
		if(!colorType)return;
		int x=GetScreenX(idx);
		int y=GetScreenY(idy);
		int sz=27;
		if(colorType==1)setcolor(RED);
		else if(colorType==2)setcolor(BLUE);
		line(x-sz,y-sz,x-sz+10,y-sz);
		line(x-sz,y-sz,x-sz,y-sz+10);
		line(x+sz,y-sz,x+sz-10,y-sz);
		line(x+sz,y-sz,x+sz,y-sz+10);
		line(x-sz,y+sz,x-sz+10,y+sz);
		line(x-sz,y+sz,x-sz,y+sz-10);
		line(x+sz,y+sz,x+sz-10,y+sz);
		line(x+sz,y+sz,x+sz,y+sz-10);
	}
	void PaintChess(int idx,int idy){
		int x=GetScreenX(idx);
		int y=GetScreenY(idy);
		chess[idx][idy].Paint(x,y);
	}
	void PaintChessAll(int idx,int idy,int colorType=0){
		PaintGrid(idx,idy);
		PaintChess(idx,idy);
		PaintFrame(idx,idy,colorType);
	}
	void Render(){
		for(int i=0;i<9;i++)for(int j=0;j<10;j++){
			int colorType=0;
			if(lastMove.IsBegin(i,j)||lastMove.IsEnd(i,j))colorType=1;
			PaintChessAll(i,j,colorType);
		}
	}
};
ScreenChessBoard screenBoard;

struct ScreenPut{
	int flip;
	int color;
	int roleRed;
	int roleBlack;
	vector<ScreenChessBoard>boards;
	int GetIdx(mouse_msg msg){
		assert(!boards.empty());
		return boards[0].GetIdx(msg);
	}
	int GetIdy(mouse_msg msg){
		assert(!boards.empty());
		return boards[0].GetIdy(msg);
	}
	bool Legal(int x,int y,int color,int type){
		return boards.back().Legal(x,y,color,type);
	}
	void AddChess(int x,int y,int color,int type){
		ScreenChessBoard board=boards.back();
		if(board.chess[x][y].type!=Shuai){
			board.chess[x][y].color=color;
			board.chess[x][y].type=type;
		}
		boards.push_back(board);
	}
	bool DeleteChess(int x,int y){
		ScreenChessBoard board=boards.back();
		if(board.chess[x][y].type!=Shuai){
			board.chess[x][y].type=0;
			boards.push_back(board);
			return true;
		}
		return false;
	}
	bool MoveChess(int x1,int y1,int x2,int y2){
		ScreenChessBoard board=boards.back();
		int canMove=Legal(x2,y2,board.chess[x1][y1].color,board.chess[x1][y1].type);
		if(board.chess[x2][y2].type==Shuai)canMove=0;
//		if(board.chess[x1][y1].type==Shuai&&!Legal(x2,y2,board.chess[x1][y1].color,Shuai))canMove=0;
		if(canMove){
			board.chess[x2][y2]=board.chess[x1][y1];
			board.chess[x1][y1].type=0;
			boards.push_back(board);
			return true;
		}
		return false;
	}
	void Revert(){//还原
		assert(!boards.empty());
		boards.push_back(boards[0]);
		flip=boards.back().flip;
	}
	void Clean(){//清空
		ScreenChessBoard board=boards.back();
		flip=board.flip=0;
		for(int i=0;i<9;i++)for(int j=0;j<10;j++)board.chess[i][j].type=0;
		board.chess[4][0].type=Shuai;
		board.chess[4][0].color=Black;
		board.chess[4][9].type=Shuai;
		board.chess[4][9].color=Red;
		boards.push_back(board);
	}
	void Initialize(){//初始
		ScreenChessBoard board=boards.back();
		flip=board.flip=0;
		for(int i=0;i<9;i++)for(int j=0;j<10;j++)board.chess[i][j].type=0;
		auto add=[](ScreenChessBoard &board,int x,int y,int color,int type){
			board.chess[x][y].color=color;
			board.chess[x][y].type=type;
			board.chess[8-x][9-y].color=!color;
			board.chess[8-x][9-y].type=type;
		};
		add(board,0,0,Black,Ju);
		add(board,1,0,Black,Ma);
		add(board,2,0,Black,Xiang);
		add(board,3,0,Black,Shi);
		add(board,4,0,Black,Shuai);
		add(board,5,0,Black,Shi);
		add(board,6,0,Black,Xiang);
		add(board,7,0,Black,Ma);
		add(board,8,0,Black,Ju);
		add(board,1,2,Black,Pao);
		add(board,7,2,Black,Pao);
		add(board,0,3,Black,Bing);
		add(board,2,3,Black,Bing);
		add(board,4,3,Black,Bing);
		add(board,6,3,Black,Bing);
		add(board,8,3,Black,Bing);
		boards.push_back(board);
	}
	void Swap(){//交换
		ScreenChessBoard board=boards.back();
		flip=board.flip=!board.flip;
		for(int i=0;i<9;i++)for(int j=0;j<10;j++)
			if(board.chess[i][j].type)board.chess[i][j].color^=1;
		boards.push_back(board);
	}
	void Flip(){//翻转
		ScreenChessBoard board=boards.back();
		flip=board.flip=!board.flip;
		for(int i=0;i<9;i++)for(int j=0;j<5;j++){
			swap(board.chess[i][j],board.chess[8-i][9-j]);
		}
		boards.push_back(board);
	}
	bool Rescind(){//撤销
		if(boards.size()>1){
			boards.pop_back();
			flip=boards.back().flip;
			return true;
		}
		return false;
	}
	void Complete(ScreenChessBoard &board){
		board.flip=flip;
		board.color=color;
		board.roleRed=roleRed;
		board.roleBlack=roleBlack;
		for(int i=0;i<9;i++)for(int j=0;j<10;j++){
			board.chess[i][j]=boards.back().chess[i][j];
		}
		board.lastMove.clear();
	}
	void Scan(ScreenChessBoard board){
		flip=board.flip;
		color=board.color;
		roleRed=board.roleRed;
		roleBlack=board.roleBlack;
		board.lastMove.clear();
		boards.clear();
		boards.push_back(board);
	}
	void Render(){
		boards.back().Render();
	}
};
ScreenPut screenPut;
struct ScreenPutAdd{
	int cnt[20];
	int len[2];
	vector<ScreenChess>chess;
	vector<Area>area;
	Area areaTotal;
	bool empty(){
		return chess.empty();
	}
	void Scan(ScreenChessBoard board,int idx,int idy){
		chess.clear();
		area.clear();
		memset(cnt,0,sizeof(cnt));
		memset(len,0,sizeof(len));
		for(int i=0;i<9;i++)for(int j=0;j<10;j++)
			if(board.chess[i][j].type){
				cnt[board.chess[i][j].color*10+board.chess[i][j].type]++;
			}
		int color=Red;
		do{
			for(int i=1;i<=7;i++)if(i!=Shuai){
				int can=board.Legal(idx,idy,color,i);
				int v=cnt[i+color*10];
				if(i==Bing){
					if(v==5)can=0;
				}
				else{
					if(v==2)can=0;
				}
				if(can){
					chess.push_back(ScreenChess(color,i));
					len[color]++;
				}
			}
			color^=1;
		}
		while(color!=Red);
//		for(int i=0;i<chess.size();i++){
//			printf("(%d,%d) ",chess[i].color,chess[i].type);
//		}
//		puts("");
		int x,y;
		int basicX=board.basicScreenX;
		int basicY=board.basicScreenY;
		int H=(len[0]>0)+(len[1]>0);
		int W=max(len[0],len[1]);
		if(idy+H<10)y=basicY+GRID_LEN*(idy+1);
		else y=basicY+GRID_LEN*(idy-H);
		if(idx+W<9)x=basicX+GRID_LEN*idx;
		else x=basicX+GRID_LEN*(8-W+1);
		areaTotal=Area(x,y,x+GRID_LEN*W,y+GRID_LEN*H);
		for(int i=1;i<=len[Red];i++){
			area.push_back(Area(x+GRID_LEN*(i-1),y,x+GRID_LEN*i,y+GRID_LEN));
		}
		for(int i=1;i<=len[Black];i++){
			area.push_back(Area(x+GRID_LEN*(i-1),y+GRID_LEN*(H-1),x+GRID_LEN*i,y+GRID_LEN*H));
		}
//		printf("idx=%d idy=%d W=%d H=%d\n",idx,idy,W,H);
//		for(int i=0;i<area.size();i++){
//			if(i&1)area[i].Bar(BLUE);
//			else area[i].Bar(RED);
//			getch();
//		}
	}
	void Paint(){
		areaTotal.Bar(YELLOW);
		for(int i=0;i<chess.size();i++){
			chess[i].Paint(area[i].x1+GRID_LEN/2,area[i].y1+GRID_LEN/2);
		}
	}
};
ScreenPutAdd screenPutAdd;

bool IsLeftClick(mouse_msg msg,int x1,int y1,int x2,int y2){//判断是否完整左击
//	printf("%d %d %d %d\n",x1,y1,x2,y2);
	int x=msg.x,y=msg.y;
	if((int)msg.is_left()&&(int)msg.is_down()&&x>=x1&&x<x2&&y>=y1&&y<y2)while(is_run()){
		while(mousemsg())msg=getmouse();
		if((int)msg.is_left()&&(int)msg.is_up())return true;
	}
	return false;
}
bool InArea(mouse_msg msg,int x1,int y1,int x2,int y2){
	int x=msg.x,y=msg.y;
	return x>=x1&&x<x2&&y>=y1&&y<y2;
}


void PaintPut(){
	cleardevice();
	setcolor(BLACK);
	setfillcolor(MENU_COLOR);
	setfont(&standardFont);
	settextjustify(CENTER_TEXT,CENTER_TEXT);
	setbkmode(TRANSPARENT);
	for(int i=1;i<=8;i++)areaPutMenu[i].Bar();
	areaPutMenu[1].PrintCenter("还原");
	areaPutMenu[2].PrintCenter("清空");
	areaPutMenu[3].PrintCenter("初始");
	areaPutMenu[4].PrintCenter("交换");
	areaPutMenu[5].PrintCenter("撤销");
	areaPutMenu[6].PrintCenter("翻转");
	areaPutMenu[7].PrintCenter("完成");
	areaPutMenu[8].PrintCenter("返回");
	areaMenu2.Bar(MENU2_COLOR);
//	setcolor(BLACK);
	setfillcolor(MENU_COLOR);
//	setfont(&standardFont);
//	settextjustify(CENTER_TEXT,CENTER_TEXT);
//	setbkmode(TRANSPARENT);
	xyprintf(MENU2_X+45,MENU2_Y+30,"摆放");
	xyprintf(MENU2_X+95,MENU2_Y+21+1*60,"  先手方：");
	xyprintf(MENU2_X+95,MENU2_Y+21+2*60,"    红方：");
	xyprintf(MENU2_X+95,MENU2_Y+21+3*60,"    黑方：");
	for(int i=1;i<=3;i++)areaPutMenu2[i].Bar();
	if(screenPut.color==Red)areaPutMenu2[1].PrintCenter("红方");
	else areaPutMenu2[1].PrintCenter("黑方");
	if(screenPut.roleRed==RolePlayer)areaPutMenu2[2].PrintCenter("棋手");
	else areaPutMenu2[2].PrintCenter("电脑");
	if(screenPut.roleBlack==RolePlayer)areaPutMenu2[3].PrintCenter("棋手");
	else areaPutMenu2[3].PrintCenter("电脑");
	screenPut.Render();
}
void OperationPut(int newGame=0){
	if(newGame);
	else screenPut.Scan(screenBoard);
	PaintPut();
	mouse_msg msg={0};
	int up=0;
	int adding=0;
	int chooseIdx=-1;
	int chooseIdy=-1;
	
	for(;is_run();delay_fps(60)){
		while(mousemsg())msg=getmouse();
		if(msg.is_left()&&msg.is_up()&&up)up=0;
		else if(msg.is_left()&&msg.is_down()&&!up){
			up=1;
			if(0);
			else if(areaPutMenu[1].InArea(msg)){//还原
				screenPut.Revert();
				screenPut.Render();
				chooseIdx=chooseIdy=-1;
			}
			else if(areaPutMenu[2].InArea(msg)){//清空
				screenPut.Clean();
				screenPut.Render();
				chooseIdx=chooseIdy=-1;
			}
			else if(areaPutMenu[3].InArea(msg)){//初始
				screenPut.Initialize();
				screenPut.Render();
				chooseIdx=chooseIdy=-1;
			}
			else if(areaPutMenu[4].InArea(msg)){//交换
				screenPut.Swap();
				screenPut.Render();
				chooseIdx=chooseIdy=-1;
			}
			else if(areaPutMenu[5].InArea(msg)){//撤销
				screenPut.Rescind();
				screenPut.Render();
				chooseIdx=chooseIdy=-1;
			}
			else if(areaPutMenu[6].InArea(msg)){//翻转
				screenPut.Flip();
				screenPut.Render();
				chooseIdx=chooseIdy=-1;
			}
			else if(areaPutMenu[7].InArea(msg)){//完成
				screenPut.Complete(screenBoard);
				//更新chessBoard
				screenBoard.Print(chessBoardTemp);
				ApiPlace(chessBoardTemp);
				screenBoard.Scan(chessBoard);////
				return;
			}
			else if(areaPutMenu[8].InArea(msg)){//返回
				return;
			}
			else if(areaPutMenu2[1].InArea(msg)){
				setfont(&standardFont);
				settextjustify(CENTER_TEXT,CENTER_TEXT);
				setbkmode(TRANSPARENT);
				if(screenPut.color==Red){
					screenPut.color=Black;
					areaPutMenu2[1].Bar(MENU_COLOR);
					areaPutMenu2[1].PrintCenter("黑方",BLACK);
				}
				else if(screenPut.color==Black){
					screenPut.color=Red;
					areaPutMenu2[1].Bar(MENU_COLOR);
					areaPutMenu2[1].PrintCenter("红方",BLACK);
				}
			}
			else if(areaPutMenu2[2].InArea(msg)){
				setfont(&standardFont);
				settextjustify(CENTER_TEXT,CENTER_TEXT);
				setbkmode(TRANSPARENT);
				if(screenPut.roleRed==RolePlayer){
					screenPut.roleRed=RoleComputer;
					areaPutMenu2[2].Bar(MENU_COLOR);
					areaPutMenu2[2].PrintCenter("电脑",BLACK);
				}
				else if(screenPut.roleRed==RoleComputer){
					screenPut.roleRed=RolePlayer;
					areaPutMenu2[2].Bar(MENU_COLOR);
					areaPutMenu2[2].PrintCenter("棋手",BLACK);
				}
			}
			else if(areaPutMenu2[3].InArea(msg)){
				setfont(&standardFont);
				settextjustify(CENTER_TEXT,CENTER_TEXT);
				setbkmode(TRANSPARENT);
				if(screenPut.roleBlack==RolePlayer){
					screenPut.roleBlack=RoleComputer;
					areaPutMenu2[3].Bar(MENU_COLOR);
					areaPutMenu2[3].PrintCenter("电脑",BLACK);
				}
				else if(screenPut.roleBlack==RoleComputer){
					screenPut.roleBlack=RolePlayer;
					areaPutMenu2[3].Bar(MENU_COLOR);
					areaPutMenu2[3].PrintCenter("棋手",BLACK);
				}
			}
			else if(areaBoard.InArea(msg)){
				int x=screenPut.GetIdx(msg);
				int y=screenPut.GetIdy(msg);
				assert(~x&&~y);//
				if(adding){
					if(screenPutAdd.areaTotal.InArea(msg)){
						int choose=-1;
						for(int i=0;i<screenPutAdd.area.size();i++)
							if(screenPutAdd.area[i].InArea(msg))choose=i;
						if(~choose){
							ScreenChess chess=screenPutAdd.chess[choose];
							screenPut.AddChess(chooseIdx,chooseIdy,chess.color,chess.type);
							screenPut.Render();
							adding=0;
							chooseIdx=chooseIdy=-1;
						}
					}
					else{
						screenPut.Render();
						adding=0;
						chooseIdx=chooseIdy=-1;
					}
				}
				else if(~chooseIdx&&~chooseIdy){
					if(x==chooseIdx&&y==chooseIdy){
						if(!screenPut.DeleteChess(x,y)){
							puts("删除失败");
						}
						screenPut.Render();
						chooseIdx=chooseIdy=-1;
					}
					else{
						if(!screenPut.MoveChess(chooseIdx,chooseIdy,x,y)){
							puts("移动失败");
						}
						screenPut.Render();
						chooseIdx=chooseIdy=-1;
					}
				}
				else{
					if(screenPut.boards.back().chess[x][y].type){
						chooseIdx=x;
						chooseIdy=y;
						screenPut.boards.back().PaintFrame(x,y,2);
					}
					else{
						adding=1;
						chooseIdx=x;
						chooseIdy=y;
						screenPutAdd.Scan(screenPut.boards.back(),x,y);
						if(screenPutAdd.empty()){
							adding=0;
							chooseIdx=chooseIdy=-1;
							screenPut.Render();
						}
						else{
							screenPut.boards.back().PaintFrame(x,y,2);
							screenPutAdd.Paint();
						}
					}
				}
			}
		}
	}
	getch();
}

bool ApiCanMove(int x1,int y1,int x2,int y2){
	return ApiCanMove(CalculatePosition(x1,y1,screenBoard.flip),CalculatePosition(x2,y2,screenBoard.flip));
}
void ApiPlayerMove(int x1,int y1,int x2,int y2){
	ApiPlayerMove(CalculatePosition(x1,y1,screenBoard.flip),CalculatePosition(x2,y2,screenBoard.flip));
}
void ChangeLevel(int &level,int &step){
	if(0);
//	else if(step==1)step=2;
	else{
		level++;
		if(level>12)level=6;
		step=2;
//		step=1;
	}
}
void PaintMain(){
	cleardevice();
	screenBoard.Render();
	setcolor(BLACK);
	setfillcolor(MENU_COLOR);
	setfont(&standardFont);
	settextjustify(CENTER_TEXT,CENTER_TEXT);
	setbkmode(TRANSPARENT);
	for(int i=1;i<=5;i++)areaMenu[i].Bar();
	areaMenu[1].PrintCenter("新局");
	areaMenu[2].PrintCenter("设置");
	areaMenu[3].PrintCenter("悔棋");
	areaMenu[4].PrintCenter("摆放");
	areaMenu[5].PrintCenter("提示");
}
void OperationMain(int mode=ModePlayerComputer){
	ApiNewGame(mode,screenBoard.computerLevel,screenBoard.computerStep);
	screenBoard.Scan(chessBoard);
	PaintMain();
	mouse_msg msg={0};
	int leave=1;
	int chooseIdx=-1;
	int chooseIdy=-1;
	int loser=-1;
	int computerComputerStop=0;
	int think=0;
	int up=1;
	int menuOpen=0;
	int roleRedSet;
	int roleBlackSet;
	int computerLevelSet;
	int computerStepSet;
	int hintLevelSet;
	int hintStepSet;
	for(;is_run();delay_fps(60)){
		if(leave){
			leave=0;
			chooseIdx=chooseIdy=-1;
			up=0;//
		}
		if(loser==-1&&screenBoard.color==Red&&screenBoard.roleRed==RoleComputer){
			think=1;
			ApiComputerMove();
			screenBoard.Scan(chessBoard);
			screenBoard.Render();
			if(ApiCheckLose())loser=screenBoard.color;
		}
		else if(loser==-1&&screenBoard.color==Black&&screenBoard.roleBlack==RoleComputer){
			think=1;
			ApiComputerMove();
			screenBoard.Scan(chessBoard);
			screenBoard.Render();
			if(ApiCheckLose())loser=screenBoard.color;
		}
		while(mousemsg())msg=getmouse();
		if(think){
//			if(msg.is_up())
				up=1;
			think=0;
		}
		if((int)msg.is_left()&&(int)msg.is_up())up=1;
		else if(up&&(int)msg.is_left()&&(int)msg.is_down()){
			up=0;
			if(0);
			else if(areaMenu[1].InArea(msg)){//新局
				if(menuOpen==1){
					menuOpen=0;
					areaMenu2.Bar(BK_COLOR);
				}
				else{
					menuOpen=1;
					roleRedSet=screenBoard.roleRed;
					roleBlackSet=screenBoard.roleBlack;
					computerLevelSet=screenBoard.computerLevel;
					computerStepSet=screenBoard.computerStep;
					hintLevelSet=screenBoard.hintLevel;
					hintStepSet=screenBoard.hintStep;
					areaMenu2.Bar(MENU2_COLOR);
					setcolor(BLACK);
					setfillcolor(MENU_COLOR);
					setfont(&standardFont);
					settextjustify(CENTER_TEXT,CENTER_TEXT);
					setbkmode(TRANSPARENT);
					xyprintf(MENU2_X+45,MENU2_Y+30,"新局");
					xyprintf(MENU2_X+95,MENU2_Y+21+1*60,"    红方：");
					xyprintf(MENU2_X+95,MENU2_Y+21+2*60,"    黑方：");
					xyprintf(MENU2_X+95,MENU2_Y+21+3*60,"电脑水平：");
					for(int i=1;i<=6;i++)areaNewGame[i].Bar();
					if(roleRedSet==RolePlayer)areaNewGame[1].PrintCenter("棋手");
					else areaNewGame[1].PrintCenter("电脑");
					if(roleBlackSet==RolePlayer)areaNewGame[2].PrintCenter("棋手");
					else areaNewGame[2].PrintCenter("电脑");
					areaNewGame[3].PrintCenter(to_string(computerLevelSet)+"步("+to_string(computerStepSet)+")");
					areaNewGame[4].PrintCenter("确定");
					areaNewGame[5].PrintCenter("一键先手");
					areaNewGame[6].PrintCenter("一键后手");
				}
			}
			else if(areaMenu[2].InArea(msg)){//设置
				if(menuOpen==2){
					menuOpen=0;
					areaMenu2.Bar(BK_COLOR);
				}
				else{
					menuOpen=2;
					roleRedSet=screenBoard.roleRed;
					roleBlackSet=screenBoard.roleBlack;
					computerLevelSet=screenBoard.computerLevel;
					computerStepSet=screenBoard.computerStep;
					hintLevelSet=screenBoard.hintLevel;
					hintStepSet=screenBoard.hintStep;
					areaMenu2.Bar(MENU2_COLOR);
					setcolor(BLACK);
					setfillcolor(MENU_COLOR);
					setfont(&standardFont);
					settextjustify(CENTER_TEXT,CENTER_TEXT);
					setbkmode(TRANSPARENT);
					xyprintf(MENU2_X+45,MENU2_Y+30,"设置");
					xyprintf(MENU2_X+95,MENU2_Y+21+1*60,"    红方：");
					xyprintf(MENU2_X+95,MENU2_Y+21+2*60,"    黑方：");
					xyprintf(MENU2_X+95,MENU2_Y+21+3*60,"电脑水平：");
					xyprintf(MENU2_X+95,MENU2_Y+21+4*60,"提示水平：");
					for(int i=1;i<=5;i++)areaSet[i].Bar();
					if(roleRedSet==RolePlayer)areaSet[1].PrintCenter("棋手");
					else areaSet[1].PrintCenter("电脑");
					if(roleBlackSet==RolePlayer)areaSet[2].PrintCenter("棋手");
					else areaSet[2].PrintCenter("电脑");
					areaSet[3].PrintCenter(to_string(computerLevelSet)+"步("+to_string(computerStepSet)+")");
					areaSet[4].PrintCenter(to_string(hintLevelSet)+"步("+to_string(hintStepSet)+")");
					areaSet[5].PrintCenter("确定");
				}
			}
			else if(areaMenu[3].InArea(msg)){//悔棋
				ApiRepent();
				chooseIdx=chooseIdy=-1;
				loser=-1;
				screenBoard.Scan(chessBoard);
				screenBoard.Render();
			}
			else if(areaMenu[4].InArea(msg)){//摆放
				menuOpen=4;
				leave=1;
				OperationPut();
				PaintMain();
			}
			else if(areaMenu[5].InArea(msg)){//提示
				if(loser==-1){
					think=1;
					chooseIdx=chooseIdy=-1;
					ApiHint();
					screenBoard.Scan(chessBoard);
					screenBoard.Render();
					if(ApiCheckLose())loser=screenBoard.color;
				}
			}
			else if(areaMenu2.InArea(msg)){
				if(0);
				else if(menuOpen==1){//新局展开
					if(areaNewGame[1].InArea(msg)){
						setfont(&standardFont);
						settextjustify(CENTER_TEXT,CENTER_TEXT);
						setbkmode(TRANSPARENT);
						if(roleRedSet==RolePlayer){
							roleRedSet=RoleComputer;
							areaNewGame[1].Bar(MENU_COLOR);
							areaNewGame[1].PrintCenter("电脑",BLACK);
						}
						else if(roleRedSet==RoleComputer){
							roleRedSet=RolePlayer;
							areaNewGame[1].Bar(MENU_COLOR);
							areaNewGame[1].PrintCenter("棋手",BLACK);
						}
					}
					else if(areaNewGame[2].InArea(msg)){
						setfont(&standardFont);
						settextjustify(CENTER_TEXT,CENTER_TEXT);
						setbkmode(TRANSPARENT);
						if(roleBlackSet==RolePlayer){
							roleBlackSet=RoleComputer;
							areaNewGame[2].Bar(MENU_COLOR);
							areaNewGame[2].PrintCenter("电脑",BLACK);
						}
						else if(roleBlackSet==RoleComputer){
							roleBlackSet=RolePlayer;
							areaNewGame[2].Bar(MENU_COLOR);
							areaNewGame[2].PrintCenter("棋手",BLACK);
						}
					}
					else if(areaNewGame[3].InArea(msg)){
						ChangeLevel(computerLevelSet,computerStepSet);
						setfont(&standardFont);
						settextjustify(CENTER_TEXT,CENTER_TEXT);
						setbkmode(TRANSPARENT);
						areaNewGame[3].Bar(MENU_COLOR);
						string str=to_string(computerLevelSet)+"步("+to_string(computerStepSet)+")";
						areaNewGame[3].PrintCenter(str,BLACK);
					}
					else if(areaNewGame[4].InArea(msg)){//新局确定
						menuOpen=0;
						chooseIdx=chooseIdy=-1;
						loser=-1;
						areaMenu2.Bar(BK_COLOR);
						if(roleRedSet==RolePlayer&&roleBlackSet==RolePlayer)
							ApiNewGame(ModePlayerPlayer,computerLevelSet,computerStepSet);
						if(roleRedSet==RolePlayer&&roleBlackSet==RoleComputer)
							ApiNewGame(ModePlayerComputer,computerLevelSet,computerStepSet);
						if(roleRedSet==RoleComputer&&roleBlackSet==RolePlayer)
							ApiNewGame(ModeComputerPlayer,computerLevelSet,computerStepSet);
						if(roleRedSet==RoleComputer&&roleBlackSet==RoleComputer)
							ApiNewGame(ModeComputerComputer,computerLevelSet,computerStepSet);
						screenBoard.Scan(chessBoard);
						screenBoard.Render();
					}
					else if(areaNewGame[5].InArea(msg)){
						menuOpen=0;
						chooseIdx=chooseIdy=-1;
						loser=-1;
						areaMenu2.Bar(BK_COLOR);
						ApiNewGame(ModeFirst,computerLevelSet,computerStepSet);
						screenBoard.Scan(chessBoard);
						screenBoard.Render();
					}
					else if(areaNewGame[6].InArea(msg)){
						menuOpen=0;
						chooseIdx=chooseIdy=-1;
						loser=-1;
						areaMenu2.Bar(BK_COLOR);
						ApiNewGame(ModeBack,computerLevelSet,computerStepSet);
						screenBoard.Scan(chessBoard);
						screenBoard.Render();
					}
				}
				else if(menuOpen==2){//设置展开
					if(areaSet[1].InArea(msg)){
						setfont(&standardFont);
						settextjustify(CENTER_TEXT,CENTER_TEXT);
						setbkmode(TRANSPARENT);
						if(roleRedSet==RolePlayer){
							roleRedSet=RoleComputer;
							areaSet[1].Bar(MENU_COLOR);
							areaSet[1].PrintCenter("电脑",BLACK);
						}
						else if(roleRedSet==RoleComputer){
							roleRedSet=RolePlayer;
							areaSet[1].Bar(MENU_COLOR);
							areaSet[1].PrintCenter("棋手",BLACK);
						}
					}
					else if(areaSet[2].InArea(msg)){
						setfont(&standardFont);
						settextjustify(CENTER_TEXT,CENTER_TEXT);
						setbkmode(TRANSPARENT);
						if(roleBlackSet==RolePlayer){
							roleBlackSet=RoleComputer;
							areaSet[2].Bar(MENU_COLOR);
							areaSet[2].PrintCenter("电脑",BLACK);
						}
						else if(roleBlackSet==RoleComputer){
							roleBlackSet=RolePlayer;
							areaSet[2].Bar(MENU_COLOR);
							areaSet[2].PrintCenter("棋手",BLACK);
						}
					}
					else if(areaSet[3].InArea(msg)){
						ChangeLevel(computerLevelSet,computerStepSet);
						setfont(&standardFont);
						settextjustify(CENTER_TEXT,CENTER_TEXT);
						setbkmode(TRANSPARENT);
						areaSet[3].Bar(MENU_COLOR);
						string str=to_string(computerLevelSet)+"步("+to_string(computerStepSet)+")";
						areaSet[3].PrintCenter(str,BLACK);
					}
					else if(areaSet[4].InArea(msg)){
						ChangeLevel(hintLevelSet,hintStepSet);
						setfont(&standardFont);
						settextjustify(CENTER_TEXT,CENTER_TEXT);
						setbkmode(TRANSPARENT);
						areaSet[4].Bar(MENU_COLOR);
						string str=to_string(hintLevelSet)+"步("+to_string(hintStepSet)+")";
						areaSet[4].PrintCenter(str,BLACK);
					}
					else if(areaSet[5].InArea(msg)){//设置确定
						menuOpen=0;
						chooseIdx=chooseIdy=-1;
						areaMenu2.Bar(BK_COLOR);
						ApiSet(roleRedSet,roleBlackSet,computerLevelSet,computerStepSet,hintLevelSet,hintStepSet);
						screenBoard.Scan(chessBoard);
						screenBoard.Render();
					}
				}
			}
			else if(areaBoard.InArea(msg)){
				if(0);
				else if(~loser);
				else if(screenBoard.color==Red&&screenBoard.roleRed==RolePlayer||
				chessBoard.color==Black&&chessBoard.roleBlack==RolePlayer){
					int x=screenBoard.GetIdx(msg);
					int y=screenBoard.GetIdy(msg);
					int id=CalculatePosition(x,y,screenBoard.flip);//
					if(screenBoard.chess[x][y].type&&screenBoard.chess[x][y].color==screenBoard.color){
						printf("choose%d\n",id);
						if(!CheckSame(chooseIdx,chooseIdy,x,y)){
							if(~chooseIdx)screenBoard.PaintChessAll(chooseIdx,chooseIdy,0);
							chooseIdx=x;
							chooseIdy=y;
							screenBoard.PaintChessAll(chooseIdx,chooseIdy,2);
						}
					}
					else if(~chooseIdx&&~chooseIdy){
						if(ApiCanMove(chooseIdx,chooseIdy,x,y)){
							printf("move to %d\n",id);
							ApiPlayerMove(chooseIdx,chooseIdy,x,y);
							if(ApiCheckLose())loser=screenBoard.color;
							screenBoard.Scan(chessBoard);
							screenBoard.Render();
						}
						else{
							printf("can't move\n");
							screenBoard.PaintChessAll(chooseIdx,chooseIdy,0);
							chooseIdx=chooseIdy=-1;
						}
					}
				}
			}
		}
	}
}
void init(){
	Ctrl::SetWindowSize(80,50);
	srand(time(NULL));
	randomize();
	initgraph(SCREEN_W,SCREEN_H,INIT_RENDERMANUAL);
	setcaption("Chinse chess  writen by jiedai and nudun");
	setbkcolor(BK_COLOR);
	
	standardFont.lfHeight=22;
	strcpy(standardFont.lfFaceName,"宋体");
	standardFont.lfWeight=FW_DONTCARE;
	
	chessFont.lfHeight=34;
	strcpy(chessFont.lfFaceName,"宋体");
	chessFont.lfWeight=FW_BLACK;
	
//	setfont(32,0,"宋体");
	
	screenBoard.basicScreenX=BOARD_X-GRID_LEN/2;
	screenBoard.basicScreenY=BOARD_Y-GRID_LEN/2;
	screenBoard.computerLevel=4;
	screenBoard.computerStep=2;
	
	areaBoard=Area(BOARD_X-GRID_LEN/2,BOARD_Y-GRID_LEN/2,int(BOARD_X+GRID_LEN*8.5),int(BOARD_Y+GRID_LEN*9.5));
	for(int i=0;i<2;i++)for(int j=0;j<3;j++)if(i*3+j<5){
		areaMenu[i*3+j+1]=Area(MENU_X+j*100,MENU_Y+i*76,MENU_X+j*100+80,MENU_Y+i*76+46);
	}
	areaMenu2=Area(MENU2_X,MENU2_Y,MENU2_X+360,MENU2_Y+360);
	areaNewGame[1]=Area(MENU2_X+170,MENU2_Y+1*60,MENU2_X+170+140,MENU2_Y+1*60+42);
	areaNewGame[2]=Area(MENU2_X+170,MENU2_Y+2*60,MENU2_X+170+140,MENU2_Y+2*60+42);
	areaNewGame[3]=Area(MENU2_X+170,MENU2_Y+3*60,MENU2_X+170+140,MENU2_Y+3*60+42);
	areaNewGame[4]=Area(MENU2_X+120,MENU2_Y+4*60,MENU2_X+120+120,MENU2_Y+4*60+42);
	areaNewGame[5]=Area(MENU2_X+30,MENU2_Y+5*60,MENU2_X+30+120,MENU2_Y+5*60+42);
	areaNewGame[6]=Area(MENU2_X+190,MENU2_Y+5*60,MENU2_X+190+120,MENU2_Y+5*60+42);
	areaSet[1]=Area(MENU2_X+170,MENU2_Y+1*60,MENU2_X+170+140,MENU2_Y+1*60+42);
	areaSet[2]=Area(MENU2_X+170,MENU2_Y+2*60,MENU2_X+170+140,MENU2_Y+2*60+42);
	areaSet[3]=Area(MENU2_X+170,MENU2_Y+3*60,MENU2_X+170+140,MENU2_Y+3*60+42);
	areaSet[4]=Area(MENU2_X+170,MENU2_Y+4*60,MENU2_X+170+140,MENU2_Y+4*60+42);
	areaSet[5]=Area(MENU2_X+120,MENU2_Y+5*60,MENU2_X+120+120,MENU2_Y+5*60+42);
	for(int i=0;i<2;i++)for(int j=0;j<4;j++){
		areaPutMenu[i*4+j+1]=Area(PUT_X+10+j*90,PUT_Y+i*76,PUT_X+10+j*90+70,PUT_Y+i*76+46);
	}
	areaPutMenu2[1]=Area(MENU2_X+170,MENU2_Y+1*60,MENU2_X+170+140,MENU2_Y+1*60+42);
	areaPutMenu2[2]=Area(MENU2_X+170,MENU2_Y+2*60,MENU2_X+170+140,MENU2_Y+2*60+42);
	areaPutMenu2[3]=Area(MENU2_X+170,MENU2_Y+3*60,MENU2_X+170+140,MENU2_Y+3*60+42);
	
	ApiInit();
}
signed main(){
//	cleardevice();
	init();
	OperationMain();
	getch();
	closegraph();
	return (0-0);
}
