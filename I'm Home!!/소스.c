#include <stdio.h>
#include <Windows.h>	// gotoxy
#include <stdlib.h>		// srand
#include <time.h>		// time
#include <conio.h>		// _getch
#include <string.h>		// memset

// 워닝 무시
#pragma warning(disable:6835)
#pragma warning(disable:6836)


// 콘솔 크기 정의
#define SCREEN_WIDTH  60
#define SCREEN_HEIGHT 30

// 맵 사이즈
#define SIZE 10

/*
EMPTY	= 길						= 0
WALL	= 벽						= 1
D		= 술 취한 사람 (Drunk)	= 68
H		= 집	 (Home)				= 72
O		= 경찰 (Police Officer)	= 79
P		= 술집 (Pub)				= 68
*/

// 숫자는 각 알파벳의 아스키코드 & 길 벽 구분
#define WALL 1
#define EMPTY 0
#define P 80
#define O 79
#define H 72
#define D 68

// 방향키
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77


// 열거형
typedef enum Directions {
	Up = 0,
	Down,
	Left,
	Right
}Directions;
typedef enum ObjectType {
	Player = 0,
	Police
} OT;
typedef enum MovementMode {
	Auto = 0,
	Manual = 1
}MV;



// 현재 D의 위치
int playerX;
int playerY;

int prePlayerX;
int prePlayerY;

int curQuad;	// 현재 위치한 사분면
int prevQuad;	// 이전에 방문한 사분면

// 경찰들의 위치
// cops = { {경찰1 xPos, 경찰1 yPos, 스폰 된 사분면}, {경찰2 xPos, 경찰2 yPos, 스폰 된 사분면} };
int cops[2][3] = { 0 };
int policeNum;			// 경찰의 수

// 술집이 위치한 사분면
int pubQuad;

// 술집의 위치
int PUB_X;
int PUB_Y;

// 집의 위치
int HOME_X;
int HOME_Y;

/* 게임 이동 관련 */
int m_t, current_m_t;	// 자동 이동 횟수
int r_t, current_r_t;	// 수동 이동 횟수
int moveInterval;		// 이동 주기


// 맵
int map[SIZE][SIZE] = {
	// 1사분면
	
	/*
	* 
	*	   1┌─┬─┐2
	*		├─┼─┤
	*	   3└─┴─┘4
	* 
	* 각 숫자는 사분면 번호로 정해기로 함
	* 1은 벽, 0은 길
	*/

	{1,1,0,1,1,79,0,0,0,0},
	{0,0,0,0,0,0,1,0,1,1},
	{0,1,1,1,1,0,1,0,0,0},
	{0,0,0,0,0,0,1,0,1,0},
	{0,1,1,0,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,1,1,1,1,0,1,0,1,0},
	{0,0,0,0,0,0,1,0,0,0},
	{0,1,1,0,1,0,1,0,1,0},
	{0,0,0,0,0,0,0,0,0,0}
	
};
int front_buffer[SIZE][SIZE];


/*
* 커서 깜빡임 제거
* parameter : void
* return : void
*/
void CursorInvisible()
{
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };//콘솔 커서 정보 구조체 초기화
	cursorInfo.dwSize = 1;					//커서 굵기 (1 ~ 100)
	cursorInfo.bVisible = FALSE;			//커서 Visible TRUE(보임) FALSE(숨김)
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo); // 콘솔 커서 정보를 만들어둔 구조체의 정보에 맞게 설정
}

