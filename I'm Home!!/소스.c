#include <stdio.h>
#include <Windows.h>	// gotoxy
#include <stdlib.h>		// srand
#include <time.h>		// time
#include <conio.h>		// _getch
#include <string.h>		// memset

// ���� ����
#pragma warning(disable:6835)
#pragma warning(disable:6836)


// �ܼ� ũ�� ����
#define SCREEN_WIDTH  60
#define SCREEN_HEIGHT 30

// �� ������
#define SIZE 10

/*
EMPTY	= ��						= 0
WALL	= ��						= 1
D		= �� ���� ��� (Drunk)	= 68
H		= ��	 (Home)				= 72
O		= ���� (Police Officer)	= 79
P		= ���� (Pub)				= 68
*/

// ���ڴ� �� ���ĺ��� �ƽ�Ű�ڵ� & �� �� ����
#define WALL 1
#define EMPTY 0
#define P 80
#define O 79
#define H 72
#define D 68

// ����Ű
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77


// ������
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



// ���� D�� ��ġ
int playerX;
int playerY;

int prePlayerX;
int prePlayerY;

int curQuad;	// ���� ��ġ�� ��и�
int prevQuad;	// ������ �湮�� ��и�

// �������� ��ġ
// cops = { {����1 xPos, ����1 yPos, ���� �� ��и�}, {����2 xPos, ����2 yPos, ���� �� ��и�} };
int cops[2][3] = { 0 };
int policeNum;			// ������ ��

// ������ ��ġ�� ��и�
int pubQuad;

// ������ ��ġ
int PUB_X;
int PUB_Y;

// ���� ��ġ
int HOME_X;
int HOME_Y;

/* ���� �̵� ���� */
int m_t, current_m_t;	// �ڵ� �̵� Ƚ��
int r_t, current_r_t;	// ���� �̵� Ƚ��
int moveInterval;		// �̵� �ֱ�


// ��
int map[SIZE][SIZE] = {
	// 1��и�
	
	/*
	* 
	*	   1����������2
	*		����������
	*	   3����������4
	* 
	* �� ���ڴ� ��и� ��ȣ�� ���ر�� ��
	* 1�� ��, 0�� ��
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
* Ŀ�� ������ ����
* parameter : void
* return : void
*/
void CursorInvisible()
{
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };//�ܼ� Ŀ�� ���� ����ü �ʱ�ȭ
	cursorInfo.dwSize = 1;					//Ŀ�� ���� (1 ~ 100)
	cursorInfo.bVisible = FALSE;			//Ŀ�� Visible TRUE(����) FALSE(����)
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo); // �ܼ� Ŀ�� ������ ������ ����ü�� ������ �°� ����
}

