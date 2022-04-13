#define F_CPU 16000000UL
#include "Game.h"
#include "lcd.h"	// LCD ���� ���̺귯��
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

// �ִ� ���� Ƚ��
#define		MAX_ATTEMPTS	30

// ���� state�� ǥ��(STOP:����, GAME:���� ������)
#define		STOP	0
#define		GAME	1

// ���� mode�� ǥ��(10���� ���, 16���� ���)
#define		MODE_DEC	10
#define		MODE_HEX	16	
#define		MODE_NO		0

// FND�� ����� ���׸�Ʈ�� �̸� �迭�� ����
unsigned char FND[4];
unsigned char hex_seg[16] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67, 0x77, 0x7c, 0x58, 0x5e, 0x79, 0x71};
unsigned char fnd_sel[4] = {0x01, 0x02, 0x04, 0x08};
unsigned char seg_INIT[4] = {0x30, 0x54, 0x30, 0x78};
unsigned char seg_WIN[4] = {0x00, 0x6a, 0x30, 0x54};
unsigned char seg_LOSE[4] = {0x38, 0x5c, 0x6d, 0x79};
unsigned char seg_OUT[4] = {0x00, 0x5c, 0x1c, 0x78};
unsigned char seg_EASY[4] = {0x79, 0x77, 0x6d, 0x6e};
unsigned char seg_HARD[4] = {0x76, 0x77, 0x50, 0x5e};
unsigned char seg_HI[4] = {0x00, 0x76, 0x30, 0x00};
unsigned char seg_BYE[4] = {0x00, 0x7f, 0x6e, 0x79};

char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
char answer[4];		// �������� ������ 4�ڸ� ���ڰ� ����Ǵ� �迭
char guess[4];		// ����ڰ� ������ 4�ڸ� ���ڰ� ����Ǵ� �迭

volatile int guess_index = 0;
volatile int rand_time = 0;		// answer�� �ʱ�ȭ�ϱ� ���� time
volatile int cur_time = 0;
volatile int time_rate = 2000;	// �� ����� �ð� ����

volatile int state = STOP;		// ���� state
volatile int mode = MODE_NO;	// ���� mode

volatile int attempt = 0;		// ���� �õ� Ƚ��
volatile int now_see = 0;		// ���� ���� �ִ� �����丮 ��ȣ
char history[50][16];			// ���� ���� �����丮�� �����ϴ� �迭

// FND�迭�� ����� 4���� ���ڸ� FND�� ���
void displayFND() {
	for(int i=0; i<4; i++) {
		PORTG = fnd_sel[3-i];
		PORTC = FND[i];
		_delay_us(200);
	} 
}

// FND �迭 �ʱ�ȭ
void clearFND() {
	for(int i=0; i<4; i++) {
		FND[i] = 0x00;
		guess[i] = 0;
	} 
	guess_index = 0;
	displayFND();
}

// ���ڷ� ������ 4���� ���׸�Ʈ ���ڸ� ���÷��̿� �뷫 2�ʰ� ���
// ���ڷ� ũ�� 4�� �迭�� ������
void displayFND_2s(unsigned char array[]) {

	// FND �迭 �� ���ڷ� ���� �迭�� �ʱ�ȭ
	for(int i=0;i<4;i++)
		FND[i] = array[i];
		
	// �� 2�ʰ� FND�� ���
	int cnt = 0;
	while(1)
	{
		for(int i=0; i<4; i++) {
			PORTG = fnd_sel[3-i];
			PORTC = FND[i];
			_delay_us(200);
		} 
		cnt++;

		if (cnt==3000){
			break;
		}
	}
	clearFND();
}

// history �迭 �ʱ�ȭ
void clearHistory() {
	for(int i=0;i<50;i++) {
		for(int j=0;j<16;j++)
			history[i][j] = '\0';
	}
}

// LCD�� ��������� ���� ����� �����ش�.
void showHistory() {
	// ���� ���� �ִ� ������(now_see)�� �������� 2���� �����丮 ���
	LCD_showMsg(history[now_see], history[now_see+1]);
}

