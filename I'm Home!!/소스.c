// 최종 수정 2021 - 11 - 29 (월) 09:27
#include <stdio.h>
#include <Windows.h>	// gotoxy
#include <stdlib.h>		// srand, abs
#include <time.h>		// time
#include <conio.h>		// _getch
#include <string.h>		// memset
#include <stdbool.h>	// true false




// 워닝 무시 (버퍼 오버런 때문에 실행이 안됨.)
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





// 현재 D의 위치
int playerX;
int playerY;

// 이동하기 전 D의 위치
int prePlayerX;
int prePlayerY;

int curQuad;	// 현재 위치한 사분면
int prevQuad;	// 이전에 방문한 사분면
int tempQuad;	// 사분면 임시 저장 변수

// 경찰들의 위치
// cops = { {경찰1 xPos, 경찰1 yPos, 스폰 된 사분면}, {경찰2 xPos, 경찰2 yPos, 스폰 된 사분면} };
int cops[2][3] = { 0 };
int policeNum = 0;			// 경찰의 수

// 술집이 위치한 사분면
int pubQuad;
int homeQuad;

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

int detect;				// 플레이어 발각
int detectorQuad;		// 플레이어를 발견한 경찰의 quadrant
int isAuto;			// 이동 모드 ( 자동은 true, 수동은 false)
bool pauseSpawning;	// 경찰 스폰 일시정지
int lastQuad;		// 마지막 남은 사분면 (경찰이 안나온)

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

	{1,1,0,1,1,0,0,0,0,0},
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
* 커서 깜빡임 제거 (커서 비가시화)
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
* 엔터 누르기 전까지 계속 일시정지
*/
void pause() {
	// 엔터 누르기 전까지 계속 정지

	int k;
	do {

		k = _getch();
	} while (k != 13);
}

/* 게임 시작 전 설정 함수 */