/*
* �Լ��� : gotoxy
* ��� �뵵: ���ڷ� ���� x, y���� ��ǥ�� ����Ͽ� ��ǥ(x, y)�� Ŀ���� �̵���Ų��.
* �Ű�����: int x int y
*/
void gotoxy(int x, int y) {
	COORD pos = { x * 2, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

/*
* ȭ�� �����
* parameter : void
* return : void
*/
void cls(void) {
	system("CLS");
}



/* ���� ���� �� ���� �Լ� */
/* 
* m_t, r_t, moveInterval �ʱⰪ �ܺ��Է�
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
* ����, �� ���� ��ġ ����
* parameter : void
* return : void
*/ 
void setSpawnPos(void) {
	
	// �����̶� �� ��и� ����
	pubQuad = rand() % 4 + 1;
	


	// ������ ���� ��� ������ �ƴ��� Ȯ���ϴ� ����(Boolean)
	int isSpawned = 0;
	
	pubQuad % 2;

	
	while (!isSpawned) {
		// ���� ��ġ
		int rangeX	= NULL;
		int rangeY	= NULL;

		// �� ��ġ
		int _rangeX = NULL;
		int _rangeY = NULL;

		if (pubQuad == 1) {
			//   Y    X
			// [0~4][0~4] = P
			// [5~9][5~9] = H

			// P ���� ��ġ
			rangeX = rand() % 5;
			rangeY = rand() % 5;

			// H ���� ��ġ
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
			map[rangeY][rangeX]	= P;	// ���� ��ġ ����
			map[_rangeY][_rangeX]	= H;	// �� ��ġ ����

			PUB_X = rangeX;
			PUB_Y = rangeY;

			HOME_X = _rangeX;
			HOME_Y = _rangeY;
			isSpawned = 1;
		}
		 
	}
}

/*
* �������� �� ���� ����� ������ ����
* parameter : void
* return : void
*/ 
void setSpawnDrunk(void) {
	
	// �������� ������ ����
	int direction;
	int isSpawned = 0;
	while (!isSpawned) {	

		// 0: ��, 1: �Ʒ�, 2: ����, 3: ������
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
* front_buffer�� -1�� �ʱ�ȭ
* parameter : void
* return : void
*/
void memsetBuffer() {
	for (int i = 0; i < SIZE; i++)
		memset(front_buffer[i], -1, sizeof(int) * SIZE);
}



/* ����� ���� �Լ�*/

/*
* �ѱ��� ���
* parameter : x��ǥ, y��ǥ, �ٲ� ����
* return : void
*/
void drawMap(int x, int y, int c) {
	// ����� ��ġ�� 
	gotoxy(x, y);

	// SetConsoleTextAttribute 
	// ����� ������ ������ ���� ���� �ٲ��.
	
	if (c == WALL) {
		// ���� ���
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
		printf("��");
	}
	if (c == EMPTY) {
		// ���� ȸ��
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
		printf("��");
	}
	if (c == P) {
		// ��ũ�� (������ΰ�?)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13);
		printf("P ");
	}
	if (c == H) {
		// �ʷϻ�
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10);
		printf("H ");
	}
	if (c == O) {
		// ������
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12);
		printf("O ");
	}
	if (c == D) {
		// �����
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14);
		printf("D ");
	}

	// ��������� �̻��Ѱ��� Ŀ���� ���� �ʰ� �� ������ Ŀ���� �̵�
	gotoxy(SIZE, SIZE);

}

/*
* ���۸� ����Ʈ
* parameter : ��(2���� �迭 �Ű�����)
* return : void
* function : 
*	�ٲ�κ� ���
*/
void buffer_print(int map[][10])
{
	
	/* ���� �̷ο� front_buffer(���� �̷�)�� �ִ� �̷� �� */
	for (int i = 0; i < SIZE; ++i)
		for (int j = 0, j2 = 0; j < SIZE; ++j)
		{
			if (front_buffer[i][j] != map[i][j])
			{
				drawMap(j2, i, map[i][j]);
				// �ٲ� �κ� ȭ�鿡 ���
				front_buffer[i][j] = map[i][j];
				// �ٲ� �κ��� ��� �� front_buffer�� ����
			}
			++j2;
			// ����� ��ġ�� ��� �����ϱ� ������ ���������ڸ� �̿��Ѵ�.
		}
}



/* �̵� ���� �Լ�*/

/*
* �÷��̾� ���� �̵�
* parameter : void
* return : void
*/
void manualMovement(void)
 {
	// ����Ű �Է�
	int dir = _getch();

	// ������ �ϴ� ������ �� ���� �ƴϰų� ���� �ƴ� ��쿡�� ����
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

	// �÷��̾ ���̳� ���� ���� ���� �� ������� ���� ���� ��.
	if (map[PUB_Y][PUB_X] == 0) map[PUB_Y][PUB_X] = P;
	if (map[HOME_Y][HOME_X] == 0) map[HOME_Y][HOME_X] = H;

}

/*
* �÷��̾� �ڵ� �̵�
* parameter : void
* return : void
*/
void autoMovement() {

	prePlayerX = playerX;
	prePlayerY = playerY;

	// �̵� ���� ����
	int dirs[4] = { UP, DOWN, LEFT, RIGHT };
	int random = NULL;
	int temp = 1;

	// �÷��̾� �̵�
	while (temp) {
		// ���� ����
		random = rand() % 4;

		// ������ �ϴ� ������ �� ���� �ƴϰų� ���� �ƴ� ��쿡�� ����
		if (dirs[random] == UP && playerY > 0 && map[playerY - 1][playerX] != 1) {
			//���� ��и� �湮 ����
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
			// ���� ��ġ ����~
			map[playerY][playerX] = D;
			current_m_t--;

			// �÷��̾ ���̳� ���� ���� ���� �� ������� ���� ���� ��.
			if (map[PUB_Y][PUB_X] == 0) map[PUB_Y][PUB_X] = P;
			if (map[HOME_Y][HOME_X] == 0) map[HOME_Y][HOME_X] = H;
		}
	}

	
}

/*
* �÷��̾� �̵� ��� ��ȯ
*/
void playerControl(int isDetected) {
	if (isDetected) manualMovement();
	else autoMovement();
}

/*
* ������ ���� �̵�
* parameter : void
* return : void
*/
void copPatrol(void) {

}

/*
* ������ �÷��̾ �߰�
* parameter : void
* return : int(�߰��ϸ� 1, �ƴϸ� 0)
*/
int detectedPlayer(void){}


/* 
* �湮�� ���� �и�� ���� �и��� ����
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
	// ���� �õ� ����
	srand(time(NULL));

	// ���� 60ĭ ���� 30ĭ���� �ܼ� ��������
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