// ���� guess�� ���� ���� ����� history�� ����
void recordHistory(int strike_cnt, int ball_cnt) {
	history[attempt][0] = (char)((attempt+1)/10 + 48);
	history[attempt][1] = (char)((attempt+1)%10 + 48);
	history[attempt][2] = ')';
	history[attempt][3] = ' ';
	history[attempt][4] = guess[0];
	history[attempt][5] = guess[1];
	history[attempt][6] = guess[2];
	history[attempt][7] = guess[3];
	history[attempt][8] = ' ';
	history[attempt][9] = ' ';
	history[attempt][14] = ' ';
	history[attempt][15] = '\0';

	// OUT�̶�� 'OUT'���� ����
	if(strike_cnt == 0 && ball_cnt == 0) {
		history[attempt][10] = 'O';
		history[attempt][11] = 'U';
		history[attempt][12] = 'T';
		history[attempt][13] = ' ';
		
	// �� �̿��� ��쿡�� '_S_B' ���·� ����
	} else {
		history[attempt][10] = (char)(strike_cnt + 48);
		history[attempt][11] = 'S';
		history[attempt][12] = (char)(ball_cnt + 48);
		history[attempt][13] = 'B';
	}
	attempt++;	// �õ� Ƚ�� ����
	_delay_ms(20);
}

// ���� LOSE, ��ȸ ��� �Ҹ� �� ������ �޽���
void Msg_YouDied() {
	LCD_showMsg("    Y        ", "");
	_delay_ms(300);
	LCD_showMsg("    Y    I   ", "");
	_delay_ms(300);
	LCD_showMsg("    Y U  I   ", "");
	_delay_ms(300);
	LCD_showMsg("    Y U  I D ", "");
	_delay_ms(300);
	LCD_showMsg("    YOU  I D ", "");
	_delay_ms(300);
	LCD_showMsg("    YOU  IED ", "");
	_delay_ms(300);
	LCD_showMsg("    YOU DIED ", "");
	_delay_ms(300);
	LCD_showMsg("    YOU DIED.", "");
	_delay_ms(800);
}

// ���� CLEAR, ���� ���� �� ������ �޽���
void Msg_CONGRATULATIONS() {
	LCD_showMsg("C","");
	_delay_ms(100);
	LCD_showMsg("CO ","");
	_delay_ms(100);
	LCD_showMsg("CON ","");
	_delay_ms(100);
	LCD_showMsg("CONG","");
	_delay_ms(100);
	LCD_showMsg("CONGR ","");
	_delay_ms(100);
	LCD_showMsg("CONGRA","");
	_delay_ms(100);
	LCD_showMsg("CONGRAT ","");
	_delay_ms(100);
	LCD_showMsg("CONGRATU ","");
	_delay_ms(100);
	LCD_showMsg("CONGRATUL","");
	_delay_ms(100);
	LCD_showMsg("CONGRATULA","");
	_delay_ms(100);
	LCD_showMsg("CONGRATULAT","");
	_delay_ms(100);
	LCD_showMsg("CONGRATULATI","");
	_delay_ms(100);
	LCD_showMsg("CONGRATULATIO","");
	_delay_ms(100);
	LCD_showMsg("CONGRATULATION","");
	_delay_ms(100);
	LCD_showMsg("CONGRATULATIONS","");
	_delay_ms(100);
	LCD_showMsg("CONGRATULATIONS!","");
	_delay_ms(200);
	LCD_showMsg("CONGRATULATIONS!","  GAME CLEAR!!");
	_delay_ms(200);
	LCD_showMsg("CONGRATULATIONS!","");
	_delay_ms(200);
	LCD_showMsg("CONGRATULATIONS!","  GAME CLEAR!!");
	_delay_ms(200);
	LCD_showMsg("CONGRATULATIONS!","");
	_delay_ms(200);
	LCD_showMsg("CONGRATULATIONS!","  GAME CLEAR!!");
	_delay_ms(400);
}

// �ð� ����
// ���� �ѹ� �������� time���� 1 ����
// time.h�� ��� ���
void timeUp() {
	cur_time++;
	rand_time++;
	if(cur_time >= 20000)
		cur_time = 30;
	if(rand_time >= 20000)
		rand_time = 30;
}

// ���� �ð� �ʱ�ȭ(LED�� ����)
void timeClear() {
	cur_time = 0;
	PORTA = 0x00;
}

// �ð� üũ
// �ð��� ������ ���� LED�� �ϳ��� ī��Ʈ
void timeCheck() {
	if(cur_time == time_rate)
		PORTA = 0x01;
	else if(cur_time == time_rate*2)
		PORTA = 0x03;
	else if(cur_time == time_rate*3)
		PORTA = 0x07;
	else if(cur_time == time_rate*4)
		PORTA = 0x0f;
	else if(cur_time == time_rate*5)
		PORTA = 0x1f;
	else if(cur_time == time_rate*6)
		PORTA = 0x3f;
	else if(cur_time == time_rate*7)
		PORTA = 0x7f;
	else if(cur_time == time_rate*8)
		PORTA = 0xff;
	
	// LED 8���� ��� ���� �ð� �ʰ��� ���� ����(LOSE)
	else if(cur_time == time_rate*8 + 127) {
		clearFND();
		Msg_YouDied();
		LCD_showMsg("PRESS ANY SWITCH", "  TO CONTINUE.");
		displayFND_2s(seg_LOSE);
		stopGame();
		_delay_ms(20);
	}
}