/* 
* m_t, r_t, moveInterval 초기값 외부입력
* parameter : void
* return : void
*/
void setOption(void) {
	printf("m_t:\t\t"); scanf_s("%d", &m_t);
	printf("r_t:\t\t"); scanf_s("%d", &r_t);
	printf("moveInterval:\t"); scanf_s("%d", &moveInterval);
	current_m_t = m_t;
	current_r_t = r_t;
	isAuto = true;
	pauseSpawning = false;
	lastQuad = NULL;
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
	
	// |5 - pubQuad|하면 집 분면이 나옴
	// 예: 5-2 = 3, 5 - 1 = 4
	homeQuad = abs(pubQuad - 5);

	
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
	int dir[4] = { UP, DOWN, LEFT, RIGHT };
	int random;
	int isSpawned = 0;

	while (!isSpawned) {	

		// 0: 위, 1: 아래, 2: 왼쪽, 3: 오른쪽
		random = rand() % 4;


		if (dir[random] == UP && PUB_Y > 0 && PUB_Y != 5) {
			if (map[PUB_Y - 1][PUB_X] == 0) {
				playerX = PUB_X; 
				playerY = PUB_Y - 1;
				map[PUB_Y - 1][PUB_X] = D;
				isSpawned = 1;
			}
		}

		else if (dir[random] == DOWN && PUB_Y < 9 && PUB_Y != 4) {
			if (map[PUB_Y + 1][PUB_X] == 0) {
				playerX = PUB_X;
				playerY = PUB_Y + 1;
				map[PUB_Y + 1][PUB_X] = D;
				isSpawned = 1;
			}
		}

		else if (dir[random] == LEFT && PUB_X > 0 && PUB_X != 5) {
			if (map[PUB_Y][PUB_X - 1] == 0) {
				playerX = PUB_X - 1;
				playerY = PUB_Y;
				map[PUB_Y][PUB_X - 1] = D;
				isSpawned = 1;
			}
		}

		else if (dir[random] == RIGHT && PUB_X < 9 && PUB_X != 4) {
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
		// memset(1차원 배열, 초기화 할 값, 배열의 크기);
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
		printf("Ｐ");
	}
	if (c == H) {
		// 초록색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
		printf("Ｈ");
	}
	if (c == O) {
		// 빨간색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
		printf("Ｏ");
	}
	if (c == D) {
		// 노란색
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
		printf("Ｄ");
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

/*
* r_t 횟수 넘어가면 게임 종료
* parameter : void
* return : void
*/
void gameOver(void) {
	
	// 집 도착
	if (playerX == HOME_X && playerY == HOME_Y) {
		buffer_print(map);
		gotoxy(0, 23);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
		printf("집 도착!");

		// 엔터 누르기 전까지 계속 정지
		pause();
	}
	// 다른 분면으로 넘어감
	if (tempQuad != curQuad) {
		
		// r_t 잔여 횟수가 남아있지 않으면 게임 종료
		if (current_r_t < 0) {
			gotoxy(0, 23);
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
			printf("Game Over");
			pause();
		}
		else {
			current_m_t = m_t;
			current_r_t = r_t;
			isAuto = true;
			return;
		}
	}
	// 넘어가기도 전에 r_t를 모두 소진하면
	if (current_r_t < 0) {
		// 겜 정지
		gotoxy(0, 23);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
		printf("Game Over");
		pause();
	}
}


/* 이동 관련 함수*/
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

/*
* 플레이어 수동 이동
* parameter : void
* return : void
*/
void manualMovement(void)
 {
	// 이동하기 전 플레이어의 위치를 저장
	prePlayerX = playerX;
	prePlayerY = playerY;
	
	pauseSpawning = false;

	// 버퍼 비우기
	fflush(stdin);
	// 방향키 입력
	int dir = _getch();

	//printf("%d", dir);
	// 가려고 하는 방향에 벽, 맵 밖, 경찰만 아니면 이동 가능
	if (dir == UP && playerY > 0 && map[playerY - 1][playerX] != 1 && map[playerY - 1][playerX] != 79) {
		// r_t값 1 감소
		current_r_t -= 1;

		// 이전 위치에 길 설정
		map[playerY--][playerX] = 0;

		// 현재 위치에 플레이어 설정
		map[playerY][playerX] = D;

	}
	if (dir == DOWN && playerY < 9 && map[playerY + 1][playerX] != 1 && map[playerY + 1][playerX] != 79) {
		--current_r_t;
		map[playerY++][playerX] = 0;
		map[playerY][playerX] = D;
			
	}
	if (dir == LEFT && playerX > 0 && map[playerY][playerX - 1] != 1 && map[playerY][playerX - 1] != 79) {
		--current_r_t;
		map[playerY][playerX--] = 0;
		map[playerY][playerX] = D;
			
	}

	if (dir == RIGHT && playerX < 9 && map[playerY][playerX + 1] != 1 && map[playerY][playerX + 1] != 79) {
		--current_r_t;
		map[playerY][playerX++] = 0;
		map[playerY][playerX] = D;
	}

	// 플레이어가 펍이나 집을 지나 갔을 때 사라지는 것을 방지 함.
	if (map[PUB_Y][PUB_X] == 0) map[PUB_Y][PUB_X] = P;
	if (map[HOME_Y][HOME_X] == 0) map[HOME_Y][HOME_X] = H;
	visitedQuad();
	gameOver();
	
}

/*
* 플레이어 자동 이동
* parameter : void
* return : void
*/
void autoMovement() {

	// m_t가 더이상 안줄게 해줄 bool 변수
	static bool freeze_m_t = false;

	// 이동하기 전 현재 위치를 저장
	prePlayerX = playerX;
	prePlayerY = playerY;

	// 이동 방향 결정
	int dirs[4] = { UP, DOWN, LEFT, RIGHT };
	int random = NULL;
	int temp = true;

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

		// 경찰이 0명이고 m_t내로 첫 분면에서 옆 분면으로 넘어가는게 성공하면 
		if (current_m_t >= 0 && curQuad != pubQuad && curQuad != homeQuad && freeze_m_t == false && policeNum == 0) {
			// m_t 고정
			freeze_m_t = true;
		}
		
		// 이동에 성공하면
		if (!temp) {
			// 현재 위치 갱신~
			map[playerY][playerX] = D;
			if(freeze_m_t == false) --current_m_t;

			// 플레이어가 펍이나 집을 지나 갔을 때 사라지는 것을 방지 함.
			if (map[PUB_Y][PUB_X] == 0) map[PUB_Y][PUB_X] = P;
			if (map[HOME_Y][HOME_X] == 0) map[HOME_Y][HOME_X] = H;
			if (playerX == HOME_X && playerY == HOME_Y) {
				buffer_print(map);
				gotoxy(0, 23);
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
				printf("집 도착!");
				pause();
			}
		}
	}
	
}

/*
* 경찰의 순찰 이동
* parameter : void
* return : void
*/
void copPatrol(void) {
	if (policeNum == 0) return;

	int dir[4] = { UP, DOWN, LEFT, RIGHT };
	int random;
	int temp;
	for (int i = 0; i < policeNum; i++) {
		
		// 각 경찰이 이동을 완료했을 때 while을 끄고 다음 경찰을 이동시키기 위한 임시 변수
		temp = true;


		int COP_X = cops[i][0];		// 경찰의 x좌표의 주소
		int COP_Y = cops[i][1];		// 경찰의 y좌표의 주소
		int COP_Q = cops[i][2];		// 경찰의 사분면. 사분면 위치정보는 수정할 필요가 없기 때문에 복사 초기화

		// 이동 범위 제한 (특정 사분면에서만 움직일 수 있게)
		
		while (temp) {
			random = rand() % 4;
			
			// 1사분면이고
			if (COP_Q == 1) {

				// 방향은 UP
				if (dir[random] == UP ) {

					// 해당 방향이 맵밖이 아니고 길일 경우에만
					if (COP_Y > 0 && map[cops[i][1] - 1][cops[i][0]] == EMPTY) {

						// 이동을 수행
						map[cops[i][1]--][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == DOWN) {
					if (COP_Y < 4 && map[cops[i][1] + 1][cops[i][0]] == EMPTY) {
						map[cops[i][1]++][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == LEFT) {
					if (COP_X > 0 && map[cops[i][1]][cops[i][0] - 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]--] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == RIGHT) {
					if (COP_X < 4 && map[cops[i][1]][cops[i][0] + 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]++] = EMPTY;
						temp = !temp;
					}
				}
			}
			else if (COP_Q == 2) {
				if (dir[random] == UP) {
					if (COP_Y > 0 && map[cops[i][1] - 1][cops[i][0]] == EMPTY) {
						map[cops[i][1]--][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == DOWN) {
					if (COP_Y < 4 && map[cops[i][1] + 1][cops[i][0]] == EMPTY) {
						map[cops[i][1]++][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == LEFT) {
					if (COP_X > 5 && map[cops[i][1]][cops[i][0] - 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]--] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == RIGHT) {
					if (COP_X < 9 && map[cops[i][1]][cops[i][0] + 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]++] = EMPTY;
						temp = !temp;
					}
				}
			}
			else if (COP_Q == 3) {
				if (dir[random] == UP) {
					if (COP_Y > 5 && map[cops[i][1] - 1][cops[i][0]] == EMPTY) {
						map[cops[i][1]--][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == DOWN) {
					if (COP_Y < 9 && map[cops[i][1] + 1][cops[i][0]] == EMPTY) {
						map[cops[i][1]++][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == LEFT) {
					if (COP_X > 0 && map[cops[i][1]][cops[i][0] - 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]--] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == RIGHT) {
					if (COP_X < 4 && map[cops[i][1]][cops[i][0] + 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]++] = EMPTY;
						temp = !temp;
					}
				}
			}
			else if (COP_Q == 4) {
				if (dir[random] == UP) {
					if (COP_Y > 5 && map[cops[i][1] - 1][cops[i][0]] == EMPTY) {
						map[cops[i][1]--][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == DOWN) {
					if (COP_Y < 9 && map[cops[i][1] + 1][cops[i][0]] == EMPTY) {
						map[cops[i][1]++][cops[i][0]] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == LEFT) {
					if (COP_X > 5 && map[cops[i][1]][cops[i][0] - 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]--] = EMPTY;
						temp = !temp;
					}
				}
				else if (dir[random] == RIGHT) {
					if (COP_X < 9 && map[cops[i][1]][cops[i][0] + 1] == EMPTY) {
						map[cops[i][1]][cops[i][0]++] = EMPTY;
						temp = !temp;
					}
				}
			}

			if (temp == false) {
				map[cops[i][1]][cops[i][0]] = O;
			}
			
		}
	}
	
}

/*
* 경찰이 플레이어를 발견(감지). 감지거리는 반경 3칸.
* parameter : void
* return : int(발견하면 1(true), 아니면 0(false))
*/
int detectPlayer(void){
	// 경찰이 0명이면 감지할 필요가 없음
	if (policeNum == 0) return false;

	int i;
	for (i = 0; i < policeNum; i++) {
		int start = NULL, end = NULL, detect = false;
		if (playerX == cops[i][0])
		{
			if (playerY > cops[i][1]) {
				start = cops[i][1];
				end = playerY;
			}
			else {
				start = playerY;
				end = cops[i][1];
			}

			// 서로의 거리가 3칸 밖이면
			if (end - start > 3)
				// 감지 불가능
				return false;

			// 서로의 거리가 반경 3칸이라면
			else {
				for (int i = start; i <= end; i++) {
					// 벽으로 하나라도 막혀있으면 return으로 인해 함수를 나가게 됨.
					if (map[i][playerX] == 1) return false;


				}
				// 반대로 벽이 없으면 감지 성공
				detect = true;

			}
		}
		else if (playerY == cops[i][1])
		{
			if (playerX > cops[i][0]) {
				start = cops[i][0];
				end = playerX;
			}
			else {
				start = playerX;
				end = cops[i][0];
			}

			// 서로의 거리가 4칸 밖이면
			if (end - start > 3)
				// 감지 불가능
				return false;

			// 서로의 거리가 반경 4칸이라면
			else {
				for (int i = start; i <= end; i++) {
					// 벽으로 하나라도 막혀있으면 return으로 인해 함수를 나가게 됨.
					if (map[playerY][i] == 1) return false;


				}
				// 반대로 벽이 없으면 감지 성공
				detect = true;

			}
		}

		if (start == NULL && end == NULL) continue;
		

		if (detect) {
			// 임시로 현재 curQuad를 저장
			tempQuad = curQuad;
			detectorQuad = cops[i][2];
			return true;
		}
	}
	return 0;
}

/*
* 경찰 스폰 함수
* parameter : void
* return : void
*/
void spawnPolice() {
	static int firstSpawnQuad = NULL;
	static int temp = NULL;
	int x, y, isSpawned = false;
	
	// m_t 초과 직전 있었던 사분면 저장
	if (current_m_t == 0) temp = curQuad;
	
	//m_t 초과한 상태 && 경찰 0명
	if (current_m_t < 0 && policeNum == 0) {

		// 넘어간 분면이 나머지 분면이여야 함.
		if (curQuad == pubQuad || curQuad == homeQuad) return;
		while (true) {

			// 좌표 범위 지정.
			if (curQuad == 1)
			{
				x = rand() % 5;
				y = rand() % 5;
			}
			if (curQuad == 2)
			{
				x = rand() % 5 + 5;
				y = rand() % 5;
			}
			if (curQuad == 3)
			{
				x = rand() % 5;
				y = rand() % 5 + 5;
			}
			if (curQuad == 4)
			{
				x = rand() % 5 + 5;
				y = rand() % 5 + 5;
			}
			
			// 길 위에서만 스폰
			if (map[y][x] == EMPTY) {

				// 경찰 객체 데이터 생성
				cops[0][0] = x;
				cops[0][1] = y;
				cops[0][2] = curQuad;
					
				policeNum++;

				// 맵에 경찰 추가
				map[y][x] = O;
				pauseSpawning = true;
				firstSpawnQuad = curQuad;
				// 함수 종료
				return;
			}
		}
	}
	if (current_m_t < 0 && pauseSpawning == false && policeNum == 1 && temp == curQuad) {
		for (int i = 1; i <= 4; i++) {
			if (i == pubQuad || i == homeQuad || i == firstSpawnQuad) continue;
			lastQuad = i;
		}
		while (true) {

			// 좌표 범위 지정.
			if (lastQuad == 1)
			{
				x = rand() % 5;
				y = rand() % 5;
			}
			if (lastQuad == 2)
			{
				x = rand() % 5 + 5;
				y = rand() % 5;
			}
			if (lastQuad == 3)
			{
				x = rand() % 5;
				y = rand() % 5 + 5;
			}
			if (lastQuad == 4)
			{
				x = rand() % 5 + 5;
				y = rand() % 5 + 5;
			}

			// 길 위에서만 스폰
			if (map[y][x] == EMPTY) {

				// 경찰 객체 데이터 생성
				cops[1][0] = x;
				cops[1][1] = y;
				cops[1][2] = lastQuad;

				policeNum++;
				pauseSpawning = true;

				// 맵에 경찰 추가
				map[y][x] = O;
				// 함수 종료
				return;
			}
		}
	}
}

/*
* 게임의 모든 진행을 담당
* parameter : void
* return : void
*/
void gameControl() {

	// 패트롤하다가
	if (isAuto == true)
		// 플레이어를 감지함
		if (detectPlayer())
			// 수동 모드로 이행
			isAuto = false;


	if (isAuto == true) {

		autoMovement(); //stopped
		copPatrol();
		if(policeNum <= 1) spawnPolice();

		Sleep(moveInterval);
	}
	else {
		manualMovement();
	}
	visitedQuad();
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
	setOption();

	//현재분면 이전분면을 술집분면으로
	curQuad = prevQuad = pubQuad;
	
	buffer_print(map);
	printf("\n\n아무 키나 눌러 시작...");
	_getch();
	while (1) {
		
		buffer_print(map);
		gameControl();

		// 모니터링?
		// 텍스트 컬러를 밝은 흰색으로
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	
		gotoxy(0, 11);
		printf("현재  위치 : %-1d  사분면\n이전  위치 : %-1d  사분면", curQuad, prevQuad);
		gotoxy(0, 14);
		printf("Player X : %d\t Player Y : %d", playerX, playerY);
		gotoxy(0, 15);
		printf("m_t : %d / %d\nr_t : %d / %d\nInterval : %d (ms)", current_m_t, m_t, current_r_t, r_t, moveInterval);
		gotoxy(0, 18);
		printf("MovementType : %s\n", isAuto ? "auto" : "manual");
		printf("Home Quad: %d  |  Pub Quad : %d\n", homeQuad, pubQuad);
		printf("Police Num: %d\n1st: {%d %d %d} | 2nd: {%d %d %d}", policeNum, cops[0][0], cops[0][1], cops[0][2], cops[1][0], cops[1][1], cops[1][2]);


	}

}