/*
* 함수명 : gotoxy
* 사용 용도: 인자로 받은 x, y값을 좌표로 사용하여 좌표(x, y)로 커서를 이동시킨다.
* 매개변수: int x int y
*/
void gotoxy(int x, int y) {
	COORD pos = { x * 2, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

/*
* 화면 지우기
* parameter : void
* return : void
*/
void cls(void) {
	system("CLS");
}



/* 게임 시작 전 설정 함수 */
/* 
* m_t, r_t, moveInterval 초기값 외부입력
* parameter : void
* return : void
*/
void setOption(void) {
	printf("m_t:\t\t"); scanf_s("%d", &m_t, 4);
	printf("r_t:\t\t"); scanf_s("%d", &r_t, 4);
	printf("moveInterval:\t"); scanf_s("%d", &moveInterval, 4);
	current_m_t = m_t;
	current_r_t = r_t;
}

/*
* 술집, 집 스폰 위치 설정
* parameter : void
* return : void
*/ 
void setSpawnPos(void) {
	
	// 술집이랑 집 사분면 지정
	pubQuad = rand() % 4 + 1;
	


	// 술집과 집이 모두 스폰이 됐는지 확인하는 변수(Boolean)
	int isSpawned = 0;
	
	pubQuad % 2;

	
	while (!isSpawned) {
		// 술집 위치
		int rangeX	= NULL;
		int rangeY	= NULL;

		// 집 위치
		int _rangeX = NULL;
		int _rangeY = NULL;

		if (pubQuad == 1) {
			//   Y    X
			// [0~4][0~4] = P
			// [5~9][5~9] = H

			// P 스폰 위치
			rangeX = rand() % 5;
			rangeY = rand() % 5;

			// H 스폰 위치
			_rangeX = rand() % (9 - 5 + 1) + 5;
			_rangeY = rand() % (9 - 5 + 1) + 5;
			
		}
		else if (pubQuad == 2) {
			// [0~4][5~9] = P
			// [5~9][0~4] = H

			rangeX = rand() % (9 - 5 + 1) + 5;
			rangeY = rand() % 5;

			_rangeX = rand() % 5;
			_rangeY = rand() % (9 - 5 + 1) + 5;

		}
		else if (pubQuad == 3) {
			// [5~9][0~4] = P
			// [0~4][5~9] = H

			rangeX = rand() % 5;
			rangeY = rand() % (9 - 5 + 1) + 5;

			_rangeX = rand() % (9 - 5 + 1) + 5;
			_rangeY = rand() % 5;

		}
		else if (pubQuad == 4) {
			// [5~9][5~9] = P
			// [0~4][0~4] = H

			rangeX = rand() % (9 - 5 + 1) + 5;
			rangeY = rand() % (9 - 5 + 1) + 5;

			_rangeX = rand() % 5;
			_rangeY = rand() % 5;

		}

		if (map[rangeY][rangeX] == 0 && map[_rangeY][_rangeX] == 0) {
			map[rangeY][rangeX]	= P;	// 술집 위치 결정
			map[_rangeY][_rangeX]	= H;	// 집 위치 결정

			PUB_X = rangeX;
			PUB_Y = rangeY;

			HOME_X = _rangeX;
			HOME_Y = _rangeY;
			isSpawned = 1;
		}
		 
	}
}

/*
* 술집에서 술 취한 사람이 나오는 방향
* parameter : void
* return : void
*/ 
void setSpawnDrunk(void) {
	
	// 술집에서 나오는 방향
	int direction;
	int isSpawned = 0;
	while (!isSpawned) {	

		// 0: 위, 1: 아래, 2: 왼쪽, 3: 오른쪽
		direction = rand() % 4;


		if (direction == 0 && PUB_Y > 0) {
			if (map[PUB_Y - 1][PUB_X] == 0) {
				playerX = PUB_X; 
				playerY = PUB_Y - 1;
				map[PUB_Y - 1][PUB_X] = D;
				isSpawned = 1;
			}
		}

		else if (direction == 1 && PUB_Y < 9) {
			if (map[PUB_Y + 1][PUB_X] == 0) {
				playerX = PUB_X;
				playerY = PUB_Y + 1;
				map[PUB_Y + 1][PUB_X] = D;
				isSpawned = 1;
			}
		}

		else if (direction == 2 && PUB_X > 0) {
			if (map[PUB_Y][PUB_X - 1] == 0) {
				playerX = PUB_X - 1;
				playerY = PUB_Y;
				map[PUB_Y][PUB_X - 1] = D;
				isSpawned = 1;
			}
		}

		else if (direction == 3 && PUB_X < 9) {
			if (map[PUB_Y][PUB_X + 1] == 0) {
				playerX = PUB_X + 1;
				playerY = PUB_Y;
				map[PUB_Y][PUB_X + 1] = D; 
				isSpawned = 1;
			}
		}
	}
}

/*
* front_buffer를 -1로 초기화
* parameter : void
* return : void
*/
void memsetBuffer() {
	for (int i = 0; i < SIZE; i++)
		memset(front_buffer[i], -1, sizeof(int) * SIZE);
}



/* 드로잉 관련 함수*/

/*
* 한글자 출력
* parameter : x좌표, y좌표, 바꿀 문자
* return : void
*/
void drawMap(int x, int y, int c) {
	// 출력할 위치로 
	gotoxy(x, y);

	// SetConsoleTextAttribute 
	// 출력의 색상이 설정한 값에 따라서 바뀐다.
	
	if (c == WALL) {
		// 밝은 흰색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		printf("■");
	}
	if (c == EMPTY) {
		// 옅은 회색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
		printf("□");
	}
	if (c == P) {
		// 핑크색 (보라색인가?)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
		printf("P ");
	}
	if (c == H) {
		// 초록색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
		printf("H ");
	}
	if (c == O) {
		// 빨간색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
		printf("O ");
	}
	if (c == D) {
		// 노란색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
		printf("D ");
	}

	// 출력했으면 이상한곳에 커서가 있지 않게 맵 끝으로 커서를 이동
	gotoxy(SIZE, SIZE);

}

/*
* 버퍼를 프린트
* parameter : 맵(2차원 배열 매개변수)
* return : void
* function : 
*	바뀐부분 출력
*/
void buffer_print(int map[][10])
{
	
	/* 현재 미로와 front_buffer(이전 미로)에 있는 미로 비교 */
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0, j2 = 0; j < SIZE; ++j)
		{
			if (front_buffer[i][j] != map[i][j])
			{
				drawMap(j2, i, map[i][j]);
				// 바뀐 부분 화면에 출력
				front_buffer[i][j] = map[i][j];
				// 바뀐 부분은 출력 후 front_buffer에 저장
			}
			++j2;
			// 출력할 위치를 계속 변경하기 때문에 증감연산자를 이용한다.
		}
}



/* 이동 관련 함수*/

/*
* 플레이어 수동 이동
* parameter : void
* return : void
*/
void manualMovement(void)
 {
	// 방향키 입렵
	int dir = _getch();

	// 가려고 하는 방향이 맵 밖이 아니거나 벽이 아닐 경우에만 가능
	if (dir == UP	&& playerY > 0 && map[playerY - 1][playerX] != 1) {
		map[playerY--][playerX] = 0;
		map[playerY][playerX]	= D;
	}
	if (dir == DOWN && playerY < 9 && map[playerY + 1][playerX] != 1) {
		map[playerY++][playerX] = 0;
		map[playerY][playerX]	= D;
	}
	if (dir == LEFT && playerX > 0 && map[playerY][playerX - 1] != 1) {
		map[playerY][playerX--] = 0;
		map[playerY][playerX] = D;
	}

	if (dir == RIGHT && playerX < 9 && map[playerY][playerX + 1] != 1) {
		map[playerY][playerX++] = 0;
		map[playerY][playerX] = D;
	}

	// 플레이어가 펍이나 집을 지나 갔을 때 사라지는 것을 방지 함.
	if (map[PUB_Y][PUB_X] == 0) map[PUB_Y][PUB_X] = P;
	if (map[HOME_Y][HOME_X] == 0) map[HOME_Y][HOME_X] = H;

}

/*
* 플레이어 자동 이동
* parameter : void
* return : void
*/
void autoMovement() {

	prePlayerX = playerX;
	prePlayerY = playerY;

	// 이동 방향 결정
	int dirs[4] = { UP, DOWN, LEFT, RIGHT };
	int random = NULL;
	int temp = 1;

	// 플레이어 이동
	while (temp) {
		// 방향 지정
		random = rand() % 4;

		// 가려고 하는 방향이 맵 밖이 아니거나 벽이 아닐 경우에만 가능
		if (dirs[random] == UP && playerY > 0 && map[playerY - 1][playerX] != 1) {
			//이전 사분면 방문 금지
			if (prevQuad == 1 || prevQuad == 2) {
				if (playerY == 5) continue;
			}
			
			map[playerY--][playerX] = 0;
			temp = !temp;
		}
		else if (dirs[random] == DOWN && playerY < 9 && map[playerY + 1][playerX] != 1) {
			if (prevQuad == 3 || prevQuad == 4) {
				if (playerY == 4) continue;
			}

			map[playerY++][playerX] = 0;
			temp = !temp;
		}
		else if (dirs[random] == LEFT && playerX > 0 && map[playerY][playerX - 1] != 1) {
			if (prevQuad == 1 || prevQuad == 3)
				if (playerX == 5) continue;

			map[playerY][playerX--] = 0;
			temp = !temp;
		}
		else if (dirs[random] == RIGHT && playerX < 9 && map[playerY][playerX + 1] != 1) {
			if (prevQuad == 2 || prevQuad == 4)
				if (playerX == 4) continue;

			map[playerY][playerX++] = 0;
			temp = !temp;
		}

		
		if (!temp) {
			// 현재 위치 갱신~
			map[playerY][playerX] = D;
			current_m_t--;

			// 플레이어가 펍이나 집을 지나 갔을 때 사라지는 것을 방지 함.
			if (map[PUB_Y][PUB_X] == 0) map[PUB_Y][PUB_X] = P;
			if (map[HOME_Y][HOME_X] == 0) map[HOME_Y][HOME_X] = H;
		}
	}

	
}

/*
* 플레이어 이동 모드 변환
*/
void playerControl(int isDetected) {
	if (isDetected) manualMovement();
	else autoMovement();
}

/*
* 경찰의 순찰 이동
* parameter : void
* return : void
*/
void copPatrol(void) {

}

/*
* 경찰이 플레이어를 발견
* parameter : void
* return : int(발견하면 1, 아니면 0)
*/
int detectedPlayer(void){}


/* 
* 방문한 이전 분면과 현재 분면을 설정
* parameter : void
* return : void
*/
void visitedQuad(void) {
	
	if (prePlayerX == 4 && playerX == 5) {
		if (playerY < 5) {
			prevQuad = 1;
			curQuad = 2;
		}
		if (playerY >= 5) {
			prevQuad = 3;
			curQuad = 4;
		}
	}
	if (prePlayerX == 5 && playerX == 4) {
		if (playerY < 5) {
			prevQuad = 2;
			curQuad = 1;
		}
		if (playerY >= 5) {
			prevQuad = 4;
			curQuad = 3;
		}
	}
	if (prePlayerY == 4 && playerY == 5) {
		if (playerX < 5) {
			prevQuad = 1;
			curQuad = 3;
		}
		if (playerX >= 5) {
			prevQuad = 2;
			curQuad = 4;
		}
	}
	if (prePlayerY == 5 && playerY == 4) {
		if (playerX < 5) {
			prevQuad = 3;
			curQuad = 1;
		}
		if (playerX >= 5) {
			prevQuad = 4;
			curQuad = 2;
		}
	}
	
}



int main() {
	// 랜덤 시드 생성
	srand(time(NULL));

	// 가로 60칸 세로 30칸으로 콘솔 리사이즈
	system("mode con cols=60 lines=30"); 
	memsetBuffer();
	CursorInvisible();
	setSpawnPos();
	setSpawnDrunk();
	
	cops[0][0] = 5;
	cops[0][1] = 0;

	policeNum = 1;

	curQuad = prevQuad = pubQuad;
	while (1) {
		
		buffer_print(map);
		visitedQuad();
		autoMovement();
		//manualMovement();

		gotoxy(0,11);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		printf("curQuad = %d prevQuad = %d", curQuad, prevQuad);
		//printf("m_t : %02d/%d\nr_t : %02d/%d\nInterval : %d (ms)", current_m_t, m_t, current_r_t, r_t, moveInterval);

		Sleep(10);
	}

}