// 4x4 Ű�е忡�� ���ڸ� �Է¹����� �ش� index�� ��ȯ
int pushHex() {
	int index = 99;
	PORTF = 0xFF;       
	
	// Ű�е��� ù��° row �˻�
	PORTF = 0xFE;
	_delay_us(10);
	if ((PINF & 0x10) == 0) index = 4;
	else if ((PINF & 0x20) == 0) index = 3;
	else if ((PINF & 0x40) == 0) index = 2;
	else if ((PINF & 0x80) == 0) index = 1;

	// Ű�е��� �ι�° row �˻�
	PORTF = 0xFD;
	_delay_us(10);
	if ((PINF & 0x10) == 0) index = 8;
	else if ((PINF & 0x20) == 0) index = 7;
	else if ((PINF & 0x40) == 0) index = 6;
	else if ((PINF & 0x80) == 0) index = 5;

	// Ű�е��� ����° row �˻�
	PORTF = 0xFB;
	_delay_us(10);
	if ((PINF & 0x10) == 0) index = 11;
	else if ((PINF & 0x20) == 0) index = 10;
	else if ((PINF & 0x40) == 0) index = 0;
	else if ((PINF & 0x80) == 0) index = 9;

	// Ű�е��� �׹�° row �˻�    
	PORTF = 0xF7;
	_delay_us(10);
	if ((PINF & 0x10) == 0) index = 15;
	else if ((PINF & 0x20) == 0) index = 14;
	else if ((PINF & 0x40) == 0) index = 13;
	else if ((PINF & 0x80) == 0) index = 12;
	
	// �Է¹��� �ʴ� ��Ȳ������ index 99�� ����
	// 99�� ��ư�� ������ ���� ���� �ǹ�
	if (guess_index == 4)
		index = 99;
	if (state == STOP)
		index = 99;
	if (mode == MODE_DEC && index > 9)
		index = 99;

	return index;
}

// 4-button ����ġ���� �Է¹��� ���� cmd�� ����
int pushCmd() {
	PORTB = 0x0f;    
	int cmd = 0;
	
	// cmd ��ɾ��
	// 0: No Action
	// 1: ���� �ߴ�, 2: ��ũ�� ��, 3: ��ũ�� �ٿ�, 4: ����

	if ((PINB & 0x01) == 0)	// red button(���� �ߴ�)
		cmd = 1;
	else if ((PINB & 0x02) == 0)	// yellow1(LCD ��ũ�� ��)
		cmd = 2;
	else if ((PINB & 0x04) == 0)	// yellow2(LCD ��ũ�� �ٿ�)
		cmd = 3;
	else if ((PINB & 0x08) == 0)	// green button(���� ���� ����)
		cmd = 4;

	// ���� ��� ��ȸ�� �� ����ߴٸ� ���� �ߴ� ��ɾ� ����
	if(attempt == MAX_ATTEMPTS) {
		Msg_YouDied();	// ��� �޽���
		cmd = 1;		// �ߴ� ��ɾ�
	}
	return cmd;
}

// ����ڰ� �Է��� ���ڸ� guess �迭�� ����
void enterGuess(int key) {
	if(key == 99)	// �Էµ� key�� ������ ���� X
		return;
	
	// �ߺ��Ǵ� ���ڰ� ������ flag�� ����Ͽ� üũ
	int flag = 0;
	for (int i=0; i<4; i++) {
		if(guess[i] == hex[key])
			flag = 1;
	}

	// �ߺ��� ���ٸ� guess ������ �߰�
	if (flag == 0) {
		FND[guess_index] = hex_seg[key];
		guess[guess_index] = hex[key];
		guess_index++;
	}
}

// �Է¹��� cmd���� ���� �׼� ����
void cmdSwitch(int cmd) {
	switch(cmd)
	{
		case 0:
			break;
		case 1:
			LCD_showMsg("PRESS ANY SWITCH", "  TO CONTINUE.");
			if (state == GAME)
				displayFND_2s(seg_BYE);
			stopGame();
			_delay_ms(20);
			break;
		case 2:
			if(now_see > 0) {
				now_see--;
				showHistory();
			}
			_delay_ms(20);
			break;
		case 3:
			if(now_see < attempt - 2) {
				now_see++;
				showHistory();
			}
			_delay_ms(20);
			break;
		case 4:
			compare();
			_delay_ms(20);
			break;
		default:
			// Error
			break;
	}
}

// ����ڰ� switch�� ��带 �����ߴ��� üũ
void checkModeSwitch() {
	// ���� switch�� �����ϸ� EASY ���(10���� ���) ����
	if ((PINE & 0x10) == 0) {
		mode = MODE_DEC;	// 10���� ��� ����
		LCD_showMsg("   EASY  MODE","     START!");
		_delay_ms(2000);
		LCD_showMsg("  DECIMAL MODE","YOU CAN USE 0TO9");
		_delay_ms(20);
	}
	
	// ���� switch�� �����ϸ� HARD ���(16���� ���) ����
	if ((PINE & 0x20) == 0) {
		mode = MODE_HEX;	// 16���� ��� ����
		LCD_showMsg("   HARD  MODE","     START!");
		_delay_ms(2000);
		LCD_showMsg("HEXADECIMAL MODE","YOU CAN USE 0TOF");
		_delay_ms(20);
	}
}

// green ��ư Ŭ�� ��
// guess �迭�� ���� answer �迭�� ���� ���Ͽ� ��� ���
void compare() {
	if(guess_index < 4)
		return;
	if(state == STOP)
		return;

	int ball_cnt = 0;		// �� ����
	int strike_cnt = 0;		// ��Ʈ����ũ ����
	timeClear();			// �ð� �ʱ�ȭ

	for(int i=0; i<4; i++) {
		if(guess[i] == answer[i])
			strike_cnt++;

		for(int j=0; j<4; j++){
			if(guess[j] == answer[i])
				ball_cnt++;
		}
	}
	ball_cnt -= strike_cnt;
	
	recordHistory(strike_cnt, ball_cnt);	// history ���
	now_see = attempt - 2;		// LCD�� �ֱ� ����� �����ֵ��� �̵�
	if(attempt == 1)
		now_see++;
	showHistory();
	clearFND();

	// ���� 0S0B �̶�� 'OUT'�� FND�� ���
	if(strike_cnt == 0 && ball_cnt == 0) {
		displayFND_2s(seg_OUT);
		return;
	}
	
	// 4S ��� FND�� 'WIN'�� ����ϰ�
	// ���� �޽����� LCD�� ����� ��, ���� ����
	if(strike_cnt == 4){
		displayFND_2s(seg_WIN);
		Msg_CONGRATULATIONS();
		stopGame();
		return;
	}
	
	// �� �̿��� ����� FND�� ��� ���
	unsigned char seg_array[4] = {hex_seg[strike_cnt], hex_seg[5],hex_seg[ball_cnt], hex_seg[8]};
	displayFND_2s(seg_array);
}

// ���� �ߴ�
void stopGame() {
	mode = MODE_NO;		// ���� ��� ���� X
	state = STOP;		// ���� state�� �ߴ� ���·� ����
	timeClear();		// �ð� �ʱ�ȭ
	clearFND();			// FND �ʱ�ȭ
	now_see = 0;
	attempt = 0;		// �õ� Ƚ�� �ʱ�ȭ
	clearHistory();		// history �ʱ�ȭ
	LCD_showMsg(" SELECT MODE","      EASY/HARD");
}

// ���� �ʱ�ȭ
void initGame() {
	state = GAME;		// ���� ���� ���·� ����
	int i=0;
	while(i<4) {		// �������� answer ���� �ʱ�ȭ
		answer[i] = hex[rand_time % mode];
		for(int j=0;j<i;j++) {
			if (answer[j] == answer[i]) {
				i--;
				break;
			}
		}
		i++;
		rand_time = (rand_time + 17);
		timeUp();
		_delay_us(20);
	}
	timeClear();

	// ������ ��带 FND�� �������.
	if (mode == MODE_DEC)
		displayFND_2s(seg_EASY);
	if (mode == MODE_HEX)
		displayFND_2s(seg_HARD);
	_delay_ms(20);
}

// ���� ����
void progress() {
	int key,cmd;

	initGame();	// �ʱ�ȭ

	// state�� �����̸� ���� ����
	// cmd�� red��ư Ŭ�� �� state�� STOP�� �Ǿ� ���� Ż��
	while(state == GAME) {
		displayFND();
		timeCheck();

		key = pushHex();
		enterGuess(key);

		cmd = pushCmd();
		cmdSwitch(cmd);

		timeUp();
	}
}

// ���� ���� �Լ�
void game() {
	PORTB = 0x0f;
	// ���� ���� �޽���
	LCD_showMsg("    WELCOME!","  STOVE LEAGUE");
	displayFND_2s(seg_HI);

	LCD_showMsg(" SELECT MODE","      EASY/HARD");
	while(1) {
		checkModeSwitch();	// ��� ����
		if (mode == MODE_NO)	// ��� �������� ������ ���� ����
			continue;
			
		progress();		// ��尡 ���õǸ� �������� �Ѿ��.
	}
}
